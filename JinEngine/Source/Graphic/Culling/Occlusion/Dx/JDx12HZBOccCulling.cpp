#include"JDx12HZBOccCulling.h"
#include"../../JCullingInfo.h"
#include"../../JCullingInterface.h"
#include"../../JCullingConstants.h"
#include"../../Dx/JDx12CullingManager.h"
#include"../../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../../JGraphicInfo.h"
#include"../../../JGraphicOption.h"
#include"../../../JGraphicUpdateHelper.h"
#include"../../../DepthMap/Dx/JDx12DepthTest.h"
#include"../../../DepthMap/Dx/JDx12DepthMapDebug.h"
#include"../../../Shader/Dx/JDx12ShaderDataHolder.h" 
#include"../../../GraphicResource/Dx/JDx12GraphicResourceInfo.h"
#include"../../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../../../FrameResource/Dx/JDx12FrameResource.h"
#include"../../../FrameResource/JOcclusionConstants.h"
#include"../../../FrameResource/JBoundingObjectConstants.h"
#include"../../../../Core/Exception/JExceptionMacro.h" 
#include"../../../../Core/Guid/JGuidCreator.h"
#include"../../../../Core/Math/JMathHelper.h"
#include"../../../../Object/GameObject/JGameObject.h"  
#include"../../../../Object/Component/Transform/JTransform.h"
#include"../../../../Object/Component/Camera/JCamera.h"
#include"../../../../Object/Component/Camera/JCameraPrivate.h"
#include"../../../../Object/Component/Light/JLight.h"
#include"../../../../Object/Component/Light/JLightPrivate.h"
#include"../../../../Object/Component/RenderItem/JRenderItem.h"
#include"../../../../Object/Resource/JResourceManager.h" 
#include"../../../../Object/Resource/Shader/JShader.h"
#include"../../../../Object/Resource/Shader/JShaderPrivate.h"
#include"../../../../Object/Resource/Scene/JScene.h"
#include"../../../../Object/Resource/Mesh/JMeshGeometry.h"
#include"../../../../Object/Resource/Material/JMaterial.h"
#include"../../../../Application/JApplicationEngine.h"
#include"../../../../ThirdParty/DirectX/Tk/Src/d3dx12.h"

//Debug
#include"../../../../Core/File/JFileIOHelper.h"

#if defined(DEVELOP)
#include"../../../../Develop/Debug/JDevelopDebug.h"
#endif 
namespace JinEngine::Graphic
{
	namespace
	{
		//shadow map draw root
		static constexpr int depthMapBuffIndex = 0;	//same as objCB(graphic root)
		static constexpr int mipMapBuffIndex = depthMapBuffIndex + 1;
		static constexpr int lastMipMapInex = mipMapBuffIndex + 1;
		static constexpr int objectBuffIndex = lastMipMapInex + 1;
		static constexpr int queryResultIndex = objectBuffIndex + 1;
		static constexpr int depthMapInfoCBIndex = queryResultIndex + 1;
		static constexpr int computePassCBIndex = depthMapInfoCBIndex + 1;
		static constexpr int debugPassCBIndex = computePassCBIndex + 1;
		//static constexpr int slotCount = computePassCBIndex + 1;
		static constexpr int slotCount = debugPassCBIndex + 1;
		static constexpr bool allowHzbDebug = true;
	}
	namespace
	{
		static void _StreamOutDebugInfo(JHlslDebug<HZBDebugInfo>* debugInfo, const std::wstring& path)
		{
			if (!allowHzbDebug)
				return;
 
			auto rawVec = JRenderItem::StaticTypeInfo().GetInstanceRawPtrVec();
			std::wofstream stream;
			stream.open(path.c_str(), std::ios::app | std::ios::out);
			if (stream.is_open())
			{
				int count = 0;
				int offset = 0;
				HZBDebugInfo* info = debugInfo->Map(count);
				for (uint i = 0; i < count; ++i)
				{
					//JFileIOHelper::StoreXMFloat3(stream, L"Center", info[i].center);
					//JFileIOHelper::StoreXMFloat3(stream, L"Extents", info[i].extents);
	 
					JFileIOHelper::StoreJString(stream, L"Name:", static_cast<JRenderItem*>(rawVec[info[i].queryIndex])->GetOwner()->GetName());
					JFileIOHelper::StoreVector3(stream, L"Scale:", static_cast<JRenderItem*>(rawVec[info[i].queryIndex])->GetOwner()->GetTransform()->GetScale());

					//JFileIOHelper::StoreXMFloat3(stream, L"BBox0", info[i].bboxV[0]);
					//JFileIOHelper::StoreXMFloat3(stream, L"BBox1", info[i].bboxV[1]);
					//JFileIOHelper::StoreXMFloat3(stream, L"BBox2", info[i].bboxV[2]);
					//JFileIOHelper::StoreXMFloat3(stream, L"BBox3", info[i].bboxV[3]);
					//JFileIOHelper::StoreXMFloat3(stream, L"BBox4", info[i].bboxV[4]);
					//JFileIOHelper::StoreXMFloat3(stream, L"BBox5", info[i].bboxV[5]);
					//JFileIOHelper::StoreXMFloat3(stream, L"BBox6", info[i].bboxV[6]);
					//JFileIOHelper::StoreXMFloat3(stream, L"BBox7", info[i].bboxV[7]);

					//JFileIOHelper::StoreXMFloat3(stream, L"NearP0", info[i].nearPoint[0]);
					//JFileIOHelper::StoreXMFloat3(stream, L"NearP1", info[i].nearPoint[1]);
					//JFileIOHelper::StoreXMFloat3(stream, L"NearP2", info[i].nearPoint[2]);
					///JFileIOHelper::StoreXMFloat3(stream, L"NearP3", info[i].nearPoint[3]);
					//JFileIOHelper::StoreXMFloat3(stream, L"NearP4", info[i].nearPoint[4]);
					//JFileIOHelper::StoreXMFloat3(stream, L"NearP5", info[i].nearPoint[5]);

				//	JFileIOHelper::StoreXMFloat4(stream, L"MinNearPoint", info[i].minNearPoint);
				//	JFileIOHelper::StoreXMFloat4(stream, L"ClipNearH", info[i].clipNearH);
					//JFileIOHelper::StoreXMFloat3(stream, L"ClipNearC", info[i].clipNearC);
					 
					JFileIOHelper::StoreAtomicData(stream, L"QueryIdex", info[i].queryIndex);
					JFileIOHelper::StoreAtomicData(stream, L"ThreadIndex", info[i].threadIndex);
					JFileIOHelper::StoreAtomicData(stream, L"CullingRes", info[i].cullingRes);
					JFileIOHelper::StoreAtomicData(stream, L"CenterDepth", info[i].centerDepth);
					JFileIOHelper::StoreAtomicData(stream, L"FinalCompareDepth", info[i].finalCompareDepth);
					JFileIOHelper::InputSpace(stream, 1);
				}
				stream.close();
			}
		}
		static void _StreamOutDebugBufferInfo(JHlslDebug<HZBDebugInfo>* debugInfo)
		{
#if defined(DEVELOP)
			if (!allowHzbDebug || !Develop::JDevelopDebug::IsActivate())
				return;

			int count = 0;
			HZBDebugInfo* info = debugInfo->Map(count);
			auto rawVec = JRenderItem::StaticTypeInfo().GetInstanceRawPtrVec();

			Develop::JDevelopDebug::PushLog("OccResult.. OutCount: " + std::to_string(count));
			for (uint i = 0; i < 3; ++i)
			{  
				if (rawVec.size() <= i)
					break;

				JRenderItem* rItem = static_cast<JRenderItem*>(rawVec[info[i].queryIndex]);
				Develop::JDevelopDebug::PushLog(L"Name: " + rItem->GetOwner()->GetName());
				Develop::JDevelopDebug::PushLog("World Pos: " + rItem->GetOwner()->GetTransform()->GetWorldPosition().ToString());
				Develop::JDevelopDebug::PushLog("QueryIndex: " + std::to_string(info[i].queryIndex));
				Develop::JDevelopDebug::PushLog("ThreadIndex: " + std::to_string(info[i].threadIndex));
				Develop::JDevelopDebug::PushLog("CullingRes: " + std::to_string(info[i].cullingRes));
				Develop::JDevelopDebug::PushLog("CenterDepth: " + std::to_string(info[i].centerDepth));
				Develop::JDevelopDebug::PushLog("FinalCompareDepth: " + std::to_string(info[i].finalCompareDepth)); 
				Develop::JDevelopDebug::PushLog("MinNearPoint: " + info[i].minNearPoint.ToString());
				Develop::JDevelopDebug::PushLog("ClipNearH: " + info[i].clipNearH.ToString());
				Develop::JDevelopDebug::PushLog("ClipNearC: " + info[i].clipNearC.ToString());

				Develop::JDevelopDebug::PushLog("BBox0: " + info[i].bboxV[0].ToString());
				Develop::JDevelopDebug::PushLog("BBox1: " + info[i].bboxV[1].ToString());
				Develop::JDevelopDebug::PushLog("BBox2: " + info[i].bboxV[2].ToString());
				Develop::JDevelopDebug::PushLog("BBox3: " + info[i].bboxV[3].ToString());
				Develop::JDevelopDebug::PushLog("BBox4: " + info[i].bboxV[4].ToString());
				Develop::JDevelopDebug::PushLog("BBox5: " + info[i].bboxV[5].ToString());
				Develop::JDevelopDebug::PushLog("BBox6: " + info[i].bboxV[6].ToString());
				Develop::JDevelopDebug::PushLog("BBox7: " + info[i].bboxV[7].ToString());
				 
				Develop::JDevelopDebug::PushLog("nearPoint0: " + info[i].nearPoint[0].ToString());
				Develop::JDevelopDebug::PushLog("nearPoint1: " + info[i].nearPoint[1].ToString());
				Develop::JDevelopDebug::PushLog("nearPoint2: " + info[i].nearPoint[2].ToString());
				Develop::JDevelopDebug::PushLog("nearPoint3: " + info[i].nearPoint[3].ToString());
				Develop::JDevelopDebug::PushLog("nearPoint4: " + info[i].nearPoint[4].ToString());
				Develop::JDevelopDebug::PushLog("nearPoint5: " + info[i].nearPoint[5].ToString());

				Develop::JDevelopDebug::PushLog("Ndc0: " + info[i].ndc[0].ToString());
				Develop::JDevelopDebug::PushLog("Ndc1: " + info[i].ndc[1].ToString());
				Develop::JDevelopDebug::PushLog("Ndc2: " + info[i].ndc[2].ToString());
				Develop::JDevelopDebug::PushLog("Ndc3: " + info[i].ndc[3].ToString());
				Develop::JDevelopDebug::PushLog("Ndc4: " + info[i].ndc[4].ToString());
				Develop::JDevelopDebug::PushLog("Ndc5: " + info[i].ndc[5].ToString());
				Develop::JDevelopDebug::PushLog("Ndc6: " + info[i].ndc[6].ToString());
				Develop::JDevelopDebug::PushLog("Ndc7: " + info[i].ndc[7].ToString());

				Develop::JDevelopDebug::PushLog("compareDepth0: " + std::to_string(info[i].compareDepth[0]));
				Develop::JDevelopDebug::PushLog("compareDepth1: " + std::to_string(info[i].compareDepth[1]));
				Develop::JDevelopDebug::PushLog("compareDepth2: " + std::to_string(info[i].compareDepth[2]));
				Develop::JDevelopDebug::PushLog("compareDepth3: " + std::to_string(info[i].compareDepth[3]));
				Develop::JDevelopDebug::PushLog("compareDepth4: " + std::to_string(info[i].compareDepth[4]));
				Develop::JDevelopDebug::PushLog("compareDepth5: " + std::to_string(info[i].compareDepth[5]));
				Develop::JDevelopDebug::PushLog("compareDepth6: " + std::to_string(info[i].compareDepth[6]));
				Develop::JDevelopDebug::PushLog("compareDepth7: " + std::to_string(info[i].compareDepth[7]));
			}
			Develop::JDevelopDebug::Write();
#endif
		}
	}
	void JDx12HZBOccCulling::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;
		 
		BuildRootSignature(device, info.occlusionMapCapacity);
		BuildUploadBuffer(device, info.upObjCapacity, info.occlusionMapCapacity);

		uint nowWidth = info.occlusionWidth;
		uint nowHeight = info.occlusionHeight;
		for (uint i = 0; i < info.occlusionMapCount; ++i)
		{
			JHzbOccDepthMapInfoConstants constants;
			constants.nowWidth = nowWidth;
			constants.nowHeight = nowHeight;
			constants.nowIndex = i;

			occDepthMapInfoCB->CopyData(i, constants);
			nowWidth /= 2;
			nowHeight /= 2;
		}
	}
	void JDx12HZBOccCulling::Clear()
	{
		mRootSignature.Reset();
		occQueryOutBuffer.reset();
		//Debug 
		occDebugBuffer.clear();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12HZBOccCulling::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	bool JDx12HZBOccCulling::CanReadBackDebugInfo()const noexcept
	{
		return true;
	}
	bool JDx12HZBOccCulling::HasPreprocessing()const noexcept
	{
		return true;
	}
	bool JDx12HZBOccCulling::HasPostprocessing()const noexcept
	{
		return true;
	}
	void JDx12HZBOccCulling::NotifyBuildNewHzbOccBuffer(JGraphicDevice* device, const size_t initCapacity, const JUserPtr<JCullingInfo>& cullingInfo)
	{
		BuildOccDebugBuffer(device, initCapacity, cullingInfo);
		 
		cullingInfo->SetUpdateFrequency(1.0f);
		cullingInfo->SetUpdatePerObjectRate(1.0f);
		cullingInfo->SetUpdateEnd(true);
	}
	void JDx12HZBOccCulling::NotifyReBuildHzbOccBuffer(JGraphicDevice* device, const size_t capacity, const std::vector<JUserPtr<JCullingInfo>>& cullingInfo)
	{
		ReBuildOccDebugBuffer(device, capacity, cullingInfo);
		ReBuildObjectConstants(device, capacity);
		for (const auto& data : cullingInfo)
		{
			data->SetUpdateFrequency(1.0f);
			data->SetUpdatePerObjectRate(1.0f);
			data->SetUpdateEnd(true);
		}
	}
	void JDx12HZBOccCulling::NotifyDestroyHzbOccBuffer(JCullingInfo* cullingInfo)
	{
		DestroyOccDebugBuffer(cullingInfo);
	}
	bool JDx12HZBOccCulling::BuildOccDebugBuffer(JGraphicDevice* device, const size_t initCapacity, const JUserPtr<JCullingInfo>& cullingInfo)
	{
		if (!IsSameDevice(device))
			return false;

		if (!allowHzbDebug || cullingInfo->GetCullingType() != J_CULLING_TYPE::HZB_OCCLUSION)
			return false;
		 
		auto debugBuff = std::make_unique<JHlslDebug<HZBDebugInfo>>(L"Hzb", debugPassCBIndex);
		debugBuff->Build(device, initCapacity);
		occDebugBuffer.push_back(std::move(debugBuff));
		return true;
	}
	void JDx12HZBOccCulling::ReBuildOccDebugBuffer(JGraphicDevice* device, const size_t capacity, const std::vector<JUserPtr<JCullingInfo>>& cullingInfo)
	{ 
		if (!IsSameDevice(device) || !allowHzbDebug)
			return;
		 
		for (const auto& data : cullingInfo)
		{
			if (data->GetCullingType() != J_CULLING_TYPE::HZB_OCCLUSION)
				continue;

			uint arrayIndex = data->GetArrayIndex();
			occDebugBuffer[arrayIndex]->Clear();
			occDebugBuffer[arrayIndex]->Build(device, capacity);
		}
	}
	void JDx12HZBOccCulling::ReBuildObjectConstants(JGraphicDevice* device, const uint objectCapacity)
	{
		if (!IsSameDevice(device))
			return;

		occQueryOutBuffer->Clear();
		occQueryOutBuffer->Build(device, objectCapacity);

		JShader* shader = _JResourceManager::Instance().GetDefaultShader(J_DEFAULT_COMPUTE_SHADER::DEFUALT_HZB_OCCLUSION_SHADER).Get();
		RecompileShader(shader);
	}
	void JDx12HZBOccCulling::DestroyOccDebugBuffer(JCullingInfo* cullingInfo)
	{
		if (!allowHzbDebug)
			return;
		 
		occDebugBuffer.erase(occDebugBuffer.begin() + cullingInfo->GetArrayIndex());
	} 
	void JDx12HZBOccCulling::ReBuildOcclusionMapInfoConstants(JGraphicDevice* device, const JGraphicInfo& info)
	{
		if (!IsSameDevice(device))
			return;

		uint nowWidth = info.occlusionWidth;
		uint nowHeight = info.occlusionHeight;
		for (uint i = 0; i < info.occlusionMapCount; ++i)
		{
			JHzbOccDepthMapInfoConstants constants;
			constants.nowWidth = nowWidth;
			constants.nowHeight = nowHeight;
			constants.nowIndex = i;

			occDepthMapInfoCB->CopyData(i, constants);
			nowWidth /= 2;
			nowHeight /= 2;
		}

		//기본값으로 설정된 shader는 app이 실행될때 default shader에있는 정보를 참조해서 자동으로 로드됨 
		JShader* shader = _JResourceManager::Instance().GetDefaultShader(J_DEFAULT_COMPUTE_SHADER::DEFUALT_HZB_DOWNSAMPLING_SHADER).Get();
		RecompileShader(shader);
	}
	void JDx12HZBOccCulling::StreamOutDebugInfo(const JUserPtr<JCullingInfo>& cullingInfo, const std::wstring& path)
	{
		if (cullingInfo == nullptr)
			return;

		int arrayIndex = cullingInfo->GetArrayIndex();
		if (arrayIndex == -1)
			return;

		_StreamOutDebugInfo(occDebugBuffer[arrayIndex].get(), path);
	}
	void JDx12HZBOccCulling::StreamOutDebugInfo(const std::wstring& path)
	{
		if (occDebugBuffer.size() == 0)
			return;

		_StreamOutDebugBufferInfo(occDebugBuffer[0].get());
		//_StreamOutDebugInfo(occDebugBuffer[0].get(), path);
	}
	void JDx12HZBOccCulling::BeginDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(bindSet))
			return;

		const JDx12GraphicBindSet* dx12BindSet = static_cast<const JDx12GraphicBindSet*>(bindSet);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12BindSet->graphicResourceM);
		ID3D12GraphicsCommandList* cmdList = dx12BindSet->cmdList;

		auto gRInterface = helper.GetOccGResourceInterface();
		const uint dataCount = gRInterface.GetDataCount(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP);
		for (uint i = 0; i < dataCount; ++i)
		{
			const int occVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, i);
			const int occHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, J_GRAPHIC_BIND_TYPE::DSV, i);

			ID3D12Resource* occDepthMap = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, occVecIndex);
			D3D12_CPU_DESCRIPTOR_HANDLE dsv = dx12Gm->GetCpuDsvDescriptorHandle(occHeapIndex);

			JD3DUtility::ResourceTransition(cmdList, occDepthMap, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		}
	}
	void JDx12HZBOccCulling::EndDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(bindSet))
			return;

		const JDx12GraphicBindSet* dx12BindSet = static_cast<const JDx12GraphicBindSet*>(bindSet);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12BindSet->graphicResourceM);
		ID3D12GraphicsCommandList* cmdList = dx12BindSet->cmdList;

		auto gRInterface = helper.GetOccGResourceInterface();
		const uint dataCount = gRInterface.GetDataCount(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP);
		for (uint i = 0; i < dataCount; ++i)
		{
			const int occVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, i);
			ID3D12Resource* occDepthMap = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, occVecIndex);
			JD3DUtility::ResourceTransition(cmdList, occDepthMap, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
		}
	}
	void JDx12HZBOccCulling::DrawOcclusionDepthMap(const JGraphicOccDrawSet* occDrawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(occDrawSet))
			return;

		if (!helper.CanOccCulling() || !helper.GetCullingUserAccess()->AllowHzbOcclusionCulling())
			return;

		const JDx12GraphicOccDrawSet* dx12DrawSet = static_cast<const JDx12GraphicOccDrawSet*>(occDrawSet);
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(dx12DrawSet->device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12DrawSet->graphicResourceM);
		ID3D12GraphicsCommandList* cmdList = dx12DrawSet->cmdList;

		//cmdList->SetGraphicsRootSignature(mRootSignature.Get());
		auto gRInterface = helper.GetOccGResourceInterface();
		const uint dataCount = gRInterface.GetDataCount(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP);
		for (uint i = 0; i < dataCount; ++i)
		{
			const int occVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, i);
			const int occHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, J_GRAPHIC_BIND_TYPE::DSV, i);
			//const uint occMipMapVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP, i);

			D3D12_VIEWPORT mViewport = { 0.0f, 0.0f,(float)helper.info.occlusionWidth, (float)helper.info.occlusionHeight, 0.0f, 1.0f };
			D3D12_RECT mScissorRect = { 0, 0, helper.info.occlusionWidth, helper.info.occlusionHeight };

			cmdList->RSSetViewports(1, &mViewport);
			cmdList->RSSetScissorRects(1, &mScissorRect);

			ID3D12Resource* occDepthMap = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, occVecIndex); 
			D3D12_CPU_DESCRIPTOR_HANDLE dsv = dx12Gm->GetCpuDsvDescriptorHandle(occHeapIndex);

			JD3DUtility::ResourceTransition(cmdList, occDepthMap, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
			cmdList->OMSetRenderTargets(0, nullptr, false, &dsv);

			JAcceleratorAlignInfo alignInfo;
			if (helper.occCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
			{
				alignInfo = JAcceleratorAlignInfo(J_ACCELERATOR_LAYER::COMMON_OBJECT,
					J_ACCELERATOR_TYPE::BVH,
					helper.cam->GetBoundingFrustum(), false);
				//true);
			}
			else
			{
				alignInfo = JAcceleratorAlignInfo(J_ACCELERATOR_LAYER::COMMON_OBJECT,
					J_ACCELERATOR_TYPE::BVH,
					helper.lit->GetOwner()->GetTransform()->GetWorldPosition());
			}

			//caution 
			//kd tree activated 되있어야함
			alignInfo.alignPassCondPtr = [](JGameObject* obj)
			{
				return obj->GetRenderItem()->IsOccluder();
			};
			alignInfo.alignRange = JAcceleratorAlignInfo::ALIGN_RANGE::ALL;
			//draw specific count
			 
			JDx12GraphicDepthMapDrawSet depthMapSet(dx12DrawSet);
			occDrawSet->depthTest->DrawSceneBoundingBox(&depthMapSet,
				helper.scene->AlignedObject(alignInfo),
				helper,
				JDrawCondition(helper, false, true, false));

			JD3DUtility::ResourceTransition(cmdList, occDepthMap, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
		}
	}
	void JDx12HZBOccCulling::DrawOcclusionDepthMapMultiThread(const JGraphicOccDrawSet* occDrawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(occDrawSet))
			return;

		if (!helper.CanOccCulling() || !helper.GetCullingUserAccess()->AllowHzbOcclusionCulling())
			return;

		const JDx12GraphicOccDrawSet* dx12DrawSet = static_cast<const JDx12GraphicOccDrawSet*>(occDrawSet);
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(dx12DrawSet->device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12DrawSet->graphicResourceM);
		ID3D12GraphicsCommandList* cmdList = dx12DrawSet->cmdList;

		//cmdList->SetGraphicsRootSignature(mRootSignature.Get());
		auto gRInterface = helper.GetOccGResourceInterface();
		const uint dataCount = gRInterface.GetDataCount(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP);
		for (uint i = 0; i < dataCount; ++i)
		{
			const int occHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, J_GRAPHIC_BIND_TYPE::DSV, i);
			D3D12_VIEWPORT mViewport = { 0.0f, 0.0f,(float)helper.info.occlusionWidth, (float)helper.info.occlusionHeight, 0.0f, 1.0f };
			D3D12_RECT mScissorRect = { 0, 0, helper.info.occlusionWidth, helper.info.occlusionHeight };

			cmdList->RSSetViewports(1, &mViewport);
			cmdList->RSSetScissorRects(1, &mScissorRect);

			D3D12_CPU_DESCRIPTOR_HANDLE dsv = dx12Gm->GetCpuDsvDescriptorHandle(occHeapIndex);
			cmdList->OMSetRenderTargets(0, nullptr, false, &dsv);

			JAcceleratorAlignInfo alignInfo;
			if (helper.occCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
			{
				alignInfo = JAcceleratorAlignInfo(J_ACCELERATOR_LAYER::COMMON_OBJECT,
					J_ACCELERATOR_TYPE::BVH,
					helper.cam->GetBoundingFrustum(),
					true);
			}
			else
			{
				alignInfo = JAcceleratorAlignInfo(J_ACCELERATOR_LAYER::COMMON_OBJECT,
					J_ACCELERATOR_TYPE::BVH,
					helper.lit->GetOwner()->GetTransform()->GetWorldPosition());
			}

			alignInfo.alignPassCondPtr = [](JGameObject* obj)
			{
				return obj->GetRenderItem()->IsOccluder();
			};
			alignInfo.alignRange = JAcceleratorAlignInfo::ALIGN_RANGE::ALL;
			//draw specific count
			JDx12GraphicDepthMapDrawSet depthMapSet(dx12DrawSet);
			occDrawSet->depthTest->DrawSceneBoundingBox(&depthMapSet,
				helper.scene->AlignedObject(alignInfo),
				helper,
				JDrawCondition(helper, false, true, false));
		}
	}
	void JDx12HZBOccCulling::DrawOcclusionDebugMap(const JGraphicOccDebugDrawSet* occDebugDrawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(occDebugDrawSet))
			return;

		if (!helper.CanOccCulling() ||
			!helper.allowDrawOccDepthMap ||
			!helper.GetCullingUserAccess()->AllowHzbOcclusionCulling())
			return;

		const JDx12GraphicOccDebugDrawSet* dx12DrawSet = static_cast<const JDx12GraphicOccDebugDrawSet*>(occDebugDrawSet);
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(dx12DrawSet->device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12DrawSet->graphicResourceM);
		ID3D12GraphicsCommandList* cmdList = dx12DrawSet->cmdList;

		JVector2<uint> occlusionSize = JVector2<uint>(helper.info.occlusionWidth, helper.info.occlusionHeight);
		float camNear = 0;
		float camFar = 0;
		bool isNonlinear = true;

		if (helper.occCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
		{
			camNear = helper.cam->GetNear();
			camFar = helper.cam->GetFar();
		}
		else
		{
			camNear = helper.lit->GetNear();
			camFar = helper.lit->GetFar();
		}

		J_GRAPHIC_RESOURCE_TYPE srcType;
		J_GRAPHIC_RESOURCE_TYPE destType;
		if (helper.option.IsHZBOccActivated())
		{
			srcType = J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP;
			//Debug and mipmap viwe count is same 
			destType = J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG;
			isNonlinear = false;
		}
		else
			return;

		auto gRInterface = helper.GetOccGResourceInterface();
		const uint dataCount = gRInterface.GetDataCount(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP);
		for (uint i = 0; i < dataCount; ++i)
		{
			auto srcInfo = dx12Gm->GetDxInfo(srcType, gRInterface.GetResourceArrayIndex(srcType, i));
			auto destInfo = dx12Gm->GetDxInfo(destType, gRInterface.GetResourceArrayIndex(destType, i));
			if (srcInfo == nullptr || destInfo == nullptr)
				return;

			const uint viewCount = destInfo->GetViewCount(J_GRAPHIC_BIND_TYPE::SRV);
			for (uint j = 0; j < viewCount; ++j)
			{
				JDx12GraphicDepthMapDebugHandleSet handleSet(occlusionSize,
					camNear,
					camFar,
					cmdList,
					dx12Gm->GetGpuSrvDescriptorHandle(srcInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV) + j),
					dx12Gm->GetGpuSrvDescriptorHandle(destInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::UAV) + j));
				if (isNonlinear)
					dx12DrawSet->depthDebug->DrawNonLinearDepthDebug(&handleSet);
				else
					dx12DrawSet->depthDebug->DrawLinearDepthDebug(&handleSet);
				//mipmap
				if (helper.option.IsHZBOccActivated())
					occlusionSize /= 2.0f;
			}
		}
	}
	void JDx12HZBOccCulling::ComputeOcclusionCulling(const JGraphicHzbOccComputeSet* computeSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(computeSet))
			return;

		const JDx12GraphicHzbOccComputeSet* dx12ComputeSet = static_cast<const JDx12GraphicHzbOccComputeSet*>(computeSet);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(dx12ComputeSet->currFrame);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12ComputeSet->graphicResourceM);
		JDx12CullingManager* dx12Cm = static_cast<JDx12CullingManager*>(dx12ComputeSet->cullingM);
		ID3D12GraphicsCommandList* cmdList = dx12ComputeSet->cmdList;

		auto gRInterface = helper.GetOccGResourceInterface();
		const uint dataCount = gRInterface.GetDataCount(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP);

		auto cInterface = helper.GetCullInterface();
		for (uint i = 0; i < dataCount; ++i)
		{
			const int occVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, i);
			const int occMipMapVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP, i);

			auto occDsInfo = dx12Gm->GetDxInfo(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, occVecIndex);
			auto occMipMapInfo = dx12Gm->GetDxInfo(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP, occMipMapVecIndex);
			uint occPassFrameIndex = 0;

			if (helper.cam != nullptr)
				occPassFrameIndex = helper.GetCamHzbOccComputeFrameIndex(); 
			else if (helper.lit != nullptr)
				occPassFrameIndex = helper.GetLitHzbOccComputeFrameIndex(); 

			DepthMapDownSampling(cmdList,
				dx12Frame,
				dx12Gm->GetGpuSrvDescriptorHandle(occDsInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV)),
				dx12Gm->GetGpuSrvDescriptorHandle(occMipMapInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV)),
				dx12Gm->GetGpuSrvDescriptorHandle(occMipMapInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::UAV)),
				helper.info.occlusionMapCount,
				dx12Gm->GetDescriptorSize(J_GRAPHIC_BIND_TYPE::SRV),
				occPassFrameIndex);
			OcclusionCulling(cmdList,
				dx12Frame,
				dx12Cm,
				dx12Gm->GetGpuSrvDescriptorHandle(occMipMapInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV)),
				occPassFrameIndex,
				helper.GetCullInterface());
		}
		//ComputeOcclusionCulling은 항상 single thread에서 수행된다.
		dx12Cm->GetCullingInfo(J_CULLING_TYPE::HZB_OCCLUSION, cInterface.GetArrayIndex(J_CULLING_TYPE::HZB_OCCLUSION))->SetUpdateEnd(true);
	}
	void JDx12HZBOccCulling::DepthMapDownSampling(ID3D12GraphicsCommandList* commandList,
		JDx12FrameResource* dx12Frame,
		CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapSrvHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE mipMapSrvHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE mipMapUavHandle,
		const uint samplingCount,
		const uint srvDescriptorSize,
		const uint passCBIndex)
	{
		const uint depthMapInfoCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JHzbOccDepthMapInfoConstants));
		commandList->SetComputeRootSignature(mRootSignature.Get());

		JShader* copyShader = _JResourceManager::Instance().GetDefaultShader(J_DEFAULT_COMPUTE_SHADER::DEFUALT_HZB_COPY_SHADER).Get();
		auto copyShaderData = static_cast<JDx12ComputeShaderDataHolder*>(copyShader->GetComputeData().Get());
 
		commandList->SetPipelineState(copyShaderData->pso.Get());

		uint passCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JHzbOccComputeConstants));
		D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = dx12Frame->hzbOccReqCB->GetResource()->GetGPUVirtualAddress() + passCBIndex * passCBByteSize;

		commandList->SetComputeRootDescriptorTable(depthMapBuffIndex, depthMapSrvHandle);
		commandList->SetComputeRootDescriptorTable(lastMipMapInex, mipMapUavHandle);
		commandList->SetComputeRootConstantBufferView(depthMapInfoCBIndex, occDepthMapInfoCB->GetResource()->GetGPUVirtualAddress());
		commandList->SetComputeRootConstantBufferView(computePassCBIndex, passCBAddress);

		JVector3<uint> cgroupDim = copyShader->GetComputeGroupDim();
		commandList->Dispatch(cgroupDim.x, cgroupDim.y, cgroupDim.z);

		JShader* downSampleShader = _JResourceManager::Instance().GetDefaultShader(J_DEFAULT_COMPUTE_SHADER::DEFUALT_HZB_DOWNSAMPLING_SHADER).Get();
		auto downSamplerShaderData = static_cast<JDx12ComputeShaderDataHolder*>(downSampleShader->GetComputeData().Get());
		commandList->SetPipelineState(downSamplerShaderData->pso.Get());

		const uint loopCount = samplingCount - 1;
		for (uint i = 0; i < loopCount; ++i)
		{
			CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle = mipMapSrvHandle;
			srcHandle.Offset(i, srvDescriptorSize);

			CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle = mipMapUavHandle;
			destHandle.Offset(i + 1, srvDescriptorSize);

			commandList->SetComputeRootDescriptorTable(mipMapBuffIndex, srcHandle);
			commandList->SetComputeRootDescriptorTable(lastMipMapInex, destHandle);

			D3D12_GPU_VIRTUAL_ADDRESS depthMapCBAddress = occDepthMapInfoCB->GetResource()->GetGPUVirtualAddress() + i * depthMapInfoCBByteSize;
			commandList->SetComputeRootConstantBufferView(depthMapInfoCBIndex, depthMapCBAddress);
			JVector3<uint> dgroupDim = downSampleShader->GetComputeGroupDim();
			commandList->Dispatch(dgroupDim.x, dgroupDim.y, dgroupDim.z);
		}
	}
	void JDx12HZBOccCulling::OcclusionCulling(ID3D12GraphicsCommandList* commandList,
		JDx12FrameResource* dx12Frame,
		JDx12CullingManager* dx12Cm,
		CD3DX12_GPU_DESCRIPTOR_HANDLE mipMapStHandle,
		const uint passCBIndex,
		const JCullingUserInterface& cullUser)
	{
		uint passCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JHzbOccComputeConstants));
		D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = dx12Frame->hzbOccReqCB->GetResource()->GetGPUVirtualAddress() + passCBIndex * passCBByteSize;

		commandList->SetComputeRootDescriptorTable(mipMapBuffIndex, mipMapStHandle);
		commandList->SetComputeRootShaderResourceView(objectBuffIndex, dx12Frame->hzbOccObjectBuffer->GetResource()->GetGPUVirtualAddress());
		commandList->SetComputeRootUnorderedAccessView(queryResultIndex, occQueryOutBuffer->GetResource()->GetGPUVirtualAddress());
		commandList->SetComputeRootConstantBufferView(computePassCBIndex, passCBAddress);

		//Debug
		if (allowHzbDebug)
			occDebugBuffer[cullUser.GetArrayIndex(J_CULLING_TYPE::HZB_OCCLUSION)]->SettingCompute(commandList);

		JShader* shader = _JResourceManager::Instance().GetDefaultShader(J_DEFAULT_COMPUTE_SHADER::DEFUALT_HZB_OCCLUSION_SHADER).Get();
		auto shaderData = static_cast<JDx12ComputeShaderDataHolder*>(shader->GetComputeData().Get());
		commandList->SetPipelineState(shaderData->pso.Get());

		JD3DUtility::ResourceTransition(commandList, occQueryOutBuffer->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		JVector3<uint> groupDim = shader->GetComputeGroupDim();
		commandList->Dispatch(groupDim.x, groupDim.y, groupDim.z); 

		JD3DUtility::ResourceTransition(commandList, occQueryOutBuffer->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
		auto resource = dx12Cm->GetResource(J_CULLING_TYPE::HZB_OCCLUSION, cullUser.GetArrayIndex(J_CULLING_TYPE::HZB_OCCLUSION));		
		commandList->CopyResource(resource, occQueryOutBuffer->GetResource());
		JD3DUtility::ResourceTransition(commandList, occQueryOutBuffer->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);
	
		if (allowHzbDebug)
			occDebugBuffer[cullUser.GetArrayIndex(J_CULLING_TYPE::HZB_OCCLUSION)]->End(commandList);
	}
	void JDx12HZBOccCulling::BuildRootSignature(JGraphicDevice* device, const uint occlusionDsvCapacity)
	{
		if (!IsSameDevice(device))
			return;

		ID3D12Device* d3d12Device = static_cast<JDx12GraphicDevice*>(device)->GetDevice();
		//Debug
		//static constexpr int slotCount = 8; 
		CD3DX12_ROOT_PARAMETER slotRootParameter[slotCount];

		CD3DX12_DESCRIPTOR_RANGE depthMapTable;
		depthMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		CD3DX12_DESCRIPTOR_RANGE mipmapTable;
		mipmapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		CD3DX12_DESCRIPTOR_RANGE lastMipmapTable;
		lastMipmapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		// Create root CBV. 
		slotRootParameter[depthMapBuffIndex].InitAsDescriptorTable(1, &depthMapTable);
		slotRootParameter[mipMapBuffIndex].InitAsDescriptorTable(1, &mipmapTable);
		slotRootParameter[lastMipMapInex].InitAsDescriptorTable(1, &lastMipmapTable);

		slotRootParameter[objectBuffIndex].InitAsShaderResourceView(2);
		slotRootParameter[queryResultIndex].InitAsUnorderedAccessView(1, 1);
		slotRootParameter[depthMapInfoCBIndex].InitAsConstantBufferView(0);
		slotRootParameter[computePassCBIndex].InitAsConstantBufferView(1);

		//Debug
		slotRootParameter[debugPassCBIndex].InitAsUnorderedAccessView(2, 1);

		std::vector< CD3DX12_STATIC_SAMPLER_DESC> samDesc
		{
			//hzb downSampling
			CD3DX12_STATIC_SAMPLER_DESC(0,
			D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
			0.0f,                               // mipLODBias
			1,                                 // maxAnisotropy
			D3D12_COMPARISON_FUNC_LESS_EQUAL,
			D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE),

			//hzb compute
			CD3DX12_STATIC_SAMPLER_DESC(1,
			D3D12_FILTER_MIN_MAG_MIP_LINEAR ,	//filter... point, Anisortpic filter사용시 근접했을때 비정확한 값을 sample하는 빈도가 높아짐... 반드시 linear 사용
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
			0.0f,                               // mipLODBias
			1,                                 // maxAnisotropy
			D3D12_COMPARISON_FUNC_ALWAYS,
			D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK)
		};
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(slotCount, slotRootParameter, (uint)samDesc.size(), samDesc.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		// create a root signature with a single slot which points to a descriptor length consisting of a single constant buffer
		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		ThrowIfFailedHr(hr);
		ThrowIfFailedHr(d3d12Device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(mRootSignature.GetAddressOf())));

		mRootSignature->SetName(L"HZB RootSignature");
	}
	void JDx12HZBOccCulling::BuildUploadBuffer(JGraphicDevice* device, const uint objectCapacity, const uint occlusionMapCapacity)
	{
		if (!IsSameDevice(device))
			return;

		occDepthMapInfoCB = std::make_unique<JDx12GraphicBuffer<JHzbOccDepthMapInfoConstants>>(L"HzbOccDepthMapInfo", J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT);
		occQueryOutBuffer = std::make_unique<JDx12GraphicBuffer<uint>>(L"HzbOccResultOut", J_GRAPHIC_BUFFER_TYPE::UNORDERED_ACCEESS);

		occDepthMapInfoCB->Build(device, occlusionMapCapacity);
		occQueryOutBuffer->Build(device, objectCapacity);
	}
	JOwnerPtr<JComputeShaderDataHolderBase> JDx12HZBOccCulling::CreateComputeShader(JGraphicDevice* device, JGraphicResourceManager* gResourceM, const JComputeShaderInitData& initData)
	{
		if (!IsSameDevice(device))
			return nullptr;

		if (initData.cFunctionFlag != J_COMPUTE_SHADER_FUNCTION::HZB_COPY &&
			initData.cFunctionFlag != J_COMPUTE_SHADER_FUNCTION::HZB_DOWN_SAMPLING &&
			initData.cFunctionFlag != J_COMPUTE_SHADER_FUNCTION::HZB_OCCLUSION)
			return nullptr;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(gResourceM);
		auto holder = Core::JPtrUtil::MakeOwnerPtr<JDx12ComputeShaderDataHolder>();

		CompileShader(holder.Get(), initData); 
		StuffPso(holder.Get(),
			dx12Device,
			dx12Gm, 
			initData);

		return std::move(holder);
	}
	void JDx12HZBOccCulling::CompileShader(JDx12ComputeShaderDataHolder* holder, const JComputeShaderInitData& initData)
	{
		JShaderType::CompileInfo compileInfo = JShaderType::ComputeShaderCompileInfo(initData.cFunctionFlag);
		std::wstring computeShaderPath = JApplicationEngine::ShaderPath() + L"\\" + compileInfo.fileName;

		auto macro = JDxShaderDataUtil::ToD3d12Macro(initData.macro);
		holder->cs = JD3DUtility::CompileShader(computeShaderPath, macro.data(), compileInfo.functionName, "cs_5_1");
		holder->dispatchInfo = initData.dispatchInfo;
	}
	void JDx12HZBOccCulling::StuffPso(JDx12ComputeShaderDataHolder* holder,
		JDx12GraphicDevice* dx12Device,
		JDx12GraphicResourceManager* dx12Gm,
		const JComputeShaderInitData& initData)
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC newShaderPso;
		ZeroMemory(&newShaderPso, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
		newShaderPso.pRootSignature = mRootSignature.Get();

		holder->RootSignature = newShaderPso.pRootSignature;
		newShaderPso.CS =
		{
			reinterpret_cast<BYTE*>(holder->cs->GetBufferPointer()),
			holder->cs->GetBufferSize()
		};
		newShaderPso.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		ThrowIfFailedG(dx12Device->GetDevice()->CreateComputePipelineState(&newShaderPso, IID_PPV_ARGS(holder->pso.GetAddressOf())));
	}
}