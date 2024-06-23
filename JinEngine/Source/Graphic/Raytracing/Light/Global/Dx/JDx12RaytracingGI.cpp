/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/
   
#include"JDx12RaytracingGI.h"
#include"../../../Dx/JDx12RaytracingUtility.h"
#include"../../../Dx/JDx12RaytracingConstants.h" 
#include"../../../../GraphicResource/Dx/JDx12GraphicResourceInfo.h"    
#include"../../../../Accelerator/Dx/JDx12GpuAcceleratorHolder.h"
#include"../../../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../../../Command/Dx/JDx12CommandContext.h"
#include"../../../../Utility/Dx/JDx12ObjectCreation.h" 
#include"../../../../Utility/JSampler.h"
#include"../../../../FrameResource/JFrameUpdate.h"
#include"../../../../FrameResource/Dx/JDx12FrameResource.h" 
#include"../../../../JGraphicUpdateHelper.h"  
#include"../../../../../Core/Math/JVectorExtend.h" 
#include"../../../../../Core/Geometry/Mesh/JMeshStruct.h" 
#include"../../../../../Core/Log/JLogMacro.h" 
#include"../../../../../Object/Component/Camera/JCamera.h"  
#include"../../../../../Object/Component/RenderItem/JRenderItem.h"  
#include"../../../../../Object/Component/Transform/JTransform.h"  
#include"../../../../../Object/Resource/Scene/JScene.h"    
#include"../../../../../Object/Resource/Mesh/JMeshGeometry.h"    
#include"../../../../../Object/Resource/Material/JMaterial.h"    
#include"../../../../../Object/Resource/Texture/JTexture.h"     
#include"../../../../../Object/Resource/JResourceManager.h"
#include"../../../../../Object/GameObject/JGameObject.h"
 
#define T_MIN L"T_MIN"
#define TEXTURE_2D_COUNT L"TEXTURE_2D_COUNT"
#define MIN_DEPTH L"MIN_DEPTH"
#define MAX_DEPTH L"MAX_DEPTH"
#define SAMPLE_COUNT L"SAMPLE_COUNT"

#define OBJECT_REF_INSTANCE_ID_OFFSET L"OBJECT_REF_INSTANCE_ID_OFFSET"
#define POINT_LIGHT_INSTANCE_ID_OFFSET L"POINT_LIGHT_INSTANCE_ID_OFFSET"
#define SPOT_LIGHT_INSTANCE_ID_OFFSET L"SPOT_LIGHT_INSTANCE_ID_OFFSET"
#define RECT_LIGHT_INSTANCE_ID_OFFSET L"RECT_LIGHT_INSTANCE_ID_OFFSET"
#define LIGHT_INSTANCE_ID_END L"LIGHT_INSTANCE_ID_END"

#define OBJECT_MASK L"OBJECT_MASK"
#define POINT_LIGHT_MASK L"POINT_LIGHT_MASK"
#define SPOIT_LIGHT_MASK L"SPOIT_LIGHT_MASK"
#define RECT_LIGHT_MASK L"RECT_LIGHT_MASK"
 
#define TEMPORAL_SAMPLE_MAX L"TEMPORAL_SAMPLE_MAX"
#define SPATIAL_SAMPLE_MAX L"SPATIAL_SAMPLE_MAX"
#define SAMPLE_MAX_AGE L"SAMPLE_MAX_AGE"
#define NEIGHBOR_WEIGHT_CLAMP_THRESHOLD L"NEIGHBOR_WEIGHT_CLAMP_THRESHOLD"
#define SPATIAL_WEIGHT_CLAMP_THRESHOLD L"SPATIAL_WEIGHT_CLAMP_THRESHOLD"
#define AO_MAP L"USE_AO_MAP"

namespace JinEngine::Graphic
{	
	//ref ReSTIR GI: Path Resampling for Real-Time Path Tracing
	/*
	* Step 1. generate initial sampling	- ray tracing shader
	* Step 2. reuse	sampling			- compute shader
	* Step 3. compute fixel color		- compute shader
	*/
	namespace Common
	{
#ifdef _DEBUG
		static constexpr bool allowDebug = false;
#else
		static constexpr bool allowDebug = false;
#endif
		static constexpr uint canadidateCount[] = { 16, 32, 64, 128, 256 };
		static constexpr uint canadidateVariation = SIZE_OF_ARRAY(canadidateCount);
		static constexpr uint defaultCanadidateCount = canadidateCount[2];
		static constexpr uint sampleSize = Constants::restirSampleSize;
		static constexpr uint reservoirSize = Constants::restirReserviorSize;
		static constexpr uint clearUserDataFrequency = 7680;

		static constexpr int threadDimX = 16;
		static constexpr int threadDimY = 16;
		static JVector3<uint> ThreadDim()noexcept
		{
			return JVector3<uint>(threadDimX, threadDimY, 1);
		}
	}
	namespace Raytracing
	{
		static constexpr int passCBIndex = 0;
		static constexpr int asStructureIndex = passCBIndex + 1;
		static constexpr int objRefBuffIndex = asStructureIndex + 1;
		//static constexpr int sampleBuffIndex = objRefBuffIndex + 1;
		static constexpr int dLitBuffIndex = objRefBuffIndex + 1;
		static constexpr int pLitBuffIndex = dLitBuffIndex + 1;
		static constexpr int sLitBuffIndex = pLitBuffIndex + 1;
		static constexpr int rLitBuffIndex = sLitBuffIndex + 1;
		static constexpr int matBuffIndex = rLitBuffIndex + 1;
		static constexpr int texture2DBufferIndex = matBuffIndex + 1;
		static constexpr int textureCubeBufferIndex = texture2DBufferIndex + 1;
		static constexpr int depthMapIndex = textureCubeBufferIndex + 1;
		static constexpr int albedoMapIndex = depthMapIndex + 1; 
		static constexpr int lightPropMapIndex = albedoMapIndex + 1;
		static constexpr int normalMapIndex = lightPropMapIndex + 1;
		static constexpr int srcBuferIndex = normalMapIndex + 1;
		static constexpr int rootSlotCount = srcBuferIndex + 1;
		static constexpr int samplerCount = 3;

		struct MeshMaterial
		{
			JVector3F albedoColor;
			float specularColor;
			float metallic;
			float roughness; 
		};
		struct MeshVertex
		{
		public:
			JVector3<float> position = JVector3<float>(0, 0, 0);
			JVector3<float> normal = JVector3<float>(0, 0, 0); 
			JVector3<float> tangentU = JVector3<float>(0, 0, 0);
		};
		struct RayPayload
		{
			MeshVertex vertex;					 //f9
			MeshMaterial material;				 //f6    
			uint hitObjType;					//out 
		};
		struct ShadowPayload
		{ 
			float isVisible;
		};
		static constexpr size_t payloadSize = sizeof(RayPayload);
		static constexpr size_t attributeSize = sizeof(JVector2F);		//barycentrics

		static constexpr uint sampleCount[] = { 1, 2, 4, 8 };
		static constexpr uint sampleSetCount = 64;
		static constexpr uint sampleSetSize = 8;
		static constexpr uint sampleSetDistribution = 8;

		static constexpr float tMin = 1e-2;
		static constexpr uint minDepth = Constants::restirBounceRange.GetMin();
		static constexpr uint maxDepth = Constants::restirBounceRange.GetMax();
		static constexpr uint spp = 1;

		static constexpr uint rayGenCount = 1;
		static constexpr uint hitGroupCount = 2;
		static constexpr uint missShaderCount = 5;

		//RayGen
		static std::wstring RayGenShaderName()
		{
			return L"RayGenShader";
		}
		//HitGroup
		static std::wstring HemisphereHitGroupName()
		{
			return L"HemisphereHitGroup";
		}
		static std::wstring LightSourceHitGroupName()
		{
			return L"LightSourceHitGroup";
		}
		//Hit
		static std::wstring HemisphereHitShaderName()
		{
			return L"HemisphereHitShader";
		}
		static std::wstring LightSourceHitShaderName()
		{
			return L"LightSourceHitShader";
		}
		//AnyHit
		static std::wstring HemispherAnyHitShaderName()
		{
			return L"HemispherAnyHitShader";
		}
		//Miss
		static std::wstring HemispherMissShaderName()
		{
			return L"HemispherMissShader";
		}
		static std::wstring LightSourceMissShader()
		{
			return L"LightSourceMissShader";
		} 

		//any hitshader Ãß°² ÇÊ¿ä
		static std::vector<std::wstring> HitGroupNameVec()
		{
			return std::vector<std::wstring>{HemisphereHitGroupName(), LightSourceHitGroupName()};
		}
		static std::vector<std::wstring> CloseHitShaderNameVec()
		{
			return std::vector<std::wstring>{HemisphereHitShaderName(), LightSourceHitShaderName()};
		}
		static std::vector<std::wstring> AnyHitShaderNameVec()
		{
			//HemispherAnyHitShaderName()
			return std::vector<std::wstring>{ L"", L""};
		}
		static std::vector<std::wstring> MissShaderNameVec()
		{
			return std::vector<std::wstring>{HemispherMissShaderName(), LightSourceMissShader()};
		}
	}
	namespace Reuse
	{ 
		static constexpr int passCBIndex = 0;									//cv
		static constexpr int asStructureIndex = passCBIndex + 1;
		static constexpr int initialSampleIndex = asStructureIndex + 1;
		static constexpr int preTemporalIndex = initialSampleIndex + 1;
		static constexpr int preSpatialIndex = preTemporalIndex + 1;
		static constexpr int albedoMapIndex = preSpatialIndex + 1; 
		static constexpr int lightPropMapIndex = albedoMapIndex + 1;
		static constexpr int depthMapIndex = lightPropMapIndex + 1;
		static constexpr int preNormalMapIndex = depthMapIndex + 1;
		static constexpr int preDepthMapIndex = preNormalMapIndex + 1;
		static constexpr int aoMapIndex = preDepthMapIndex + 1;

		static constexpr int temporalIndex = aoMapIndex + 1;				//srv
		static constexpr int spatialIndex = temporalIndex + 1;					//uav
		static constexpr int rootSlotCount = spatialIndex + 1;
		static constexpr int samplerCount = 1;	  
	}
	namespace Final
	{
		static constexpr int passCBIndex = 0;									//cv
		static constexpr int srcIndex = passCBIndex + 1;
		//static constexpr int testIndex = srcIndex + 1;
		static constexpr int destIndex = srcIndex + 1;
		static constexpr int rootSlotCount = destIndex + 1;
	}
	namespace Upsample
	{
		static constexpr int passCBIndex = 0;									//cv
		static constexpr int srcIndex = passCBIndex + 1;
		//static constexpr int testIndex = srcIndex + 1;
		static constexpr int destIndex = srcIndex + 1;
		static constexpr int rootSlotCount = destIndex + 1;
	}
	namespace Clear
	{
		static constexpr int passCBIndex = 0;					
		static constexpr int intialIndex = passCBIndex + 1;
		static constexpr int temporal00Index = intialIndex + 1;
		static constexpr int temporal01Index = temporal00Index + 1;
		static constexpr int spatial00Index = temporal01Index + 1;
		static constexpr int spatial01Index = spatial00Index + 1;
		static constexpr int rootSlotCount = spatial01Index + 1;
	}
	JDx12RaytracingGI::UserPrivateData::UserPrivateData(JGraphicDevice* device)
		:frameBuffer(JDx12GraphicBufferT<GIPassConstants>(L"GiPass", J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT))
	{
		SetWaitFrame(Constants::gNumFrameResources);
		SetClearTrigger(); 
		frameBuffer.Build(device, Constants::gNumFrameResources);
		 
		//pointLitShape = _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::LOW_SPHERE);
		//spotLitShape = _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::LOW_SPHERE);
		//rectLitShape = _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::LOW_SPHERE);
	}
	JDx12RaytracingGI::UserPrivateData::~UserPrivateData()
	{ 
		frameBuffer.Clear();
	}
	void JDx12RaytracingGI::UserPrivateData::Begin(const JDrawHelper& helper)
	{ 
		const JUserPtr<JCamera>& cam = helper.cam;
		const JVector2F camRtSize = cam->GetRenderTargetSize(); 
		const JVector2F camHalfRtSize = camRtSize * 0.5f;
		//const JVector2<uint> quaterRtSize = camRtSize / 4.0f;
		const JUserPtr<JScene>& scene = helper.scene;
		const size_t sceneGuid = scene->GetGuid();
		 
		const uint directionalLitCount = JFrameUpdateData::GetAreaRegistedCount(J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT, sceneGuid);
		const uint pointLitCount = JFrameUpdateData::GetAreaRegistedCount(J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT, sceneGuid);
		const uint spotLitCount = JFrameUpdateData::GetAreaRegistedCount(J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT, sceneGuid);
		const uint rectLitCount = JFrameUpdateData::GetAreaRegistedCount(J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT, sceneGuid);
		const uint lightSum = directionalLitCount + pointLitCount + spotLitCount + rectLitCount;

		const uint directionalLitOffset = JFrameUpdateData::GetAreaRegistedOffset(J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT, sceneGuid);
		const uint pointLitOffset = JFrameUpdateData::GetAreaRegistedOffset(J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT, sceneGuid);
		const uint spotLitOffset = JFrameUpdateData::GetAreaRegistedOffset(J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT, sceneGuid);
		const uint rectLitOffset = JFrameUpdateData::GetAreaRegistedOffset(J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT, sceneGuid);
		 
		GIPassConstants constants; 
		constants.camInvView.StoreXM(DirectX::XMMatrixTranspose(cam->GetInvView()));
		constants.camPreViewProj.StoreXM(DirectX::XMMatrixTranspose(cam->GetPreViewProj()));
		constants.camNearFar = JVector2F(cam->GetNear(), cam->GetFar());
		cam->GetUvToView(constants.uvToViewA, constants.uvToViewB);
		constants.rtSize = camHalfRtSize;
		constants.invRtSize = 1.0f / camHalfRtSize;
		constants.origianlRtSize = camRtSize;
		constants.invOrigianlRtSize = 1.0f / camRtSize;
		constants.tMax = (JVector3F(cam->GetOwner()->GetOwnerScene()->GetSceneBBox().Extents) * 2).Length();
		constants.totalNumPixels = camHalfRtSize.x * camHalfRtSize.y;
		 
		constants.camPosW = cam->GetTransform()->GetWorldPosition();
		constants.camNearMulFar = constants.camNearFar.x * constants.camNearFar.y;
		constants.cameraPrePosW = camPrePosW;
		constants.sampleSetSize = Raytracing::sampleSetSize;
		constants.sampleSetMax = Raytracing::sampleSetCount;
		constants.currSampleSetIndex = currSampleSetIndex;
		constants.updateCount = GetUpdateCount(); 

		constants.directionalLightRange = directionalLitCount;
		constants.pointLightRange = constants.directionalLightRange + pointLitCount;
		constants.spotLightRange = constants.pointLightRange + spotLitCount;
		constants.rectLightRange = constants.spotLightRange + rectLitCount;

		constants.directionalLightOffset = directionalLitOffset;
		constants.pointLightOffset = pointLitOffset;
		constants.spotLightOffset = spotLitOffset;
		constants.rectLightOffset = rectLitOffset;

		constants.totalLightCount = lightSum;
		constants.invTotalLightCount = 1.0f / (float)constants.totalLightCount; 
		constants.forceClearReservoirs = 0;
		//constants.rectLightVerticesIndex = rectLitShape->GraphicResourceUserInterface().GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::VERTEX, J_GRAPHIC_BIND_TYPE::SRV, 0);
		//constants.rectLightIndiciesIndex = rectLitShape->GraphicResourceUserInterface().GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::INDEX, J_GRAPHIC_BIND_TYPE::SRV, 0);

		frameBuffer.CopyData(helper.info.frame.currIndex, constants);
	}
	void JDx12RaytracingGI::UserPrivateData::End(const JDrawHelper& helper)
	{
		camPrePosW = helper.cam->GetTransform()->GetWorldPosition();
		preReserviorIndex = currReserviorIndex;
		++currReserviorIndex;
		if (currReserviorIndex >= Constants::resitrReserviorBufferCount)
			currReserviorIndex = 0;
  
		++currSampleSetIndex;
		if (currSampleSetIndex >= Raytracing::sampleSetCount)
			currSampleSetIndex = 0;

		AddUpdateCount();
		SetAliveTrigger();
	}
	
	JDx12RaytracingGI::GIDataSet::GIDataSet(const JGraphicRtGiComputeSet* computeSet, const JDrawHelper& helper)
	{
		const JDx12GraphicRtGiComputeSet* set = static_cast<const JDx12GraphicRtGiComputeSet*>(computeSet);
		context = static_cast<JDx12CommandContext*>(set->context);
		device = static_cast<JDx12GraphicDevice*>(set->device);
		cam = helper.cam;

		const size_t sceneGuid = helper.scene->GetGuid();
		auto gInterface = helper.cam->GraphicResourceUserInterface();
		auto aInterface = helper.scene->GpuAcceleratorUserInterface();
		auto reserviorIndex = gInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::RESTIR_RESERVOIR, J_GRAPHIC_TASK_TYPE::RAYTRACING_GI);
		auto ssaoIndex = gInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP, J_GRAPHIC_TASK_TYPE::RAYTRACING_GI);

		rtSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		dsSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		if (!rtSet.IsValid() || !dsSet.IsValid())
			return;

		oriResolution = rtSet.info->GetResourceSize();
		halfResolution = oriResolution * 0.5f;
		threadDim = Common::ThreadDim().XY();

		sharedata = static_cast<JDx12GraphicResourceShareData*>(set->shareData)->GetRestirTemporalAccumulationData(oriResolution.x, oriResolution.y);
		if (sharedata == nullptr)
			return;

		albedoSet = context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP); 
		lightPropSet = context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY);
		normalSet = context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP);
		//velocitySet = context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY);
		aoMapSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP, ssaoIndex);

		preRsSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::RAYTRACING_GI);
		preDsSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::RAYTRACING_GI);
		preNormalSet = context->ComputeSet(preRsSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP);
		//preTangentSet = context->ComputeSet(preRsSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY);

		initialSampleSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::RESTIR_INITIAL_SAMPLE, J_GRAPHIC_TASK_TYPE::RAYTRACING_GI);
		temporalReserviorSet[0] = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::RESTIR_RESERVOIR, reserviorIndex);
		temporalReserviorSet[1] = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::RESTIR_RESERVOIR, reserviorIndex + 1);
		spatialReserviorSet[0] = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::RESTIR_RESERVOIR, reserviorIndex + 2);
		spatialReserviorSet[1] = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::RESTIR_RESERVOIR, reserviorIndex + 3);
		colorIntermediate = context->ComputeSet(sharedata->restirColorIntermediate00);
		destSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::RAYTRACING_GI);
		accelSet = context->ComputeSet(aInterface);
		
		currFrameIndex = helper.info.frame.currIndex;
		preFrameIndex = currFrameIndex == 0 ? Constants::gNumFrameResources - 1 : currFrameIndex - 1;
 
		auto skyObj = helper.scene->GetGameObjectVec(J_RENDER_LAYER::SKY, Core::J_MESHGEOMETRY_TYPE::STATIC);
		skyMapSrvHeapIndex = invalidIndex;
		if (skyObj.size() > 0)
		{
			auto albedoMap = skyObj[0]->GetRenderItem()->GetValidMaterial(0)->GetAlbedoMap();
			skyMapSrvHeapIndex = albedoMap != nullptr ? albedoMap->GraphicResourceUserInterface().GetFirstResourceHeapStart(J_GRAPHIC_BIND_TYPE::SRV) : invalidIndex;
		}  
	}
	void JDx12RaytracingGI::GIDataSet::SetUserPrivate(UserPrivateData* data, const JDrawHelper& helper)
	{ 
		userPrivate = data;
		userPrivate->Begin(helper);

		preTemporalReserviorSet = &temporalReserviorSet[userPrivate->preReserviorIndex];
		currTemporalReserviorSet = &temporalReserviorSet[userPrivate->currReserviorIndex];
		preSpatialReserviorSet = &spatialReserviorSet[userPrivate->preReserviorIndex];
		currSpatialReserviorSet = &spatialReserviorSet[userPrivate->currReserviorIndex];
	}
	bool JDx12RaytracingGI::GIDataSet::IsValid()const noexcept
	{
		return sharedata != nullptr && accelSet.IsValid() && accelSet.holder->HasData();
	}

	JDx12RaytracingGI::~JDx12RaytracingGI()
	{
		ClearResource();
	}
	void JDx12RaytracingGI::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;

		BuildResource(device, gM);
	}
	void JDx12RaytracingGI::Clear()
	{
		ClearResource();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12RaytracingGI::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	bool JDx12RaytracingGI::HasDependency(const JGraphicInfo::TYPE type)const noexcept
	{
		if (type == JGraphicInfo::TYPE::RESOURCE)
			return true;
		else
			return false;
	}
	bool JDx12RaytracingGI::HasDependency(const JGraphicOption::TYPE type)const noexcept
	{
		return type == JGraphicOption::TYPE::RENDERING || type == JGraphicOption::TYPE::DEBUGGING;
	}
	void JDx12RaytracingGI::NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set)
	{
		auto dx12Set = static_cast<const JDx12GraphicInfoChangedSet&>(set);
		if (dx12Set.preInfo.resource.binding2DTextureCapacity != dx12Set.newInfo.resource.binding2DTextureCapacity)
		{
			raytracingRootSignature = nullptr;
			ClearStateObject();
			ClearShaderTable();

			auto dx12Device = static_cast<JDx12GraphicDevice*>(dx12Set.device);
			BuildRootSignature(dx12Device);
			const JGraphicInfo& gInfo = GetGraphicInfo();
			const JGraphicOption& gOption = GetGraphicOption();
			for (uint i = 0; i < STATE_OBJECT_TYPE_COUNT; ++i)
				BuildRtStateObject(dx12Device->GetRaytracingDevice(), gInfo, gOption, (STATE_OBJECT_TYPE)i);
			BuildRtShaderTables(dx12Device->GetRaytracingDevice(), gInfo);
		}
	}
	void JDx12RaytracingGI::NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)
	{ 
		auto dx12Set = static_cast<const JDx12GraphicOptionChangedSet&>(set);
		if (set.preOption.rendering.restir != set.newOption.rendering.restir || set.newOption.debugging.requestRecompileRtGiShader)
		{
			Clear();
			BuildResource(dx12Set.device, dx12Set.gm);
			for (auto& data : userPrivate)
				data.second->SetClearTrigger();
		}
	}
	void JDx12RaytracingGI::ComputeGI(const JGraphicRtGiComputeSet* computeSet, const JDrawHelper& helper)
	{   
		if (!IsSameDevice(computeSet) || !helper.allowRtGi)
			return;

		GIDataSet set(computeSet, helper);
		if (!set.IsValid())
		{
			//J_LOG_PRINT_OUT("Invalid GIDataSet", " ");
			return;
		}

		Begin(set, helper);
		if (set.userPrivate->HasWaitFrame())
			set.userPrivate->MinusWaitFrame();
		else
		{
			if (set.userPrivate->HasClearRequest())
				ClearRestirResource(set, helper);
			InitializeSampling(set, helper);
			//InitializeSamplingTest(set, helper);
			ReuseSampling(set, helper);
			FinalColor(set, helper);
			Upsample(set, helper);
		} 
		End(set, helper);
	}
	void JDx12RaytracingGI::Begin(GIDataSet& set, const JDrawHelper& helper)
	{
		auto data = userPrivate.find(helper.cam->GetGuid());
		if (data == userPrivate.end())
		{
			userPrivate.emplace(helper.cam->GetGuid(), std::make_unique<UserPrivateData>(set.device));
			data = userPrivate.find(helper.cam->GetGuid());
		}
		set.SetUserPrivate(data->second.get(), helper);
	}
	void JDx12RaytracingGI::InitializeSampling(const GIDataSet& set, const JDrawHelper& helper)
	{
		set.context->Transition(set.dsSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.albedoSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE); 
		set.context->Transition(set.lightPropSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.normalSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.initialSampleSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(raytracingRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(Raytracing::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetTlasView(Raytracing::asStructureIndex, set.accelSet);
		set.context->SetComputeRootShaderResourceView(Raytracing::objRefBuffIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT_REF_INFO);
		set.context->SetComputeRootShaderResourceView(Raytracing::dLitBuffIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT);
		set.context->SetComputeRootShaderResourceView(Raytracing::pLitBuffIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT);
		set.context->SetComputeRootShaderResourceView(Raytracing::sLitBuffIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT);
		set.context->SetComputeRootShaderResourceView(Raytracing::rLitBuffIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT);
		set.context->SetComputeRootShaderResourceView(Raytracing::matBuffIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL);
		set.context->SetComputeRootDescriptorTable(Raytracing::texture2DBufferIndex, J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D);
		if(set.skyMapSrvHeapIndex != invalidIndex)
			set.context->SetComputeRootDescriptorTable(Raytracing::textureCubeBufferIndex, set.skyMapSrvHeapIndex);
		set.context->SetComputeRootDescriptorTable(Raytracing::depthMapIndex, set.dsSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Raytracing::albedoMapIndex, set.albedoSet.GetGpuSrvHandle()); 
		set.context->SetComputeRootDescriptorTable(Raytracing::lightPropMapIndex, set.lightPropSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Raytracing::normalMapIndex, set.normalSet.GetGpuSrvHandle());   
		set.context->SetComputeRootDescriptorTable(Raytracing::srcBuferIndex, set.initialSampleSet.GetGpuUavHandle());
		//set.context->SetComputeRootShaderResourceView(Raytracing::sampleBuffIndex, hemiSample->GetGpuAddress());

		DispatchRays(set.context, STATE_OBJECT_TYPE_DEFAULT, set.halfResolution.x, set.halfResolution.y);
	} 
	void JDx12RaytracingGI::ReuseSampling(const GIDataSet& set, const JDrawHelper& helper)
	{
		set.context->InsertUAVBarrier(set.initialSampleSet.holder); 
		set.context->Transition(set.preNormalSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.preDsSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.initialSampleSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.preTemporalReserviorSet->holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.preSpatialReserviorSet->holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.preSpatialReserviorSet->holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.aoMapSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.currTemporalReserviorSet->holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.currSpatialReserviorSet->holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->FlushResourceBarriers();
 
		set.context->SetComputeRootSignature(reuseRootSignature.Get()); 
		set.context->SetComputeRootConstantBufferView(Reuse::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(Reuse::initialSampleIndex, set.initialSampleSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Reuse::preTemporalIndex, set.preTemporalReserviorSet->GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Reuse::preSpatialIndex, set.preSpatialReserviorSet->GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Reuse::albedoMapIndex, set.albedoSet.GetGpuSrvHandle()); 
		//set.context->SetComputeRootDescriptorTable(Reuse::normalMapIndex, set.normalSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Reuse::lightPropMapIndex, set.lightPropSet.GetGpuSrvHandle());  
		set.context->SetComputeRootDescriptorTable(Reuse::depthMapIndex, set.dsSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Reuse::preNormalMapIndex, set.preNormalSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Reuse::preDepthMapIndex, set.preDsSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Reuse::aoMapIndex, set.aoMapSet.GetGpuSrvHandle());

		set.context->SetComputeRootDescriptorTable(Reuse::temporalIndex, set.currTemporalReserviorSet->GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(Reuse::spatialIndex, set.currSpatialReserviorSet->GetGpuUavHandle());

		if(helper.option.debugging.testTrigger01)
			set.context->SetPipelineState(reuseSamplingShader[1].get());
		else
			set.context->SetPipelineState(reuseSamplingShader[0].get());

		set.context->Dispatch2D(set.halfResolution, set.threadDim);
	}
	void JDx12RaytracingGI::FinalColor(const GIDataSet& set, const JDrawHelper& helper)
	{ 
		set.context->Transition(set.preSpatialReserviorSet->holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.colorIntermediate.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		//set.context->InsertUAVBarrier(set.currTemporalReserviorSet->holder, true);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(finalRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(Final::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(Final::srcIndex, set.preSpatialReserviorSet->GetGpuSrvHandle());
		//set.context->SetComputeRootDescriptorTable(Final::testIndex, set.initialSampleSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Final::destIndex, set.colorIntermediate.GetGpuUavHandle());

		set.context->SetPipelineState(finalShader.get());
		set.context->Dispatch2D(set.halfResolution, set.threadDim);
	} 
	void JDx12RaytracingGI::Upsample(const GIDataSet& set, const JDrawHelper& helper)
	{ 
		set.context->Transition(set.colorIntermediate.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.destSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(upsampleRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(Upsample::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(Upsample::srcIndex, set.colorIntermediate.GetGpuSrvHandle()); 
		set.context->SetComputeRootDescriptorTable(Upsample::destIndex, set.destSet.GetGpuUavHandle());

		set.context->SetPipelineState(upsampleShader.get());
		set.context->Dispatch2D(set.oriResolution, set.threadDim);
	}
	void JDx12RaytracingGI::ClearRestirResource(const GIDataSet& set, const JDrawHelper& helper)
	{ 
		set.context->Transition(set.initialSampleSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.temporalReserviorSet[0].holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.temporalReserviorSet[1].holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.spatialReserviorSet[0].holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.spatialReserviorSet[1].holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	 
		set.context->SetComputeRootSignature(clearRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(Clear::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(Clear::intialIndex, set.initialSampleSet.GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(Clear::temporal00Index, set.temporalReserviorSet[0].GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(Clear::temporal01Index, set.temporalReserviorSet[1].GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(Clear::spatial00Index, set.spatialReserviorSet[0].GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(Clear::spatial01Index, set.spatialReserviorSet[1].GetGpuUavHandle());

		set.context->SetPipelineState(clearShader.get());
		set.context->Dispatch2D(set.halfResolution, set.threadDim);

		set.context->InsertUAVBarrier(set.initialSampleSet.holder);
		set.context->InsertUAVBarrier(set.temporalReserviorSet[0].holder);
		set.context->InsertUAVBarrier(set.temporalReserviorSet[1].holder);
		set.context->InsertUAVBarrier(set.spatialReserviorSet[0].holder);
		set.context->InsertUAVBarrier(set.spatialReserviorSet[1].holder);
		set.context->FlushResourceBarriers();

		set.userPrivate->OffClearTrigger();
	}
	void JDx12RaytracingGI::InitializeSamplingTest(const GIDataSet& set, const JDrawHelper& helper)
	{ 
		set.context->Transition(set.initialSampleSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.colorIntermediate.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->FlushResourceBarriers();
		 
		set.context->SetComputeRootSignature(finalRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(Final::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(Final::srcIndex, set.initialSampleSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Final::destIndex, set.colorIntermediate.GetGpuUavHandle());

		set.context->SetPipelineState(finalShader.get());
		set.context->Dispatch2D(set.halfResolution, set.threadDim);
		set.context->Transition(set.colorIntermediate.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		set.context->InsertUAVBarrier(set.colorIntermediate.holder, true);
	}
	void JDx12RaytracingGI::End(const GIDataSet& set, const JDrawHelper& helper)
	{
		set.userPrivate->End(helper);
		++computeCount;
		if (computeCount >= Common::clearUserDataFrequency)
		{
			for (auto& data : userPrivate)
			{
				if (data.second->CanAlive())
					data.second->OffAliveTrigger();
				else
					userPrivate.erase(data.first); 
			}
			computeCount = 0;
		}
	}
	void JDx12RaytracingGI::DispatchRays(JDx12CommandContext* context, const STATE_OBJECT_TYPE type, const uint width, const uint height)
	{
		D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
		dispatchDesc.HitGroupTable.StartAddress = hitGroupShaderTable->GetGpuAddress(type);
		dispatchDesc.HitGroupTable.SizeInBytes = hitGroupShaderTable->GetHolder()->GetWidth();
		dispatchDesc.HitGroupTable.StrideInBytes = hitGroupShaderTable->GetElementByteSize();
		dispatchDesc.MissShaderTable.StartAddress = missShaderTable->GetGpuAddress(type);
		dispatchDesc.MissShaderTable.SizeInBytes = missShaderTable->GetHolder()->GetWidth();
		dispatchDesc.MissShaderTable.StrideInBytes = missShaderTable->GetElementByteSize();
		dispatchDesc.RayGenerationShaderRecord.StartAddress = rayGenShaderTable->GetGpuAddress(type);
		dispatchDesc.RayGenerationShaderRecord.SizeInBytes = rayGenShaderTable->GetHolder()->GetWidth();
		dispatchDesc.Width = width;
		dispatchDesc.Height = height;
		dispatchDesc.Depth = 1;
		context->SetPipelineState(stateObject[type].Get());
		context->DispatchRays(&dispatchDesc);
	}
	void JDx12RaytracingGI::BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM)
	{  
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(gM);

		BuildRootSignature(dx12Device);
		BuildShaderResource(dx12Device);
		//BuildBuffer(dx12Device);
	}
	void JDx12RaytracingGI::BuildRootSignature(JDx12GraphicDevice* device)
	{
		BuildRtRootSignature(device->GetRaytracingDevice());
		BuildComputeRootSignature(device->GetDevice());
	}
	void JDx12RaytracingGI::BuildShaderResource(JDx12GraphicDevice* device)
	{
		const JGraphicInfo& gInfo = GetGraphicInfo();
		const JGraphicOption& gOption = GetGraphicOption();
		for (uint i = 0; i < STATE_OBJECT_TYPE_COUNT; ++i)
			BuildRtStateObject(device->GetRaytracingDevice(), gInfo, gOption, (STATE_OBJECT_TYPE)i);
		BuildRtShaderTables(device->GetRaytracingDevice(), gInfo);
		BuildComputePso(device->GetDevice(), gInfo, gOption);
	}
	void JDx12RaytracingGI::BuildRtRootSignature(ID3D12Device5* device)
	{
		JDx12RootSignatureBuilder2<Raytracing::rootSlotCount, Raytracing::samplerCount> builder;
		builder.PushConstantsBuffer(0);
		builder.PushShaderResource(0);
		builder.PushShaderResource(1);
		builder.PushShaderResource(2, 0);
		builder.PushShaderResource(2, 1);
		builder.PushShaderResource(2, 2);
		builder.PushShaderResource(2, 3);
		builder.PushShaderResource(3, 0);

		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, GetGraphicInfo().resource.binding2DTextureCapacity, 4, 5);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 6);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 7);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 8);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 9);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 10);  
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		std::vector<CD3DX12_STATIC_SAMPLER_DESC> sampler
		{  
			//
			CD3DX12_STATIC_SAMPLER_DESC(0, // shaderRegister
				D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, // filter
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
				0.0f,                             // mipLODBias
				8),				                  // maxAnisotropy

			//LTC
			CD3DX12_STATIC_SAMPLER_DESC(1, // shaderRegister
				D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR,
				//D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
				0.0f,
				16,
				D3D12_COMPARISON_FUNC_LESS_EQUAL,
				D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK),

				//LTC source texture
			CD3DX12_STATIC_SAMPLER_DESC(2, // shaderRegister
				D3D12_FILTER_MIN_MAG_MIP_LINEAR,
				//D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP  // addressW
				)

		}; 
		for (const auto& data : sampler)
			builder.PushSampler(data); 
		builder.Create(device, L"rtRootSignature", raytracingRootSignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED);
	}
	void JDx12RaytracingGI::BuildRtStateObject(ID3D12Device5* device, const JGraphicInfo& info, const JGraphicOption& option, const STATE_OBJECT_TYPE type)
	{
		JStateObjectBuildData buildData;
		buildData.maxRecursion = Raytracing::maxDepth;

		// DXIL library
		BuildDxilLibrarySubobject(buildData, option);

		// Hit groups
		BuildHitGroupSubobjects(buildData);

		//Shader config
		//Defines the maximum sizes in bytes for the ray rayPayload and attribute structure.
		BuildShaderConfig(buildData);

		// Local root signature and shader association
		//This is a root signature that enables a shader to have unique arguments that come from shader tables.
		//BuildLocalRootSignatureSubobjects(buildData);

		// Global root signature
		// This is a root signature that is shared across all RTAO shaders invoked during a DispatchRays() call.
		BuildGlobalRootSignatureSubobjects(buildData);

		// Pipeline config
		// Defines the maximum TraceRay() recursion depth.
		BuildPipelineConfig(buildData);

		// Create the state object.
		ThrowIfFailedG(device->CreateStateObject(buildData.raytracingPipeline, IID_PPV_ARGS(&stateObject[type])));
	}
	void JDx12RaytracingGI::BuildDxilLibrarySubobject(JStateObjectBuildData& buildData, const JGraphicOption& option)
	{
		JComputeShaderInitData initData;
		initData.macro.push_back({ TEXTURE_2D_COUNT, std::to_wstring(GetGraphicInfo().resource.binding2DTextureCapacity) });
		//initData.macro.push_back({ T_MIN, std::to_wstring(Raytracing::tMin) });
		initData.macro.push_back({ MIN_DEPTH, std::to_wstring(Raytracing::minDepth) });
		initData.macro.push_back({ MAX_DEPTH, std::to_wstring(option.rendering.restir.bounceCount.Get()) });
		initData.macro.push_back({ SAMPLE_COUNT, std::to_wstring(Raytracing::spp) });

		initData.macro.push_back({ OBJECT_REF_INSTANCE_ID_OFFSET, std::to_wstring(Constants::objRefAsInstanceIdoffset) });
		initData.macro.push_back({ POINT_LIGHT_INSTANCE_ID_OFFSET, std::to_wstring(Constants::pointLightAsInstanceIdOffset) });
		initData.macro.push_back({ SPOT_LIGHT_INSTANCE_ID_OFFSET, std::to_wstring(Constants::spotLightAsInstanceIdOffset) });
		initData.macro.push_back({ RECT_LIGHT_INSTANCE_ID_OFFSET, std::to_wstring(Constants::rectLightAsInstanceIdOffset) });
		initData.macro.push_back({ LIGHT_INSTANCE_ID_END, std::to_wstring(Constants::rectLightAsInstanceIdOffset) });

		initData.macro.push_back({ OBJECT_MASK, std::to_wstring(Constants::renderItemMask) });
		initData.macro.push_back({ POINT_LIGHT_MASK, std::to_wstring(Constants::pointLightMask) });
		initData.macro.push_back({ SPOIT_LIGHT_MASK, std::to_wstring(Constants::spotLightMask) });
		initData.macro.push_back({ RECT_LIGHT_MASK, std::to_wstring(Constants::rectLightMask) });
 
		buildData.shader = JDxShaderDataUtil::CompileShader(JCompileInfo(ShaderRelativePath::RestirGi(L"PathTracing.hlsl"), L"RayGenShader"), initData.macro, L"lib_6_6");

		CD3DX12_SHADER_BYTECODE libdxil;
		libdxil.pShaderBytecode = buildData.shader->GetBufferPointer();
		libdxil.BytecodeLength = buildData.shader->GetBufferSize();

		auto lib = buildData.raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
		lib->SetDXILLibrary(&libdxil);
		 
		auto closeHitShaderName = Raytracing::CloseHitShaderNameVec();
		auto anyHitShaderName = Raytracing::AnyHitShaderNameVec();
		auto missShaderName = Raytracing::MissShaderNameVec();

		lib->DefineExport(Raytracing::RayGenShaderName().c_str());
		for(const auto& data : closeHitShaderName)
			lib->DefineExport(data.c_str());
		for (const auto& data : anyHitShaderName)
		{
			if(!data.empty())
				lib->DefineExport(data.c_str());
		} 
		for (const auto& data : missShaderName)
			lib->DefineExport(data.c_str());
		// Use default shader exports for a DXIL library/collection subobject ~ surface all shaders.
	}
	void JDx12RaytracingGI::BuildHitGroupSubobjects(JStateObjectBuildData& buildData)
	{ 
		auto hitGroupName = Raytracing::HitGroupNameVec();
		auto closeHitShaderName = Raytracing::CloseHitShaderNameVec();
		auto anyHitShaderName = Raytracing::AnyHitShaderNameVec();
		for (uint i = 0; i < Raytracing::hitGroupCount; ++i)
		{
			auto hitGroup = buildData.raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
			hitGroup->SetClosestHitShaderImport(closeHitShaderName[i].c_str());
			if(!anyHitShaderName[i].empty())
				hitGroup->SetAnyHitShaderImport(anyHitShaderName[i].c_str());
			hitGroup->SetHitGroupExport(hitGroupName[i].c_str());
			hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
		}
	}
	void JDx12RaytracingGI::BuildShaderConfig(JStateObjectBuildData& buildData)
	{
		auto shaderConfig = buildData.raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
		shaderConfig->Config(Raytracing::payloadSize, Raytracing::attributeSize);
	}
	void JDx12RaytracingGI::BuildGlobalRootSignatureSubobjects(JStateObjectBuildData& buildData)
	{
		auto glRootSub = buildData.raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
		glRootSub->SetRootSignature(raytracingRootSignature.Get());
	}
	void JDx12RaytracingGI::BuildPipelineConfig(JStateObjectBuildData& buildData)
	{
		auto pipelineConfig = buildData.raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
		// PERFOMANCE TIP: Set max recursion depth as low as needed
		// as drivers may apply optimization strategies for low recursion depths. 
		pipelineConfig->Config(buildData.maxRecursion);
	}
	void JDx12RaytracingGI::BuildRtShaderTables(ID3D12Device5* device, const JGraphicInfo& info)
	{
		JShaderTableBuild<Raytracing::rayGenCount* STATE_OBJECT_TYPE_COUNT> rayGenBuild(device);
		JShaderTableBuild<Raytracing::hitGroupCount* STATE_OBJECT_TYPE_COUNT> hitGroupBuild(device);
		JShaderTableBuild<Raytracing::missShaderCount* STATE_OBJECT_TYPE_COUNT> missBuild(device);

		// A shader name look-up table for shader table debug print out.
		JShaderTable::ShaderIDMap shaderIdToStringMap;

		auto hitGroupName = Raytracing::HitGroupNameVec();  
		auto missShaderName = Raytracing::MissShaderNameVec();

		Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> stateObjectProperties[STATE_OBJECT_TYPE_COUNT];
		for (uint i = 0; i < STATE_OBJECT_TYPE_COUNT; ++i)
		{
			ThrowIfFailedG(stateObject[i].As(&stateObjectProperties[i]));
			rayGenBuild.PushID(stateObjectProperties[i].Get(), Raytracing::RayGenShaderName().c_str(), shaderIdToStringMap);
			for (const auto& data : hitGroupName)
				hitGroupBuild.PushID(stateObjectProperties[i].Get(), data.c_str(), shaderIdToStringMap);
			for (const auto& data : missShaderName)
				missBuild.PushID(stateObjectProperties[i].Get(), data.c_str(), shaderIdToStringMap);
		}

		for (uint i = 0; i < STATE_OBJECT_TYPE_COUNT; ++i)
		{
			rayGenBuild.PushRecord();
			rayGenBuild.DebugPrint(shaderIdToStringMap);
			for (const auto& data : hitGroupName)
				hitGroupBuild.PushRecord();
			for (const auto& data : missShaderName)
				missBuild.PushRecord();		 
			hitGroupBuild.DebugPrint(shaderIdToStringMap);
			missBuild.DebugPrint(shaderIdToStringMap);
		}
		rayGenShaderTable = rayGenBuild.Release();
		hitGroupShaderTable = hitGroupBuild.Release();
		missShaderTable = missBuild.Release();
	}
	void JDx12RaytracingGI::BuildComputeRootSignature(ID3D12Device* device)
	{  
		JDx12RootSignatureBuilder2<Reuse::rootSlotCount, Reuse::samplerCount> rBuilder;
		rBuilder.PushConstantsBuffer(0);
		rBuilder.PushShaderResource(0);
		rBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0);
		rBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0);
		rBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0);
		rBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 0);
		rBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5, 0);
		rBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6, 0);
		rBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7, 0);
		rBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 8, 0); 
		rBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 9, 0);
		rBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);
		rBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1, 0);
		rBuilder.PushBorderSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);
		rBuilder.Create(device, L"ReuseRootSignature", reuseRootSignature.GetAddressOf());

		JDx12RootSignatureBuilder<Final::rootSlotCount> fBuilder;
		fBuilder.PushConstantsBuffer(0);
		fBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		//fBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		fBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); 
		fBuilder.Create(device, L"FinalRootSignature", finalRootSignature.GetAddressOf());

		JDx12RootSignatureBuilder2<Upsample::rootSlotCount, 1> uBuilder;
		uBuilder.PushConstantsBuffer(0);
		uBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		//fBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		uBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		uBuilder.PushSampler(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		uBuilder.Create(device, L"UpsampleRootSignature", upsampleRootSignature.GetAddressOf());

		JDx12RootSignatureBuilder<Clear::rootSlotCount> cBuilder;
		cBuilder.PushConstantsBuffer(0);
		cBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		cBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);
		cBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);
		cBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3);
		cBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 4);
		cBuilder.Create(device, L"ClearRootSignature", clearRootSignature.GetAddressOf());
	}
	void JDx12RaytracingGI::BuildComputePso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{ 
		finalShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		upsampleShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		clearShader = std::make_unique<JDx12ComputeShaderDataHolder>();

		constexpr uint shaderCount = 3 + REUSE_OPTION::COUNT;
		JDx12ComputePsoBulder<shaderCount> psoBuilder("JDx12RaytracingGI");
		for (uint i = 0; i < REUSE_OPTION::COUNT; ++i)
		{
			const REUSE_OPTION reuseOption = (REUSE_OPTION)i;
			reuseSamplingShader[i] = std::make_unique<JDx12ComputeShaderDataHolder>();
			psoBuilder.PushHolder(reuseSamplingShader[i].get());
			psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::RestirGi(L"Reuse.hlsl"), L"main"));

			psoBuilder.PushMacroSet({ TEMPORAL_SAMPLE_MAX , std::to_wstring(option.rendering.restir.temporalSampleCount.Get()) });
			psoBuilder.PushMacroSet({ SPATIAL_SAMPLE_MAX , std::to_wstring(option.rendering.restir.spatialSampleCount.Get()) });
			psoBuilder.PushMacroSet({ SAMPLE_MAX_AGE , std::to_wstring(option.rendering.restir.sampleMaxAge.Get()) });
			psoBuilder.PushMacroSet({ NEIGHBOR_WEIGHT_CLAMP_THRESHOLD , std::to_wstring(option.rendering.restir.neighborWeightSumThreshold.Get()) });
			psoBuilder.PushMacroSet({ SPATIAL_WEIGHT_CLAMP_THRESHOLD , std::to_wstring(option.rendering.restir.spatialWeightAverageThreshold.Get()) });
			if (Core::HasSQValueEnum(reuseOption, REUSE_OPTION::USE_AO_MAP))
				psoBuilder.PushMacroSet({ AO_MAP , std::to_wstring(1) });

			psoBuilder.PushThreadDim(Common::ThreadDim());
			psoBuilder.PushRootSignature(reuseRootSignature.Get());
			psoBuilder.Next();
		} 

		psoBuilder.PushHolder(finalShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::RestirGi(L"Final.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Common::ThreadDim());
		psoBuilder.PushRootSignature(finalRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(upsampleShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::RestirGi(L"Upsample.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Common::ThreadDim());
		psoBuilder.PushRootSignature(upsampleRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(clearShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::RestirGi(L"Clear.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Common::ThreadDim());
		psoBuilder.PushRootSignature(clearRootSignature.Get());
		psoBuilder.Create(device);
	}
	void JDx12RaytracingGI::BuildBuffer(JDx12GraphicDevice* device)
	{
		UINT pixelsInSampleSet1D = Raytracing::sampleSetDistribution;
		UINT samplesPerSet = Raytracing::sampleCount[0] * pixelsInSampleSet1D * pixelsInSampleSet1D;

		JSampler sampler;
		sampler.Reset(samplesPerSet, Raytracing::sampleSetCount, UniformDistributionPattern::MultiJittered, HemisphereDistribution::Cosine);

		const uint sampleCount = sampler.GetTotalSampleCount();
		if (hemiSample == nullptr)
			hemiSample = std::make_unique<JDx12GraphicBufferT<HemisphereSample>>(L"HemiSample", J_GRAPHIC_BUFFER_TYPE::UPLOAD_BUFFER);
		hemiSample->Clear();
		hemiSample->Build(device->GetDevice(), sampleCount);

		std::vector<HemisphereSample> sampleVec(sampleCount);
		for (uint i = 0; i < sampleCount; ++i)
			sampleVec[i].direction = sampler.GetHemisphereSample3D();

		hemiSample->CopyData(0, sampleCount, sampleVec.data());
	}
	void JDx12RaytracingGI::ClearResource()
	{ 
		//ClearBuffer();
		ClearUserPrivateData();
		ClearPso();
		ClearShaderTable();
		ClearStateObject();
		ClearRootSignature();
	}
	void JDx12RaytracingGI::ClearRootSignature()
	{ 
		raytracingRootSignature = nullptr;
		reuseRootSignature = nullptr;
		finalRootSignature = nullptr; 
		upsampleRootSignature = nullptr;
		clearRootSignature = nullptr;
	}
	void JDx12RaytracingGI::ClearStateObject()
	{
		for (uint i = 0; i < SIZE_OF_ARRAY(stateObject); ++i)
			stateObject[i] = nullptr;
	}
	void JDx12RaytracingGI::ClearShaderTable()
	{
		rayGenShaderTable = nullptr;
		hitGroupShaderTable = nullptr;
		missShaderTable = nullptr;
	}
	void JDx12RaytracingGI::ClearPso()
	{
		for (uint i = 0; i < SIZE_OF_ARRAY(reuseSamplingShader); ++i)
			reuseSamplingShader[i] = nullptr; 
		finalShader = nullptr;
		upsampleShader = nullptr;
		clearShader = nullptr;
	}
	void JDx12RaytracingGI::ClearBuffer()
	{
		hemiSample = nullptr;
	}
	void JDx12RaytracingGI::ClearUserPrivateData()
	{
		userPrivate.clear();
		computeCount = 0; 
	}
}