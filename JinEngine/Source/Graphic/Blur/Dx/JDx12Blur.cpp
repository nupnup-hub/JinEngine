#include"JDx12Blur.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h" 
#include"../../Shader/Dx/JDx12ShaderDataHolder.h" 
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../../Application/JApplicationEngine.h"
#include"../../../Core/Platform/JHardwareInfo.h"
#include"../../../Core/Math/JVectorExtend.h"

#define THREAD_DIM_X_SYMBOL "DIMX"
#define THREAD_DIM_Y_SYMBOL "DIMY"
#define THREAD_DIM_Z_SYMBOL "DIMZ"

#define USE_3x3_KERNEL "USE_3x3_KERNEL"
#define USE_5x5_KERNEL "USE_5x5_KERNEL"
#define USE_7x7_KERNEL "USE_7x7_KERNEL"

namespace JinEngine::Graphic
{
	namespace
	{
		static constexpr uint srcTextureHandleIndex = 0;
		static constexpr uint destTextureHandleIndex = srcTextureHandleIndex + 1;

		static constexpr uint blurInfoCBIndex = destTextureHandleIndex + 1;
		static constexpr uint kernelCBIndex = blurInfoCBIndex + 1;
		static constexpr uint slotCount = kernelCBIndex + 1;
	}
	namespace Private
	{
		static void StuffComputeShaderMacro(_Inout_ JComputeShaderInitData& initHelper, const J_BLUR_TYPE type, const J_BLUR_KENEL_SIZE size)
		{
			initHelper.macro.push_back({ THREAD_DIM_X_SYMBOL, std::to_string(initHelper.dispatchInfo.threadDim.x) });
			initHelper.macro.push_back({ THREAD_DIM_Y_SYMBOL, std::to_string(initHelper.dispatchInfo.threadDim.y) });
			initHelper.macro.push_back({ THREAD_DIM_Z_SYMBOL, std::to_string(initHelper.dispatchInfo.threadDim.z) });

			switch (size)
			{
			case JinEngine::Graphic::J_BLUR_KENEL_SIZE::_3x3:
				initHelper.macro.push_back({ USE_3x3_KERNEL, std::to_string(1) });
				break;
			case JinEngine::Graphic::J_BLUR_KENEL_SIZE::_5x5:
				initHelper.macro.push_back({ USE_5x5_KERNEL, std::to_string(1) });
				break;
			case JinEngine::Graphic::J_BLUR_KENEL_SIZE::_7x7:
				initHelper.macro.push_back({ USE_7x7_KERNEL, std::to_string(1) });
				break;
			default:
				break;
			}
		}
		static void StuffComputeShaderDispatchInfo(_Inout_ JComputeShaderInitData& initHelper,
			const JGraphicInfo& graphicInfo,
			const J_BLUR_TYPE type, 
			const J_BLUR_KENEL_SIZE size)
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
				return JShaderType::CompileInfo(L"Blur.hlsl", "BoxBlur");
			case J_BLUR_TYPE::GAUSIAAN:
				return JShaderType::CompileInfo(L"Blur.hlsl", "GausiaanBlur");
			default:
				return JShaderType::CompileInfo(L"Error", "Error");
			}
		}
	}
	namespace Private
	{
		static constexpr float PI = 3.1415926535f;
		static uint ConvertKenelSize(const J_BLUR_KENEL_SIZE kenelSize)
		{
			switch (kenelSize)
			{
			case JinEngine::Graphic::J_BLUR_KENEL_SIZE::_3x3:
				return 3;
			case JinEngine::Graphic::J_BLUR_KENEL_SIZE::_5x5:
				return 5;
			case JinEngine::Graphic::J_BLUR_KENEL_SIZE::_7x7:
				return 7;
			default:
				break;
			}
		} 
		static float Gaus(const int x, const int y, const float sig)
		{
			return std::exp(-(x * x + y * y) / (2.0f * sig * sig)) / (2.0f * PI * sig * sig);
		}
		template<int size, typename ...Param>
		static void BindKenel(ID3D12GraphicsCommandList* cmd, float(*calKenel)(int, int, Param...),Param... param)
		{ 
			constexpr static int radius = size / 2;
			float kenel[size][size];
			for (int i = -radius; i <= radius; ++i)
			{
				for (int j = -radius; j <= radius; ++j)
					kenel[i][j] = calKenel(i, j, param...);
			}
			cmd->SetGraphicsRoot32BitConstants(kernelCBIndex, size * size, &kenel, 0);
		}
	}

	void JDx12Blur::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;

		ID3D12Device* d3d12Device = static_cast<JDx12GraphicDevice*>(device)->GetDevice();
		BuildRootSignature(d3d12Device);
		BuildPso(d3d12Device, info);
	}
	void JDx12Blur::Clear()
	{
		mRootSignature = nullptr;
		for (uint i = 0; i < (uint)J_BLUR_TYPE::COUNT; ++i)
		{
			for (uint j = 0; j < (uint)J_BLUR_KENEL_SIZE::COUNT; ++j)
				blur[i][j] = nullptr;
		}
	}
	J_GRAPHIC_DEVICE_TYPE JDx12Blur::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	void JDx12Blur::ApplyBlur(JGraphicBlurTaskSet* taskSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(taskSet))
			return;

		JDx12GraphicBlurApplySet* blurSet = static_cast<JDx12GraphicBlurApplySet*>(taskSet);
		JVector2F invSize = 1.0f / blurSet->desc->imageSize;

		blurSet->cmdList->SetComputeRootSignature(mRootSignature.Get());
		blurSet->cmdList->SetComputeRootDescriptorTable(srcTextureHandleIndex, blurSet->srcHandle);
		blurSet->cmdList->SetComputeRootDescriptorTable(destTextureHandleIndex, blurSet->destHandle);
		blurSet->cmdList->SetComputeRoot32BitConstants(blurInfoCBIndex, 2, &blurSet->desc->imageSize, 0); 
		blurSet->cmdList->SetComputeRoot32BitConstants(blurInfoCBIndex, 2, &invSize, 2); 

		if (blurSet->desc->GetBlurType() == J_BLUR_TYPE::GAUSIAAN)
		{
			auto gausDesc = static_cast<JGausiaanBlurDesc*>(blurSet->desc.get());
			switch (gausDesc->kenelSize)
			{
			case JinEngine::Graphic::J_BLUR_KENEL_SIZE::_3x3:
			{
				Private::BindKenel<3>(blurSet->cmdList, &Private::Gaus, gausDesc->sigma);
				break;
			}
			case JinEngine::Graphic::J_BLUR_KENEL_SIZE::_5x5:
			{
				Private::BindKenel<5>(blurSet->cmdList, &Private::Gaus, gausDesc->sigma);
				break;
			}
			case JinEngine::Graphic::J_BLUR_KENEL_SIZE::_7x7:
			{
				Private::BindKenel<7>(blurSet->cmdList, &Private::Gaus, gausDesc->sigma);
				break;
			}
			default:
				break;
			}
		}

		auto shaderData = blur[(uint)blurSet->desc->GetBlurType()][(uint)blurSet->desc->kenelSize].get();
		auto groupDim = shaderData->dispatchInfo.groupDim;

		blurSet->cmdList->SetPipelineState(shaderData->pso.Get());
		blurSet->cmdList->Dispatch(groupDim.x, groupDim.y, groupDim.z);
	}
	void JDx12Blur::BuildRootSignature(ID3D12Device* device)
	{ 
		CD3DX12_DESCRIPTOR_RANGE srcTable;
		srcTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_DESCRIPTOR_RANGE destTable;
		destTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		CD3DX12_ROOT_PARAMETER slotRootParameter[slotCount];
		slotRootParameter[srcTextureHandleIndex].InitAsDescriptorTable(1, &srcTable);
		slotRootParameter[destTextureHandleIndex].InitAsDescriptorTable(1, &destTable);
		slotRootParameter[blurInfoCBIndex].InitAsConstantBufferView(0);
		slotRootParameter[kernelCBIndex].InitAsConstantBufferView(1);

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(slotCount, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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
			IID_PPV_ARGS(mRootSignature.GetAddressOf())));

		mRootSignature->SetName(L"Blur RootSignature");
	}
	void JDx12Blur::BuildPso(ID3D12Device* device, const JGraphicInfo& info)
	{
		for (uint i = 0; i < (uint)J_BLUR_TYPE::COUNT; ++i)
		{
			for (uint j = 0; j < (uint)J_BLUR_KENEL_SIZE::COUNT; ++j)
			{
				const J_BLUR_TYPE type = (J_BLUR_TYPE)i;
				const J_BLUR_KENEL_SIZE size = (J_BLUR_KENEL_SIZE)j;

				blur[i][j] = std::make_unique<JDx12ComputeShaderDataHolder>();
				JDx12ComputeShaderDataHolder* holder = blur[i][j].get();

				JComputeShaderInitData initData;
				Private::StuffComputeShaderDispatchInfo(initData, info, type, size);
				Private::StuffComputeShaderMacro(initData, type, size);

				auto macro = JDxShaderDataUtil::ToD3d12Macro(initData.macro);
				auto compileInfo = Private::ComputeShaderCompileInfo(type);

				holder->cs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, macro.data(), compileInfo.functionName, "cs_5_1");
				holder->dispatchInfo = initData.dispatchInfo;

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
				ThrowIfFailedG(device->CreateComputePipelineState(&newShaderPso, IID_PPV_ARGS(holder->pso.GetAddressOf())));
			}	 
		}
	}
}