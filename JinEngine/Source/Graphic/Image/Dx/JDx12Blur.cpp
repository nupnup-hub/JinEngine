#include"JDx12Blur.h"
#include"../JImageProcessingFilter.h"
#include"../../Command/Dx/JDx12CommandContext.h"
#include"../../Device/Dx/JDx12GraphicDevice.h"
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceInfo.h"  
#include"../../Utility/Dx/JDx12ObjectCreation.h" 
#include"../../JGraphicUpdateHelper.h"  
#include"../../../Core/Math/JVectorExtend.h" 
#include"../../../Core/Math/JMathHelper.h" 
//#include"../../../Develop/Debug/JDevelopDebug.h"
 
#define THREAD_COUNT L"THREAD_COUNT"
#define THREAD_X_COUNT L"THREAD_X_COUNT"
#define THREAD_Y_COUNT L"THREAD_Y_COUNT"

#define USE_KERNEL_DATA L"USE_KERNEL_DATA"
#define USE_2x2_KERNEL L"USE_2x2_KERNEL"
#define USE_3x3_KERNEL L"USE_3x3_KERNEL"
#define USE_5x5_KERNEL L"USE_5x5_KERNEL"
#define USE_7x7_KERNEL L"USE_7x7_KERNEL"
#define KERNEL_MAX_SIZE L"KERNEL_MAX_SIZE"
#define KERNEL_COUNT_IS_EVEN_NUMBER L"KERNEL_COUNT_IS_EVEN_NUMBER"
   
#define VERTIVAL_SYMBOL L"VERTIVAL_SYMBOL"
#define HORIZONTAL_SYMBOL L"HORIZONTAL_SYMBOL"

#define BOX_BLUR_SYMBOL L"BOX_BLUR"
#define GAUS_BLUR_SYMBOL L"GAUS_BLUR"
#define KAISER_BLUR_SYMBOL L"KAISER_BLUR"

namespace JinEngine::Graphic
{
	namespace Blur
	{
		static constexpr uint srcTextureHandleIndex = 0;
		static constexpr uint destTextureHandleIndex = srcTextureHandleIndex + 1;
		static constexpr uint blurPassCBIndex = destTextureHandleIndex + 1; 
		static constexpr uint slotCount = blurPassCBIndex + 1;
		static constexpr uint threadDimX = 32;
		static constexpr uint threadDimY = 16;

		static constexpr bool useKernelData = true;
		static constexpr uint cb32BitCount = (useKernelData ? JDx12Blur::constantsSize : JDx12Blur::constantsExceptKenelSize) / 4;
	} 
	namespace Private
	{
		static void StuffComputeShaderMacro(_Inout_ JComputeShaderInitData& initHelper)
		{
			initHelper.PushThreadDimensionMacro();
			initHelper.macro.push_back({ THREAD_COUNT, std::to_wstring(1) });
		}
		static void StuffComputeShaderMacro(_Inout_ JComputeShaderInitData& initHelper,
			const J_BLUR_TYPE blurType,
			const J_KERNEL_SIZE size,
			const bool useKenealCB,
			const bool isVertical,
			const size_t kenelMaxSize = JKenelType::MaxSize())
		{
			StuffComputeShaderMacro(initHelper);
			initHelper.macro.push_back({ KERNEL_MAX_SIZE, std::to_wstring(kenelMaxSize) });
			switch (blurType)
			{
			case JinEngine::Graphic::J_BLUR_TYPE::BOX:
				initHelper.macro.push_back({ BOX_BLUR_SYMBOL, std::to_wstring(kenelMaxSize) });
				break;
			case JinEngine::Graphic::J_BLUR_TYPE::GAUSIAAN:
				initHelper.macro.push_back({ GAUS_BLUR_SYMBOL, std::to_wstring(kenelMaxSize) });
				break;
			case JinEngine::Graphic::J_BLUR_TYPE::KAISER:
				initHelper.macro.push_back({ KAISER_BLUR_SYMBOL, std::to_wstring(kenelMaxSize) });
				break; 
			default:
				break;
			}
			switch (size)
			{
			case JinEngine::Graphic::J_KERNEL_SIZE::_3x3:
				initHelper.macro.push_back({ USE_3x3_KERNEL, std::to_wstring(1) });
				break;
			case JinEngine::Graphic::J_KERNEL_SIZE::_5x5:
				initHelper.macro.push_back({ USE_5x5_KERNEL, std::to_wstring(1) });
				break;
			case JinEngine::Graphic::J_KERNEL_SIZE::_7x7:
				initHelper.macro.push_back({ USE_7x7_KERNEL, std::to_wstring(1) });
				break;
			default:
				break;
			}
			if (isVertical)
				initHelper.macro.push_back({ VERTIVAL_SYMBOL, std::to_wstring(1) });
			else
				initHelper.macro.push_back({ HORIZONTAL_SYMBOL, std::to_wstring(1) });
			if (useKenealCB)
				initHelper.macro.push_back({ USE_KERNEL_DATA, std::to_wstring(1) });
		}
		static JCompileInfo ComputeShaderCompileInfo(const J_BLUR_TYPE type)
		{
			return JCompileInfo(L"Blur.hlsl", L"Blur");
		} 
	 
		static uint CalIndex(const J_BLUR_TYPE blurType, const J_KERNEL_SIZE size, bool isVertical)
		{
			return (uint)blurType +
				(uint)size * (uint)J_BLUR_TYPE::COUNT +
				isVertical * (uint)J_BLUR_TYPE::COUNT * (uint)J_KERNEL_SIZE::COUNT;
		}
	}
	  
	JDx12Blur::JDx12Blur()
		:guid(Core::MakeGuid())
	{}
	JDx12Blur::~JDx12Blur()
	{
		ClearResource();
	}
	void JDx12Blur::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;
		  
		BuildResource(device, gM);
	}
	void JDx12Blur::Clear()
	{ 
		ClearResource();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12Blur::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	void JDx12Blur::ApplyBlur(JGraphicBlurComputeSet* computeSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(computeSet))
			return;

		JDx12GraphicBlurComputeSet* blurSet = static_cast<JDx12GraphicBlurComputeSet*>(computeSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(blurSet->context);

		JVector2F size = blurSet->desc->imageSize;
		JVector2F invSize = 1.0f / blurSet->desc->imageSize;
		int mipLevel = blurSet->desc->mipLevel;
		int blurCount = blurSet->desc->tryBlurSubResourcr ? blurSet->desc->blurCount : 1;

		CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle = blurSet->srcHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle = blurSet->destHandle;
		uint srvSize = context->GetDescriptorSize(J_GRAPHIC_BIND_TYPE::SRV);
		uint uavSize = context->GetDescriptorSize(J_GRAPHIC_BIND_TYPE::UAV);
		float sharpness = 0;

		context->SetComputeRootSignature(blurRootSignature.Get());
		if (blurSet->desc->GetBlurType() == J_BLUR_TYPE::GAUSIAAN)
		{
			auto gausDesc = static_cast<JGaussianBlurDesc*>(blurSet->desc.get());
			sharpness = gausDesc->sigma; 
		}
		else if (blurSet->desc->GetBlurType() == J_BLUR_TYPE::KAISER)
		{
			auto kaiserDesc = static_cast<JKaiserBlurDesc*>(blurSet->desc.get());
			sharpness = kaiserDesc->beta;		 
		}	 
		BlurConstants constants;
		if constexpr(Blur::useKernelData)
		{
			if (blurSet->desc->GetBlurType() == J_BLUR_TYPE::GAUSIAAN)
			{
				auto gausDesc = static_cast<JGaussianBlurDesc*>(blurSet->desc.get()); 
				switch (gausDesc->kernelSize)
				{
				case JinEngine::Graphic::J_KERNEL_SIZE::_3x3:
				{
					JFilter::ComputeFilter<3, JKenelType::MaxSize()>(constants.kernel, &JFilter::Gaus1D, gausDesc->sigma);
					break;
				}
				case JinEngine::Graphic::J_KERNEL_SIZE::_5x5:
				{
					JFilter::ComputeFilter<5, JKenelType::MaxSize()>(constants.kernel, &JFilter::Gaus1D, gausDesc->sigma);
					break;
				}
				case JinEngine::Graphic::J_KERNEL_SIZE::_7x7:
				{
					JFilter::ComputeFilter<7, JKenelType::MaxSize()>(constants.kernel, &JFilter::Gaus1D, gausDesc->sigma);
					break;
				}
				default:
					break;
				}
			}
			else if (blurSet->desc->GetBlurType() == J_BLUR_TYPE::KAISER)
			{
				auto kaiserDesc = static_cast<JKaiserBlurDesc*>(blurSet->desc.get()); 
				switch (kaiserDesc->kernelSize)
				{
				case JinEngine::Graphic::J_KERNEL_SIZE::_3x3:
				{
					JFilter::ComputeFilter<3, JKenelType::MaxSize()>(constants.kernel, &JFilter::Kaiser1D, kaiserDesc->beta, 3);
					break;
				}
				case JinEngine::Graphic::J_KERNEL_SIZE::_5x5:
				{
					JFilter::ComputeFilter<5, JKenelType::MaxSize()>(constants.kernel, &JFilter::Kaiser1D, kaiserDesc->beta, 5);
					break;
				}
				case JinEngine::Graphic::J_KERNEL_SIZE::_7x7:
				{
					JFilter::ComputeFilter<7, JKenelType::MaxSize()>(constants.kernel, &JFilter::Kaiser1D, kaiserDesc->beta, 7);
					break;
				}
				default:
					break;
				}
			}
			else if (blurSet->desc->GetBlurType() == J_BLUR_TYPE::BOX)
			{
				auto boxDesc = static_cast<JBoxBlurDesc*>(blurSet->desc.get());
				switch (boxDesc->kernelSize)
				{
				case JinEngine::Graphic::J_KERNEL_SIZE::_3x3:
				{
					JFilter::ComputeFilter<3, JKenelType::MaxSize()>(constants.kernel, &JFilter::Box1D, 3);
					break;
				}
				case JinEngine::Graphic::J_KERNEL_SIZE::_5x5:
				{
					JFilter::ComputeFilter<5, JKenelType::MaxSize()>(constants.kernel, &JFilter::Box1D, 5);
					break;
				}
				case JinEngine::Graphic::J_KERNEL_SIZE::_7x7:
				{
					JFilter::ComputeFilter<7, JKenelType::MaxSize()>(constants.kernel, &JFilter::Box1D, 7);
					break;
				}
				default:
					break;
				}
			} 
			//context->SetComputeRootConstantBufferView(Blur::kernelCBIndex, kenelCB.get(), helper.info.frame.currIndex);
		}

		auto srcSet = context->ComputeSet(blurSet->srcInfo);
		auto destSet = context->ComputeSet(blurSet->destInfo);
		D3D12_RESOURCE_STATES srcBefore = (D3D12_RESOURCE_STATES)-1;
		D3D12_RESOURCE_STATES destBefore = (D3D12_RESOURCE_STATES)-1;

		if (srcSet.IsValid() && destSet.IsValid())
		{
			srcBefore = srcSet.holder->GetState();
			destBefore = destSet.holder->GetState();
			context->Transition(srcSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			context->Transition(destSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true); 
		}
		for (uint i = 0; i < blurSet->desc->blurCount; ++i)
		{
			constants.size = size;
			constants.invSize = invSize;
			constants.mipLevel = mipLevel;
			constants.sharpness = sharpness;

			context->SetComputeRootDescriptorTable(Blur::srcTextureHandleIndex, srcHandle);
			context->SetComputeRootDescriptorTable(Blur::destTextureHandleIndex, destHandle);
			context->SetComputeRoot32BitConstants(Blur::blurPassCBIndex, 0, constants, Blur::cb32BitCount);

			const uint verticalIndex = Private::CalIndex(blurSet->desc->GetBlurType(), blurSet->desc->kernelSize, 0);
			const uint horizontalIndex = Private::CalIndex(blurSet->desc->GetBlurType(), blurSet->desc->kernelSize, 1);

			auto shaderData = blur[verticalIndex].get();
			auto threadDim = shaderData->dispatchInfo.threadDim;

			//vertical
			context->SetPipelineState(shaderData);
			context->Dispatch2D(size, threadDim.XY());

			//horizontal
			shaderData = blur[horizontalIndex].get();
			context->SetPipelineState(shaderData);
			context->Dispatch2D(size, threadDim.XY());

			srcHandle.Offset(srvSize);
			destHandle.Offset(uavSize);
			size *= 0.5f;
			invSize = 1.0f / size;
			++mipLevel;
		}

		if (srcSet.IsValid() && destSet.IsValid())
		{
			context->Transition(srcSet.holder, srcBefore);
			context->Transition(destSet.holder, destBefore);
			context->InsertUAVBarrier(destSet.holder);
		}
	}
	void JDx12Blur::BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(gM);
		ID3D12Device* d3d12Device = dx12Device->GetDevice();

		BuildRootSingnature(d3d12Device, GetGraphicInfo(), GetGraphicOption());
		BuildPso(d3d12Device, GetGraphicInfo(), GetGraphicOption()); 
	}
	void JDx12Blur::BuildRootSingnature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		BuildBlurRootSignature(device); 
	}
	void JDx12Blur::BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		BuildBlurPso(device, info); 
	} 
	void JDx12Blur::BuildBlurRootSignature(ID3D12Device* device)
	{ 
		JDx12RootSignatureBuilder2<Blur::slotCount, 1> builder;
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); 
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		builder.PushConstants(Blur::cb32BitCount, 0);
		//builder.PushConstantsBuffer(1);
		builder.PushSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		builder.Create(device, L"BlurRootSignature", blurRootSignature.GetAddressOf()); 
	}
	void JDx12Blur::BuildBlurPso(ID3D12Device* device, const JGraphicInfo& info)
	{
		for (uint i = 0; i < (uint)J_BLUR_TYPE::COUNT; ++i)
		{
			for (uint j = 0; j < (uint)J_KERNEL_SIZE::COUNT; ++j)
			{
				for (uint k = 0; k < 2; ++k)
				{
					const J_BLUR_TYPE type = (J_BLUR_TYPE)i;
					const J_KERNEL_SIZE size = (J_KERNEL_SIZE)j;
					const uint index = Private::CalIndex(type, size, k);
					blur[index] = std::make_unique<JDx12ComputeShaderDataHolder>();
					JDx12ComputeShaderDataHolder* holder = blur[index].get();

					JComputeShaderInitData initData;
					initData.dispatchInfo.threadDim = JVector3F(Blur::threadDimX, Blur::threadDimY, 1);
					Private::StuffComputeShaderMacro(initData, type, size, Blur::useKernelData, (bool)k);

					// type != J_BLUR_TYPE::BOX
					auto compileInfo = Private::ComputeShaderCompileInfo(type);
					holder->cs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro, compileInfo.functionName, L"cs_6_0");
					holder->dispatchInfo = initData.dispatchInfo;

					JDx12ComputePso::Create(device, holder, blurRootSignature.Get());
				}
			}
		}
	}
	void JDx12Blur::ClearResource()
	{
		ClearRootSignature();
		ClearPso(); 
	}
	void JDx12Blur::ClearRootSignature()
	{
		blurRootSignature = nullptr;
	}
	void JDx12Blur::ClearPso()
	{ 
		for (uint i = 0; i < SIZE_OF_ARRAY(blur); ++i)
			blur[i] = nullptr;
	} 
} 