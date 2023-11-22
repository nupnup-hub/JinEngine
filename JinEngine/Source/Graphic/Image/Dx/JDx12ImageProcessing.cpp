#include"JDx12ImageProcessing.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceInfo.h" 
#include"../../Shader/Dx/JDx12ShaderDataHolder.h" 
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../FrameResource/JCameraConstants.h" 
#include"../../FrameResource/Dx/JDx12FrameResource.h" 
#include"../../JGraphicUpdateHelper.h"
#include"../../../Application/JApplicationEngine.h"
#include"../../../Core/Platform/JHardwareInfo.h"
#include"../../../Core/Math/JVectorExtend.h"
#include"../../../Object/Component/Camera/JCamera.h"
#include"../../../Object/Resource/JResourceManager.h"
#include"../../../Object/Resource/Texture/JTexture.h"
#include"../../../Develop/Debug/JDevelopDebug.h"

#define THREAD_DIM_X_SYMBOL L"DIMX"
#define THREAD_DIM_Y_SYMBOL L"DIMY"
#define THREAD_DIM_Z_SYMBOL L"DIMZ"

#define THREAD_COUNT L"THREAD_COUNT"
#define THREAD_X_COUNT L"THREAD_X_COUNT"
#define THREAD_Y_COUNT L"THREAD_Y_COUNT"

#define USE_2x2_KERNEL L"USE_2x2_KERNEL"
#define USE_3x3_KERNEL L"USE_3x3_KERNEL"
#define USE_5x5_KERNEL L"USE_5x5_KERNEL"
#define USE_7x7_KERNEL L"USE_7x7_KERNEL"

#define WIDTH_HEIGHT_EVEN 1
#define WIDTH_EVEN_HEIGHT_ODD 2
#define WIDTH_ODD_HEIGHT_EVEN 3
#define WIDTH_HEIGHT_ODD 4

#define USE_SSAO L"USE_SSAO"
#define USE_HBAO L"USE_HBAO"

namespace JinEngine::Graphic
{
	namespace Blur
	{
		static constexpr uint srcTextureHandleIndex = 0;
		static constexpr uint destTextureHandleIndex = srcTextureHandleIndex + 1;
		static constexpr uint blurInfoCBIndex = destTextureHandleIndex + 1;
		static constexpr uint kernelCBIndex = blurInfoCBIndex + 1;
		static constexpr uint slotCount = kernelCBIndex + 1;
		static constexpr uint blurInfo32BitCount = 8;
		static constexpr uint kernel32BitCount = 7 * 7 + 3; 
	}
	namespace DownSample
	{
		static constexpr uint srcTextureHandleIndex = 0;
		static constexpr uint destTextureHandleIndex = srcTextureHandleIndex + 1;
		static constexpr uint downSampleInfoCBIndex = destTextureHandleIndex + 1;
		static constexpr uint kernelCBIndex = downSampleInfoCBIndex + 1;
		static constexpr uint slotCount = kernelCBIndex + 1;
		static constexpr uint  downSampleInfo32BitCount = 8;//5;
		static constexpr uint kernel32BitCount = 7 * 7 + 3; 
	}
	namespace Ssao
	{
		static constexpr uint depthMapIndex = 0;
		static constexpr uint normalMapIndex = depthMapIndex + 1;
		static constexpr uint randomMapIndex = normalMapIndex + 1;
		static constexpr uint aoMapindex = randomMapIndex + 1;

		static constexpr uint ssaoInfoCBIndex = aoMapindex + 1;
		static constexpr uint camCBIndex = ssaoInfoCBIndex + 1;
		static constexpr uint slotCount = camCBIndex + 1;
		static constexpr uint ssaoInfo32BitCount = 16;
		static constexpr uint initSampleCount = 16;
	}
	namespace Private
	{
		static void StuffComputeShaderMacro(_Inout_ JComputeShaderInitData& initHelper)
		{
			initHelper.macro.push_back({ THREAD_DIM_X_SYMBOL, std::to_wstring(initHelper.dispatchInfo.threadDim.x) });
			initHelper.macro.push_back({ THREAD_DIM_Y_SYMBOL, std::to_wstring(initHelper.dispatchInfo.threadDim.y) });
			initHelper.macro.push_back({ THREAD_DIM_Z_SYMBOL, std::to_wstring(initHelper.dispatchInfo.threadDim.z) });

			initHelper.macro.push_back({ THREAD_COUNT, std::to_wstring(1) });
			initHelper.macro.push_back({ THREAD_X_COUNT, std::to_wstring(initHelper.dispatchInfo.threadDim.x) });
			initHelper.macro.push_back({ THREAD_Y_COUNT, std::to_wstring(initHelper.dispatchInfo.groupDim.y) });
		}
		static void StuffComputeShaderMacro(_Inout_ JComputeShaderInitData& initHelper, const J_KENEL_SIZE size)
		{
			StuffComputeShaderMacro(initHelper);
			switch (size)
			{
			case JinEngine::Graphic::J_KENEL_SIZE::_2x2:
				initHelper.macro.push_back({ USE_2x2_KERNEL, std::to_wstring(1) });
				break;
			case JinEngine::Graphic::J_KENEL_SIZE::_3x3:
				initHelper.macro.push_back({ USE_3x3_KERNEL, std::to_wstring(1) });
				break;
			case JinEngine::Graphic::J_KENEL_SIZE::_5x5:
				initHelper.macro.push_back({ USE_5x5_KERNEL, std::to_wstring(1) });
				break;
			case JinEngine::Graphic::J_KENEL_SIZE::_7x7:
				initHelper.macro.push_back({ USE_7x7_KERNEL, std::to_wstring(1) });
				break;
			default:
				break;
			}
		}
		static void StuffComputeShaderMacro(_Inout_ JComputeShaderInitData& initHelper, const J_SSAO_TYPE type)
		{
			StuffComputeShaderMacro(initHelper);
			switch (type)
			{
			case J_SSAO_TYPE::DEFAULT:
			{
				initHelper.macro.push_back({ USE_SSAO, std::to_wstring(1) });
				break;
			}
			case J_SSAO_TYPE::HORIZON_BASED:
			{
				initHelper.macro.push_back({ USE_HBAO, std::to_wstring(1) });
				break;
			}
			default: 
				break;
			}
		}
		static void StuffComputeShaderDispatchInfo(_Inout_ JComputeShaderInitData& initHelper, const JGraphicInfo& graphicInfo)
		{
			using GpuInfo = Core::JHardwareInfo::GpuInfo;
			std::vector<GpuInfo> gpuInfo = Core::JHardwareInfo::GetGpuInfo();

			//수정필요 
			//thread per group factor가 하드코딩됨
			//이후 amd graphic info 추가와 동시에 수정할 예정
			uint warpFactor = gpuInfo[0].vendor == Core::J_GRAPHIC_VENDOR::AMD ? 64 : 32;
			uint groupDimX = (uint)std::ceil(512.0f / float(gpuInfo[0].maxThreadsDim.x));
			uint groupDimY = 512;

			//textuer size is always 2 squared
			uint threadDimX = 512;
			uint threadDimY = (uint)std::ceil(512.0f / float(gpuInfo[0].maxGridDim.y));

			initHelper.dispatchInfo.threadDim = JVector3<uint>(threadDimX, threadDimY, 1);
			initHelper.dispatchInfo.groupDim = JVector3<uint>(groupDimX, groupDimY, 1);
			initHelper.dispatchInfo.taskOriCount = 512 * 512;
		}
		static JShaderType::CompileInfo ComputeShaderCompileInfo(const J_BLUR_TYPE type)
		{
			switch (type)
			{
			case J_BLUR_TYPE::BOX:
				return JShaderType::CompileInfo(L"Blur.hlsl", L"Blur");
			case J_BLUR_TYPE::GAUSIAAN:
				return JShaderType::CompileInfo(L"Blur.hlsl", L"Blur");
			default:
				return JShaderType::CompileInfo(L"Error", L"Error");
			}
		}
		static JShaderType::CompileInfo ComputeShaderCompileInfo(const J_DOWN_SAMPLING_TYPE type)
		{
			switch (type)
			{
			case J_DOWN_SAMPLING_TYPE::BOX:
				return JShaderType::CompileInfo(L"DownSampling.hlsl", L"DownSamplingUseBox");
			case J_DOWN_SAMPLING_TYPE::GAUSIAAN:
				return JShaderType::CompileInfo(L"DownSampling.hlsl", L"DownSamplingUseKernel");
			case J_DOWN_SAMPLING_TYPE::KAISER:
				return JShaderType::CompileInfo(L"DownSampling.hlsl", L"DownSamplingUseKernel");
			default:
				return JShaderType::CompileInfo(L"Error", L"Error");
			}
		}
		static JShaderType::CompileInfo ComputeShaderCompileInfo(const J_SSAO_TYPE type)
		{
			switch (type)
			{
			case J_SSAO_TYPE::DEFAULT:
				return JShaderType::CompileInfo(L"Ssao.hlsl", L"SSAO");
			case J_SSAO_TYPE::HORIZON_BASED:
				return JShaderType::CompileInfo(L"Ssao.hlsl", L"HBAO"); 
			default:
				return JShaderType::CompileInfo(L"Error", L"Error");
			}
		}
	}
	namespace Private
	{
		static constexpr float PI = 3.1415926535f;
		static uint ConvertKernelSize(const J_KENEL_SIZE kernelSize)
		{
			switch (kernelSize)
			{
			case JinEngine::Graphic::J_KENEL_SIZE::_3x3:
				return 3;
			case JinEngine::Graphic::J_KENEL_SIZE::_5x5:
				return 5;
			case JinEngine::Graphic::J_KENEL_SIZE::_7x7:
				return 7;
			default:
				break;
			}
		}
		static float Box(const int x, const int y, const int size)
		{
			return 1.0f / size;
		}
		static float Gaus(const int x, const int y, const float sig)
		{
			return std::exp(-(x * x + y * y) / (2.0f * sig * sig)) / (2.0f * PI * sig * sig);
		}
		/**
		* @param beta controll sharpness
		* @param N =  kenel(row or column) count - 1
		* @brief 2-D window function => w(n1, n2) = w1(n1)w2(n2)
		*/
		static float Kaiser(const int x, const int y, const float beta, const int N)
		{
			//https://en.wikipedia.org/wiki/Kaiser_window, https://en.wikipedia.org/wiki/Two-dimensional_window_design#Kaiser_window 
			float topX = std::cyl_bessel_i(0, beta * sqrt(1.0f - pow((x - (N / 2.0f)) / (N / 2.0f), 2)));
			float topY = std::cyl_bessel_i(0, beta * sqrt(1.0f - pow((y - (N / 2.0f)) / (N / 2.0f), 2)));
			float bottom = std::cyl_bessel_i(0, beta);
			return (topX * topY) / (bottom * bottom);
		}
		template<int size, bool zeroIsMiddle, typename ...Param >
		static void BindKernel(ID3D12GraphicsCommandList* cmd,
			int shaderKernelCBIndex,
			float(*calKernel)(int, int, Param...), Param... param)
		{
			constexpr static int radius = size / 2;
			float kenel[size][size];
			float sum = 0;
			int start = 0;
			int end = 0;
			int offset = 0;

			if constexpr (zeroIsMiddle)
			{
				if (size % 2)
				{
					start = -radius;
					end = radius;
				}
				else
				{
					start = -radius + 1;
					end = radius;
				}
				offset = -start;
			}
			else
				end = size - 1;

			for (int i = start; i <= end; ++i)
			{
				for (int j = start; j <= end; ++j)
				{
					kenel[i + offset][j + offset] = calKernel(i, j, param...);
					sum += kenel[i + offset][j + offset];
				}
			}
			for (int i = 0; i < size; ++i)
			{
				for (int j = 0; j < size; ++j)
					kenel[i][j] /= sum;
			}
			cmd->SetComputeRoot32BitConstants(shaderKernelCBIndex, size * size, &kenel, 0);
			/*
			loat dSum = 0;
			for (int i = 0; i < size; ++i)
			{
				std::string str;
				for (int j = 0; j < size; ++j)
				{
					str += std::to_string(kenel[i][j]) + " ";
					dSum += kenel[i][j];
				}
				Develop::JDevelopDebug::PushLog(str);
			}
			Develop::JDevelopDebug::PushLog("Sum: " + std::to_string(sum) + " DSum: " + std::to_string(dSum));
			Develop::JDevelopDebug::PushLog("Size: " + std::to_string(size));
			Develop::JDevelopDebug::Write();
			*/
		}
	}

	void JDx12ImageProcessing::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;

		ID3D12Device* d3d12Device = static_cast<JDx12GraphicDevice*>(device)->GetDevice();
		BuildBlurRootSignature(d3d12Device);
		BuildBlurPso(d3d12Device, info);

		BuildDownSampleRootSignature(d3d12Device);
		BuildDownSamplePso(d3d12Device, info);

		BuildSsaoRooutSignature(d3d12Device);
		BuildSsaoPso(d3d12Device, info);
	}
	void JDx12ImageProcessing::Clear()
	{
		blurRootSignature = nullptr;
		for (uint i = 0; i < (uint)J_BLUR_TYPE::COUNT; ++i)
		{
			for (uint j = 0; j < (uint)J_KENEL_SIZE::COUNT; ++j)
				blur[i][j] = nullptr;
		}

		downSampleRootSignature = nullptr;
		for (uint i = 0; i < (uint)J_DOWN_SAMPLING_TYPE::COUNT; ++i)
		{
			for (uint j = 0; j < (uint)J_KENEL_SIZE::COUNT; ++j)
				downSample[i][j] = nullptr;
		}

		ssaoRootSignature = nullptr;
		for (uint i = 0; i < (uint)J_SSAO_TYPE::COUNT; ++i)
			ssao[i] = nullptr;
	}
	J_GRAPHIC_DEVICE_TYPE JDx12ImageProcessing::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	void JDx12ImageProcessing::ApplyBlur(JGraphicBlurTaskSet* taskSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(taskSet))
			return;

		JDx12GraphicBlurApplySet* blurSet = static_cast<JDx12GraphicBlurApplySet*>(taskSet);
		JVector2F invSize = 1.0f / blurSet->desc->imageSize;

		blurSet->cmdList->SetComputeRootSignature(blurRootSignature.Get());
		blurSet->cmdList->SetComputeRootDescriptorTable(Blur::srcTextureHandleIndex, blurSet->srcHandle);
		blurSet->cmdList->SetComputeRootDescriptorTable(Blur::destTextureHandleIndex, blurSet->destHandle);
		blurSet->cmdList->SetComputeRoot32BitConstants(Blur::blurInfoCBIndex, 2, &blurSet->desc->imageSize, 0);
		blurSet->cmdList->SetComputeRoot32BitConstants(Blur::blurInfoCBIndex, 2, &invSize, 2);
		blurSet->cmdList->SetComputeRoot32BitConstants(Blur::blurInfoCBIndex, 1, &blurSet->desc->mipLevel, 4);

		if (blurSet->desc->GetBlurType() == J_BLUR_TYPE::BOX)
		{
			auto gausDesc = static_cast<JGaussianBlurDesc*>(blurSet->desc.get());
			switch (gausDesc->kernelSize)
			{
			case JinEngine::Graphic::J_KENEL_SIZE::_2x2:
			{
				Private::BindKernel<2, true>(blurSet->cmdList, Blur::kernelCBIndex, &Private::Box, 4);
				break;
			}
			case JinEngine::Graphic::J_KENEL_SIZE::_3x3:
			{
				Private::BindKernel<3, true>(blurSet->cmdList, Blur::kernelCBIndex, &Private::Box, 9);
				break;
			}
			case JinEngine::Graphic::J_KENEL_SIZE::_5x5:
			{
				Private::BindKernel<5, true>(blurSet->cmdList, Blur::kernelCBIndex, &Private::Box, 25);
				break;
			}
			case JinEngine::Graphic::J_KENEL_SIZE::_7x7:
			{
				Private::BindKernel<7, true>(blurSet->cmdList, Blur::kernelCBIndex, &Private::Box, 49);
				break;
			}
			default:
				break;
			}
		}
		else if (blurSet->desc->GetBlurType() == J_BLUR_TYPE::GAUSIAAN)
		{
			auto gausDesc = static_cast<JGaussianBlurDesc*>(blurSet->desc.get());
			switch (gausDesc->kernelSize)
			{
			case JinEngine::Graphic::J_KENEL_SIZE::_2x2:
			{
				Private::BindKernel<2, true>(blurSet->cmdList, Blur::kernelCBIndex, &Private::Gaus, gausDesc->sigma);
				break;
			}
			case JinEngine::Graphic::J_KENEL_SIZE::_3x3:
			{
				Private::BindKernel<3, true>(blurSet->cmdList, Blur::kernelCBIndex, &Private::Gaus, gausDesc->sigma);
				break;
			}
			case JinEngine::Graphic::J_KENEL_SIZE::_5x5:
			{
				Private::BindKernel<5, true>(blurSet->cmdList, Blur::kernelCBIndex, &Private::Gaus, gausDesc->sigma);
				break;
			}
			case JinEngine::Graphic::J_KENEL_SIZE::_7x7:
			{
				Private::BindKernel<7, true>(blurSet->cmdList, Blur::kernelCBIndex, &Private::Gaus, gausDesc->sigma);
				break;
			}
			default:
				break;
			}
		}

		auto shaderData = blur[(uint)blurSet->desc->GetBlurType()][(uint)blurSet->desc->kernelSize].get();
		auto groupDim = shaderData->dispatchInfo.groupDim;

		blurSet->cmdList->SetPipelineState(shaderData->pso.Get());
		blurSet->cmdList->Dispatch(groupDim.x, groupDim.y, groupDim.z);
	}
	void JDx12ImageProcessing::ApplyMipmapGeneration(JGraphicDownSampleTaskSet* taskSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(taskSet))
			return;

		JDx12GraphicDownSampleApplySet* downSamSet = static_cast<JDx12GraphicDownSampleApplySet*>(taskSet);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(downSamSet->graphicResourceM);

		downSamSet->cmdList->SetComputeRootSignature(downSampleRootSignature.Get());
		if (downSamSet->desc->GetDownSampleType() == J_DOWN_SAMPLING_TYPE::GAUSIAAN)
		{
			auto gausDesc = static_cast<JGaussianDownSampleDesc*>(downSamSet->desc.get());
			switch (gausDesc->kernelSize)
			{
			case JinEngine::Graphic::J_KENEL_SIZE::_2x2:
			{
				Private::BindKernel<2, true>(downSamSet->cmdList, Blur::kernelCBIndex, &Private::Gaus, gausDesc->sigma);
				break;
			}
			case JinEngine::Graphic::J_KENEL_SIZE::_3x3:
			{
				Private::BindKernel<3, true>(downSamSet->cmdList, DownSample::kernelCBIndex, &Private::Gaus, gausDesc->sigma);
				break;
			}
			case JinEngine::Graphic::J_KENEL_SIZE::_5x5:
			{
				Private::BindKernel<5, true>(downSamSet->cmdList, DownSample::kernelCBIndex, &Private::Gaus, gausDesc->sigma);
				break;
			}
			case JinEngine::Graphic::J_KENEL_SIZE::_7x7:
			{
				Private::BindKernel<7, true>(downSamSet->cmdList, DownSample::kernelCBIndex, &Private::Gaus, gausDesc->sigma);
				break;
			}
			default:
				break;
			}
		}
		else if (downSamSet->desc->GetDownSampleType() == J_DOWN_SAMPLING_TYPE::KAISER)
		{
			auto kaiserDesc = static_cast<JKaiserDownSampleDesc*>(downSamSet->desc.get());
			switch (kaiserDesc->kernelSize)
			{
			case JinEngine::Graphic::J_KENEL_SIZE::_2x2:
			{
				Private::BindKernel<2, true>(downSamSet->cmdList, Blur::kernelCBIndex, &Private::Kaiser, kaiserDesc->beta, 1);
				break;
			}
			case JinEngine::Graphic::J_KENEL_SIZE::_3x3:
			{
				Private::BindKernel<3, false>(downSamSet->cmdList, DownSample::kernelCBIndex, &Private::Kaiser, kaiserDesc->beta, 2);
				break;
			}
			case JinEngine::Graphic::J_KENEL_SIZE::_5x5:
			{
				Private::BindKernel<5, false>(downSamSet->cmdList, DownSample::kernelCBIndex, &Private::Kaiser, kaiserDesc->beta, 4);
				break;
			}
			case JinEngine::Graphic::J_KENEL_SIZE::_7x7:
			{
				Private::BindKernel<7, false>(downSamSet->cmdList, DownSample::kernelCBIndex, &Private::Kaiser, kaiserDesc->beta, 6);
				break;
			}
			default:
				break;
			}
		}

		auto shaderData = downSample[(uint)downSamSet->desc->GetDownSampleType()][(uint)downSamSet->desc->kernelSize].get();
		auto dispatchInfo = shaderData->dispatchInfo;
		auto dispatchDim = JVector2<uint>(dispatchInfo.groupDim.x * dispatchInfo.threadDim.x, dispatchInfo.groupDim.y * dispatchInfo.threadDim.y);
		downSamSet->cmdList->SetPipelineState(shaderData->pso.Get());

		CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle = downSamSet->srcHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle = downSamSet->destHandle;
		uint srvSize = dx12Gm->GetDescriptorSize(J_GRAPHIC_BIND_TYPE::SRV);
		uint uavSize = dx12Gm->GetDescriptorSize(J_GRAPHIC_BIND_TYPE::UAV);

		JVector2<uint> srcSize = downSamSet->desc->imageSize;
		JVector2<uint> destSize = srcSize / 2.0f;

		const uint mipLevelCount = downSamSet->desc->mipLevelCount;
		for (uint i = 1; i < mipLevelCount; ++i)
		{
			uint mipLevel = i - 1;
			uint xMod = srcSize.x % 2;
			uint yMod = srcSize.y % 2;
			uint samplingState = 0;

			if (xMod == 0 && yMod == 0)
				samplingState = WIDTH_HEIGHT_EVEN;
			else if (xMod == 0)
				samplingState = WIDTH_EVEN_HEIGHT_ODD;
			else if (yMod == 0)
				samplingState = WIDTH_ODD_HEIGHT_EVEN;
			else
				samplingState = WIDTH_HEIGHT_ODD;

			downSamSet->cmdList->SetComputeRootDescriptorTable(DownSample::srcTextureHandleIndex, srcHandle);
			downSamSet->cmdList->SetComputeRootDescriptorTable(DownSample::destTextureHandleIndex, destHandle);
			downSamSet->cmdList->SetComputeRoot32BitConstants(DownSample::downSampleInfoCBIndex, 2, &srcSize, 0);
			downSamSet->cmdList->SetComputeRoot32BitConstants(DownSample::downSampleInfoCBIndex, 2, &destSize, 2);
			downSamSet->cmdList->SetComputeRoot32BitConstants(DownSample::downSampleInfoCBIndex, 1, &mipLevel, 4);
			downSamSet->cmdList->SetComputeRoot32BitConstants(DownSample::downSampleInfoCBIndex, 1, &samplingState, 5);
			downSamSet->cmdList->SetComputeRoot32BitConstants(DownSample::downSampleInfoCBIndex, 2, &dispatchDim, 6);

			downSamSet->cmdList->Dispatch(dispatchInfo.groupDim.x, dispatchInfo.groupDim.y, dispatchInfo.groupDim.z);
			srcHandle.Offset(srvSize);
			destHandle.Offset(uavSize);
			srcSize /= 2;
			destSize /= 2;

			srcSize.Clamp(srcSize, JVector2F(1.0f, 1.0f), srcSize);
			destSize.Clamp(destSize, JVector2F(1.0f, 1.0f), destSize);
		}
	}
	void JDx12ImageProcessing::ApplySsao(JGraphicSsaoTaskSet* taskSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(taskSet) || helper.cam == nullptr)
			return;

		JDx12GraphicSsaoTaskSet* ssaoSet = static_cast<JDx12GraphicSsaoTaskSet*>(taskSet);
		JDx12GraphicResourceManager* dx12gm = static_cast<JDx12GraphicResourceManager*>(taskSet->graphicResourceM); 
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(taskSet->currFrame);

		auto gInterface = helper.cam->GraphicResourceUserInterface();	  
		uint width = gInterface.GetResourceWidth(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, 0);
		uint height = gInterface.GetResourceHeight(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, 0);
		JVector2<uint> size(width, height);
		JVector2F nearFar(helper.cam->GetNear(), helper.cam->GetFar());
		 
		int depthHeapIndex = gInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_BIND_TYPE::SRV, 0);
		int normalHeapIndex = gInterface.GetOptionHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_BIND_TYPE::SRV, J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP, 0);
		int aoHeapIndex = gInterface.GetOptionHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_BIND_TYPE::SRV, J_GRAPHIC_RESOURCE_OPTION_TYPE::AMBIENT_OCCLISION_MAP, 0);
		if (normalHeapIndex == invalidIndex || aoHeapIndex == invalidIndex)
			return;

		if (randomTexture == nullptr)
		{
			randomTexture = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::BLUE_NOISE);
			if (randomTexture == nullptr)
				return;
		}
		 
		JVector2F invTextureSize = 1.0f / JVector2F(randomTexture->GetTextureWidth(), randomTexture->GetTextureHeight());
		auto depthMapHandle = dx12gm->GetGpuSrvDescriptorHandle(depthHeapIndex);
		auto normalMapHandle = dx12gm->GetGpuSrvDescriptorHandle(normalHeapIndex);
		auto randomHandle = dx12gm->GetGpuSrvDescriptorHandle(randomTexture->GraphicResourceUserInterface().GetFirstResourceHeapStart(J_GRAPHIC_BIND_TYPE::SRV));
		auto aoMapHandle = dx12gm->GetGpuSrvDescriptorHandle(aoHeapIndex);

		const DirectX::XMMATRIX invProj = DirectX::XMMatrixInverse(nullptr, helper.cam->GetProj());
		//left-bottom, right-top
		const JVector4F ndcCoord[2]
		{
			JVector4F{-1.0f, -1.0f, 0.0f, 1.0f}, 
			JVector4F{1.0f, 1.0f, 0.0f, 1.0f}
		};
		JVector3F viewNearPlane[2];
		for (uint i = 0; i < 2; ++i)
		{
			JVector4F projP = DirectX::XMVector4Transform(ndcCoord[i].ToXmV(), invProj);
			viewNearPlane[i] = JVector3F(projP.xyz) / projP.w;
		}

		JSsaoDesc desc = helper.cam->GetSsaoDesc();
		JVector4F descVector;
		descVector.x = desc.radius;
		descVector.y = desc.fadeStart;
		descVector.z = desc.fadeEnd;
		descVector.w = desc.surfaceEpsilon;

		ssaoSet->cmdList->SetComputeRootSignature(ssaoRootSignature.Get());
		ssaoSet->cmdList->SetComputeRootDescriptorTable(Ssao::depthMapIndex, depthMapHandle);
		ssaoSet->cmdList->SetComputeRootDescriptorTable(Ssao::normalMapIndex, normalMapHandle);
		ssaoSet->cmdList->SetComputeRootDescriptorTable(Ssao::randomMapIndex, randomHandle);
		ssaoSet->cmdList->SetComputeRootDescriptorTable(Ssao::aoMapindex, aoMapHandle);
		ssaoSet->cmdList->SetComputeRoot32BitConstants(Ssao::ssaoInfoCBIndex, 4, &descVector, 0);
		ssaoSet->cmdList->SetComputeRoot32BitConstants(Ssao::ssaoInfoCBIndex, 6, &viewNearPlane, 4); 
		ssaoSet->cmdList->SetComputeRoot32BitConstants(Ssao::ssaoInfoCBIndex, 2, &nearFar, 10);
		ssaoSet->cmdList->SetComputeRoot32BitConstants(Ssao::ssaoInfoCBIndex, 2, &invTextureSize, 12);
		ssaoSet->cmdList->SetComputeRoot32BitConstants(Ssao::ssaoInfoCBIndex, 2, &size, 14);
		dx12Frame->cameraCB->SetComputeCBBufferView(ssaoSet->cmdList, Ssao::camCBIndex, helper.GetCamFrameIndex());

		auto shaderData = ssao[(uint)desc.ssaoType].get();
		auto groupDim = shaderData->dispatchInfo.groupDim;

		ssaoSet->cmdList->SetPipelineState(shaderData->pso.Get());
		ssaoSet->cmdList->Dispatch(groupDim.x, groupDim.y, groupDim.z);
	}
	void JDx12ImageProcessing::BuildBlurRootSignature(ID3D12Device* device)
	{
		CD3DX12_DESCRIPTOR_RANGE srcTable;
		srcTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_DESCRIPTOR_RANGE destTable;
		destTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		CD3DX12_ROOT_PARAMETER slotRootParameter[Blur::slotCount];
		slotRootParameter[Blur::srcTextureHandleIndex].InitAsDescriptorTable(1, &srcTable);
		slotRootParameter[Blur::destTextureHandleIndex].InitAsDescriptorTable(1, &destTable);
		slotRootParameter[Blur::blurInfoCBIndex].InitAsConstants(Blur::blurInfo32BitCount, 0);
		slotRootParameter[Blur::kernelCBIndex].InitAsConstants(Blur::kernel32BitCount, 1);

		CD3DX12_STATIC_SAMPLER_DESC sampler(0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(DownSample::slotCount, slotRootParameter, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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
			IID_PPV_ARGS(blurRootSignature.GetAddressOf())));

		blurRootSignature->SetName(L"Blur RootSignature");
	}
	void JDx12ImageProcessing::BuildBlurPso(ID3D12Device* device, const JGraphicInfo& info)
	{
		for (uint i = 0; i < (uint)J_BLUR_TYPE::COUNT; ++i)
		{
			for (uint j = 0; j < (uint)J_KENEL_SIZE::COUNT; ++j)
			{
				const J_BLUR_TYPE type = (J_BLUR_TYPE)i;
				const J_KENEL_SIZE size = (J_KENEL_SIZE)j;

				blur[i][j] = std::make_unique<JDx12ComputeShaderDataHolder>();
				JDx12ComputeShaderDataHolder* holder = blur[i][j].get();

				JComputeShaderInitData initData;
				Private::StuffComputeShaderDispatchInfo(initData, info);
				Private::StuffComputeShaderMacro(initData, size);

				auto compileInfo = Private::ComputeShaderCompileInfo(type);

				holder->cs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro, compileInfo.functionName, L"cs_6_0");
				holder->dispatchInfo = initData.dispatchInfo;

				D3D12_COMPUTE_PIPELINE_STATE_DESC newShaderPso;
				ZeroMemory(&newShaderPso, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
				newShaderPso.pRootSignature = blurRootSignature.Get();

				holder->RootSignature = newShaderPso.pRootSignature;
				newShaderPso.CS =
				{
					reinterpret_cast<BYTE*>(holder->cs->GetBufferPointer()),
					holder->cs->GetBufferSize()
				};
				newShaderPso.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
				ThrowIfFailedG(device->CreateComputePipelineState(&newShaderPso, IID_PPV_ARGS(holder->pso.GetAddressOf())));
			}
		}
	}
	void JDx12ImageProcessing::BuildDownSampleRootSignature(ID3D12Device* device)
	{
		CD3DX12_DESCRIPTOR_RANGE srcTable;
		srcTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_DESCRIPTOR_RANGE destTable;
		destTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		CD3DX12_ROOT_PARAMETER slotRootParameter[DownSample::slotCount];
		slotRootParameter[DownSample::srcTextureHandleIndex].InitAsDescriptorTable(1, &srcTable);
		slotRootParameter[DownSample::destTextureHandleIndex].InitAsDescriptorTable(1, &destTable);
		slotRootParameter[DownSample::downSampleInfoCBIndex].InitAsConstants(DownSample::downSampleInfo32BitCount, 0);
		slotRootParameter[DownSample::kernelCBIndex].InitAsConstants(DownSample::kernel32BitCount, 1);

		CD3DX12_STATIC_SAMPLER_DESC sampler(0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(DownSample::slotCount, slotRootParameter, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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
			IID_PPV_ARGS(downSampleRootSignature.GetAddressOf())));

		downSampleRootSignature->SetName(L"DownSample RootSignature");
	}
	void JDx12ImageProcessing::BuildDownSamplePso(ID3D12Device* device, const JGraphicInfo& info)
	{
		for (uint i = 0; i < (uint)J_DOWN_SAMPLING_TYPE::COUNT; ++i)
		{
			for (uint j = 0; j < (uint)J_KENEL_SIZE::COUNT; ++j)
			{
				const J_DOWN_SAMPLING_TYPE type = (J_DOWN_SAMPLING_TYPE)i;
				const J_KENEL_SIZE size = (J_KENEL_SIZE)j;

				downSample[i][j] = std::make_unique<JDx12ComputeShaderDataHolder>();
				JDx12ComputeShaderDataHolder* holder = downSample[i][j].get();

				JComputeShaderInitData initData;
				Private::StuffComputeShaderDispatchInfo(initData, info);
				Private::StuffComputeShaderMacro(initData, size);

				auto compileInfo = Private::ComputeShaderCompileInfo(type);

				holder->cs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro, compileInfo.functionName, L"cs_6_0");
				holder->dispatchInfo = initData.dispatchInfo;

				D3D12_COMPUTE_PIPELINE_STATE_DESC newShaderPso;
				ZeroMemory(&newShaderPso, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
				newShaderPso.pRootSignature = downSampleRootSignature.Get();

				holder->RootSignature = newShaderPso.pRootSignature;
				newShaderPso.CS =
				{
					reinterpret_cast<BYTE*>(holder->cs->GetBufferPointer()),
					holder->cs->GetBufferSize()
				};
				newShaderPso.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
				ThrowIfFailedG(device->CreateComputePipelineState(&newShaderPso, IID_PPV_ARGS(holder->pso.GetAddressOf())));
			}
		}
	}
	void JDx12ImageProcessing::BuildSsaoRooutSignature(ID3D12Device* device)
	{
		CD3DX12_DESCRIPTOR_RANGE depthMapTable;
		depthMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_DESCRIPTOR_RANGE normalMapTable;
		normalMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

		CD3DX12_DESCRIPTOR_RANGE randomMapTable;
		randomMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);

		CD3DX12_DESCRIPTOR_RANGE aoMapTable;
		aoMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		CD3DX12_ROOT_PARAMETER slotRootParameter[Ssao::slotCount];
		slotRootParameter[Ssao::depthMapIndex].InitAsDescriptorTable(1, &depthMapTable);
		slotRootParameter[Ssao::normalMapIndex].InitAsDescriptorTable(1, &normalMapTable);
		slotRootParameter[Ssao::randomMapIndex].InitAsDescriptorTable(1, &randomMapTable);
		slotRootParameter[Ssao::aoMapindex].InitAsDescriptorTable(1, &aoMapTable);
		slotRootParameter[Ssao::ssaoInfoCBIndex].InitAsConstants(16, 0);
		slotRootParameter[Ssao::camCBIndex].InitAsConstantBufferView(1);

		std::vector< CD3DX12_STATIC_SAMPLER_DESC> sam
		{
			CD3DX12_STATIC_SAMPLER_DESC(0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP), // addressW


			CD3DX12_STATIC_SAMPLER_DESC(1, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP) // addressW
		};

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(Ssao::slotCount, 
			slotRootParameter, 
			(uint)sam.size(), sam.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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
			IID_PPV_ARGS(ssaoRootSignature.GetAddressOf())));

		ssaoRootSignature->SetName(L"Ssao RootSignature");
	}
	void JDx12ImageProcessing::BuildSsaoPso(ID3D12Device* device, const JGraphicInfo& info)
	{
		for (uint i = 0; i < (uint)J_SSAO_TYPE::COUNT; ++i)
		{
			const J_SSAO_TYPE type = (J_SSAO_TYPE)i;

			ssao[i] = std::make_unique<JDx12ComputeShaderDataHolder>();
			JDx12ComputeShaderDataHolder* holder = ssao[i].get();

			JComputeShaderInitData initData;
			Private::StuffComputeShaderDispatchInfo(initData, info);
			Private::StuffComputeShaderMacro(initData, type);

			auto compileInfo = Private::ComputeShaderCompileInfo(type);

			holder->cs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro, compileInfo.functionName, L"cs_6_0");
			holder->dispatchInfo = initData.dispatchInfo;

			D3D12_COMPUTE_PIPELINE_STATE_DESC newShaderPso;
			ZeroMemory(&newShaderPso, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
			newShaderPso.pRootSignature = ssaoRootSignature.Get();

			holder->RootSignature = newShaderPso.pRootSignature;
			newShaderPso.CS =
			{
				reinterpret_cast<BYTE*>(holder->cs->GetBufferPointer()),
				holder->cs->GetBufferSize()
			};
			newShaderPso.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
			ThrowIfFailedG(device->CreateComputePipelineState(&newShaderPso, IID_PPV_ARGS(holder->pso.GetAddressOf())));
		}
	}
}