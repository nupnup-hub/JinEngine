#include"JDx12LightCulling.h"
#include"../../../JGraphicUpdateHelper.h"
#include"../../../JGraphicOption.h"
#include"../../../Device/Dx/JDx12GraphicDevice.h" 
#include"../../../DataSet/Dx/JDx12GraphicDataSet.h" 
#include"../../../Shader/Dx/JDx12ShaderDataHolder.h"
#include"../../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../../../GraphicResource/Dx/JDx12GraphicResourceInfo.h"
#include"../../../FrameResource/Dx/JDx12FrameResource.h"
#include"../../../FrameResource/JFrameUpdate.h"
#include"../../../Culling/Dx/JDx12CullingManager.h"
#include"../../../Utility/Dx/JD3DUtility.h"
#include"../../../../Core/Platform/JHardwareInfo.h"
#include"../../../../Core/Geometry/Mesh/JMeshType.h"
#include"../../../../Core/Geometry/Mesh/Loader/ObjLoader/JObjFileLoader.h"
#include"../../../../Application/JApplicationProject.h"
#include"../../../../Object/Component/Camera/JCamera.h"
#include"../../../../Object/Component/Light/JPointLight.h"
#include"../../../../Object/Component/Light/JPointLightPrivate.h"
#include"../../../../Object/Resource/Scene/JScene.h"
#include"../../../../Object/Resource/JResourceManager.h"
#include"../../../../Object/Resource/Mesh/JMeshGeometry.h" 
#include"../../../../Object/GameObject/JGameObject.h"

#include"../../../../Develop/Debug/JDevelopDebug.h"
//local light만 culling을 적용한다.
#define POINT_LIGHT_ L"POINT_LIGHT"
#define SPOT_LIGHT_ L"SPOT_LIGHT"
#define RECT_LIGHT_ L"RECT_LIGHT"

#define CLUSTER_DIM_X L"CLUSTER_DIM_X"
#define CLUSTER_DIM_Y L"CLUSTER_DIM_Y"
#define CLUSTER_DIM_Z L"CLUSTER_DIM_Z"

#define THREAD_DIM_X L"THREAD_DIM_X"
#define THREAD_DIM_Y L"THREAD_DIM_Y"
#define THREAD_DIM_Z L"THREAD_DIM_Z"

#define NEAR_CLUST L"NEAR_CLUST"
#define CLEAR_BUFFER L"CLEAR_BUFFER"  
#define OFFER_BUFFER_CLEAR_VALUE L"OFFER_BUFFER_CLEAR_VALUE" 
#define LINEAR_DEPTH_DIST L"LINEAR_DEPTH_DIST"
#define CB_CAM_REIGSTER L"CB_CAM_REIGSTER"
#define USE_HEMISPHERE L"HEMISPHERE"

#define LIGHT_RANGE_OFFSET L"LIGHT_RANGE_OFFSET"

namespace JinEngine::Graphic
{
	namespace Private
	{
		//graphic shader
		static constexpr uint gLightBufferIndex = 0;
		static constexpr uint gCamCBIndex = gLightBufferIndex + 1;
		static constexpr uint gPassCBIndex = gCamCBIndex + 1;
		static constexpr int gSlotCount = gPassCBIndex + 1;

		//computeShader
		static constexpr uint cLightTypeCBIndex = 0;
		static constexpr uint cRtTextureArrayIndex = cLightTypeCBIndex + 1;
		static constexpr uint cOffsetBufferIndex = cRtTextureArrayIndex + 1;
		static constexpr uint cLinkedlistndex = cOffsetBufferIndex + 1;
		static constexpr uint cOutBufferIndex = cLinkedlistndex + 1;
		static constexpr int cSlotCount = cOutBufferIndex + 1;

		//deubg Shdaer 
		static constexpr uint dPassCBIndex = 0;
		static constexpr uint dDepthMapIndex = dPassCBIndex + 1;
		static constexpr uint dOffsetBufferIndex = dDepthMapIndex + 1;
		static constexpr uint dLinkedlistndex = dOffsetBufferIndex + 1;
		static constexpr int dSlotCount = dLinkedlistndex + 1;

		static constexpr uint staticMeshIndex = (uint)Core::J_MESHGEOMETRY_TYPE::STATIC;
		static constexpr uint litListCounterCount = 1;
		static constexpr bool useLinearDepth = false;
		static constexpr bool rectLightShapeIsHemisphere = false;
#if defined (_DEBUG) && defined(DEVELOP)
		static constexpr bool allowDebugging = true;
#else
		static constexpr bool allowDebugging = false;
#endif

		static uint CalLitClusterShaderIndex(const uint xIndex, const uint yIndex, const uint zIndex)
		{
			return xIndex +
				Constants::litClusterXVariation * yIndex +
				(Constants::litClusterXVariation + Constants::litClusterYVariation) * zIndex;
		}
		static void DecomposeClusterIndex(const uint index, _Out_ uint& x, _Out_ uint& y, _Out_ uint& z)
		{
			static constexpr uint yOffset = Constants::litClusterXVariation;
			static constexpr uint zOffset = Constants::litClusterXVariation + Constants::litClusterYVariation;

			uint restIndex = index;
			x = 0;
			y = 0;
			z = 0;
			if (restIndex >= zOffset)
			{
				z = restIndex / zOffset;
				restIndex -= (z * zOffset);
			}
			if (restIndex >= yOffset)
			{
				y = restIndex / yOffset;
				restIndex -= (y * yOffset);
			}
			x = restIndex;
		}
		static void StuffShaderMacro(_Inout_ JGraphicShaderInitData& initHelper, const JGraphicOption& option, const J_LIGHT_TYPE type)
		{
			switch (type)
			{
			case JinEngine::J_LIGHT_TYPE::POINT:
			{
				initHelper.macro[staticMeshIndex].push_back({ POINT_LIGHT_, L"0" });
				initHelper.macro[staticMeshIndex].push_back({ LIGHT_RANGE_OFFSET, std::to_wstring(option.clusterPointLightRangeOffset) });
				break;
			}
			case JinEngine::J_LIGHT_TYPE::SPOT:
			{
				initHelper.macro[staticMeshIndex].push_back({ SPOT_LIGHT_, L"1" });
				initHelper.macro[staticMeshIndex].push_back({ LIGHT_RANGE_OFFSET, std::to_wstring(option.clusterSpotLightRangeOffset) });
				break;
			}
			case JinEngine::J_LIGHT_TYPE::RECT:
			{
				initHelper.macro[staticMeshIndex].push_back({ RECT_LIGHT_, L"2" });
				initHelper.macro[staticMeshIndex].push_back({ LIGHT_RANGE_OFFSET, std::to_wstring(option.clusterRectLightRangeOffset) });
				if (rectLightShapeIsHemisphere)
					initHelper.macro->push_back({ USE_HEMISPHERE, L"1"});
				break;
			}
			default:
				break;
			}
			initHelper.macro->push_back({ CB_CAM_REIGSTER, L"b0" });
		}
		static void StuffShaderMacro(_Inout_ JGraphicShaderInitData& initHelper, const JGraphicOption& option)
		{
			switch (JinEngine::Graphic::Constants::litClusterXRange[option.clusterXIndex])
			{
			case JinEngine::Graphic::Constants::litClusterXRange[0]:
			case JinEngine::Graphic::Constants::litClusterXRange[1]:
			{
				initHelper.macro[staticMeshIndex].push_back({ CLUSTER_DIM_X, std::to_wstring(option.GetClusterXCount()) });
				break;
			}
			default:
				break;
			}
			switch (JinEngine::Graphic::Constants::litClusterYRange[option.clusterYIndex])
			{
			case JinEngine::Graphic::Constants::litClusterYRange[0]:
			case JinEngine::Graphic::Constants::litClusterYRange[1]:
			{
				initHelper.macro[staticMeshIndex].push_back({ CLUSTER_DIM_Y, std::to_wstring(option.GetClusterYCount()) });
				break;
			}
			default:
				break;
			}
			switch (JinEngine::Graphic::Constants::litClusterZRange[option.clusterZIndex])
			{
			case JinEngine::Graphic::Constants::litClusterZRange[0]:
			case JinEngine::Graphic::Constants::litClusterZRange[1]:
			case JinEngine::Graphic::Constants::litClusterZRange[2]:
			{
				initHelper.macro[staticMeshIndex].push_back({ CLUSTER_DIM_Z, std::to_wstring(option.GetClusterZCount()) });
				break;
			}
			default:
				break;
			}

			initHelper.macro[staticMeshIndex].push_back({ NEAR_CLUST, std::to_wstring(option.clusterNear) });
			if (useLinearDepth)
				initHelper.macro->push_back({ LINEAR_DEPTH_DIST, L"1" });
		}
		static void StuffShaderMacro(_Inout_ JComputeShaderInitData& initHelper, const JGraphicOption& option)
		{
			switch (JinEngine::Graphic::Constants::litClusterXRange[option.clusterXIndex])
			{
			case JinEngine::Graphic::Constants::litClusterXRange[0]:
			case JinEngine::Graphic::Constants::litClusterXRange[1]:
			{
				initHelper.macro.push_back({ CLUSTER_DIM_X, std::to_wstring(option.GetClusterXCount()) });
				break;
			}
			default:
				break;
			}
			switch (JinEngine::Graphic::Constants::litClusterYRange[option.clusterYIndex])
			{
			case JinEngine::Graphic::Constants::litClusterYRange[0]:
			case JinEngine::Graphic::Constants::litClusterYRange[1]:
			{
				initHelper.macro.push_back({ CLUSTER_DIM_Y, std::to_wstring(option.GetClusterYCount()) });
				break;
			}
			default:
				break;
			}
			switch (JinEngine::Graphic::Constants::litClusterZRange[option.clusterZIndex])
			{
			case JinEngine::Graphic::Constants::litClusterZRange[0]:
			case JinEngine::Graphic::Constants::litClusterZRange[1]:
			case JinEngine::Graphic::Constants::litClusterZRange[2]:
			{
				initHelper.macro.push_back({ CLUSTER_DIM_Z, std::to_wstring(option.GetClusterZCount()) });
				break;
			}
			default:
				break;
			}

			initHelper.macro.push_back({ NEAR_CLUST, std::to_wstring(option.clusterNear) });
			initHelper.dispatchInfo.threadDim = JVector3<uint>(Constants::litClusterXRange[0], Constants::litClusterYRange[0], 1);
			initHelper.dispatchInfo.groupDim = JVector3<uint>(std::ceil(option.GetClusterXCount() / Constants::litClusterXRange[0]), std::ceil(option.GetClusterYCount() / Constants::litClusterYRange[0]), 1);

			initHelper.macro.push_back({ THREAD_DIM_X, std::to_wstring(initHelper.dispatchInfo.threadDim.x) });
			initHelper.macro.push_back({ THREAD_DIM_Y, std::to_wstring(initHelper.dispatchInfo.threadDim.y) });
			initHelper.macro.push_back({ THREAD_DIM_Z, std::to_wstring(initHelper.dispatchInfo.threadDim.z) });
		}
		static void StuffClearShaderMacro(_Inout_ JComputeShaderInitData& initHelper, const JGraphicOption& option)
		{
			auto gpuInfo = Core::JHardwareInfo::GetGpuInfo();
			JVector3<uint> cluster(option.GetClusterXCount(), option.GetClusterYCount(), option.GetClusterZCount());

			initHelper.dispatchInfo.threadDim = JVector3<uint>(Constants::litClusterXRange[0], Constants::litClusterYRange[0], 1);
			initHelper.dispatchInfo.groupDim = JVector3<uint>(std::ceil(option.GetClusterXCount() / Constants::litClusterXRange[0]), std::ceil(option.GetClusterYCount() / Constants::litClusterYRange[0]), option.GetClusterZCount());
			initHelper.macro.push_back({ CLEAR_BUFFER, std::to_wstring(1) });
			initHelper.macro.push_back({ OFFER_BUFFER_CLEAR_VALUE, std::to_wstring((1 << 30) - 1) });
		}
		static JShaderType::CompileInfo GraphicDrawLightShapeVsCompileInfo()
		{
			return JShaderType::CompileInfo(L"LightShapeDrawVertex.hlsl", L"VS");
		}
		static JShaderType::CompileInfo GraphicDrawLightShapeGsCompileInfo()
		{
			return JShaderType::CompileInfo(L"LightShapeDrawGeo.hlsl", L"GS");
		}
		static JShaderType::CompileInfo GraphicDrawLightShapePsCompileInfo()
		{
			return JShaderType::CompileInfo(L"LightShapeDrawPixel.hlsl", L"PS");
		}
		static JShaderType::CompileInfo ComputeLightClusterCompileInfo()
		{
			return JShaderType::CompileInfo(L"LightClusterCompute.hlsl", L"CS");
		}
		static JShaderType::CompileInfo GraphicDrawDebugVsCompileInfo()
		{
			return JShaderType::CompileInfo(L"LightClusterDebug.hlsl", L"VS");
		}
		static JShaderType::CompileInfo GraphicDrawDebugPsCompileInfo()
		{
			return JShaderType::CompileInfo(L"LightClusterDebug.hlsl", L"PS");
		}

		static void StreamOutLightInfo(const JUserPtr<JCamera> cam, JDx12GraphicResourceManager* dx12Gm, const size_t sceneGuid)
		{
			if constexpr (!Private::allowDebugging)
				return;

			static int count = 0;
			if (!Develop::JDevelopDebug::IsActivate())
			{
				count = 1;
				return;
			}
			else if (count == 0)
				return;
			--count;

			const uint pointLitCount = JFrameUpdateData::GetAreaRegistedCount(J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT, sceneGuid);
			const uint spotLitCount = JFrameUpdateData::GetAreaRegistedCount(J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT, sceneGuid);
			const uint rectLitCount = JFrameUpdateData::GetAreaRegistedCount(J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT, sceneGuid);

			const uint pointLitOffset = JFrameUpdateData::GetAreaRegistedOffset(J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT, sceneGuid);
			const uint spotLitOffset = JFrameUpdateData::GetAreaRegistedOffset(J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT, sceneGuid);
			const uint rectLitOffset = JFrameUpdateData::GetAreaRegistedOffset(J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT, sceneGuid);

			auto gInterface = cam->GraphicResourceUserInterface();
			const uint rsDataIndex = gInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING, J_GRAPHIC_TASK_TYPE::LIGHT_CULLING);

			const int pointRtvVecIndex = gInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING, rsDataIndex);
			const int spotRtvVecIndex = gInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING, rsDataIndex + 1);
			const int rectRtvVecIndex = gInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING, rsDataIndex + 2);

			const int pointRtvHeapIndex = gInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING, J_GRAPHIC_BIND_TYPE::RTV, rsDataIndex);
			const int spotRtvHeapIndex = gInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING, J_GRAPHIC_BIND_TYPE::RTV, rsDataIndex + 1);
			const int rectRtvHeapIndex = gInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING, J_GRAPHIC_BIND_TYPE::RTV, rsDataIndex + 2);

			auto pointResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING, pointRtvVecIndex);
			auto spotResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING, spotRtvVecIndex);
			auto rectResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING, rectRtvVecIndex);

			Develop::JDevelopDebug::PushLog("Point count, offset " + std::to_string(pointLitCount) + " " + std::to_string(pointLitOffset));
			Develop::JDevelopDebug::PushLog("Spot count, offset " + std::to_string(spotLitCount) + " " + std::to_string(spotLitOffset));
			Develop::JDevelopDebug::PushLog("Rect count, offset " + std::to_string(rectLitCount) + " " + std::to_string(rectLitOffset));

			Develop::JDevelopDebug::PushLog("Point ArraySize " + std::to_string(pointResource->GetDesc().DepthOrArraySize));
			Develop::JDevelopDebug::PushLog("Spot ArraySize " + std::to_string(spotResource->GetDesc().DepthOrArraySize));
			Develop::JDevelopDebug::PushLog("Rect ArraySize " + std::to_string(rectResource->GetDesc().DepthOrArraySize));
			Develop::JDevelopDebug::Write();
		}
	}
	void JDx12LightCulling::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicBaseDataSet& baseDataSet)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(gM);
		BuildGraphicRootSignature(dx12Device->GetDevice());
		BuildComputeRootSignature(dx12Device->GetDevice());
		BuildDebugRootSignature(dx12Device->GetDevice());
		BuildGraphicPso(dx12Device->GetDevice(), baseDataSet);
		BuildComputePso(dx12Device->GetDevice(), baseDataSet);
		if constexpr (Private::allowDebugging)
			BuildDebugPso(dx12Device->GetDevice(), baseDataSet);
		BuildCounterClearBuffer(dx12Device);
		BuildResultBuffer(dx12Device, baseDataSet.info.upPLightCapacity + baseDataSet.info.upSLightCapacity + baseDataSet.info.upRLightCapacity);
		if constexpr (Private::allowDebugging)
			BuildDebugBuffer(dx12Device, baseDataSet);
		BuildRtResource(dx12Device, dx12Gm, baseDataSet);
		//LoadLightShape(dx12Device, dx12Gm);
		device->EndPublicCommand();
		device->FlushCommandQueue();
		device->StartPublicCommand();
		resultClearUploadBuffer = nullptr;
		counterClearUploadBuffer = nullptr;
	}
	void JDx12LightCulling::Clear()
	{
		counterClearBuffer = nullptr;
		counterClearUploadBuffer = nullptr;
		resultOutBuffer = nullptr;
		resultOutClearBuffer = nullptr;
		resultClearUploadBuffer = nullptr;
		for (uint i = 0; i < JLightType::GetLocalLightCount(); ++i)
			drawLightShader[i] = nullptr;
		computeLightClusterShader = nullptr;
		clearOffsetBufferShader = nullptr;
		drawDebugShader = nullptr;

		mGRootSignature = nullptr;
		mCRootSignature = nullptr;
		mDRootSignature = nullptr;
		if constexpr (Private::allowDebugging)
		{
			listDebugBuffer = nullptr;
			offsetDebugBuffer = nullptr;
		}
		for (uint i = 0; i < SIZE_OF_ARRAY(lightRt); ++i)
			JGraphicResourceInfo::Destroy(lightRt[i].Release());
		//Unuse
		/*
		JGraphicResourceInfo::Destroy(lowSphereVertex.Release());
		JGraphicResourceInfo::Destroy(lowSphereIndex.Release());
		JGraphicResourceInfo::Destroy(lowConeVertex.Release());
		JGraphicResourceInfo::Destroy(lowConeIndex.Release());
		*/
	}
	J_GRAPHIC_DEVICE_TYPE JDx12LightCulling::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	void JDx12LightCulling::NotifyNewClusterOption(JGraphicDevice* device, const JGraphicBaseDataSet& baseDataSet)
	{
		if (!IsSameDevice(device))
			return;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(device);
		BuildGraphicPso(dx12Device->GetDevice(), baseDataSet);
		BuildComputePso(dx12Device->GetDevice(), baseDataSet);
		if constexpr (Private::allowDebugging)
		{
			BuildDebugPso(dx12Device->GetDevice(), baseDataSet);
			BuildDebugBuffer(dx12Device, baseDataSet);
		}
	}
	void JDx12LightCulling::NotifyLocalLightCapacityChanged(JGraphicDevice* device, JGraphicResourceManager* gM, const size_t capacity)
	{
		bool startCommandThisFunc = false;
		device->StartPublicCommandSet(startCommandThisFunc);
		BuildResultBuffer(static_cast<JDx12GraphicDevice*>(device), capacity); 
		device->EndPublicCommandSet(startCommandThisFunc);
		if (!startCommandThisFunc)
			device->ReStartPublicCommandSet();
		resultClearUploadBuffer = nullptr;
	}
	void JDx12LightCulling::BindDrawResource(const JGraphicBindSet* bindSet)
	{
		if (!IsSameDevice(bindSet))
			return;

		const JDx12GraphicBindSet* dx12BindSet = static_cast<const JDx12GraphicBindSet*>(bindSet);
		ID3D12GraphicsCommandList* cmdList = dx12BindSet->cmdList;

		//cmdList->SetGraphicsRootSignature(mGRootSignature.Get());
	} 
	void JDx12LightCulling::BindDebugResource(const JGraphicBindSet* bindSet)
	{
		if (!IsSameDevice(bindSet))
			return;

		const JDx12GraphicBindSet* dx12BindSet = static_cast<const JDx12GraphicBindSet*>(bindSet);
		ID3D12GraphicsCommandList* cmdList = dx12BindSet->cmdList;

		cmdList->SetGraphicsRootSignature(mDRootSignature.Get());
	}
	void JDx12LightCulling::ExecuteLightClusterTask(const JGraphicLightCullingTaskSet* taskSet, const JDrawHelper& helper)
	{
		if (taskSet == nullptr || !IsSameDevice(taskSet) || !helper.allowLightCulling || !helper.option.allowLightCluster)
			return;

		const JDx12GraphicLightCullingTaskSet* dx12Set = static_cast<const JDx12GraphicLightCullingTaskSet*>(taskSet);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12Set->graphicResourceM);
		JDx12CullingManager* dx12Cm = static_cast<JDx12CullingManager*>(dx12Set->cullingManager);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(dx12Set->currFrame);
		ID3D12GraphicsCommandList* cmdlist = dx12Set->cmdList;

		DrawLight(cmdlist, dx12Gm, dx12Cm, dx12Frame, helper);
	}
	void JDx12LightCulling::ExecuteLightClusterDebug(const JGraphicLightCullingDebugDrawSet* drawSet, const JDrawHelper& helper)
	{
		if constexpr (!Private::allowDebugging)
			return;

		if (drawSet == nullptr || !IsSameDevice(drawSet) || !helper.allowLightCullingDebug || !helper.option.allowLightCluster)
			return;
		 
		const JDx12GraphicLightCullingDebugDrawSet* dx12Set = static_cast<const JDx12GraphicLightCullingDebugDrawSet*>(drawSet);
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(dx12Set->device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12Set->graphicResourceM);
		ID3D12GraphicsCommandList* cmdlist = dx12Set->cmdList;

		DrawLightClusterDebug(cmdlist, dx12Device, dx12Gm, helper);
	}
	void JDx12LightCulling::StreamOutDebugInfo(const std::wstring& path, const JGraphicBaseDataSet& baseDataSet)
	{
		if constexpr (!Private::allowDebugging)
			return;

		static int count = 0;
		if (!Develop::JDevelopDebug::IsActivate())
		{
			count = 1;
			return;
		}
		else if (count == 0)
			return;
		--count;

		auto offsetPtr = offsetDebugBuffer->GetCpuPointer();
		auto listPtr = listDebugBuffer->GetCpuPointer();

		const uint offsetCount = offsetDebugBuffer->GetElementCount();
		const uint listCount = listDebugBuffer->GetElementCount();

		const uint clusterX = baseDataSet.option.GetClusterXCount();
		const uint clusterY = baseDataSet.option.GetClusterYCount();
		const uint clusterZ = baseDataSet.option.GetClusterZCount();

		Develop::JDevelopDebug::CreatePublicLogHandler("LightCulling");
		Develop::JDevelopDebug::PushDefaultLogHandler("LightCulling");
		Develop::JDevelopDebug::PushLog("Offset");

		uint clusterXY[12][24];
		int maxlink = 0;

		for (uint j = 0; j < clusterY; ++j)
		{
			for (uint k = 0; k < clusterX; ++k)
				clusterXY[j][k] = 0;
		}

		for (uint i = 0; i < clusterZ; ++i)
		{
			for (uint j = 0; j < clusterY; ++j)
			{
				for (uint k = 0; k < clusterX; ++k)
				{
					std::string contents = "X: " + std::to_string(k);
					contents += " Y: " + std::to_string(j);
					contents += " Z: " + std::to_string(i);

					int index = k + j * clusterX + i * clusterX * clusterY;
					int count = 0;
					uint nodeIndex = offsetPtr[index];
					if (nodeIndex != 0x3FFFFFFF)
						++clusterXY[j][k];

					Develop::JDevelopDebug::PushLog(contents);
					if (nodeIndex > maxlink && nodeIndex != 0x3FFFFFFF)
						maxlink = nodeIndex;
					while (nodeIndex != 0x3FFFFFFF)
					{
						uint* nodePtr = static_cast<uint*>(static_cast<void*>(&listPtr[nodeIndex]));
						uint id = nodePtr[0];
						uint link = nodePtr[1];
						uint lightID = (id & 0xFFFFFF);
						uint lightType = (id >> 24);

						std::string contents = "";
						contents += "lightType: " + std::to_string(lightType);
						contents += " lightID: " + std::to_string(lightID);
						contents += " link: " + std::to_string(link);
						Develop::JDevelopDebug::PushLog(contents);

						nodeIndex = link;
						if (link > maxlink && link != 0x3FFFFFFF)
							maxlink = link;
						++count;
					}
					Develop::JDevelopDebug::PushLog("maxlink: " + std::to_string(maxlink));
				}
			}
		}

		uint sum = 0;
		uint hasSum = 0;
		for (uint j = 0; j < clusterY; ++j)
		{
			for (uint k = 0; k < clusterX; ++k)
			{
				std::string contents = "X: " + std::to_string(k);
				contents += " Y: " + std::to_string(j);

				contents += +" value: " + std::to_string(clusterXY[j][k]);
				Develop::JDevelopDebug::PushLog(contents);
				sum += clusterXY[j][k];
				hasSum += clusterXY[j][k] > 0;
			}
		}
		Develop::JDevelopDebug::PushLog("MaxLink: " + std::to_string(maxlink));
		Develop::JDevelopDebug::PushLog("Sum: " + std::to_string(sum) + " StuffSum: " + std::to_string(hasSum) + " Rate: " + std::to_string((hasSum / float(clusterX * clusterY))));
		Develop::JDevelopDebug::Write();
		Develop::JDevelopDebug::PopDefaultLogHandler("LightCulling");
		Develop::JDevelopDebug::DestroyPublicLogHandler("LightCulling");
	}
	void JDx12LightCulling::RecompileShader(const JGraphicShaderCompileSet& dataSet)
	{
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(dataSet.device);
		BuildGraphicPso(dx12Device->GetDevice(), dataSet.base);
		BuildComputePso(dx12Device->GetDevice(), dataSet.base);
		if constexpr (Private::allowDebugging)
			BuildDebugPso(dx12Device->GetDevice(), dataSet.base);
	}
	void JDx12LightCulling::DrawLight(ID3D12GraphicsCommandList* cmdList,
		JDx12GraphicResourceManager* dx12Gm,
		JDx12CullingManager* dx12Cm,
		JDx12FrameResource* dx12Frame,
		const JDrawHelper& helper)
	{
		JUserPtr<JMeshGeometry> cone = _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::LOW_CONE);
		JUserPtr<JMeshGeometry> sphere = _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::LOW_SPHERE);
		JUserPtr<JMeshGeometry> hemiSphere;
		if (Private::rectLightShapeIsHemisphere)
			hemiSphere = _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::LOW_HEMI_SPHERE);
		else
			hemiSphere = _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::LOW_SPHERE);
		const size_t sceneGuid = helper.scene->GetGuid();
		const uint pointLitCount = JFrameUpdateData::GetAreaRegistedCount(J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT, sceneGuid);
		const uint spotLitCount = JFrameUpdateData::GetAreaRegistedCount(J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT, sceneGuid);
		const uint rectLitCount = JFrameUpdateData::GetAreaRegistedCount(J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT, sceneGuid);
		 
		const uint pointLitOffset = JFrameUpdateData::GetAreaRegistedOffset(J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT, sceneGuid);
		const uint spotLitOffset = JFrameUpdateData::GetAreaRegistedOffset(J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT, sceneGuid);
		const uint rectLitOffset = JFrameUpdateData::GetAreaRegistedOffset(J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT, sceneGuid);

		const uint pointLitLocalOffset = 0;
		const uint spotLitLocalOffset = pointLitCount + pointLitOffset;
		const uint rectLitLocalOffset = spotLitCount + spotLitOffset;

		auto gInterface = helper.cam->GraphicResourceUserInterface();
		auto cInterface = helper.cam->CullingUserInterface();

		const uint rsDataIndex = gInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING, J_GRAPHIC_TASK_TYPE::LIGHT_CULLING);
		const uint offsetBufferDataIndex = gInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET, J_GRAPHIC_TASK_TYPE::LIGHT_CULLING);
		const uint linkedDataIndex = gInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST, J_GRAPHIC_TASK_TYPE::LIGHT_CULLING);
		 
		const int offsetVecIndex = gInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET, offsetBufferDataIndex);
		const int linkedVecIndex = gInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST, linkedDataIndex);
		const int readBackVecIndex = cInterface.GetArrayIndex(J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::LIGHT);
		 
		const int offsetUavHeapIndex = gInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET, J_GRAPHIC_BIND_TYPE::UAV, offsetBufferDataIndex);
		const int linkedUavHeapIndex = gInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST, J_GRAPHIC_BIND_TYPE::UAV, linkedDataIndex);

		auto pointResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING, lightRt[0]->GetArrayIndex());
		auto spotResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING, lightRt[1]->GetArrayIndex());
		auto rectResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING, lightRt[2]->GetArrayIndex());

		auto offsetResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET, offsetVecIndex);
		auto linkedResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST, linkedVecIndex);
		auto linkedCounterResource = dx12Gm->GetOptionResource(J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST, J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNTER_BUFFER, linkedVecIndex);
		auto readBackResource = dx12Cm->GetResource(J_CULLING_TYPE::FRUSTUM, readBackVecIndex);
		auto outResource = resultOutBuffer->GetResource();

		auto pointRtv = dx12Gm->GetCpuRtvDescriptorHandle(lightRt[0]->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::RTV));
		auto spotRtv = dx12Gm->GetCpuRtvDescriptorHandle(lightRt[1]->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::RTV));
		auto rectRtv = dx12Gm->GetCpuRtvDescriptorHandle(lightRt[2]->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::RTV));

		auto pointGpuSrv = dx12Gm->GetGpuSrvDescriptorHandle(lightRt[0]->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV));
		auto spotGpuSrv = dx12Gm->GetGpuSrvDescriptorHandle(lightRt[1]->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV));
		auto rectGpuSrv = dx12Gm->GetGpuSrvDescriptorHandle(lightRt[2]->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV));
		auto offsetGpuUav = dx12Gm->GetGpuSrvDescriptorHandle(offsetUavHeapIndex);
		auto linkedGpuUav = dx12Gm->GetGpuSrvDescriptorHandle(linkedUavHeapIndex);
 
		JD3DUtility::ResourceTransition(cmdList, pointResource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
		JD3DUtility::ResourceTransition(cmdList, spotResource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
		JD3DUtility::ResourceTransition(cmdList, rectResource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
		JD3DUtility::ResourceTransition(cmdList, offsetResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		JD3DUtility::ResourceTransition(cmdList, linkedResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		//Clear resource
		cmdList->CopyResource(outResource, resultOutClearBuffer->GetResource());
		JD3DUtility::ResourceTransition(cmdList, outResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		JD3DUtility::ResourceTransition(cmdList, linkedCounterResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		cmdList->CopyResource(linkedCounterResource, counterClearBuffer->GetResource());
		JD3DUtility::ResourceTransition(cmdList, linkedCounterResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		cmdList->SetPipelineState(clearOffsetBufferShader->pso.Get());
		cmdList->SetComputeRootSignature(mCRootSignature.Get());
		cmdList->SetComputeRootDescriptorTable(Private::cOffsetBufferIndex, offsetGpuUav);
		cmdList->SetComputeRootDescriptorTable(Private::cLinkedlistndex, linkedGpuUav);
		cmdList->SetComputeRootUnorderedAccessView(Private::cOutBufferIndex, outResource->GetGPUVirtualAddress());

		auto clearDispatchInfo = clearOffsetBufferShader->dispatchInfo.groupDim;
		cmdList->Dispatch(clearDispatchInfo.x, clearDispatchInfo.y, clearDispatchInfo.z);

		const uint clusterX = helper.option.GetClusterXCount();
		const uint clusterY = helper.option.GetClusterYCount();

		const uint pointLitType = 0;
		const uint spotLitType = 1;
		const uint rectLitType = 2;

		D3D12_VIEWPORT viewPort{ 0.0f, 0.0f, (float)clusterX, (float)clusterY, 0.0f, 1.0f };
		D3D12_RECT rect{ 0, 0, clusterX, clusterY, };

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW indexBufferView;

		auto dispatchInfo = computeLightClusterShader->dispatchInfo.groupDim;
		//Draw light shape and compute node
		//point
		if (pointLitCount > 0)
		{
			cmdList->SetPipelineState(drawLightShader[0]->pso.Get());
			cmdList->SetGraphicsRootSignature(mGRootSignature.Get());

			dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT)->SetGraphicsRootShaderResourceView(cmdList, Private::gLightBufferIndex);
			dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA)->SetGraphicCBBufferView(cmdList, Private::gCamCBIndex, helper.GetCamFrameIndex());
			cmdList->SetGraphicsRoot32BitConstants(Private::gPassCBIndex, 1, &pointLitOffset, 0);

			cmdList->ClearRenderTargetView(pointRtv, Constants::WhileColor().value, 0, nullptr);
			cmdList->OMSetRenderTargets(1, &pointRtv, true, nullptr);
			cmdList->RSSetViewports(1, &viewPort);
			cmdList->RSSetScissorRects(1, &rect);

			//vertexBufferView.BufferLocation = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::VERTEX, lowSphereVertex->GetArrayIndex())->GetGPUVirtualAddress();
			//vertexBufferView.StrideInBytes = sizeof(Core::J1BytePosVertex);
			//vertexBufferView.SizeInBytes = sizeof(Core::J1BytePosVertex) * lowSphereVertexCount;

			//indexBufferView.BufferLocation = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::INDEX, lowSphereIndex->GetArrayIndex())->GetGPUVirtualAddress();
			//indexBufferView.SizeInBytes = sizeof(uint16) * lowSphereIndexCount;
			//indexBufferView.Format = DXGI_FORMAT_R16_UINT;

			vertexBufferView = dx12Gm->VertexBufferView(sphere);
			indexBufferView = dx12Gm->IndexBufferView(sphere);

			cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			cmdList->IASetVertexBuffers(0, 1, &vertexBufferView);
			cmdList->IASetIndexBuffer(&indexBufferView);
			//cmdList->DrawIndexedInstanced(lowSphereIndexCount, pointLitCount, 0, 0, 0);
			cmdList->DrawIndexedInstanced(sphere->GetTotalIndexCount(), pointLitCount, sphere->GetSubmeshStartIndexLocation(0), sphere->GetSubmeshBaseVertexLocation(0), 0);

			cmdList->SetPipelineState(computeLightClusterShader->pso.Get());
			cmdList->SetComputeRootSignature(mCRootSignature.Get());
			cmdList->SetComputeRootDescriptorTable(Private::cOffsetBufferIndex, offsetGpuUav);
			cmdList->SetComputeRootDescriptorTable(Private::cLinkedlistndex, linkedGpuUav);
			cmdList->SetComputeRootUnorderedAccessView(Private::cOutBufferIndex, outResource->GetGPUVirtualAddress());

			cmdList->SetComputeRoot32BitConstants(Private::cLightTypeCBIndex, 1, &pointLitType, 0);
			cmdList->SetComputeRoot32BitConstants(Private::cLightTypeCBIndex, 1, &pointLitLocalOffset, 1);
			cmdList->SetComputeRootDescriptorTable(Private::cRtTextureArrayIndex, pointGpuSrv);
			cmdList->Dispatch(dispatchInfo.x, dispatchInfo.y, pointLitCount);
		}
		if (spotLitCount > 0)
		{
			cmdList->SetPipelineState(drawLightShader[1]->pso.Get());
			cmdList->SetGraphicsRootSignature(mGRootSignature.Get());

			dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT)->SetGraphicsRootShaderResourceView(cmdList, Private::gLightBufferIndex);
			dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA)->SetGraphicCBBufferView(cmdList, Private::gCamCBIndex, helper.GetCamFrameIndex());
			cmdList->SetGraphicsRoot32BitConstants(Private::gPassCBIndex, 1, &spotLitOffset, 0);

			cmdList->ClearRenderTargetView(spotRtv, Constants::WhileColor().value, 0, nullptr);
			cmdList->OMSetRenderTargets(1, &spotRtv, true, nullptr);
			cmdList->RSSetViewports(1, &viewPort);
			cmdList->RSSetScissorRects(1, &rect);

			//vertexBufferView.BufferLocation = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::VERTEX, lowConeVertex->GetArrayIndex())->GetGPUVirtualAddress();
			//vertexBufferView.StrideInBytes = sizeof(Core::J1BytePosVertex);
			//vertexBufferView.SizeInBytes = sizeof(Core::J1BytePosVertex) * lowConeVertexCount;

			//indexBufferView.BufferLocation = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::INDEX, lowConeIndex->GetArrayIndex())->GetGPUVirtualAddress();
			//indexBufferView.SizeInBytes = sizeof(uint16) * lowConeIndexCount;
			//indexBufferView.Format = DXGI_FORMAT_R16_UINT;
			vertexBufferView = dx12Gm->VertexBufferView(cone);
			indexBufferView = dx12Gm->IndexBufferView(cone);

			cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			cmdList->IASetVertexBuffers(0, 1, &vertexBufferView);
			cmdList->IASetIndexBuffer(&indexBufferView);
			cmdList->DrawIndexedInstanced(cone->GetTotalIndexCount(), spotLitCount, cone->GetSubmeshStartIndexLocation(0), cone->GetSubmeshBaseVertexLocation(0), 0);
			//cmdList->DrawIndexedInstanced(lowConeIndexCount, spotLitCount, 0, 0, 0);

			cmdList->SetPipelineState(computeLightClusterShader->pso.Get());
			cmdList->SetComputeRootSignature(mCRootSignature.Get());
			cmdList->SetComputeRootDescriptorTable(Private::cOffsetBufferIndex, offsetGpuUav);
			cmdList->SetComputeRootDescriptorTable(Private::cLinkedlistndex, linkedGpuUav);
			cmdList->SetComputeRootUnorderedAccessView(Private::cOutBufferIndex, outResource->GetGPUVirtualAddress());

			cmdList->SetComputeRoot32BitConstants(Private::cLightTypeCBIndex, 1, &spotLitType, 0);
			cmdList->SetComputeRoot32BitConstants(Private::cLightTypeCBIndex, 1, &spotLitLocalOffset, 1);
			cmdList->SetComputeRootDescriptorTable(Private::cRtTextureArrayIndex, spotGpuSrv);
			cmdList->Dispatch(dispatchInfo.x, dispatchInfo.y, spotLitCount);
		}
		if (rectLitCount > 0)
		{
			cmdList->SetPipelineState(drawLightShader[2]->pso.Get());
			cmdList->SetGraphicsRootSignature(mGRootSignature.Get());

			dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT)->SetGraphicsRootShaderResourceView(cmdList, Private::gLightBufferIndex);
			dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA)->SetGraphicCBBufferView(cmdList, Private::gCamCBIndex, helper.GetCamFrameIndex());
			cmdList->SetGraphicsRoot32BitConstants(Private::gPassCBIndex, 1, &rectLitOffset, 0);

			cmdList->ClearRenderTargetView(rectRtv, Constants::WhileColor().value, 0, nullptr);
			cmdList->OMSetRenderTargets(1, &rectRtv, true, nullptr);
			cmdList->RSSetViewports(1, &viewPort);
			cmdList->RSSetScissorRects(1, &rect);

			vertexBufferView = dx12Gm->VertexBufferView(hemiSphere);
			indexBufferView = dx12Gm->IndexBufferView(hemiSphere);

			cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			cmdList->IASetVertexBuffers(0, 1, &vertexBufferView);
			cmdList->IASetIndexBuffer(&indexBufferView);
			cmdList->DrawIndexedInstanced(hemiSphere->GetTotalIndexCount(), rectLitCount, hemiSphere->GetSubmeshStartIndexLocation(0), cone->GetSubmeshBaseVertexLocation(0), 0);

			cmdList->SetPipelineState(computeLightClusterShader->pso.Get());
			cmdList->SetComputeRootSignature(mCRootSignature.Get());
			cmdList->SetComputeRootDescriptorTable(Private::cOffsetBufferIndex, offsetGpuUav);
			cmdList->SetComputeRootDescriptorTable(Private::cLinkedlistndex, linkedGpuUav);
			cmdList->SetComputeRootUnorderedAccessView(Private::cOutBufferIndex, outResource->GetGPUVirtualAddress());

			cmdList->SetComputeRoot32BitConstants(Private::cLightTypeCBIndex, 1, &rectLitType, 0);
			cmdList->SetComputeRoot32BitConstants(Private::cLightTypeCBIndex, 1, &rectLitLocalOffset, 1);
			cmdList->SetComputeRootDescriptorTable(Private::cRtTextureArrayIndex, rectGpuSrv);
			cmdList->Dispatch(dispatchInfo.x, dispatchInfo.y, rectLitCount);
		}

		JD3DUtility::ResourceTransition(cmdList, readBackResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		JD3DUtility::ResourceTransition(cmdList, outResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
		cmdList->CopyResource(readBackResource, outResource);
		JD3DUtility::ResourceTransition(cmdList, outResource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
		JD3DUtility::ResourceTransition(cmdList, readBackResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);

		if constexpr (Private::allowDebugging)
		{
			if (!helper.cam->GetOwner()->IsEditorObject())
			{
				JD3DUtility::ResourceTransition(cmdList, offsetDebugBuffer->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
				JD3DUtility::ResourceTransition(cmdList, listDebugBuffer->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
				JD3DUtility::ResourceTransition(cmdList, offsetResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
				JD3DUtility::ResourceTransition(cmdList, linkedResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
				cmdList->CopyResource(offsetDebugBuffer->GetResource(), offsetResource);
				cmdList->CopyResource(listDebugBuffer->GetResource(), linkedResource);
				JD3DUtility::ResourceTransition(cmdList, offsetResource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);
				JD3DUtility::ResourceTransition(cmdList, linkedResource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);
				JD3DUtility::ResourceTransition(cmdList, listDebugBuffer->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
				JD3DUtility::ResourceTransition(cmdList, offsetDebugBuffer->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
			}
			else
			{
				JD3DUtility::ResourceTransition(cmdList, linkedCounterResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
				JD3DUtility::ResourceTransition(cmdList, linkedResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
				JD3DUtility::ResourceTransition(cmdList, offsetResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
			}
		}
		else
		{
			JD3DUtility::ResourceTransition(cmdList, linkedCounterResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
			JD3DUtility::ResourceTransition(cmdList, linkedResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
			JD3DUtility::ResourceTransition(cmdList, offsetResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
		}

		JD3DUtility::ResourceTransition(cmdList, rectResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
		JD3DUtility::ResourceTransition(cmdList, spotResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
		JD3DUtility::ResourceTransition(cmdList, pointResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);

		auto litCullingInfo = dx12Cm->GetCullingInfo(J_CULLING_TYPE::FRUSTUM, readBackVecIndex);

		JCullingUpdatedInfo updateInfo;
		updateInfo.updatedStartIndex = 0;
		updateInfo.updatedCount = pointLitCount + spotLitCount + rectLitCount;
		litCullingInfo->SetUpdatedInfo(updateInfo, helper.info.currFrameResourceIndex);
	} 
	void JDx12LightCulling::DrawLightClusterDebug(ID3D12GraphicsCommandList* cmdList,
		JDx12GraphicDevice* dx12Device,
		JDx12GraphicResourceManager* dx12Gm,
		const JDrawHelper& helper)
	{
		JUserPtr<JMeshGeometry> quad = _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::FULL_SCREEN_QUAD);

		auto gInterface = helper.cam->GraphicResourceUserInterface();
		const uint rtDataIndex = gInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::LIGHT_LIST_DRAW);
		const uint dsDataIndex = gInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);

		const uint offsetBufferDataIndex = gInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET, J_GRAPHIC_TASK_TYPE::LIGHT_CULLING);
		const uint linkedDataIndex = gInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST, J_GRAPHIC_TASK_TYPE::LIGHT_CULLING);

		const int rtvArrayIndex = gInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, rtDataIndex);
		const int rtvHeapIndex = gInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_BIND_TYPE::RTV, rtDataIndex);
		const int dsSrvHeapIndex = gInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_BIND_TYPE::SRV, dsDataIndex);
		const int offsetSrvHeapIndex = gInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET, J_GRAPHIC_BIND_TYPE::SRV, offsetBufferDataIndex);
		const int linkedSrvHeapIndex = gInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST, J_GRAPHIC_BIND_TYPE::SRV, linkedDataIndex);

		auto rtResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, rtvArrayIndex);
		auto rtDesc = rtResource->GetDesc();

		auto rtvHandle = dx12Gm->GetCpuRtvDescriptorHandle(rtvHeapIndex);
		auto dsSrvHandle = dx12Gm->GetGpuSrvDescriptorHandle(dsSrvHeapIndex);
		auto offsetSrv = dx12Gm->GetGpuSrvDescriptorHandle(offsetSrvHeapIndex);
		auto linkedSrv = dx12Gm->GetGpuSrvDescriptorHandle(linkedSrvHeapIndex);

		JD3DUtility::ResourceTransition(cmdList, rtResource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
		cmdList->SetPipelineState(drawDebugShader->pso.Get());

		JVector2F renderTargetSize = JVector2F(rtDesc.Width, rtDesc.Height);
		float camNearZ = helper.cam->GetNear();
		float camFarZ = helper.cam->GetFar();
		//uint log2Tile = (uint32)std::log2<uint32>(renderTargetSize.x / helper.option.GetClusterXCount());
		JVector4F passPack = JVector4F(camNearZ, camFarZ, renderTargetSize.x, renderTargetSize.y);

		D3D12_VIEWPORT viewPort;
		D3D12_RECT rect;
		dx12Device->CalViewportAndRect(renderTargetSize, true, viewPort, rect);
		cmdList->RSSetViewports(1, &viewPort);
		cmdList->RSSetScissorRects(1, &rect);

		cmdList->SetGraphicsRoot32BitConstants(Private::dPassCBIndex, 4, &passPack, 0);
		cmdList->SetGraphicsRootDescriptorTable(Private::dDepthMapIndex, dsSrvHandle);
		cmdList->SetGraphicsRootDescriptorTable(Private::dOffsetBufferIndex, offsetSrv);
		cmdList->SetGraphicsRootDescriptorTable(Private::dLinkedlistndex, linkedSrv);

		cmdList->ClearRenderTargetView(rtvHandle, dx12Gm->GetBackBufferClearColor(), 0, nullptr);
		cmdList->OMSetRenderTargets(1, &rtvHandle, true, nullptr);

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView = dx12Gm->VertexBufferView(quad);
		D3D12_INDEX_BUFFER_VIEW indexBufferView = dx12Gm->IndexBufferView(quad);
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmdList->IASetVertexBuffers(0, 1, &vertexBufferView);
		cmdList->IASetIndexBuffer(&indexBufferView);

		cmdList->DrawIndexedInstanced(quad->GetTotalIndexCount(), 1, quad->GetSubmeshStartIndexLocation(0), quad->GetSubmeshBaseVertexLocation(0), 0);
		JD3DUtility::ResourceTransition(cmdList, rtResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	}
	void JDx12LightCulling::BuildGraphicRootSignature(ID3D12Device* device)
	{
		CD3DX12_ROOT_PARAMETER slotRootParameter[Private::gSlotCount];
		slotRootParameter[Private::gLightBufferIndex].InitAsShaderResourceView(0, 0);
		slotRootParameter[Private::gCamCBIndex].InitAsConstantBufferView(0);
		slotRootParameter[Private::gPassCBIndex].InitAsConstants(1, 1);

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(Private::gSlotCount, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		// create a root signature with a single slot which points to a descriptor length consisting of a single constant buffer
		Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		ThrowIfFailedHr(hr);
		ThrowIfFailedHr(device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(mGRootSignature.GetAddressOf())));

		mGRootSignature->SetName(L"LightCulling Draw RootSignature");
	}
	void JDx12LightCulling::BuildComputeRootSignature(ID3D12Device* device)
	{
		CD3DX12_DESCRIPTOR_RANGE rtTextureArrayTable;
		rtTextureArrayTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_DESCRIPTOR_RANGE offsetBufferTable;
		offsetBufferTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		CD3DX12_DESCRIPTOR_RANGE linkedListBuffertable;
		linkedListBuffertable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);

		CD3DX12_ROOT_PARAMETER slotRootParameter[Private::cSlotCount];
		slotRootParameter[Private::cLightTypeCBIndex].InitAsConstants(4, 0);
		slotRootParameter[Private::cRtTextureArrayIndex].InitAsDescriptorTable(1, &rtTextureArrayTable, D3D12_SHADER_VISIBILITY_ALL);
		slotRootParameter[Private::cOffsetBufferIndex].InitAsDescriptorTable(1, &offsetBufferTable, D3D12_SHADER_VISIBILITY_ALL);
		slotRootParameter[Private::cLinkedlistndex].InitAsDescriptorTable(1, &linkedListBuffertable, D3D12_SHADER_VISIBILITY_ALL);
		slotRootParameter[Private::cOutBufferIndex].InitAsUnorderedAccessView(2);

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(Private::cSlotCount, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		// create a root signature with a single slot which points to a descriptor length consisting of a single constant buffer
		Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		ThrowIfFailedHr(hr);
		ThrowIfFailedHr(device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(mCRootSignature.GetAddressOf())));

		mCRootSignature->SetName(L"LightCulling Compute RootSignature");
	}
	void JDx12LightCulling::BuildDebugRootSignature(ID3D12Device* device)
	{
		CD3DX12_DESCRIPTOR_RANGE depthMapTable;
		depthMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_DESCRIPTOR_RANGE offsetBufferTable;
		offsetBufferTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

		CD3DX12_DESCRIPTOR_RANGE linkedListBuffertable;
		linkedListBuffertable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);

		CD3DX12_ROOT_PARAMETER slotRootParameter[Private::dSlotCount];
		slotRootParameter[Private::dPassCBIndex].InitAsConstants(4, 0);
		slotRootParameter[Private::dDepthMapIndex].InitAsDescriptorTable(1, &depthMapTable, D3D12_SHADER_VISIBILITY_ALL);
		slotRootParameter[Private::dOffsetBufferIndex].InitAsDescriptorTable(1, &offsetBufferTable, D3D12_SHADER_VISIBILITY_ALL);
		slotRootParameter[Private::dLinkedlistndex].InitAsDescriptorTable(1, &linkedListBuffertable, D3D12_SHADER_VISIBILITY_ALL);

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(Private::dSlotCount, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		// create a root signature with a single slot which points to a descriptor length consisting of a single constant buffer
		Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		ThrowIfFailedHr(hr);
		ThrowIfFailedHr(device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(mDRootSignature.GetAddressOf())));

		mDRootSignature->SetName(L"LightCulling Debug RootSignature");
	}
	void JDx12LightCulling::BuildGraphicPso(ID3D12Device* device, const JGraphicBaseDataSet& baseDataSet)
	{
		for (uint i = 0; i < JLightType::GetLocalLightCount(); ++i)
		{
			const J_LIGHT_TYPE lType = (J_LIGHT_TYPE)(i + JLightType::GetLocalLightOffset());
			drawLightShader[i] = std::make_unique<JDx12GraphicShaderDataHolder>();

			JDx12GraphicShaderDataHolder* drawHolder = drawLightShader[i].get();
			JShaderType::CompileInfo vsCompileInfo = Private::GraphicDrawLightShapeVsCompileInfo();
			JShaderType::CompileInfo gsCompileInfo = Private::GraphicDrawLightShapeGsCompileInfo();
			JShaderType::CompileInfo psCompileInfo = Private::GraphicDrawLightShapePsCompileInfo();

			JGraphicShaderInitData initHelper;
			Private::StuffShaderMacro(initHelper, baseDataSet.option, lType);
			Private::StuffShaderMacro(initHelper, baseDataSet.option);

			drawHolder->vs = JDxShaderDataUtil::CompileShader(vsCompileInfo.filePath, initHelper.macro[Private::staticMeshIndex], vsCompileInfo.functionName, L"vs_6_0");
			drawHolder->gs = JDxShaderDataUtil::CompileShader(gsCompileInfo.filePath, initHelper.macro[Private::staticMeshIndex], gsCompileInfo.functionName, L"gs_6_0");
			drawHolder->ps = JDxShaderDataUtil::CompileShader(psCompileInfo.filePath, initHelper.macro[Private::staticMeshIndex], psCompileInfo.functionName, L"ps_6_0");
			drawHolder->inputLayout =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};
			/*
			drawHolder->inputLayout =
			{
				{ "POSITION", 0, DXGI_FORMAT_R8G8B8A8_SNORM, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};
			*/

			D3D12_GRAPHICS_PIPELINE_STATE_DESC darwShaderPso;
			ZeroMemory(&darwShaderPso, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
			darwShaderPso.InputLayout = { drawHolder->inputLayout.data(), (uint)drawHolder->inputLayout.size() };
			darwShaderPso.pRootSignature = mGRootSignature.Get();
			darwShaderPso.VS =
			{
				reinterpret_cast<BYTE*>(drawHolder->vs->GetBufferPointer()),
				drawHolder->vs->GetBufferSize()
			};
			darwShaderPso.GS =
			{
				reinterpret_cast<BYTE*>(drawHolder->gs->GetBufferPointer()),
				drawHolder->gs->GetBufferSize()
			};
			darwShaderPso.PS =
			{
				reinterpret_cast<BYTE*>(drawHolder->ps->GetBufferPointer()),
				drawHolder->ps->GetBufferSize()
			};

			D3D12_RASTERIZER_DESC rasterDesc;
			rasterDesc.AntialiasedLineEnable = FALSE;
			rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON;
			rasterDesc.CullMode = D3D12_CULL_MODE_NONE;
			rasterDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
			rasterDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
			rasterDesc.DepthClipEnable = TRUE;
			rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
			rasterDesc.ForcedSampleCount = 0;
			rasterDesc.FrontCounterClockwise = FALSE;
			rasterDesc.MultisampleEnable = FALSE;
			rasterDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;

			darwShaderPso.RasterizerState = rasterDesc;
			darwShaderPso.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			darwShaderPso.BlendState.RenderTarget[0].BlendEnable = TRUE;
			darwShaderPso.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_MIN;
			darwShaderPso.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
			darwShaderPso.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
			darwShaderPso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			darwShaderPso.DepthStencilState.DepthEnable = FALSE;
			darwShaderPso.DepthStencilState.StencilEnable = FALSE;
			darwShaderPso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
			darwShaderPso.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			darwShaderPso.SampleMask = UINT_MAX;
			darwShaderPso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			darwShaderPso.NumRenderTargets = 1;
			darwShaderPso.RTVFormats[0] = DXGI_FORMAT_R8G8_UNORM;
			darwShaderPso.SampleDesc.Count = 1;
			ThrowIfFailedG(device->CreateGraphicsPipelineState(&darwShaderPso, IID_PPV_ARGS(drawHolder->pso.GetAddressOf())));
		}
	}
	void JDx12LightCulling::BuildComputePso(ID3D12Device* device, const JGraphicBaseDataSet& baseDataSet)
	{
		computeLightClusterShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		clearOffsetBufferShader = std::make_unique<JDx12ComputeShaderDataHolder>();

		JDx12ComputeShaderDataHolder* computeHolder = computeLightClusterShader.get();
		JShaderType::CompileInfo csCompileInfo = Private::ComputeLightClusterCompileInfo();

		JComputeShaderInitData initHelper;
		Private::StuffShaderMacro(initHelper, baseDataSet.option);
		computeHolder->cs = JDxShaderDataUtil::CompileShader(csCompileInfo.filePath, initHelper.macro, csCompileInfo.functionName, L"cs_6_0");
		computeHolder->dispatchInfo = initHelper.dispatchInfo;

		D3D12_COMPUTE_PIPELINE_STATE_DESC computeShaderPso;
		ZeroMemory(&computeShaderPso, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
		computeShaderPso.pRootSignature = mCRootSignature.Get();
		computeShaderPso.CS =
		{
			reinterpret_cast<BYTE*>(computeHolder->cs->GetBufferPointer()),
			computeHolder->cs->GetBufferSize()
		};
		computeShaderPso.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		ThrowIfFailedG(device->CreateComputePipelineState(&computeShaderPso, IID_PPV_ARGS(computeHolder->pso.GetAddressOf())));

		JDx12ComputeShaderDataHolder* clearHolder = clearOffsetBufferShader.get();
		Private::StuffClearShaderMacro(initHelper, baseDataSet.option);
		clearHolder->cs = JDxShaderDataUtil::CompileShader(csCompileInfo.filePath, initHelper.macro, csCompileInfo.functionName, L"cs_6_0");
		clearHolder->dispatchInfo = initHelper.dispatchInfo;

		D3D12_COMPUTE_PIPELINE_STATE_DESC clearShaderPso;
		ZeroMemory(&clearShaderPso, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
		clearShaderPso.pRootSignature = mCRootSignature.Get();
		clearShaderPso.CS =
		{
			reinterpret_cast<BYTE*>(clearHolder->cs->GetBufferPointer()),
			clearHolder->cs->GetBufferSize()
		};
		clearShaderPso.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		ThrowIfFailedG(device->CreateComputePipelineState(&clearShaderPso, IID_PPV_ARGS(clearHolder->pso.GetAddressOf())));

	}
	void JDx12LightCulling::BuildDebugPso(ID3D12Device* device, const JGraphicBaseDataSet& baseDataSet)
	{
		drawDebugShader = std::make_unique<JDx12GraphicShaderDataHolder>();
		JDx12GraphicShaderDataHolder* drawHolder = drawDebugShader.get();

		JShaderType::CompileInfo vsCompileInfo = Private::GraphicDrawDebugVsCompileInfo();
		JShaderType::CompileInfo psCompileInfo = Private::GraphicDrawDebugPsCompileInfo();

		JGraphicShaderInitData initHelper;
		Private::StuffShaderMacro(initHelper, baseDataSet.option);

		drawHolder->vs = JDxShaderDataUtil::CompileShader(vsCompileInfo.filePath, initHelper.macro[Private::staticMeshIndex], vsCompileInfo.functionName, L"vs_6_0");
		drawHolder->ps = JDxShaderDataUtil::CompileShader(psCompileInfo.filePath, initHelper.macro[Private::staticMeshIndex], psCompileInfo.functionName, L"ps_6_0");
		drawHolder->inputLayout =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC debugShaderPso;
		ZeroMemory(&debugShaderPso, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		debugShaderPso.InputLayout = { drawHolder->inputLayout.data(), (uint)drawHolder->inputLayout.size() };
		debugShaderPso.pRootSignature = mDRootSignature.Get();
		debugShaderPso.VS =
		{
			reinterpret_cast<BYTE*>(drawHolder->vs->GetBufferPointer()),
			drawHolder->vs->GetBufferSize()
		};
		debugShaderPso.PS =
		{
			reinterpret_cast<BYTE*>(drawHolder->ps->GetBufferPointer()),
			drawHolder->ps->GetBufferSize()
		};

		debugShaderPso.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		debugShaderPso.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		debugShaderPso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		debugShaderPso.DepthStencilState.DepthEnable = false;
		debugShaderPso.DepthStencilState.StencilEnable = false;

		debugShaderPso.SampleMask = UINT_MAX;
		debugShaderPso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		debugShaderPso.NumRenderTargets = 1;
		debugShaderPso.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		debugShaderPso.SampleDesc.Count = 1;
		ThrowIfFailedG(device->CreateGraphicsPipelineState(&debugShaderPso, IID_PPV_ARGS(drawHolder->pso.GetAddressOf())));
	}
	void JDx12LightCulling::BuildCounterClearBuffer(JDx12GraphicDevice* device)
	{
		if (counterClearBuffer == nullptr)
			counterClearBuffer = std::make_unique<JDx12GraphicBuffer<uint>>(L"LitCounterClear", J_GRAPHIC_BUFFER_TYPE::COMMON);
		counterClearBuffer->Clear();
		counterClearBuffer->Build(device, Private::litListCounterCount);

		if (counterClearUploadBuffer == nullptr)
			counterClearUploadBuffer = std::make_unique<JDx12GraphicBuffer<uint>>(L"LitCounterClearIntermediate", J_GRAPHIC_BUFFER_TYPE::UPLOAD_BUFFER);
		counterClearUploadBuffer->Clear();
		counterClearUploadBuffer->Build(device, Private::litListCounterCount);

		counterClearBuffer->SutffClearValue(device->GetPublicCmdList(), counterClearUploadBuffer->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE, 0);
		/*
		uint32 tempClearData = 0;
		D3D12_SUBRESOURCE_DATA clearData = {};
		clearData.pData = &tempClearData;
		clearData.RowPitch = sizeof(uint32);
		clearData.SlicePitch = clearData.RowPitch;

		JDx12GraphicDevice* dxDevice = static_cast<JDx12GraphicDevice*>(device);
		ID3D12GraphicsCommandList* cmdList = dxDevice->GetPublicCmdList();

		JD3DUtility::ResourceTransition(cmdList, counterClearBuffer->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		UpdateSubresources<1>(cmdList, counterClearBuffer->GetResource(), counterClearUploadBuffer->GetResource(), 0, 0, 1, &clearData);
		JD3DUtility::ResourceTransition(cmdList, counterClearBuffer->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE);
		*/
	}
	void JDx12LightCulling::BuildResultBuffer(JDx12GraphicDevice* device, const uint localLightCapacity)
	{
		//const std::wstring& name, const J_GRAPHIC_BUFFER_TYPE type
		if (resultOutBuffer == nullptr)
			resultOutBuffer = std::make_unique<JDx12GraphicBuffer<uint>>(L"LitCullingResult", J_GRAPHIC_BUFFER_TYPE::UNORDERED_ACCEESS);
		resultOutBuffer->Clear();
		resultOutBuffer->Build(device, localLightCapacity);

		if (resultOutClearBuffer == nullptr)
			resultOutClearBuffer = std::make_unique<JDx12GraphicBuffer<uint>>(L"LitCullingResultClear", J_GRAPHIC_BUFFER_TYPE::COMMON);
		resultOutClearBuffer->Clear();
		resultOutClearBuffer->Build(device, localLightCapacity);

		if (resultClearUploadBuffer == nullptr)
			resultClearUploadBuffer = std::make_unique<JDx12GraphicBuffer<uint>>(L"LitCullingResultClearIntermediate", J_GRAPHIC_BUFFER_TYPE::UPLOAD_BUFFER);
		resultClearUploadBuffer->Clear();
		resultClearUploadBuffer->Build(device, localLightCapacity);

		resultOutClearBuffer->SutffClearValue(device->GetPublicCmdList(), resultClearUploadBuffer->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE, 0);
		JD3DUtility::ResourceTransition(device->GetPublicCmdList(), resultOutBuffer->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		/*
		uint32 tempClearData = 0;
		D3D12_SUBRESOURCE_DATA clearData = {};
		clearData.pData = &tempClearData;
		clearData.RowPitch = sizeof(uint32);
		clearData.SlicePitch = clearData.RowPitch;

		JDx12GraphicDevice* dxDevice = static_cast<JDx12GraphicDevice*>(device);
		ID3D12GraphicsCommandList* cmdList = dxDevice->GetPublicCmdList();

		JD3DUtility::ResourceTransition(cmdList, resultOutClearBuffer->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		UpdateSubresources<1>(cmdList, resultOutClearBuffer->GetResource(), resultClearUploadBuffer->GetResource(), 0, 0, 1, &clearData);
		JD3DUtility::ResourceTransition(cmdList, resultOutClearBuffer->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE);
		*/
	}
	void JDx12LightCulling::BuildDebugBuffer(JDx12GraphicDevice* device, const JGraphicBaseDataSet& baseDataSet)
	{
		if (offsetDebugBuffer == nullptr)
			offsetDebugBuffer = std::make_unique<JDx12GraphicBuffer<uint>>(L"LitStartOffsetBack", J_GRAPHIC_BUFFER_TYPE::READ_BACK);

		if (listDebugBuffer == nullptr)
			listDebugBuffer = std::make_unique<JDx12GraphicBuffer<uint64>>(L"LitListReadBack", J_GRAPHIC_BUFFER_TYPE::READ_BACK);

		offsetDebugBuffer->Clear();
		offsetDebugBuffer->Build(device, baseDataSet.option.GetClusterTotalCount());

		listDebugBuffer->Clear();
		listDebugBuffer->Build(device, baseDataSet.option.GetClusterIndexCount());
	}
	void JDx12LightCulling::BuildRtResource(JDx12GraphicDevice* device, JDx12GraphicResourceManager* dx12Gm, const JGraphicBaseDataSet& baseDataSet)
	{
		JGraphicResourceCreationDesc desc;
		desc.width = baseDataSet.option.GetClusterXCount();
		desc.height = baseDataSet.option.GetClusterYCount();
		desc.arraySize = baseDataSet.info.minCapacity;
		for (uint i = 0; i < SIZE_OF_ARRAY(lightRt); ++i)
			lightRt[i] = dx12Gm->CreateResource(device, desc, Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING);
	}
	void JDx12LightCulling::LoadLightShape(JDx12GraphicDevice* device, JDx12GraphicResourceManager* gM)
	{
		//Unuse
		/*
		//const JFileImportHelpData& pathData, JStaticMeshGroup& meshGroup, ObjMaterialMap& objMatData
		Core::JFileImportHelpData lowSpherePathData(JApplicationEngine::DefaultResourcePath() + L"\\LowSphereC.obj");
		Core::JFileImportHelpData lowConePathData(JApplicationEngine::DefaultResourcePath() + L"\\LowConeC.obj");
		Core::JStaticMeshGroup lowSphereMeshGroup;
		Core::JStaticMeshGroup lowConeMeshGroup;
		Core::ObjMaterialMap lowSphereMaterialMap;
		Core::ObjMaterialMap lowConeMaterialMap;

		Core::JObjFileLoader{}.LoadObjFile(lowSpherePathData, lowSphereMeshGroup, lowSphereMaterialMap);
		Core::JObjFileLoader{}.LoadObjFile(lowConePathData, lowConeMeshGroup, lowConeMaterialMap);
		   
		lowSphereVertexCount = lowSphereMeshGroup.GetTotalVertexCount();
		lowSphereIndexCount = lowSphereMeshGroup.GetTotalIndexCount();

		lowConeVertexCount = lowConeMeshGroup.GetTotalVertexCount();
		lowConeIndexCount = lowConeMeshGroup.GetTotalIndexCount();

		uint subMeshCount = lowSphereMeshGroup.GetMeshDataCount();
		std::vector<Core::J1BytePosVertex> vertex(lowSphereVertexCount);
		uint vertexOffset = 0;
		for (uint i = 0; i < subMeshCount; ++i)
		{
			Core::JStaticMeshData* meshdata = static_cast<Core::JStaticMeshData*>(lowSphereMeshGroup.GetMeshData(i));
			const uint subMeshVertexCount = meshdata->GetVertexCount();
			for (uint j = 0; j < subMeshVertexCount; ++j)
				vertex[vertexOffset + j] =  Core::J1BytePosVertex::Encode(meshdata->GetVertex(j));
			vertexOffset += subMeshVertexCount;
		}
		lowSphereVertex = gM->CreateVertexBuffer(device, vertex);
		lowSphereVertex->SetPrivateName(L"LowSphereVertex");

		std::vector<uint16> index16(lowSphereIndexCount);
		uint indicesOffset = 0;
		for (uint i = 0; i < subMeshCount; ++i)
		{
			const Core::JMeshData* meshdata = lowSphereMeshGroup.GetMeshData(i);
			const uint subMeshIndexCount = meshdata->GetIndexCount();
			for (uint j = 0; j < subMeshIndexCount; ++j)
				index16[indicesOffset + j] = meshdata->GetIndex(j);
			indicesOffset += subMeshIndexCount;
		}
		lowSphereIndex = gM->CreateIndexBuffer(device, index16);
		lowSphereIndex->SetPrivateName(L"LowSphereIndex");

		vertex.resize(lowConeVertexCount);
		subMeshCount = lowConeMeshGroup.GetMeshDataCount();
		vertexOffset = 0;
		for (uint i = 0; i < subMeshCount; ++i)
		{
			Core::JStaticMeshData* meshdata = static_cast<Core::JStaticMeshData*>(lowConeMeshGroup.GetMeshData(i));
			const uint subMeshVertexCount = meshdata->GetVertexCount();
			for (uint j = 0; j < subMeshVertexCount; ++j)
				vertex[vertexOffset + j] = Core::J1BytePosVertex::Encode(meshdata->GetVertex(j));
			vertexOffset += subMeshVertexCount;
		}	 
		lowConeVertex = gM->CreateVertexBuffer(device, vertex);
		lowConeVertex->SetPrivateName(L"LowConeVertex");

		index16.resize(lowConeIndexCount);
		indicesOffset = 0;
		for (uint i = 0; i < subMeshCount; ++i)
		{
			const Core::JMeshData* meshdata = lowConeMeshGroup.GetMeshData(i);
			const uint subMeshIndexCount = meshdata->GetIndexCount();
			for (uint j = 0; j < subMeshIndexCount; ++j)
				index16[indicesOffset + j] = meshdata->GetIndex(j);
			indicesOffset += subMeshIndexCount;
		}
		lowConeIndex = gM->CreateIndexBuffer(device, index16);	 
		lowConeIndex->SetPrivateName(L"LowConeIndex");
		*/
	}
}