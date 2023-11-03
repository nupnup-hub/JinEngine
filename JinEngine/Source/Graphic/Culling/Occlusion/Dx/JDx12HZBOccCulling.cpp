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
#include"../../../../Core/Platform/JHardwareInfo.h"
#include"../../../../Object/GameObject/JGameObject.h"  
#include"../../../../Object/Component/Transform/JTransform.h"
#include"../../../../Object/Component/Camera/JCamera.h"
#include"../../../../Object/Component/Camera/JCameraPrivate.h"
#include"../../../../Object/Component/Light/JLight.h"
#include"../../../../Object/Component/Light/JLightPrivate.h"
#include"../../../../Object/Component/RenderItem/JRenderItem.h" 
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

#define THREAD_DIM_X_SYMBOL "DIMX"
#define THREAD_DIM_Y_SYMBOL "DIMY"
#define THREAD_DIM_Z_SYMBOL "DIMZ"

#define HZB_SAMPLING_COUNT_SYMBOL "DOWN_SAMPLING_COUNT"
#define HZB_OCC_QUERY_COUNT_SYMBOL "OCCLUSION_QUERY_COUNT" 
#define HZB_PERSPECTIVE_DEPTH_MAP_SYMBOL "PERSPECTIVE_DEPTH_MAP"

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
#ifdef _DEBUG
		static constexpr bool allowHzbDebug = true;
		static constexpr int slotCount = debugPassCBIndex + 1;
#else
		static constexpr bool allowHzbDebug = false;
		static constexpr int slotCount = computePassCBIndex + 1;
#endif
	}
	namespace
	{
		static void _StreamOutDebugInfo(JHlslDebug<JHZBDebugInfo>* debugInfo, const std::wstring& path)
		{
			if (!allowHzbDebug)
				return;

			auto rawVec = JRenderItem::StaticTypeInfo().GetInstanceRawPtrVec();
			JFileIOTool tool;
			if (!tool.Begin(path, JFileIOTool::TYPE::JSON, JFileIOTool::BEGIN_OPTION_JSON_SET_CONTENTS_ARRAY_OWNER))
				return;

			int count = 0;
			int offset = 0;
			JHZBDebugInfo* info = debugInfo->Map(count);
			for (uint i = 0; i < count; ++i)
			{
				//JFileIOHelper::StoreXMFloat3(stream, L"Center", info[i].center);
				//JFileIOHelper::StoreXMFloat3(stream, L"Extents", info[i].extents);
				tool.PushArrayMember();
				JFileIOHelper::StoreJString(tool, static_cast<JRenderItem*>(rawVec[info[i].queryIndex])->GetOwner()->GetName(), "Name:");
				JFileIOHelper::StoreVector3(tool, static_cast<JRenderItem*>(rawVec[info[i].queryIndex])->GetOwner()->GetTransform()->GetScale(), "Scale:");

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

				JFileIOHelper::StoreAtomicData(tool, info[i].queryIndex, "QueryIdex");
				JFileIOHelper::StoreAtomicData(tool, info[i].threadIndex, "ThreadIndex");
				JFileIOHelper::StoreAtomicData(tool, info[i].cullingRes, "CullingRes");
				JFileIOHelper::StoreAtomicData(tool, info[i].centerDepth, "CenterDepth");
				JFileIOHelper::StoreAtomicData(tool, info[i].finalCompareDepth, "FinalCompareDepth");
				JFileIOHelper::InputSpace(tool, 1);
				tool.PopStack();
			}
			tool.Close(JFileIOTool::CLOSE_OPTION_JSON_STORE_DATA);
		}
		static void _StreamOutDebugBufferInfo(JHlslDebug<JHZBDebugInfo>* debugInfo)
		{
#if defined(DEVELOP)
			if (!allowHzbDebug || !Develop::JDevelopDebug::IsActivate())
				return;

			int count = 0;
			JHZBDebugInfo* info = debugInfo->Map(count);
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
		static void StuffComputeShaderCommonMacro(_Out_ JComputeShaderInitData& initHelper, const JDx12HZBOccCulling::COMPUTE_TYPE type)
		{
			initHelper.macro.push_back({ THREAD_DIM_X_SYMBOL, std::to_string(initHelper.dispatchInfo.threadDim.x) });
			initHelper.macro.push_back({ THREAD_DIM_Y_SYMBOL, std::to_string(initHelper.dispatchInfo.threadDim.y) });
			initHelper.macro.push_back({ THREAD_DIM_Z_SYMBOL, std::to_string(initHelper.dispatchInfo.threadDim.z) });
		}
		static JShaderType::CompileInfo ComputeShaderCompileInfo(const JDx12HZBOccCulling::COMPUTE_TYPE type)
		{
			using COMPUTE_TYPE = JinEngine::Graphic::JDx12HZBOccCulling::COMPUTE_TYPE;
			switch (type)
			{
			case COMPUTE_TYPE::HZB_COPY_PERSPECTIVE:
				return JShaderType::CompileInfo(L"Hierarchical z-buffer.hlsl", "HZBCopyDepthMap");
			case COMPUTE_TYPE::HZB_COPY_ORTHOLOGIC:
				return JShaderType::CompileInfo(L"Hierarchical z-buffer.hlsl", "HZBCopyDepthMap");
			case COMPUTE_TYPE::HZB_DOWN_SAMPLING:
				return JShaderType::CompileInfo(L"Hierarchical z-buffer.hlsl", "HZBDownSampling");
			case COMPUTE_TYPE::HZB_CULLING_PERSPECTIVE:
				return JShaderType::CompileInfo(L"Hierarchical z-buffer.hlsl", "HZBOcclusion");
			case COMPUTE_TYPE::HZB_CULLING_ORTHOLOGIC:
				return JShaderType::CompileInfo(L"Hierarchical z-buffer.hlsl", "HZBOcclusion");
			default:
				return JShaderType::CompileInfo(L"Error", "Error");
			}
		}
		static void StuffSamplingInitHelper(_Out_ JComputeShaderInitData& initHelper,
			const JDx12HZBOccCulling::COMPUTE_TYPE type,
			const JGraphicInfo& graphicInfo)noexcept
		{
			using COMPUTE_TYPE = JinEngine::Graphic::JDx12HZBOccCulling::COMPUTE_TYPE;
			auto calThreadDim = [](const uint ori, const uint length, const uint devideFactor, uint& devideCount) -> uint
			{
				devideCount = 0;
				uint result = ori;
				while (result > length)
				{
					result /= devideFactor;
					++devideCount;
				}
				if (result == 0)
					return 1;
				else
					return result;
			};

			using GpuInfo = Core::JHardwareInfo::GpuInfo;

			auto InitHZBMaps = [](_Out_ JComputeShaderInitData& initHelper,
				const JDx12HZBOccCulling::COMPUTE_TYPE type,
				const JGraphicInfo& graphicInfo)
			{
				std::vector<GpuInfo> gpuInfo = Core::JHardwareInfo::GetGpuInfo();

				//수정필요 
				//thread per group factor가 하드코딩됨
				//이후 amd graphic info 추가와 동시에 수정할 예정
				uint warpFactor = gpuInfo[0].vendor == Core::J_GRAPHIC_VENDOR::AMD ? 64 : 32;
				uint groupDimX = (uint)std::ceil((float)graphicInfo.occlusionWidth / float(gpuInfo[0].maxThreadsDim.x));
				uint groupDimY = graphicInfo.occlusionHeight;

				//textuer size is always 2 squared
				uint threadDimX = graphicInfo.occlusionWidth;
				uint threadDimY = (uint)std::ceil((float)graphicInfo.occlusionHeight / float(gpuInfo[0].maxGridDim.y));

				initHelper.dispatchInfo.threadDim = JVector3<uint>(threadDimX, threadDimY, 1);
				initHelper.dispatchInfo.groupDim = JVector3<uint>(groupDimX, groupDimY, 1);
				 
				initHelper.dispatchInfo.taskOriCount = graphicInfo.occlusionWidth * graphicInfo.occlusionHeight;
				if (type == COMPUTE_TYPE::HZB_COPY_PERSPECTIVE)
					initHelper.macro.push_back({ HZB_PERSPECTIVE_DEPTH_MAP_SYMBOL, "1" });
				StuffComputeShaderCommonMacro(initHelper, type);
			};

			switch (type)
			{
			case COMPUTE_TYPE::HZB_COPY_PERSPECTIVE:
			case COMPUTE_TYPE::HZB_COPY_ORTHOLOGIC:
			case COMPUTE_TYPE::HZB_DOWN_SAMPLING:
			{
				InitHZBMaps(initHelper, type, graphicInfo);
				break;
			}
			default:
				break;
			}
			//initHelper.macro.push_back({ NULL, NULL });
			//initHelper.cFunctionFlag = cFunctionFlag;
		}
		static void StuffCullingInitHelper(_Out_ JComputeShaderInitData& initHelper,
			const JDx12HZBOccCulling::COMPUTE_TYPE type,
			const int capacity)noexcept
		{
			using COMPUTE_TYPE = JinEngine::Graphic::JDx12HZBOccCulling::COMPUTE_TYPE;
			auto calThreadDim = [](const uint ori, const uint length, const uint devideFactor, uint& devideCount) -> uint
			{
				devideCount = 0;
				uint result = ori;
				while (result > length)
				{
					result /= devideFactor;
					++devideCount;
				}
				if (result == 0)
					return 1;
				else
					return result;
			};

			using GpuInfo = Core::JHardwareInfo::GpuInfo;
			switch (type)
			{
			case COMPUTE_TYPE::HZB_CULLING_PERSPECTIVE:
			case COMPUTE_TYPE::HZB_CULLING_ORTHOLOGIC:
			{
				std::vector<GpuInfo> gpuInfo = Core::JHardwareInfo::GetGpuInfo();
				uint totalSmCount = 0;
				uint totalBlockPerSmCount = 0;
				uint totalThreadPerBlockCount = 0;
				for (const auto& data : gpuInfo)
				{
					totalSmCount += data.multiProcessorCount;
					totalBlockPerSmCount += data.maxBlocksPerMultiProcessor;
					totalThreadPerBlockCount += data.maxThreadsPerBlock;
				}

				//graphicInfo.upObjCapacity always 2 squared
				uint queryCount = capacity > 0 ? capacity : 1;

				//수정필요 
				//thread per group factor가 하드코딩됨
				//이후 amd graphic info 추가와 동시에 수정할 예정
				uint warpFactor = gpuInfo[0].vendor == Core::J_GRAPHIC_VENDOR::AMD ? 64 : 32;
				if (queryCount < warpFactor)
				{
					initHelper.dispatchInfo.threadDim = JVector3<uint>(queryCount, 1, 1);
					initHelper.dispatchInfo.groupDim = JVector3<uint>(1, 1, 1);
					initHelper.dispatchInfo.taskOriCount = queryCount;
				}
				else
				{
					initHelper.dispatchInfo.threadDim = JVector3<uint>(warpFactor, 1, 1);
					initHelper.dispatchInfo.groupDim = JVector3<uint>(queryCount / warpFactor, 1, 1);
					initHelper.dispatchInfo.taskOriCount = queryCount;
				}
				initHelper.macro.push_back({ HZB_OCC_QUERY_COUNT_SYMBOL, std::to_string(queryCount) });
				if (type == COMPUTE_TYPE::HZB_CULLING_PERSPECTIVE)
					initHelper.macro.push_back({ HZB_PERSPECTIVE_DEPTH_MAP_SYMBOL, "1" });
				StuffComputeShaderCommonMacro(initHelper, type);
				break;
			}
			default:
				break;
			}
			//initHelper.macro.push_back({ NULL, NULL });
			//initHelper.cFunctionFlag = cFunctionFlag;
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
		CreateSamplingShader(device, info, COMPUTE_TYPE::HZB_COPY_PERSPECTIVE);
		CreateSamplingShader(device, info, COMPUTE_TYPE::HZB_COPY_ORTHOLOGIC);
		CreateSamplingShader(device, info, COMPUTE_TYPE::HZB_DOWN_SAMPLING);
		CreateCullingShader(device, info.upObjCapacity, COMPUTE_TYPE::HZB_CULLING_PERSPECTIVE);
		CreateCullingShader(device, info.upObjCapacity, COMPUTE_TYPE::HZB_CULLING_ORTHOLOGIC);
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
		if constexpr (!allowHzbDebug)
			return false;

		if (!IsSameDevice(device) || cullingInfo->GetCullingType() != J_CULLING_TYPE::HZB_OCCLUSION)
			return false;

		auto debugBuff = std::make_unique<JHlslDebug<JHZBDebugInfo>>(L"Hzb", debugPassCBIndex);
		debugBuff->Build(device, initCapacity);
		occDebugBuffer.push_back(std::move(debugBuff));
		return true;
	}
	void JDx12HZBOccCulling::ReBuildOccDebugBuffer(JGraphicDevice* device, const size_t capacity, const std::vector<JUserPtr<JCullingInfo>>& cullingInfo)
	{
		if constexpr (!allowHzbDebug)
			return;

		if (!IsSameDevice(device))
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

		CreateCullingShader(device, objectCapacity, COMPUTE_TYPE::HZB_CULLING_PERSPECTIVE);
		CreateCullingShader(device, objectCapacity, COMPUTE_TYPE::HZB_CULLING_ORTHOLOGIC);
	}
	void JDx12HZBOccCulling::DestroyOccDebugBuffer(JCullingInfo* cullingInfo)
	{
		if constexpr (!allowHzbDebug)
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

		CreateSamplingShader(device, info, COMPUTE_TYPE::HZB_DOWN_SAMPLING);
	}
	void JDx12HZBOccCulling::StreamOutDebugInfo(const JUserPtr<JCullingInfo>& cullingInfo, const std::wstring& path)
	{
		if constexpr (!allowHzbDebug)
			return;

		if (cullingInfo == nullptr)
			return;

		int arrayIndex = cullingInfo->GetArrayIndex();
		if (arrayIndex == -1)
			return;

		_StreamOutDebugInfo(occDebugBuffer[arrayIndex].get(), path);
	}
	void JDx12HZBOccCulling::StreamOutDebugInfo(const std::wstring& path)
	{
		if constexpr (!allowHzbDebug)
			return;

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
		bool isPerspective = helper.UsePerspectiveProjection();

		if (helper.occCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
		{
			camNear = helper.cam->GetNear();
			camFar = helper.cam->GetFar();
		}
		else
		{
			camNear = helper.lit->GetFrustumNear();
			camFar = helper.lit->GetFrustumFar();
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
					isPerspective,
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

		const bool isPerspective = helper.UsePerspectiveProjection();
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
				occPassFrameIndex,
				isPerspective);
			OcclusionCulling(cmdList,
				dx12Frame,
				dx12Cm,
				dx12Gm->GetGpuSrvDescriptorHandle(occMipMapInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV)),
				occPassFrameIndex,
				helper.GetCullInterface(),
				isPerspective);
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
		const uint passCBIndex,
		const bool isPerspective)
	{
		const uint depthMapInfoCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JHzbOccDepthMapInfoConstants));
		commandList->SetComputeRootSignature(mRootSignature.Get());

		JDx12ComputeShaderDataHolder* copyShader = isPerspective ? shader[(uint)COMPUTE_TYPE::HZB_COPY_PERSPECTIVE].get() : shader[(uint)COMPUTE_TYPE::HZB_COPY_ORTHOLOGIC].get();
		commandList->SetPipelineState(copyShader->pso.Get());

		uint passCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JHzbOccComputeConstants));
		D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = dx12Frame->hzbOccReqCB->GetResource()->GetGPUVirtualAddress() + passCBIndex * passCBByteSize;

		commandList->SetComputeRootDescriptorTable(depthMapBuffIndex, depthMapSrvHandle);
		commandList->SetComputeRootDescriptorTable(lastMipMapInex, mipMapUavHandle);
		commandList->SetComputeRootConstantBufferView(depthMapInfoCBIndex, occDepthMapInfoCB->GetResource()->GetGPUVirtualAddress());
		commandList->SetComputeRootConstantBufferView(computePassCBIndex, passCBAddress);

		JVector3<uint> cgroupDim = copyShader->dispatchInfo.groupDim;
		commandList->Dispatch(cgroupDim.x, cgroupDim.y, cgroupDim.z);

		JDx12ComputeShaderDataHolder* downSampleShader = shader[(uint)COMPUTE_TYPE::HZB_DOWN_SAMPLING].get();
		commandList->SetPipelineState(downSampleShader->pso.Get());

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
			JVector3<uint> dgroupDim = downSampleShader->dispatchInfo.groupDim;
			commandList->Dispatch(dgroupDim.x, dgroupDim.y, dgroupDim.z);
		}
	}
	void JDx12HZBOccCulling::OcclusionCulling(ID3D12GraphicsCommandList* commandList,
		JDx12FrameResource* dx12Frame,
		JDx12CullingManager* dx12Cm,
		CD3DX12_GPU_DESCRIPTOR_HANDLE mipMapStHandle,
		const uint passCBIndex,
		const JCullingUserInterface& cullUser,
		const bool isPerspective)
	{
		uint passCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JHzbOccComputeConstants));
		D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = dx12Frame->hzbOccReqCB->GetResource()->GetGPUVirtualAddress() + passCBIndex * passCBByteSize;

		commandList->SetComputeRootDescriptorTable(mipMapBuffIndex, mipMapStHandle);
		commandList->SetComputeRootShaderResourceView(objectBuffIndex, dx12Frame->hzbOccObjectBuffer->GetResource()->GetGPUVirtualAddress());
		commandList->SetComputeRootUnorderedAccessView(queryResultIndex, occQueryOutBuffer->GetResource()->GetGPUVirtualAddress());
		commandList->SetComputeRootConstantBufferView(computePassCBIndex, passCBAddress);

		//Debug
		if constexpr (allowHzbDebug)
			occDebugBuffer[cullUser.GetArrayIndex(J_CULLING_TYPE::HZB_OCCLUSION)]->SettingCompute(commandList);

		JDx12ComputeShaderDataHolder* occShader = isPerspective ? shader[(uint)COMPUTE_TYPE::HZB_CULLING_PERSPECTIVE].get() : shader[(uint)COMPUTE_TYPE::HZB_CULLING_ORTHOLOGIC].get();
		commandList->SetPipelineState(occShader->pso.Get());

		JD3DUtility::ResourceTransition(commandList, occQueryOutBuffer->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		JVector3<uint> groupDim = occShader->dispatchInfo.groupDim;
		commandList->Dispatch(groupDim.x, groupDim.y, groupDim.z);

		auto resource = dx12Cm->GetResource(J_CULLING_TYPE::HZB_OCCLUSION, cullUser.GetArrayIndex(J_CULLING_TYPE::HZB_OCCLUSION));
		JD3DUtility::ResourceTransition(commandList, occQueryOutBuffer->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
		JD3DUtility::ResourceTransition(commandList, resource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		commandList->CopyResource(resource, occQueryOutBuffer->GetResource());
		JD3DUtility::ResourceTransition(commandList, resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
		JD3DUtility::ResourceTransition(commandList, occQueryOutBuffer->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);

		if constexpr (allowHzbDebug)
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
		if constexpr (allowHzbDebug)
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
		Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
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
	void JDx12HZBOccCulling::CreateSamplingShader(JGraphicDevice* device, const JGraphicInfo& info, const COMPUTE_TYPE type)
	{
		if (!IsSameDevice(device) || type == COMPUTE_TYPE::HZB_CULLING_ORTHOLOGIC || type == COMPUTE_TYPE::HZB_CULLING_PERSPECTIVE)
			return;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(device);
		shader[(uint)type] = std::make_unique<JDx12ComputeShaderDataHolder>();

		JComputeShaderInitData initData;
		StuffSamplingInitHelper(initData, type, info);

		CompileShader(shader[(uint)type].get(), initData, type);
		StuffPso(shader[(uint)type].get(), dx12Device, initData, type);
	}
	void JDx12HZBOccCulling::CreateCullingShader(JGraphicDevice* device, const uint objectCapacity, const COMPUTE_TYPE type)
	{
		if (!IsSameDevice(device) || (type != COMPUTE_TYPE::HZB_CULLING_ORTHOLOGIC && type != COMPUTE_TYPE::HZB_CULLING_PERSPECTIVE))
			return;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(device);
		shader[(uint)type] = std::make_unique<JDx12ComputeShaderDataHolder>();

		JComputeShaderInitData initData;
		StuffCullingInitHelper(initData, type, objectCapacity);

		CompileShader(shader[(uint)type].get(), initData, type);
		StuffPso(shader[(uint)type].get(), dx12Device, initData, type);
	}
	void JDx12HZBOccCulling::CompileShader(JDx12ComputeShaderDataHolder* holder, const JComputeShaderInitData& initData, const COMPUTE_TYPE type)
	{
		JShaderType::CompileInfo compileInfo = ComputeShaderCompileInfo(type); 
		auto macro = JDxShaderDataUtil::ToD3d12Macro(initData.macro);
		holder->cs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, macro.data(), compileInfo.functionName, "cs_5_1");
		holder->dispatchInfo = initData.dispatchInfo;
	}
	void JDx12HZBOccCulling::StuffPso(JDx12ComputeShaderDataHolder* holder, JDx12GraphicDevice* dx12Device, const JComputeShaderInitData& initData, const COMPUTE_TYPE type)
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