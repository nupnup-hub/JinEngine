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


#include"JDx12LightCulling.h"
#include"../../../JGraphicUpdateHelper.h"
#include"../../../JGraphicOption.h"
#include"../../../Device/Dx/JDx12GraphicDevice.h" 
#include"../../../DataSet/Dx/JDx12GraphicDataSet.h"  
#include"../../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../../../GraphicResource/Dx/JDx12GraphicResourceInfo.h"
#include"../../../FrameResource/Dx/JDx12FrameResource.h"
#include"../../../FrameResource/JFrameUpdate.h"
#include"../../../Culling/Dx/JDx12CullingManager.h"
#include"../../../Culling/Dx/JDx12CullingResourceHolder.h"
#include"../../../Command/Dx/JDx12CommandContext.h"
#include"../../../Utility/Dx/JDx12Utility.h"
#include"../../../Utility/Dx/JDx12ObjectCreation.h"
#include"../../../../Core/Platform/JHardwareInfo.h"
#include"../../../../Core/Geometry/Mesh/JMeshType.h"
#include"../../../../Core/Geometry/Mesh/Loader/ObjLoader/JObjFileLoader.h"
#include"../../../../Application/Project/JApplicationProject.h"
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

#define NEAR_CLUST L"NEAR_CLUST"
#define CLEAR_BUFFER L"CLEAR_BUFFER"  
#define OFFER_BUFFER_CLEAR_VALUE L"OFFER_BUFFER_CLEAR_VALUE" 
#define LINEAR_DEPTH_DIST L"LINEAR_DEPTH_DIST" 
#define USE_HEMISPHERE L"HEMISPHERE"

#define LIGHT_RANGE_OFFSET L"LIGHT_RANGE_OFFSET"

namespace JinEngine::Graphic
{
	namespace Private
	{
		//graphic shader
		static constexpr uint gCamCBIndex = 0; 
		static constexpr uint gLightBufferIndex = gCamCBIndex + 1;
		static constexpr int gSlotCount = gLightBufferIndex + 1;

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
				initHelper.macro[staticMeshIndex].push_back({ LIGHT_RANGE_OFFSET, std::to_wstring(option.culling.clusterPointLightRangeOffset) });
				break;
			}
			case JinEngine::J_LIGHT_TYPE::SPOT:
			{
				initHelper.macro[staticMeshIndex].push_back({ SPOT_LIGHT_, L"1" });
				initHelper.macro[staticMeshIndex].push_back({ LIGHT_RANGE_OFFSET, std::to_wstring(option.culling.clusterSpotLightRangeOffset) });
				break;
			}
			case JinEngine::J_LIGHT_TYPE::RECT:
			{
				initHelper.macro[staticMeshIndex].push_back({ RECT_LIGHT_, L"2" });
				initHelper.macro[staticMeshIndex].push_back({ LIGHT_RANGE_OFFSET, std::to_wstring(option.culling.clusterRectLightRangeOffset) });
				if (rectLightShapeIsHemisphere)
					initHelper.macro->push_back({ USE_HEMISPHERE, L"1" });
				break;
			}
			default:
				break;
			} 
		}
		static void StuffShaderMacro(_Inout_ JGraphicShaderInitData& initHelper, const JGraphicOption& option)
		{
			switch (JinEngine::Graphic::Constants::litClusterXRange[option.culling.clusterXIndex])
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
			switch (JinEngine::Graphic::Constants::litClusterYRange[option.culling.clusterYIndex])
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
			switch (JinEngine::Graphic::Constants::litClusterZRange[option.culling.clusterZIndex])
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

			initHelper.macro[staticMeshIndex].push_back({ NEAR_CLUST, std::to_wstring(option.culling.clusterNear) });
			if (useLinearDepth)
				initHelper.macro->push_back({ LINEAR_DEPTH_DIST, L"1" });
		}
		static void StuffShaderMacro(_Inout_ JComputeShaderInitData& initHelper, const JGraphicOption& option)
		{
			switch (JinEngine::Graphic::Constants::litClusterXRange[option.culling.clusterXIndex])
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
			switch (JinEngine::Graphic::Constants::litClusterYRange[option.culling.clusterYIndex])
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
			switch (JinEngine::Graphic::Constants::litClusterZRange[option.culling.clusterZIndex])
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

			initHelper.macro.push_back({ NEAR_CLUST, std::to_wstring(option.culling.clusterNear) });
			initHelper.dispatchInfo.threadDim = JVector3<uint>(Constants::litClusterXRange[0], Constants::litClusterYRange[0], 1);
			initHelper.dispatchInfo.groupDim = JVector3<uint>(std::ceil(option.GetClusterXCount() / Constants::litClusterXRange[0]), std::ceil(option.GetClusterYCount() / Constants::litClusterYRange[0]), 1);
			initHelper.PushThreadDimensionMacro();
		}
		static void StuffClearShaderMacro(_Inout_ JComputeShaderInitData& initHelper, const JGraphicOption& option)
		{  
			initHelper.dispatchInfo.threadDim = JVector3<uint>(Constants::litClusterXRange[0], Constants::litClusterYRange[0], 1);
			initHelper.dispatchInfo.groupDim = JVector3<uint>(std::ceil(option.GetClusterXCount() / Constants::litClusterXRange[0]), std::ceil(option.GetClusterYCount() / Constants::litClusterYRange[0]), option.GetClusterZCount());
			initHelper.macro.push_back({ CLEAR_BUFFER, std::to_wstring(1) });
			initHelper.macro.push_back({ OFFER_BUFFER_CLEAR_VALUE, std::to_wstring((1 << 30) - 1) });
		}
		static JCompileInfo GraphicDrawLightShapeVsCompileInfo()
		{
			return JCompileInfo(ShaderRelativePath::LightCulling(L"LightShapeDrawVertex.hlsl"), L"VS");
		}
		static JCompileInfo GraphicDrawLightShapeGsCompileInfo()
		{
			return JCompileInfo(ShaderRelativePath::LightCulling(L"LightShapeDrawGeo.hlsl"), L"GS");
		}
		static JCompileInfo GraphicDrawLightShapePsCompileInfo()
		{
			return JCompileInfo(ShaderRelativePath::LightCulling(L"LightShapeDrawPixel.hlsl"), L"PS");
		}
		static JCompileInfo ComputeLightClusterCompileInfo()
		{
			return JCompileInfo(ShaderRelativePath::LightCulling(L"ClusterCompute.hlsl"), L"CS");
		}
		static JCompileInfo GraphicDrawDebugVsCompileInfo()
		{
			return JCompileInfo(ShaderRelativePath::LightCulling(L"ClusterDebug.hlsl"), L"FullScreenTriangleVS");
		}
		static JCompileInfo GraphicDrawDebugPsCompileInfo()
		{
			return JCompileInfo(ShaderRelativePath::LightCulling(L"ClusterDebug.hlsl"), L"PS");
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
	JDx12LightCulling::~JDx12LightCulling()
	{
		ClearResource();
	}
	void JDx12LightCulling::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;

		BuildResource(device, gM);
	}
	void JDx12LightCulling::Clear()
	{
		ClearResource();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12LightCulling::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	bool JDx12LightCulling::HasDependency(const JGraphicInfo::TYPE type)const noexcept
	{
		if (type == JGraphicInfo::TYPE::FRAME)
			return true;
		else
			return false;
	}
	bool JDx12LightCulling::HasDependency(const JGraphicOption::TYPE type)const noexcept
	{
		if (type == JGraphicOption::TYPE::CULLING || type == JGraphicOption::TYPE::RENDERING ||
			type == JGraphicOption::TYPE::DEBUGGING)
			return true;
		else
			return false;
	}
	void JDx12LightCulling::NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set)
	{
		auto dx12Set = static_cast<const JDx12GraphicInfoChangedSet&>(set);
		if (dx12Set.preInfo.frame.upPLightCapacity != dx12Set.newInfo.frame.upPLightCapacity ||
			dx12Set.preInfo.frame.upSLightCapacity != dx12Set.newInfo.frame.upSLightCapacity ||
			dx12Set.preInfo.frame.upRLightCapacity != dx12Set.newInfo.frame.upRLightCapacity)
		{
			NotifyLocalLightCapacityChanged(dx12Set.device, dx12Set.gm, dx12Set.newInfo.frame.GetLocalLightCapacity());
		}
	}
	void JDx12LightCulling::NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)
	{
		auto dx12Set = static_cast<const JDx12GraphicOptionChangedSet&>(set);
		if (set.changedPart == JGraphicOption::TYPE::CULLING && set.newOption.culling.LightCullingDependencyChanged(set.preOption))
			NotifyNewClusterOption(dx12Set.device);
		else if (set.changedPart == JGraphicOption::TYPE::RENDERING && set.preOption.rendering.renderTargetFormat != set.newOption.rendering.renderTargetFormat)
			RecompileShader(JGraphicShaderCompileSet(dx12Set.device));
		else if (set.changedPart == JGraphicOption::TYPE::DEBUGGING && set.newOption.debugging.requestRecompileLightClusterShader)
			RecompileShader(JGraphicShaderCompileSet(dx12Set.device));
	}
	void JDx12LightCulling::NotifyNewClusterOption(JGraphicDevice* device)
	{
		if (!IsSameDevice(device))
			return;

		RecompileShader(JGraphicShaderCompileSet(device)); 
		if constexpr (Private::allowDebugging)
			BuildDebugBuffer(static_cast<JDx12GraphicDevice*>(device));
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

		//cmdList->SetGraphicsRootSignature(mGRootSignature.Get());
	}
	void JDx12LightCulling::BindDebugResource(const JGraphicBindSet* bindSet)
	{
		if (!IsSameDevice(bindSet))
			return;

		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(bindSet->context);
		context->SetGraphicsRootSignature(mDRootSignature.Get());
	}
	void JDx12LightCulling::ExecuteLightClusterTask(const JGraphicLightCullingTaskSet* taskSet, const JDrawHelper& helper)
	{
		if (taskSet == nullptr || !IsSameDevice(taskSet) || !helper.allowLightCulling || !helper.option.culling.allowLightCluster)
			return;

		const JDx12GraphicLightCullingTaskSet* dx12Set = static_cast<const JDx12GraphicLightCullingTaskSet*>(taskSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12Set->context);

		DrawLight(context, helper);
	}
	void JDx12LightCulling::ExecuteLightClusterDebug(const JGraphicLightCullingDebugDrawSet* drawSet, const JDrawHelper& helper)
	{
		if constexpr (!Private::allowDebugging)
			return;

		if (drawSet == nullptr || !IsSameDevice(drawSet) || !helper.allowLightCullingDebug || !helper.option.culling.allowLightCluster)
			return;

		const JDx12GraphicLightCullingDebugDrawSet* dx12Set = static_cast<const JDx12GraphicLightCullingDebugDrawSet*>(drawSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12Set->context);

		DrawLightClusterDebug(context, helper);
	}
	void JDx12LightCulling::StreamOutDebugInfo(const std::wstring& path)
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

		auto option = GetGraphicOption();
		const uint clusterX = option.GetClusterXCount();
		const uint clusterY = option.GetClusterYCount();
		const uint clusterZ = option.GetClusterZCount();

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
		BuildGraphicPso(dx12Device->GetDevice());
		BuildComputePso(dx12Device->GetDevice());
		if constexpr (Private::allowDebugging)
			BuildDebugPso(dx12Device->GetDevice());
	}
	void JDx12LightCulling::DrawLight(JDx12CommandContext* context, const JDrawHelper& helper)
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
		const uint lightSum = pointLitCount + spotLitCount + rectLitCount;

		const uint pointLitOffset = JFrameUpdateData::GetAreaRegistedOffset(J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT, sceneGuid);
		const uint spotLitOffset = JFrameUpdateData::GetAreaRegistedOffset(J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT, sceneGuid);
		const uint rectLitOffset = JFrameUpdateData::GetAreaRegistedOffset(J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT, sceneGuid);

		const uint pointLitLocalOffset = 0;
		const uint spotLitLocalOffset = pointLitCount + pointLitOffset;
		const uint rectLitLocalOffset = spotLitCount + spotLitOffset;

		auto gInterface = helper.cam->GraphicResourceUserInterface();
		auto cInterface = helper.cam->CullingUserInterface();

		auto offsetBufferSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET, J_GRAPHIC_TASK_TYPE::LIGHT_CULLING);
		auto linkedSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST, J_GRAPHIC_TASK_TYPE::LIGHT_CULLING);
		auto linkedCounterSet = context->ComputeSet(linkedSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNTER_BUFFER);
		auto readBackSet = context->ComputeSet(cInterface, J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::LIGHT);

		auto pointSet = context->ComputeSet(lightRt[0]);
		auto spotSet = context->ComputeSet(lightRt[1]);
		auto rectSet = context->ComputeSet(lightRt[2]);

		context->Transition(pointSet.holder, D3D12_RESOURCE_STATE_RENDER_TARGET);
		context->Transition(spotSet.holder, D3D12_RESOURCE_STATE_RENDER_TARGET);
		context->Transition(rectSet.holder, D3D12_RESOURCE_STATE_RENDER_TARGET);
		context->Transition(offsetBufferSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		context->Transition(linkedSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		//Clear resource 
		context->CopyResource(resultOutClearBuffer->GetHolder(), resultOutBuffer->GetHolder());
		context->Transition(resultOutBuffer->GetHolder(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		context->CopyResource(counterClearBuffer->GetHolder(), linkedCounterSet.holder);
		context->Transition(linkedCounterSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

		context->SetPipelineState(clearOffsetBufferShader.get());
		context->SetComputeRootSignature(mCRootSignature.Get());
		context->SetComputeRootDescriptorTable(Private::cOffsetBufferIndex, offsetBufferSet.GetGpuUavHandle());
		context->SetComputeRootDescriptorTable(Private::cLinkedlistndex, linkedSet.GetGpuUavHandle());
		context->SetComputeRootUnorderedAccessView(Private::cOutBufferIndex, resultOutBuffer->GetResource()->GetGPUVirtualAddress());

		auto clearDispatchInfo = clearOffsetBufferShader->dispatchInfo.groupDim;
		context->Dispatch(clearDispatchInfo.x, clearDispatchInfo.y, clearDispatchInfo.z);

		const uint clusterX = helper.option.GetClusterXCount();
		const uint clusterY = helper.option.GetClusterYCount();

		const uint pointLitType = 0;
		const uint spotLitType = 1;
		const uint rectLitType = 2;

		D3D12_VIEWPORT viewPort{ 0.0f, 0.0f, (float)clusterX, (float)clusterY, 0.0f, 1.0f };
		D3D12_RECT rect{ 0, 0, clusterX, clusterY, };
		 
		context->SetGraphicsRootSignature(mGRootSignature.Get());
		context->SetGraphicsRootConstantBufferView(Private::gCamCBIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT_CULLING_PASS, helper.GetCamFrameIndex(CameraFrameLayer::lightCulling));
		context->SetViewport(viewPort);
		context->SetScissorRect(rect);

		if (pointLitCount > 0)
		{
			context->SetPipelineState(drawLightShader[0].get());
			//context->SetGraphicsRoot32BitConstants(Private::gPassCBIndex, 0, pointLitOffset);
			context->SetGraphicsRootShaderResourceView(Private::gLightBufferIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT, pointLitOffset);

			context->ClearRenderTargetView(pointSet, Constants::GetWhiteClearColor());
			context->SetRenderTargetView(pointSet, 1, true);
			context->SetMeshGeometryData(sphere);
			context->DrawIndexedInstanced(sphere, 0, pointLitCount);
		}
		if (spotLitCount > 0)
		{
			context->SetPipelineState(drawLightShader[1].get());
			//context->SetGraphicsRoot32BitConstants(Private::gPassCBIndex, 0, spotLitOffset);
			context->SetGraphicsRootShaderResourceView(Private::gLightBufferIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT, spotLitOffset);

			context->ClearRenderTargetView(spotSet, Constants::GetWhiteClearColor());
			context->SetRenderTargetView(spotSet, 1, true);
			context->SetMeshGeometryData(cone);
			context->DrawIndexedInstanced(cone, 0, spotLitCount);
		}
		if (rectLitCount > 0)
		{
			context->SetPipelineState(drawLightShader[2].get());
			//context->SetGraphicsRoot32BitConstants(Private::gPassCBIndex, 0, rectLitOffset);
			context->SetGraphicsRootShaderResourceView(Private::gLightBufferIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT, rectLitOffset);

			context->ClearRenderTargetView(rectSet, Constants::GetWhiteClearColor());
			context->SetRenderTargetView(rectSet, 1, true);
			context->SetMeshGeometryData(hemiSphere);
			context->DrawIndexedInstanced(hemiSphere, 0, rectLitCount);
		}

		context->Transition(pointSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context->Transition(spotSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context->Transition(rectSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context->Transition(offsetBufferSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context->Transition(linkedSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, true);

		context->SetComputeRootSignature(mCRootSignature.Get());
		context->SetPipelineState(computeLightClusterShader.get());
		context->SetComputeRootDescriptorTable(Private::cOffsetBufferIndex, offsetBufferSet.GetGpuUavHandle());
		context->SetComputeRootDescriptorTable(Private::cLinkedlistndex, linkedSet.GetGpuUavHandle());
		context->SetComputeRootUnorderedAccessView(Private::cOutBufferIndex, resultOutBuffer->GetResource()->GetGPUVirtualAddress());

		auto dispatchInfo = computeLightClusterShader->dispatchInfo.groupDim;
		if (pointLitCount > 0)
		{ 
			context->SetComputeRoot32BitConstants(Private::cLightTypeCBIndex, 0, pointLitType);
			context->SetComputeRoot32BitConstants(Private::cLightTypeCBIndex, 1, pointLitLocalOffset);
			context->SetComputeRootDescriptorTable(Private::cRtTextureArrayIndex, pointSet.GetGpuSrvHandle());
			context->Dispatch(dispatchInfo.x, dispatchInfo.y, pointLitCount);
		}
		if (spotLitCount > 0)
		{
			context->InsertUAVBarrier(linkedSet.holder, true);
			context->SetComputeRoot32BitConstants(Private::cLightTypeCBIndex, 0, spotLitType);
			context->SetComputeRoot32BitConstants(Private::cLightTypeCBIndex, 1, spotLitLocalOffset);
			context->SetComputeRootDescriptorTable(Private::cRtTextureArrayIndex, spotSet.GetGpuSrvHandle());
			context->Dispatch(dispatchInfo.x, dispatchInfo.y, spotLitCount);
		}
		if (rectLitCount > 0)
		{
			context->InsertUAVBarrier(linkedSet.holder, true);
			context->SetComputeRoot32BitConstants(Private::cLightTypeCBIndex, 0, rectLitType);
			context->SetComputeRoot32BitConstants(Private::cLightTypeCBIndex, 1, rectLitLocalOffset);
			context->SetComputeRootDescriptorTable(Private::cRtTextureArrayIndex, rectSet.GetGpuSrvHandle());
			context->Dispatch(dispatchInfo.x, dispatchInfo.y, rectLitCount);
		}

		if constexpr (Private::allowDebugging)
		{
			if (!helper.cam->GetOwner()->IsEditorObject())
			{
				context->CopyResource(offsetBufferSet.holder, offsetDebugBuffer->GetHolder());
				context->CopyResource(linkedSet.holder, listDebugBuffer->GetHolder());
			}
		}
		context->Transition(offsetBufferSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		context->Transition(linkedSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		context->CopyResource(resultOutBuffer->GetHolder(), readBackSet.gHolder);
		//context->Transition(readBackSet.gHolder, D3D12_RESOURCE_STATE_COMMON, true);

		JCullingUpdatedInfo updateInfo;
		updateInfo.updatedStartIndex = 0;
		updateInfo.updatedCount = pointLitCount + spotLitCount + rectLitCount;
		readBackSet.info->SetUpdatedInfo(updateInfo, helper.info.frame.currIndex);
	}
	void JDx12LightCulling::DrawLightClusterDebug(JDx12CommandContext* context, const JDrawHelper& helper)
	{
		//JUserPtr<JMeshGeometry> quad = _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::FULL_SCREEN_QUAD);

		auto gInterface = helper.cam->GraphicResourceUserInterface();
		auto rtSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::LIGHT_LIST_DRAW);
		auto dsSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		auto offsetBufferSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET, J_GRAPHIC_TASK_TYPE::LIGHT_CULLING);
		auto linkedSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST, J_GRAPHIC_TASK_TYPE::LIGHT_CULLING);

		context->Transition(rtSet.holder, D3D12_RESOURCE_STATE_RENDER_TARGET);
		context->Transition(dsSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		context->Transition(offsetBufferSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		context->Transition(linkedSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
		context->SetPipelineState(drawDebugShader.get());

		JVector2F renderTargetSize = rtSet.info->GetResourceSize();
		float camNearZ = helper.cam->GetNear();
		float camFarZ = helper.cam->GetFar();
		//uint log2Tile = (uint32)std::log2<uint32>(renderTargetSize.x / helper.option.GetClusterXCount());
		JVector4F passPack = JVector4F(camNearZ, camFarZ, renderTargetSize.x, renderTargetSize.y);

		context->SetViewportAndRect(renderTargetSize);
		context->SetGraphicsRoot32BitConstants(Private::dPassCBIndex, 0, passPack);
		context->SetGraphicsRootDescriptorTable(Private::dDepthMapIndex, dsSet.GetGpuSrvHandle());
		context->SetGraphicsRootDescriptorTable(Private::dOffsetBufferIndex, offsetBufferSet.GetGpuSrvHandle());
		context->SetGraphicsRootDescriptorTable(Private::dLinkedlistndex, linkedSet.GetGpuSrvHandle());

		context->ClearRenderTargetView(rtSet, Constants::GetBackBufferClearColor());
		context->SetRenderTargetView(rtSet);
		context->DrawFullScreenTriangle();
		context->Transition(rtSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
	void JDx12LightCulling::BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(gM);
		JGraphicInfo gInfo = GetGraphicInfo();
		const uint localLightCapacity = gInfo.frame.GetLocalLightCapacity();

		BuildGraphicRootSignature(dx12Device->GetDevice());
		BuildComputeRootSignature(dx12Device->GetDevice());
		BuildGraphicPso(dx12Device->GetDevice());
		BuildComputePso(dx12Device->GetDevice()); 
		BuildCounterClearBuffer(dx12Device);
		BuildResultBuffer(dx12Device, localLightCapacity);
		BuildRtResource(dx12Device, dx12Gm);
		if constexpr (Private::allowDebugging)
		{
			BuildDebugRootSignature(dx12Device->GetDevice());
			BuildDebugPso(dx12Device->GetDevice());
			BuildDebugBuffer(dx12Device);
		}
		//LoadLightShape(dx12Device, dx12Gm);
		device->EndPublicCommand();
		device->FlushCommandQueue();
		device->StartPublicCommand();
		resultClearUploadBuffer = nullptr;
		counterClearUploadBuffer = nullptr;
	}
	void JDx12LightCulling::BuildGraphicRootSignature(ID3D12Device* device)
	{ 
		JDx12RootSignatureBuilder<Private::gSlotCount> builder;
		builder.PushConstantsBuffer(0);		//Private::gCamCBIndex
		//builder.PushConstants(1, 1);		//Private::gPassCBIndex
		builder.PushShaderResource(0, 0);	//Private::gLightBufferIndex
		builder.Create(device, L"LightCulling Draw RootSignature", mGRootSignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	}
	void JDx12LightCulling::BuildComputeRootSignature(ID3D12Device* device)
	{
		JDx12RootSignatureBuilder<Private::cSlotCount> builder;
		builder.PushConstants(2, 0);								//Private::cLightTypeCBIndex
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);	//Private::cOffsetBufferIndex
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);	//Private::cLinkedlistndex
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);	//Private::cOutBufferIndex
		builder.PushUnorderedAccess(2);		 
		builder.Create(device, L"LightCulling Compute RootSignature", mCRootSignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);
	}
	void JDx12LightCulling::BuildDebugRootSignature(ID3D12Device* device)
	{
		JDx12RootSignatureBuilder<Private::dSlotCount> builder;
		builder.PushConstants(4, 0);								//Private::dPassCBIndex
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);	//Private::dDepthMapIndex
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);	//Private::dOffsetBufferIndex
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);	//Private::dLinkedlistndex
		builder.Create(device, L"LightCulling Debug RootSignature", mDRootSignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	}
	void JDx12LightCulling::BuildGraphicPso(ID3D12Device* device)
	{
		for (uint i = 0; i < JLightType::GetLocalLightCount(); ++i)
		{
			const J_LIGHT_TYPE lType = (J_LIGHT_TYPE)(i + JLightType::GetLocalLightOffset());
			drawLightShader[i] = std::make_unique<JDx12GraphicShaderDataHolder>();

			JDx12GraphicShaderDataHolder* drawHolder = drawLightShader[i].get();
			JCompileInfo vsCompileInfo = Private::GraphicDrawLightShapeVsCompileInfo();
			JCompileInfo gsCompileInfo = Private::GraphicDrawLightShapeGsCompileInfo();
			JCompileInfo psCompileInfo = Private::GraphicDrawLightShapePsCompileInfo();

			JGraphicShaderInitData initHelper;
			Private::StuffShaderMacro(initHelper, GetGraphicOption(), lType);
			Private::StuffShaderMacro(initHelper, GetGraphicOption());

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
			ThrowIfFailedG(device->CreateGraphicsPipelineState(&darwShaderPso, IID_PPV_ARGS(drawHolder->GetPsoAddress())));
		}
	}
	void JDx12LightCulling::BuildComputePso(ID3D12Device* device)
	{
		computeLightClusterShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		clearOffsetBufferShader = std::make_unique<JDx12ComputeShaderDataHolder>();

		JDx12ComputeShaderDataHolder* computeHolder = computeLightClusterShader.get();
		JCompileInfo csCompileInfo = Private::ComputeLightClusterCompileInfo();

		JComputeShaderInitData initHelper;
		Private::StuffShaderMacro(initHelper, GetGraphicOption());
		computeHolder->cs = JDxShaderDataUtil::CompileShader(csCompileInfo.filePath, initHelper.macro, csCompileInfo.functionName, L"cs_6_0");
		computeHolder->dispatchInfo = initHelper.dispatchInfo;

		JDx12ComputePso::Create(device, computeHolder, mCRootSignature.Get()); 
	 
		JDx12ComputeShaderDataHolder* clearHolder = clearOffsetBufferShader.get();
		Private::StuffClearShaderMacro(initHelper, GetGraphicOption());
		clearHolder->cs = JDxShaderDataUtil::CompileShader(csCompileInfo.filePath, initHelper.macro, csCompileInfo.functionName, L"cs_6_0");
		clearHolder->dispatchInfo = initHelper.dispatchInfo;

		JDx12ComputePso::Create(device, clearHolder, mCRootSignature.Get());
	}
	void JDx12LightCulling::BuildDebugPso(ID3D12Device* device)
	{
		drawDebugShader = std::make_unique<JDx12GraphicShaderDataHolder>();
		JDx12GraphicShaderDataHolder* drawHolder = drawDebugShader.get();

		JCompileInfo vsCompileInfo = Private::GraphicDrawDebugVsCompileInfo();
		JCompileInfo psCompileInfo = Private::GraphicDrawDebugPsCompileInfo();

		JGraphicShaderInitData initHelper;
		Private::StuffShaderMacro(initHelper, GetGraphicOption());

		drawHolder->vs = JDxShaderDataUtil::CompileShader(vsCompileInfo.filePath, initHelper.macro[Private::staticMeshIndex], vsCompileInfo.functionName, L"vs_6_0");
		drawHolder->ps = JDxShaderDataUtil::CompileShader(psCompileInfo.filePath, initHelper.macro[Private::staticMeshIndex], psCompileInfo.functionName, L"ps_6_0");
 
		D3D12_GRAPHICS_PIPELINE_STATE_DESC debugShaderPso;
		ZeroMemory(&debugShaderPso, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		debugShaderPso.InputLayout.NumElements = 0; 
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
		debugShaderPso.RTVFormats[0] = Constants::GetRenderTargetFormat(GetGraphicOption().rendering.renderTargetFormat);
		debugShaderPso.SampleDesc.Count = 1;
		ThrowIfFailedG(device->CreateGraphicsPipelineState(&debugShaderPso, IID_PPV_ARGS(drawHolder->GetPsoAddress())));
	}
	void JDx12LightCulling::BuildCounterClearBuffer(JDx12GraphicDevice* device)
	{
		if (counterClearBuffer == nullptr)
			counterClearBuffer = std::make_unique<JDx12GraphicBufferT<uint>>(L"LitCounterClear", J_GRAPHIC_BUFFER_TYPE::COMMON);
		counterClearBuffer->Clear();
		counterClearBuffer->Build(device, Private::litListCounterCount);

		if (counterClearUploadBuffer == nullptr)
			counterClearUploadBuffer = std::make_unique<JDx12GraphicBufferT<uint>>(L"LitCounterClearIntermediate", J_GRAPHIC_BUFFER_TYPE::UPLOAD_BUFFER);
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

		JDx12Utility::ResourceTransition(cmdList, counterClearBuffer->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		UpdateSubresources<1>(cmdList, counterClearBuffer->GetResource(), counterClearUploadBuffer->GetResource(), 0, 0, 1, &clearData);
		JDx12Utility::ResourceTransition(cmdList, counterClearBuffer->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE);
		*/
	}
	void JDx12LightCulling::BuildResultBuffer(JDx12GraphicDevice* device, const uint localLightCapacity)
	{
		//const std::wstring& name, const J_GRAPHIC_BUFFER_TYPE type
		if (resultOutBuffer == nullptr)
			resultOutBuffer = std::make_unique<JDx12GraphicBufferT<uint>>(L"LitCullingResult", J_GRAPHIC_BUFFER_TYPE::UNORDERED_ACCEESS);
		resultOutBuffer->Clear();
		resultOutBuffer->Build(device, localLightCapacity);

		if (resultOutClearBuffer == nullptr)
			resultOutClearBuffer = std::make_unique<JDx12GraphicBufferT<uint>>(L"LitCullingResultClear", J_GRAPHIC_BUFFER_TYPE::COMMON);
		resultOutClearBuffer->Clear();
		resultOutClearBuffer->Build(device, localLightCapacity);

		if (resultClearUploadBuffer == nullptr)
			resultClearUploadBuffer = std::make_unique<JDx12GraphicBufferT<uint>>(L"LitCullingResultClearIntermediate", J_GRAPHIC_BUFFER_TYPE::UPLOAD_BUFFER);
		resultClearUploadBuffer->Clear();
		resultClearUploadBuffer->Build(device, localLightCapacity);

		resultOutClearBuffer->SutffClearValue(device->GetPublicCmdList(), resultClearUploadBuffer->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE, 0);
		JDx12Utility::ResourceTransition(device->GetPublicCmdList(), resultOutBuffer->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		/*
		uint32 tempClearData = 0;
		D3D12_SUBRESOURCE_DATA clearData = {};
		clearData.pData = &tempClearData;
		clearData.RowPitch = sizeof(uint32);
		clearData.SlicePitch = clearData.RowPitch;

		JDx12GraphicDevice* dxDevice = static_cast<JDx12GraphicDevice*>(device);
		ID3D12GraphicsCommandList* cmdList = dxDevice->GetPublicCmdList();

		JDx12Utility::ResourceTransition(cmdList, resultOutClearBuffer->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		UpdateSubresources<1>(cmdList, resultOutClearBuffer->GetResource(), resultClearUploadBuffer->GetResource(), 0, 0, 1, &clearData);
		JDx12Utility::ResourceTransition(cmdList, resultOutClearBuffer->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE);
		*/
	}
	void JDx12LightCulling::BuildDebugBuffer(JDx12GraphicDevice* device)
	{
		if (offsetDebugBuffer == nullptr)
			offsetDebugBuffer = std::make_unique<JDx12GraphicBufferT<uint>>(L"LitStartOffsetBack", J_GRAPHIC_BUFFER_TYPE::READ_BACK);

		if (listDebugBuffer == nullptr)
			listDebugBuffer = std::make_unique<JDx12GraphicBufferT<uint64>>(L"LitListReadBack", J_GRAPHIC_BUFFER_TYPE::READ_BACK);

		offsetDebugBuffer->Clear();
		offsetDebugBuffer->Build(device, GetGraphicOption().GetClusterTotalCount());

		listDebugBuffer->Clear();
		listDebugBuffer->Build(device, GetGraphicOption().GetClusterIndexCount());
	}
	void JDx12LightCulling::BuildRtResource(JDx12GraphicDevice* device, JDx12GraphicResourceManager* dx12Gm)
	{
		JGraphicResourceCreationDesc desc;
		desc.width = GetGraphicOption().GetClusterXCount();
		desc.height = GetGraphicOption().GetClusterYCount();
		desc.arraySize = GetGraphicInfo().minCapacity;
		for (uint i = 0; i < SIZE_OF_ARRAY(lightRt); ++i)
			lightRt[i] = dx12Gm->CreateResource(device, desc, Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING);
	}
	void JDx12LightCulling::ClearResource()
	{
		for (uint i = 0; i < SIZE_OF_ARRAY(lightRt); ++i)
			JGraphicResourceInfo::Destroy(lightRt[i].Release());

		counterClearBuffer = nullptr;
		counterClearUploadBuffer = nullptr;

		resultOutBuffer = nullptr;
		resultOutClearBuffer = nullptr;
		resultClearUploadBuffer = nullptr;

		for (uint i = 0; i < SIZE_OF_ARRAY(drawLightShader); ++i)
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
		//Unuse
		/*
		JGraphicResourceInfo::Destroy(lowSphereVertex.Release());
		JGraphicResourceInfo::Destroy(lowSphereIndex.Release());
		JGraphicResourceInfo::Destroy(lowConeVertex.Release());
		JGraphicResourceInfo::Destroy(lowConeIndex.Release());
		*/
	}
	void JDx12LightCulling::LoadLightShape(JDx12GraphicDevice* device, JDx12GraphicResourceManager* gM)
	{
		//Unuse
		/*
		//const JFileImportPathData& pathData, JStaticMeshGroup& meshGroup, ObjMaterialMap& objMatData
		Core::JFileImportPathData lowSpherePathData(JApplicationEngine::DefaultResourcePath() + L"\\LowSphereC.obj");
		Core::JFileImportPathData lowConePathData(JApplicationEngine::DefaultResourcePath() + L"\\LowConeC.obj");
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