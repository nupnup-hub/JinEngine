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


#include"JDx12Ssao.h"
#include"JDx12ImageConstants.h"
#include"../JImageProcessingFilter.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceInfo.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceShareData.h" 
#include"../../Shader/Dx/JDx12ShaderDataHolder.h"
#include"../../Command/Dx/JDx12CommandContext.h"
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../Utility/Dx/JDx12ObjectCreation.h"
#include"../../FrameResource/JCameraConstants.h" 
#include"../../FrameResource/Dx/JDx12FrameResource.h" 
#include"../../JGraphicUpdateHelper.h"
#include"../../../Application/Engine/JApplicationEngine.h"
#include"../../../Core/Platform/JHardwareInfo.h"
#include"../../../Core/Math/JVectorExtend.h"
#include"../../../Object/Component/Camera/JCamera.h" 
#include"../../../Object/Resource/Texture/JTexture.h"  
#include<random>
//#include"../../../Develop/Debug/JDevelopDebug.h"
    
#define USE_2x2_KERNEL L"USE_2x2_KERNEL"
#define USE_3x3_KERNEL L"USE_3x3_KERNEL"
#define USE_5x5_KERNEL L"USE_5x5_KERNEL"
#define USE_7x7_KERNEL L"USE_7x7_KERNEL"
#define KERNEL_MAX_SIZE L"KERNEL_MAX_SIZE"
#define KERNEL_COUNT_IS_EVEN_NUMBER L"KERNEL_COUNT_IS_EVEN_NUMBER"

#define USE_SSAO L"USE_SSAO"
#define USE_HBAO L"USE_HBAO"
#define USE_BLUR L"USE_BLUR"

#define BLUR_X L"BLUR_X"
#define BLUR_Y L"BLUR_Y"

#define USE_KERNEL_DATA L"USE_KERNEL_DATA"
#define USE_SSAO_BIUR_BILATERAL L"USE_SSAO_BIUR_BILATERAL"
#define USE_TRIANGLE_VERTEX_DIR L"USE_TRIANGLE_VERTEX_DIR"

#define SSAO_SAMPLE_COUNT L"SSAO_SAMPLE_COUNT" 
#define SSAO_RANDOM_MAP_SIZE L"SSAO_RANDOM_MAP_SIZE" 
#define SSAO_DIR_COUNT L"SSAO_DIR_COUNT"
#define USE_SSAO_INTERLEAVE L"USE_SSAO_INTERLEAVE" 

namespace JinEngine::Graphic
{ 
	namespace Ssao
	{
		static constexpr uint passCBIndex = 0;
		static constexpr uint typePerCBIndex = passCBIndex + 1;
		static constexpr uint depthMapIndex = typePerCBIndex + 1;
		static constexpr uint normalMapIndex = depthMapIndex + 1;
		static constexpr uint randomMapIndex = normalMapIndex + 1;

		static constexpr uint ssaoSlotCount = randomMapIndex + 1;
		static constexpr uint hbaoSlotCount = randomMapIndex + 1;

		static constexpr uint sampleCount[(uint)J_SSAO_SAMPLE_TYPE::COUNT]
		{
			 Constants::ssaoMaxSampleCount / 8,
			 Constants::ssaoMaxSampleCount / 4,
			 Constants::ssaoMaxSampleCount / 2,
			 Constants::ssaoMaxSampleCount
		};
		static constexpr uint randomWidth = 4;
		static constexpr uint randomDirCount = 8;
		static constexpr uint mrtCount = 8;
		static constexpr uint splitCount = Constants::ssaoSplitCount;
		static constexpr uint sliceCount = Constants::ssaoSliceCount;
	}
	namespace SsaoBlur
	{
		static constexpr uint srcMapIndex = 0;
		static constexpr uint passCBIndex = srcMapIndex + 1;
		static constexpr uint slotCount = passCBIndex + 1;
	}
	namespace SsaoCombine
	{
		static constexpr uint aoMapIndex = 0;
		static constexpr uint depthMapIndex = aoMapIndex + 1;
		static constexpr uint passCBIndex = depthMapIndex + 1;
		static constexpr uint slotCount = passCBIndex + 1;
	}
	namespace SsaoDepthLinearlize
	{
		static constexpr uint depthMapIndex = 0;
		static constexpr uint passCBIndex = depthMapIndex + 1;
		static constexpr uint slotCount = passCBIndex + 1;
	}
	namespace SsaoDepthInterleave
	{
		static constexpr uint depthMapIndex = 0;
		static constexpr uint passCBIndex = depthMapIndex + 1;
		static constexpr uint sliceCBIndex = passCBIndex + 1;
		static constexpr uint slotCount = sliceCBIndex + 1;
	}
	namespace Private
	{ 
		static void StuffGraphicShaderMacro(_Inout_ JGraphicShaderInitData& initHelper,
			const J_KERNEL_SIZE size,
			const bool useKenealCB,
			const size_t kenelMaxSize = JKenelType::MaxSize())
		{
			initHelper.macro[0].push_back({ KERNEL_MAX_SIZE, std::to_wstring(kenelMaxSize) });
			switch (size)
			{
			case JinEngine::Graphic::J_KERNEL_SIZE::_3x3:
				initHelper.macro[0].push_back({ USE_3x3_KERNEL, std::to_wstring(1) });
				break;
			case JinEngine::Graphic::J_KERNEL_SIZE::_5x5:
				initHelper.macro[0].push_back({ USE_5x5_KERNEL, std::to_wstring(1) });
				break;
			case JinEngine::Graphic::J_KERNEL_SIZE::_7x7:
				initHelper.macro[0].push_back({ USE_7x7_KERNEL, std::to_wstring(1) });
				break;
			default:
				break;
			}
			if (useKenealCB)
				initHelper.macro[0].push_back({ USE_KERNEL_DATA, std::to_wstring(1) });
		}
		static void StuffGraphicShaderMacro(_Inout_ JGraphicShaderInitData& initHelper, const J_SSAO_FUNCTION function)
		{
			if (function == J_SSAO_FUNCTION::BLUR)
				initHelper.macro[0].push_back({ USE_BLUR, std::to_wstring(1) });
		}
		static void StuffGraphicShaderMacro(_Inout_ JGraphicShaderInitData& initHelper,
			const J_SSAO_TYPE type,
			const J_SSAO_SAMPLE_TYPE sampleType,
			const J_SSAO_FUNCTION function)
		{
			switch (type)
			{
			case J_SSAO_TYPE::DEFAULT:
			{
				initHelper.macro[0].push_back({ USE_SSAO, std::to_wstring(1) });
				break;
			}
			case J_SSAO_TYPE::HORIZON_BASED:
			{
				initHelper.macro[0].push_back({ USE_HBAO, std::to_wstring(1) });
				break;
			}
			default:
				break;
			}

			StuffGraphicShaderMacro(initHelper, function);
			initHelper.macro[0].push_back({ SSAO_SAMPLE_COUNT, std::to_wstring(Ssao::sampleCount[(uint)sampleType]) });
			initHelper.macro[0].push_back({ SSAO_RANDOM_MAP_SIZE, std::to_wstring(Ssao::randomWidth) });
			initHelper.macro[0].push_back({ SSAO_DIR_COUNT, std::to_wstring(Ssao::randomDirCount) });
		}
		static void StuffGraphicShaderMacro(_Inout_ JGraphicShaderInitData& initHelper, const J_SSAO_BLUR_SHADER type, const J_KERNEL_SIZE size)
		{
			StuffGraphicShaderMacro(initHelper, size, true);
			switch (type)
			{
			case J_SSAO_BLUR_SHADER::BILATERAL_X:
			{
				initHelper.macro[0].push_back({ USE_SSAO_BIUR_BILATERAL, std::to_wstring(1) });
				initHelper.macro[0].push_back({ BLUR_X, std::to_wstring(1) });
				break;
			}
			case J_SSAO_BLUR_SHADER::BILATERAL_Y:
			{
				initHelper.macro[0].push_back({ USE_SSAO_BIUR_BILATERAL, std::to_wstring(1) });
				initHelper.macro[0].push_back({ BLUR_Y, std::to_wstring(1) });
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
		static JCompileInfo ComputeShaderCompileInfo(const J_BLUR_TYPE type)
		{
			switch (type)
			{
			case J_BLUR_TYPE::BOX:
				return JCompileInfo(L"Blur.hlsl", L"Blur");
			case J_BLUR_TYPE::GAUSIAAN:
				return JCompileInfo(L"Blur.hlsl", L"Blur");
			default:
				return JCompileInfo(L"Error", L"Error");
			}
		}
		static JCompileInfo ComputeShaderCompileInfo(const J_DOWN_SAMPLING_TYPE type)
		{
			switch (type)
			{
			case J_DOWN_SAMPLING_TYPE::BOX:
				return JCompileInfo(L"DownSampling.hlsl", L"DownSamplingUseBox");
			case J_DOWN_SAMPLING_TYPE::GAUSIAAN:
				return JCompileInfo(L"DownSampling.hlsl", L"DownSamplingUseKernel");
			case J_DOWN_SAMPLING_TYPE::KAISER:
				return JCompileInfo(L"DownSampling.hlsl", L"DownSamplingUseKernel");
			default:
				return JCompileInfo(L"Error", L"Error");
			}
		}
		static JCompileInfo GraphicShaderCompileInfo(const J_SSAO_TYPE type)
		{
			switch (type)
			{
			case J_SSAO_TYPE::DEFAULT:
				return JCompileInfo(L"SsaoPs.hlsl", L"SsaoPs");
			case J_SSAO_TYPE::HORIZON_BASED:
				return JCompileInfo(L"SsaoPs.hlsl", L"HbaoPs");
			default:
				return JCompileInfo(L"Error", L"Error");
			}
		}
		static JCompileInfo GraphicShaderCompileInfo(const J_SSAO_BLUR_SHADER type)
		{
			switch (type)
			{
			case J_SSAO_BLUR_SHADER::BILATERAL_X:
				return JCompileInfo(L"SsaoBlur.hlsl", L"BilateralBlurX");
			case J_SSAO_BLUR_SHADER::BILATERAL_Y:
				return JCompileInfo(L"SsaoBlur.hlsl", L"BilateralBlurY");
			default:
				return JCompileInfo(L"Error", L"Error");
			}
		}
		static std::vector<D3D12_INPUT_ELEMENT_DESC> SsaoInputLayout()
		{
			return
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};
		}
		static uint CalIndex(const J_SSAO_TYPE type, const J_SSAO_SAMPLE_TYPE sampleType, const J_SSAO_FUNCTION function)noexcept
		{
			constexpr uint yOffset = (uint)J_SSAO_TYPE::COUNT;
			constexpr uint zOffset = yOffset * (uint)J_SSAO_SAMPLE_TYPE::COUNT;
			return (uint)type + (uint)sampleType * yOffset + (uint)function * zOffset;
		}
		static uint CalIndex(const J_SSAO_TYPE type, const J_SSAO_SAMPLE_TYPE sampleType, const J_SSAO_FUNCTION function, const bool canUseInterleave)noexcept
		{
			constexpr uint yOffset = (uint)J_SSAO_TYPE::COUNT;
			constexpr uint zOffset = yOffset * (uint)J_SSAO_SAMPLE_TYPE::COUNT;
			return (uint)type + (uint)sampleType * yOffset + (canUseInterleave ? 0 : (uint)function) * zOffset;
		}
		std::vector<float> GetRandomNumberVec()
		{
			return std::vector<float>{
				0.463937f,0.340042f,0.223035f,0.468465f,0.322224f,0.979269f,0.031798f,0.973392f,0.778313f,0.456168f,0.258593f,0.330083f,0.387332f,0.380117f,0.179842f,0.910755f,
				0.511623f,0.092933f,0.180794f,0.620153f,0.101348f,0.556342f,0.642479f,0.442008f,0.215115f,0.475218f,0.157357f,0.568868f,0.501241f,0.629229f,0.699218f,0.707733f,
				0.556725f,0.005520f,0.708315f,0.583199f,0.236644f,0.992380f,0.981091f,0.119804f,0.510866f,0.560499f,0.961497f,0.557862f,0.539955f,0.332871f,0.417807f,0.920779f,
				0.730747f,0.076690f,0.008562f,0.660104f,0.428921f,0.511342f,0.587871f,0.906406f,0.437980f,0.620309f,0.062196f,0.119485f,0.235646f,0.795892f,0.044437f,0.617311f,
				0.891128f,0.263161f,0.245298f,0.276518f,0.786986f,0.059768f,0.424345f,0.433341f,0.052190f,0.699924f,0.139479f,0.402873f,0.741976f,0.557978f,0.127093f,0.946352f,
				0.205587f,0.092822f,0.422956f,0.715176f,0.711952f,0.926062f,0.368646f,0.286516f,0.241413f,0.831616f,0.232247f,0.478637f,0.366948f,0.432024f,0.268430f,0.619122f,
				0.391737f,0.056698f,0.067702f,0.509009f,0.920858f,0.298358f,0.701015f,0.044309f,0.936794f,0.485976f,0.271286f,0.108779f,0.325844f,0.682314f,0.955090f,0.658145f,
				0.295861f,0.562559f,0.867194f,0.810552f,0.487959f,0.869567f,0.224706f,0.962637f,0.646548f,0.003730f,0.228857f,0.263667f,0.365176f,0.958302f,0.606619f,0.901869f,
				0.757257f,0.306061f,0.633172f,0.407697f,0.443632f,0.979959f,0.922944f,0.946421f,0.594079f,0.604343f,0.864211f,0.187557f,0.877119f,0.792025f,0.954840f,0.976719f,
				0.350546f,0.834781f,0.945113f,0.155877f,0.411841f,0.552378f,0.855409f,0.741383f,0.761251f,0.896223f,0.782077f,0.266224f,0.128873f,0.645733f,0.591567f,0.247385f,
				0.260848f,0.811970f,0.653369f,0.976713f,0.221533f,0.957436f,0.294018f,0.159025f,0.820596f,0.569601f,0.934328f,0.467182f,0.763165f,0.835736f,0.240033f,0.389869f,
				0.998754f,0.783739f,0.758034f,0.614317f,0.221128f,0.502497f,0.978066f,0.247794f,0.619551f,0.658307f,0.769667f,0.768478f,0.337143f,0.370689f,0.084723f,0.510534f,
				0.594996f,0.994636f,0.181230f,0.868113f,0.312023f,0.480495f,0.177356f,0.367374f,0.741642f,0.202983f,0.229404f,0.108165f,0.098607f,0.010412f,0.727391f,0.942217f,
				0.023850f,0.110631f,0.958293f,0.208996f,0.584609f,0.491803f,0.238266f,0.591587f,0.297477f,0.681421f,0.215040f,0.587764f,0.704494f,0.978978f,0.911686f,0.692657f,
				0.462987f,0.273259f,0.802855f,0.651633f,0.736728f,0.986217f,0.402363f,0.524098f,0.740470f,0.799076f,0.918257f,0.705367f,0.477477f,0.102279f,0.809959f,0.860645f,
				0.118276f,0.009567f,0.280106f,0.948473f,0.025423f,0.458173f,0.512607f,0.082088f,0.536906f,0.472590f,0.835726f,0.078518f,0.357919f,0.797522f,0.570516f,0.162719f,
				0.815968f,0.874141f,0.915300f,0.392073f,0.366307f,0.766238f,0.462755f,0.087614f,0.402357f,0.277686f,0.294194f,0.392791f,0.504893f,0.263420f,0.509197f,0.518974f,
				0.738809f,0.965800f,0.003864f,0.976899f,0.292287f,0.837148f,0.525498f,0.743779f,0.359015f,0.060636f,0.595481f,0.483102f,0.900195f,0.423277f,0.981990f,0.154968f,
				0.085584f,0.681517f,0.814437f,0.105936f,0.972238f,0.207062f,0.994642f,0.989271f,0.646217f,0.330263f,0.432094f,0.139929f,0.908629f,0.271571f,0.539319f,0.845182f,
				0.140069f,0.001406f,0.340195f,0.582218f,0.693570f,0.293148f,0.733441f,0.375523f,0.676068f,0.130642f,0.606523f,0.441091f,0.113519f,0.844462f,0.399921f,0.551049f,
				0.482781f,0.894854f,0.188909f,0.431045f,0.043693f,0.394601f,0.544309f,0.798761f,0.040417f,0.022292f,0.681257f,0.598379f,0.069981f,0.255632f,0.174776f,0.880842f,
				0.412071f,0.397976f,0.932835f,0.979471f,0.244276f,0.488083f,0.313785f,0.858199f,0.390958f,0.426132f,0.754800f,0.360781f,0.862827f,0.526424f,0.090054f,0.673971f,
				0.715044f,0.237489f,0.210234f,0.952837f,0.448429f,0.738062f,0.077342f,0.260666f,0.590478f,0.127519f,0.628981f,0.136232f,0.860189f,0.596789f,0.524043f,0.897171f,
				0.648864f,0.116735f,0.666835f,0.536993f,0.811733f,0.854961f,0.857206f,0.945069f,0.434195f,0.602343f,0.823780f,0.109481f,0.684652f,0.195598f,0.213630f,0.283516f,
				0.387092f,0.182029f,0.834655f,0.948975f,0.373107f,0.249751f,0.162575f,0.587850f,0.192648f,0.737863f,0.777432f,0.651490f,0.562558f,0.918301f,0.094830f,0.260698f,
				0.629400f,0.751325f,0.362210f,0.649610f,0.397390f,0.670624f,0.215662f,0.925465f,0.908397f,0.486853f,0.141060f,0.236122f,0.926399f,0.416056f,0.781483f,0.538538f,
				0.119521f,0.004196f,0.847561f,0.876772f,0.945552f,0.935095f,0.422025f,0.502860f,0.932500f,0.116670f,0.700854f,0.995577f,0.334925f,0.174659f,0.982878f,0.174110f,
				0.734294f,0.769366f,0.917586f,0.382623f,0.795816f,0.051831f,0.528121f,0.691978f,0.337981f,0.675601f,0.969444f,0.354908f,0.054569f,0.254278f,0.978879f,0.611259f,
				0.890006f,0.712659f,0.219624f,0.826455f,0.351117f,0.087383f,0.862534f,0.805461f,0.499343f,0.482118f,0.036473f,0.815656f,0.016539f,0.875982f,0.308313f,0.650039f,
				0.494165f,0.615983f,0.396761f,0.921652f,0.164612f,0.472705f,0.559820f,0.675677f,0.059891f,0.295793f,0.818010f,0.769365f,0.158699f,0.648142f,0.228793f,0.627454f,
				0.138543f,0.639463f,0.200399f,0.352380f,0.470716f,0.888694f,0.311777f,0.571183f,0.979317f,0.457287f,0.115151f,0.725631f,0.620539f,0.629373f,0.850207f,0.949974f,
				0.254675f,0.142306f,0.688887f,0.307235f,0.284882f,0.847675f,0.617070f,0.207422f,0.550545f,0.541886f,0.173878f,0.474841f,0.678372f,0.289180f,0.528111f,0.306538f,
				0.869399f,0.040299f,0.417301f,0.472569f,0.857612f,0.917462f,0.842319f,0.986865f,0.604528f,0.731115f,0.607880f,0.904675f,0.397955f,0.627867f,0.533371f,0.656758f,
				0.627210f,0.223554f,0.268442f,0.254858f,0.834380f,0.131010f,0.838028f,0.613512f,0.821627f,0.859779f,0.405212f,0.909901f,0.036186f,0.643093f,0.187064f,0.945730f,
				0.319022f,0.709012f,0.852200f,0.559587f,0.865751f,0.368890f,0.840416f,0.950571f,0.315120f,0.331749f,0.509218f,0.468617f,0.119006f,0.541820f,0.983444f,0.115515f,
				0.299804f,0.840386f,0.445282f,0.900755f,0.633600f,0.304196f,0.996153f,0.844025f,0.462361f,0.314402f,0.850035f,0.773624f,0.958303f,0.765382f,0.567577f,0.722607f,
				0.001299f,0.189690f,0.364661f,0.192390f,0.836882f,0.783680f,0.026723f,0.065230f,0.588791f,0.937752f,0.993644f,0.597499f,0.851975f,0.670339f,0.360987f,0.755649f,
				0.571521f,0.231990f,0.425067f,0.116442f,0.321815f,0.629616f,0.701207f,0.716931f,0.146357f,0.360526f,0.498487f,0.846096f,0.307994f,0.323456f,0.288884f,0.477935f,
				0.236433f,0.876589f,0.667459f,0.977175f,0.179347f,0.479408f,0.633292f,0.957666f,0.343651f,0.871846f,0.452856f,0.895494f,0.327657f,0.867779f,0.596825f,0.907009f,
				0.417409f,0.530739f,0.547422f,0.141032f,0.721096f,0.587663f,0.830054f,0.460860f,0.563898f,0.673780f,0.035824f,0.755808f,0.331846f,0.653460f,0.926339f,0.724599f,
				0.978501f,0.495221f,0.098108f,0.936766f,0.139911f,0.851336f,0.889867f,0.376509f,0.661482f,0.156487f,0.671886f,0.487835f,0.046571f,0.441975f,0.014015f,0.440433f,
				0.235927f,0.163762f,0.075399f,0.254734f,0.214011f,0.554803f,0.712877f,0.795785f,0.471616f,0.105032f,0.355989f,0.834418f,0.498021f,0.018318f,0.364799f,0.918869f,
				0.909222f,0.858506f,0.928250f,0.946347f,0.755364f,0.408753f,0.137841f,0.247870f,0.300618f,0.470068f,0.248714f,0.521691f,0.009862f,0.891550f,0.908914f,0.227533f,
				0.702908f,0.596738f,0.581597f,0.099904f,0.804893f,0.947457f,0.080649f,0.375755f,0.890498f,0.689130f,0.600941f,0.382261f,0.814084f,0.258373f,0.278029f,0.907399f,
				0.625024f,0.016637f,0.502896f,0.743077f,0.247834f,0.846201f,0.647815f,0.379888f,0.517357f,0.921494f,0.904846f,0.805645f,0.671974f,0.487205f,0.678009f,0.575624f,
				0.910779f,0.947642f,0.524788f,0.231298f,0.299029f,0.068158f,0.569690f,0.121049f,0.701641f,0.311914f,0.447310f,0.014019f,0.013391f,0.257855f,0.481835f,0.808870f,
				0.628222f,0.780253f,0.202719f,0.024902f,0.774355f,0.783080f,0.330077f,0.788864f,0.346888f,0.778702f,0.261985f,0.696691f,0.212839f,0.713849f,0.871828f,0.639753f,
				0.711037f,0.651247f,0.042374f,0.236938f,0.746267f,0.235043f,0.442707f,0.195417f,0.175918f,0.987980f,0.031270f,0.975425f,0.277087f,0.752667f,0.639751f,0.507857f,
				0.873571f,0.775393f,0.390003f,0.415997f,0.287861f,0.189340f,0.837939f,0.186253f,0.355633f,0.803788f,0.029124f,0.802046f,0.248046f,0.354010f,0.420571f,0.109523f,
				0.731250f,0.700653f,0.716019f,0.651507f,0.250055f,0.884214f,0.364255f,0.244975f,0.472268f,0.080641f,0.309332f,0.250613f,0.519091f,0.066142f,0.037804f,0.865752f,
				0.767738f,0.617325f,0.537048f,0.743959f,0.401200f,0.595458f,0.869843f,0.193999f,0.670364f,0.018494f,0.743159f,0.979555f,0.382352f,0.191059f,0.992247f,0.946175f,
				0.306473f,0.793720f,0.687331f,0.556239f,0.958367f,0.390949f,0.357823f,0.110213f,0.977540f,0.831431f,0.485895f,0.148678f,0.847327f,0.733145f,0.397393f,0.376365f,
				0.398704f,0.463869f,0.976946f,0.844771f,0.075688f,0.473865f,0.470958f,0.548172f,0.350174f,0.727441f,0.123139f,0.347760f,0.839587f,0.562705f,0.036853f,0.564723f,
				0.960356f,0.220534f,0.906969f,0.677664f,0.841052f,0.111530f,0.032346f,0.027749f,0.468255f,0.229196f,0.508756f,0.199613f,0.298103f,0.677274f,0.526005f,0.828221f,
				0.413321f,0.305165f,0.223361f,0.778072f,0.198089f,0.414976f,0.007498f,0.464238f,0.785213f,0.534428f,0.060537f,0.572427f,0.693334f,0.865843f,0.034964f,0.586806f,
				0.161710f,0.203743f,0.656513f,0.604340f,0.688333f,0.257211f,0.246437f,0.338237f,0.839947f,0.268420f,0.913245f,0.759551f,0.289283f,0.347280f,0.508970f,0.361526f,
				0.554649f,0.086439f,0.024344f,0.661653f,0.988840f,0.110613f,0.129422f,0.405940f,0.781764f,0.303922f,0.521807f,0.236282f,0.277927f,0.699228f,0.733812f,0.772090f,
				0.658423f,0.056394f,0.153089f,0.536837f,0.792251f,0.165229f,0.592251f,0.228337f,0.147078f,0.116056f,0.319268f,0.293400f,0.872600f,0.842240f,0.306238f,0.228790f,
				0.745704f,0.821321f,0.778268f,0.611390f,0.969139f,0.297654f,0.367369f,0.815074f,0.985840f,0.693232f,0.411759f,0.366651f,0.345481f,0.609060f,0.778929f,0.640823f,
				0.340969f,0.328489f,0.898686f,0.952345f,0.272572f,0.758995f,0.111269f,0.613403f,0.864397f,0.607601f,0.357317f,0.227619f,0.177081f,0.773828f,0.318257f,0.298335f,
				0.679382f,0.454625f,0.976745f,0.244511f,0.880111f,0.046238f,0.451342f,0.709265f,0.784123f,0.488338f,0.228713f,0.041251f,0.077453f,0.718891f,0.454221f,0.039182f,
				0.614777f,0.538681f,0.856650f,0.888921f,0.184013f,0.487999f,0.880338f,0.726824f,0.112945f,0.835710f,0.943366f,0.340094f,0.167909f,0.241240f,0.125953f,0.460130f,
				0.789923f,0.313898f,0.640780f,0.795920f,0.198025f,0.407344f,0.673839f,0.414326f,0.185900f,0.353436f,0.786795f,0.422102f,0.133975f,0.363270f,0.393833f,0.748760f,
				0.328130f,0.115681f,0.253865f,0.526924f,0.672761f,0.517447f,0.686442f,0.532847f,0.551176f,0.667406f,0.382640f,0.408796f,0.649460f,0.613948f,0.600470f,0.485404f,
			};
		} 
		static void BuildSsaoPso(JDx12GraphicShaderDataHolder<1>* holder,
			ID3D12Device* device,
			ID3D12RootSignature* root,
			const DXGI_FORMAT rtvFormat,
			const uint rtvCount,
			const std::string& name)
		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC newShaderPso;
			ZeroMemory(&newShaderPso, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
			newShaderPso.InputLayout = { nullptr, 0 };
			newShaderPso.pRootSignature = root;
			newShaderPso.VS =
			{
				reinterpret_cast<BYTE*>(holder->vs->GetBufferPointer()),
				holder->vs->GetBufferSize()
			};
			if (holder->gs != nullptr)
			{
				newShaderPso.GS =
				{
					reinterpret_cast<BYTE*>(holder->gs->GetBufferPointer()),
					holder->gs->GetBufferSize()
				};
			}
			newShaderPso.PS =
			{
				reinterpret_cast<BYTE*>(holder->ps->GetBufferPointer()),
				holder->ps->GetBufferSize()
			};

			newShaderPso.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			newShaderPso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			newShaderPso.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			newShaderPso.SampleMask = UINT_MAX;
			newShaderPso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			newShaderPso.NumRenderTargets = rtvCount;
			for (uint i = 0; i < rtvCount; ++i)
				newShaderPso.RTVFormats[i] = rtvFormat;
			newShaderPso.SampleDesc.Count = 1;
			newShaderPso.SampleDesc.Quality = 0;

			newShaderPso.RasterizerState.AntialiasedLineEnable = false;
			newShaderPso.RasterizerState.MultisampleEnable = false;
			newShaderPso.DepthStencilState.DepthEnable = false;
			newShaderPso.DepthStencilState.StencilEnable = false;

			ThrowIfFailedG(device->CreateGraphicsPipelineState(&newShaderPso, IID_PPV_ARGS(holder->GetPsoAddress())));
			holder->GetPso()->SetPrivateData(WKPDID_D3DDebugObjectName, name.size(), name.c_str());
		}
	}
	 
	JDx12Ssao::SSaoDrawDataSet::SSaoDrawDataSet(JGraphicSsaoComputeSet* computeSet,
		const JDrawHelper& helper, 
		const JUserPtr<JGraphicResourceInfo>(&randomInfoVec)[(uint)J_SSAO_TYPE::COUNT])
	{
		auto ssaoSet = static_cast<JDx12GraphicSsaoComputeSet*>(computeSet);
		auto dx12Share = static_cast<JDx12GraphicResourceShareData*>(computeSet->shareData);
		context = static_cast<JDx12CommandContext*>(ssaoSet->context); 

		ssaoDesc = helper.cam->GetSsaoDesc();
		canBlur = ssaoDesc.useBlur;

		const JUserPtr<JGraphicResourceInfo>& randomInfo = randomInfoVec[(uint)ssaoDesc.ssaoType];
		auto gInterface = helper.cam->GraphicResourceUserInterface();
		int rtDataIndex = gInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		int dsDataIndex = gInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		int aoDataIndex = gInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP, J_GRAPHIC_TASK_TYPE::APPLY_SSAO);

		const JVector2<uint> size = gInterface.GetResourceSize(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, rtDataIndex);
		inter = dx12Share->GetSsaoData(size.x, size.y);
		if (inter == nullptr)
			return;
		 
		rtSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		dsSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		normalSet = context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP);
		aoSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP, J_GRAPHIC_TASK_TYPE::APPLY_SSAO);
		aoInter00Set = context->ComputeSet(inter->intermediate00);
		aoInter01Set = context->ComputeSet(inter->intermediate01);
		aoInterleaveSet = context->ComputeSet(inter->interleave);
		aoDepthSet = context->ComputeSet(inter->depth);
		aoDepthInterleaveSet = context->ComputeSet(inter->depthInterleave);
		randomSet = context->ComputeSet(randomInfo);
		 
		//aoDepthInterleaveSet.viewOffset = 1;
		//16 .. 1, 1, 1, 1, ~
		isValid = true; 
	}

	JDx12Ssao::JDx12Ssao()
		:guid(Core::MakeGuid())
	{}
	JDx12Ssao::~JDx12Ssao()
	{
		ClearResource();
	}
	void JDx12Ssao::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;

		BuildResouce(device, gM);
	}
	void JDx12Ssao::Clear()
	{ 
		ClearResource();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12Ssao::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	} 
	bool JDx12Ssao::HasDependency(const JGraphicInfo::TYPE type)const noexcept
	{
		return false;
	}
	bool JDx12Ssao::HasDependency(const JGraphicOption::TYPE type)const noexcept
	{
		if (type == JGraphicOption::TYPE::POST_PROCESS || type == JGraphicOption::TYPE::DEBUGGING)
			return true;
		else
			return false;
	}
	void JDx12Ssao::NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set)
	{}
	void JDx12Ssao::NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)
	{
		auto dx12Set = static_cast<const JDx12GraphicOptionChangedSet&>(set);
		if (set.changedPart == JGraphicOption::TYPE::POST_PROCESS && 
			set.preOption.postProcess.useSsaoInterleave != set.newOption.postProcess.useSsaoInterleave)
			RecompileShader(JGraphicShaderCompileSet(dx12Set.device));

		if (set.changedPart == JGraphicOption::TYPE::DEBUGGING && set.newOption.debugging.requestRecompileSsaoShader)
			RecompileShader(JGraphicShaderCompileSet(dx12Set.device));
	}
	void JDx12Ssao::ApplySsao(JGraphicSsaoComputeSet* computeSet, const JDrawHelper& helper)
	{ 
		if (!IsSameDevice(computeSet) || helper.cam == nullptr || !helper.allowSsao)
			return;

		JDx12GraphicSsaoComputeSet* ssaoSet = static_cast<JDx12GraphicSsaoComputeSet*>(computeSet);
		SSaoDrawDataSet set(computeSet, helper, randomVecInfo);
		if (!set.isValid)
			return;

		set.canUseHbaoInterleave = helper.option.postProcess.useSsaoInterleave && set.ssaoDesc.ssaoType == J_SSAO_TYPE::HORIZON_BASED;
		set.context->Transition(set.aoSet.holder, D3D12_RESOURCE_STATE_RENDER_TARGET);
		set.context->Transition(set.dsSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		if (set.canBlur)
		{
			set.context->Transition(set.aoInter00Set.holder, D3D12_RESOURCE_STATE_RENDER_TARGET);
			set.context->Transition(set.aoInter01Set.holder, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}
		if (set.canUseHbaoInterleave)
		{
			//transition interleave resource and call draw func
			set.context->Transition(set.aoDepthSet.holder, D3D12_RESOURCE_STATE_RENDER_TARGET);
			set.context->Transition(set.aoDepthInterleaveSet.holder, D3D12_RESOURCE_STATE_RENDER_TARGET);
			set.context->Transition(set.aoInterleaveSet.holder, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
			 
			DepthMapLinearize(set, helper);
			set.context->Transition(set.aoDepthSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);

			DepthMapInetrleave(set, helper);
			set.context->Transition(set.aoDepthInterleaveSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
			 
			DrawSsaoMap(set, helper);
			set.context->Transition(set.aoInterleaveSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
		 
			CombineSsaoMap(set, helper);
			BlurSsao(set, helper); 
		}
		else
		{
			set.context->Transition(set.randomSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			DrawSsaoMap(set, helper);
			BlurSsao(set, helper); 
		} 
	}
	void JDx12Ssao::DepthMapLinearize(const SSaoDrawDataSet& set, const JDrawHelper& helper)
	{
		//rt aoDepthSet
		//r dsSet
		set.context->SetRenderTargetView(set.aoDepthSet, 1, true);
		set.context->SetViewportAndRect(set.aoDepthSet.info->GetResourceSize());

		auto shaderData = ssaoDepthLinearize.get();
		set.context->SetPipelineState(shaderData);

		set.context->SetGraphicsRootSignature(ssaoDepthLinearizeRootSignature.Get());
		set.context->SetGraphicsRootDescriptorTable(SsaoDepthLinearlize::depthMapIndex, set.dsSet.GetGpuSrvHandle());
		set.context->SetGraphicsRootConstantBufferView(SsaoDepthLinearlize::passCBIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::SSAO_PASS, helper.GetCamFrameIndex(CameraFrameLayer::ssao));
		set.context->DrawFullScreenTriangle(); 
	}
	void JDx12Ssao::DepthMapInetrleave(SSaoDrawDataSet& set, const JDrawHelper& helper)
	{		
		//rt aoDepthInterleave
		//r aoDepth
		auto shaderData = ssaoDepthInterleave.get();
		set.context->SetPipelineState(shaderData);

		set.context->SetGraphicsRootSignature(ssaoDepthInterleaveRootSignature.Get());
		set.context->SetGraphicsRootDescriptorTable(SsaoDepthInterleave::depthMapIndex, set.aoDepthSet.GetGpuSrvHandle());
		set.context->SetGraphicsRootConstantBufferView(SsaoDepthInterleave::passCBIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::SSAO_PASS, helper.GetCamFrameIndex(CameraFrameLayer::ssao)); 
		set.context->SetViewportAndRect(set.aoDepthInterleaveSet.info->GetResourceSize());		//quater
		 
		set.aoDepthInterleaveSet.viewOffset = 1;
		for (uint i = 0; i < Ssao::sliceCount; i += Ssao::mrtCount)
		{ 
			set.context->SetGraphicsRootConstantBufferView(SsaoDepthInterleave::sliceCBIndex, ssaoAoSliceCB.get(), i);
			set.context->SetRenderTargetView(set.aoDepthInterleaveSet, Ssao::mrtCount, true);
			set.context->DrawFullScreenTriangle(); 
			set.aoDepthInterleaveSet.viewOffset += Ssao::mrtCount;
		}
		set.aoDepthInterleaveSet.viewOffset = 0;
	}
	void JDx12Ssao::DrawSsaoMap(SSaoDrawDataSet& set, const JDrawHelper& helper)
	{
		const uint shaderInedx = Private::CalIndex(set.ssaoDesc.ssaoType, set.ssaoDesc.sampleType, set.canBlur ? J_SSAO_FUNCTION::BLUR : J_SSAO_FUNCTION::NONE);
		auto shaderData = ssao[shaderInedx].get();
		set.context->SetPipelineState(shaderData);
		set.context->SetGraphicsRootSignature(set.ssaoDesc.ssaoType == J_SSAO_TYPE::DEFAULT ? ssaoRootSignature.Get() : hbaoRootSignature.Get());
		set.context->SetGraphicsRootConstantBufferView(Ssao::passCBIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::SSAO_PASS, helper.GetCamFrameIndex(CameraFrameLayer::ssao)); 

		if (set.canUseHbaoInterleave)
		{ 
			set.context->SetRenderTargetView(set.aoInterleaveSet);
			set.context->SetViewportAndRect(set.aoInterleaveSet.info->GetResourceSize()); 
			set.context->SetGraphicsRootDescriptorTable(Ssao::normalMapIndex, set.normalSet.GetGpuSrvHandle());
			set.aoDepthInterleaveSet.viewOffset = 1;
			for (uint i = 0; i < Ssao::sliceCount; ++i)
			{
				set.context->SetGraphicsRootConstantBufferView(Ssao::typePerCBIndex, ssaoAoSliceCB.get(), i);
				set.context->SetGraphicsRootDescriptorTable(Ssao::depthMapIndex, set.aoDepthInterleaveSet.GetGpuSrvHandle());
				set.context->DrawFullScreenTriangle();
				++set.aoDepthInterleaveSet.viewOffset;
			}
			set.aoDepthInterleaveSet.viewOffset = 0;
		}
		else
		{
			set.context->Transition(set.normalSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			set.context->Transition(set.randomSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);

			if (set.ssaoDesc.ssaoType == J_SSAO_TYPE::DEFAULT)
				set.context->SetGraphicsRootConstantBufferView(Ssao::typePerCBIndex, ssaoSampleCB.get(), (uint)set.ssaoDesc.sampleType);
			set.context->SetGraphicsRootDescriptorTable(Ssao::depthMapIndex, set.dsSet.GetGpuSrvHandle());
			set.context->SetGraphicsRootDescriptorTable(Ssao::normalMapIndex, set.normalSet.GetGpuSrvHandle());
			set.context->SetGraphicsRootDescriptorTable(Ssao::randomMapIndex, set.randomSet.GetGpuSrvHandle());
			set.context->SetRenderTargetView(set.canBlur ? set.aoInter00Set : set.aoSet);
			set.context->SetViewportAndRect(set.canBlur ? set.aoInter00Set.info->GetResourceSize() : set.aoSet.info->GetResourceSize());
			set.context->DrawFullScreenTriangle(); 
		}
	}
	void JDx12Ssao::CombineSsaoMap(const SSaoDrawDataSet& set, const JDrawHelper& helper)
	{
		auto shaderData = ssaoCombine[(uint)set.canBlur ? (uint)J_SSAO_FUNCTION::BLUR : (uint)J_SSAO_FUNCTION::NONE].get();
		set.context->SetPipelineState(shaderData);
		set.context->SetGraphicsRootSignature(ssaoCombineRootSignature.Get());

		set.context->SetGraphicsRootDescriptorTable(SsaoCombine::aoMapIndex, set.aoInterleaveSet.GetGpuSrvHandle());
		set.context->SetGraphicsRootDescriptorTable(SsaoCombine::depthMapIndex, set.aoDepthSet.GetGpuSrvHandle());
		set.context->SetGraphicsRootConstantBufferView(SsaoCombine::passCBIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::SSAO_PASS, helper.GetCamFrameIndex(CameraFrameLayer::ssao));
		
		if (set.canBlur)
		{
			set.context->SetRenderTargetView(set.aoInter00Set);
			set.context->SetViewportAndRect(set.aoInter00Set.info->GetResourceSize());
		}
		else
		{
			set.context->SetRenderTargetView(set.aoSet);
			set.context->SetViewportAndRect(set.aoSet.info->GetResourceSize());
		} 
		set.context->DrawFullScreenTriangle();
	}
	void JDx12Ssao::BlurSsao(const SSaoDrawDataSet& set, const JDrawHelper& helper)
	{
		if (!set.canBlur)
			return;

		if (set.ssaoDesc.blurType == J_SSAO_BLUR_TYPE::BILATERAL)
		{
			set.context->Transition(set.aoInter00Set.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
 
			auto shaderData = ssaoBlur[(uint)J_SSAO_BLUR_SHADER::BILATERAL_X][(uint)set.ssaoDesc.blurKenelSize].get();
			set.context->SetPipelineState(shaderData);
			set.context->SetGraphicsRootSignature(ssaoBlurRootSignature.Get());
			set.context->SetGraphicsRootDescriptorTable(SsaoBlur::srcMapIndex, set.aoInter00Set.GetGpuSrvHandle());
			set.context->SetGraphicsRootConstantBufferView(SsaoBlur::passCBIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::SSAO_PASS, helper.GetCamFrameIndex(CameraFrameLayer::ssao));
			 
			set.context->SetRenderTargetView(set.aoInter01Set);
			set.context->SetViewportAndRect(set.aoInter01Set.info->GetResourceSize());
			set.context->DrawFullScreenTriangle();

			set.context->Transition(set.aoInter01Set.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);

			shaderData = ssaoBlur[(uint)J_SSAO_BLUR_SHADER::BILATERAL_Y][(uint)set.ssaoDesc.blurKenelSize].get();
			set.context->SetPipelineState(shaderData);
			set.context->SetGraphicsRootDescriptorTable(SsaoBlur::srcMapIndex, set.aoInter01Set.GetGpuSrvHandle());
			
			set.context->SetRenderTargetView(set.aoSet);
			set.context->SetViewportAndRect(set.aoSet.info->GetResourceSize());
			set.context->DrawFullScreenTriangle(); 
		} 
	}
	void JDx12Ssao::RecompileShader(const JGraphicShaderCompileSet& dataSet)
	{
		ClearRootSignature();
		ClearPso();

		ID3D12Device* d3d12Device = static_cast<JDx12GraphicDevice*>(dataSet.device)->GetDevice();
		BuildRootSingnature(d3d12Device, GetGraphicInfo(), GetGraphicOption());
		BuildPso(d3d12Device, GetGraphicInfo(), GetGraphicOption());
	}
	void JDx12Ssao::BuildResouce(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(gM);
		ID3D12Device* d3d12Device = dx12Device->GetDevice();

		BuildRootSingnature(d3d12Device, GetGraphicInfo(), GetGraphicOption());
		BuildPso(d3d12Device, GetGraphicInfo(), GetGraphicOption());
		BuildBuffer(dx12Device, dx12Gm);
	}
	void JDx12Ssao::BuildRootSingnature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{ 
		BuildSsaoRootSignature(device, option);
		BuildSsaoBlurRootSignature(device);
		BuildSsaoCombineRootSignature(device);
		BuildSsaoDepthLinearizeRootSignature(device);
		BuildSsaoDepthInterleaveRootSignature(device);
	}
	void JDx12Ssao::BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{ 
		BuildSsaoPso(device, info, option);
		BuildSsaoBlurPso(device, info);
		BuildSsaoCombinePso(device, info);
		BuildSsaoDepthLinearizePso(device, info);
		BuildSsaoDepthInterleavePso(device, info);
	}
	void JDx12Ssao::BuildBuffer(JDx12GraphicDevice* device, JDx12GraphicResourceManager* gm)
	{ 
		std::random_device rd;	//use hardware
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> disSNorm(-1.0f, 1.0f);
		std::uniform_real_distribution<float> disUNorm(0.0f, 1.0f);

		if (ssaoSampleCB == nullptr)
			ssaoSampleCB = std::make_unique<JDx12GraphicBufferT<SsaoSampleConstants>>(L"SSAOSample", J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT);

		ssaoSampleCB->Clear();
		ssaoSampleCB->Build(device, (uint)J_SSAO_SAMPLE_TYPE::COUNT);

		for (uint i = 0; i < (uint)J_SSAO_SAMPLE_TYPE::COUNT; ++i)
		{
			SsaoSampleConstants constants;
			for (uint j = 0; j < Ssao::sampleCount[i]; ++j)
			{
				float scale = j / (float)Ssao::sampleCount[i];
				scale = JMathHelper::Lerp(0.1f, 1.0f, scale * scale);
				JVector4F sample(disSNorm(gen), disSNorm(gen), disUNorm(gen), 1.0f);
				sample = sample.Normalize();
				sample *= disUNorm(gen);
				sample *= scale;
				constants.sample[j] = sample;
			} 
			ssaoSampleCB->CopyData(i, constants);
		}

		//ssao
		uint randomCount = Ssao::randomWidth * Ssao::randomWidth;
		std::vector<JVector4F> ssaoRandomVec(randomCount);
		for (uint i = 0; i < randomCount; ++i)
		{
			JVector4F noise(disSNorm(gen), disSNorm(gen), 0.0f, 1.0f);
			ssaoRandomVec[i] = noise.Normalize();
		} 
		JGraphicResourceCreationDesc desc;
		desc.width = Ssao::randomWidth;
		desc.height = Ssao::randomWidth;
		desc.bindDesc.requestAdditionalBind[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;
		desc.bindDesc.useEngineDefinedBindType = false;
		desc.textureDesc = std::make_unique<JTextureCreationDesc>();
		desc.textureDesc->mipMapDesc.type = J_GRAPHIC_MIP_MAP_TYPE::NONE;
		desc.uploadBufferDesc = std::make_unique<JUploadBufferCreationDesc>(ssaoRandomVec.data(), ssaoRandomVec.size() * sizeof(JVector4F));
		desc.formatHint = std::make_unique<JGraphicFormatHint>(); 
		desc.formatHint->format = J_GRAPHIC_RESOURCE_FORMAT::R16G16B16A16_UNORM;

		//desc.formatHint = std::make_unique< JGraphicFormatHint>();
		//desc.formatHint->componentPerByte = 2;

		randomVecInfo[(uint)J_SSAO_TYPE::DEFAULT] = gm->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON);

		randomCount = Ssao::randomWidth * Ssao::randomWidth;
		std::vector<JVector4F> hbaoRandomVec(randomCount);
		std::vector<float> randomNumberVec = Private::GetRandomNumberVec();
		uint randomNumberVecCount = (uint)randomNumberVec.size();

		uint k = 0;
		for (uint i = 0; i < randomCount; ++i)
		{
			float r1 = randomNumberVec[k % randomNumberVecCount]; ++k;
			float r2 = randomNumberVec[k % randomNumberVecCount]; ++k;
			float r3 = randomNumberVec[k % randomNumberVecCount]; ++k;

			// Use random rotation angles in [0,2PI/NUM_DIRECTIONS).
			float angle = JMathHelper::Pi2 * r1 / Ssao::randomDirCount;
			hbaoRandomVec[i].x = cosf(angle);
			hbaoRandomVec[i].y = sinf(angle);
			hbaoRandomVec[i].z = r2;
			hbaoRandomVec[i].w = r3;
		}

		desc.width = Ssao::randomWidth;
		desc.height = Ssao::randomWidth;
		desc.bindDesc.requestAdditionalBind[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;
		desc.bindDesc.useEngineDefinedBindType = false;
		desc.textureDesc = std::make_unique<JTextureCreationDesc>();
		desc.textureDesc->mipMapDesc.type = J_GRAPHIC_MIP_MAP_TYPE::NONE;
		desc.uploadBufferDesc = std::make_unique<JUploadBufferCreationDesc>(hbaoRandomVec.data(), hbaoRandomVec.size() * sizeof(JVector4F));
		desc.formatHint = std::make_unique<JGraphicFormatHint>(); 
		desc.formatHint->format = J_GRAPHIC_RESOURCE_FORMAT::R16G16B16A16_UNORM;

		randomVecInfo[(uint)J_SSAO_TYPE::HORIZON_BASED] = gm->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON);

		ssaoAoSliceCB = std::make_unique<JDx12GraphicBufferT<SsaoAoSliceConstants>>(L"SsaoAoSliceCB", J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT);
		ssaoAoSliceCB->Build(device, Ssao::sliceCount);

		SsaoAoSliceConstants constants;
		for (uint i = 0; i < Ssao::sliceCount; ++i)
		{
			constants.jitter = hbaoRandomVec[i];
			constants.posOffset = JVector2F(i % 4, i / 4) + 0.5f;
			constants.sliceIndex = i;
			constants.uSliceIndex = (uint)i;

			ssaoAoSliceCB->CopyData(i, constants);
		}
	} 
	void JDx12Ssao::BuildSsaoRootSignature(ID3D12Device* device, const JGraphicOption& option)
	{
		JDx12RootSignatureBuilder2<Ssao::ssaoSlotCount, 2> ssaoBuilder;
		ssaoBuilder.PushConstantsBuffer(Ssao::passCBIndex);
		ssaoBuilder.PushConstantsBuffer(Ssao::typePerCBIndex);
		ssaoBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);		//depthMapTable
		ssaoBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);		//normalMapTable
		ssaoBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);		//randomMapTable 
		std::vector< CD3DX12_STATIC_SAMPLER_DESC> sam
		{
			CD3DX12_STATIC_SAMPLER_DESC(0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP), // addressW

			CD3DX12_STATIC_SAMPLER_DESC(1, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP) // addressW

			/*
			//depth
			CD3DX12_STATIC_SAMPLER_DESC(2, // shaderRegister
			D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
			0.0f,
			4,
			D3D12_COMPARISON_FUNC_LESS_EQUAL,
			D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE)*/
		};
		for (const auto& data : sam)
			ssaoBuilder.PushSampler(data);
		ssaoBuilder.Create(device, L"SsaoRootSignature", ssaoRootSignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);
	
		
		JDx12RootSignatureBuilder2<Ssao::hbaoSlotCount, 2> hbaoBuilder;
		hbaoBuilder.PushConstantsBuffer(Ssao::passCBIndex);
		hbaoBuilder.PushConstantsBuffer(Ssao::typePerCBIndex);
		hbaoBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);		//depthMapTable
		hbaoBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);		//normalMapTable
		if (!option.postProcess.useSsaoInterleave)
		{
			hbaoBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);		//randomMapTable 
			for (const auto& data : sam)
				hbaoBuilder.PushSampler(data);
		}
		else
			hbaoBuilder.PushSampler(sam[0]);
		hbaoBuilder.Create(device, L"HbaoRootSignature", hbaoRootSignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);
	}
	void JDx12Ssao::BuildSsaoPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		for (uint i = 0; i < (uint)J_SSAO_TYPE::COUNT; ++i)
		{
			for (uint j = 0; j < (uint)J_SSAO_SAMPLE_TYPE::COUNT; ++j)
			{
				for (uint k = 0; k < (uint)J_SSAO_FUNCTION::COUNT; ++k)
				{
					const J_SSAO_TYPE type = (J_SSAO_TYPE)i;
					const J_SSAO_SAMPLE_TYPE sampleType = (J_SSAO_SAMPLE_TYPE)j;
					const J_SSAO_FUNCTION function = (J_SSAO_FUNCTION)k;
					const bool isHbao = type == J_SSAO_TYPE::HORIZON_BASED;
					const bool canUseInterleave = option.postProcess.useSsaoInterleave && isHbao;

					const uint index = Private::CalIndex(type, sampleType, function);
					ssao[index] = std::make_unique<JDx12GraphicShaderDataHolder>();
					auto* holder = ssao[index].get();

					JGraphicShaderInitData initData;
					Private::StuffGraphicShaderMacro(initData, type, sampleType, function);
					if (canUseInterleave)
						initData.macro[0].push_back({ USE_SSAO_INTERLEAVE, std::to_wstring(1) });

					auto compileInfo = Private::GraphicShaderCompileInfo(type);
					holder->vs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro[0], L"FullScreenTriangleVS", L"vs_6_0");
					holder->ps = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro[0], compileInfo.functionName, L"ps_6_0");
					if (canUseInterleave)
					{
						auto gsInfo = JCompileInfo(L"SsaoGs.hlsl", L"SsaoGs");
						holder->gs = JDxShaderDataUtil::CompileShader(gsInfo.filePath, initData.macro[0], gsInfo.functionName, L"gs_6_0");
					}

					ID3D12RootSignature* root = isHbao ? hbaoRootSignature.Get() : ssaoRootSignature.Get();
					DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
					if (canUseInterleave)
						format = DXGI_FORMAT_R16_UNORM;
					else if (function == J_SSAO_FUNCTION::BLUR)
						format = DXGI_FORMAT_R16G16_UNORM;
					else
						format = DXGI_FORMAT_R16_UNORM;
					Private::BuildSsaoPso(holder, device, root, format, 1, "SsaoPso");
				}
			}
		}
	}
	void JDx12Ssao::BuildSsaoBlurRootSignature(ID3D12Device* device)
	{
		JDx12RootSignatureBuilder2<SsaoBlur::slotCount, 2> builder;
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);		//srcMap
		builder.PushConstantsBuffer(0);									//passCBIndex
		std::vector< CD3DX12_STATIC_SAMPLER_DESC> sam
		{
			CD3DX12_STATIC_SAMPLER_DESC(0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP), // addressW

			CD3DX12_STATIC_SAMPLER_DESC(1, // shaderRegister
			D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP) // addressW
		};
		for (const auto& data : sam)
			builder.PushSampler(data);

		builder.Create(device, L"SsaoBlurRootSignature", ssaoBlurRootSignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);
	}
	void JDx12Ssao::BuildSsaoBlurPso(ID3D12Device* device, const JGraphicInfo& info)
	{
		for (uint i = 0; i < (uint)J_SSAO_BLUR_SHADER::COUNT; ++i)
		{
			for (uint j = 0; j < (uint)J_KERNEL_SIZE::COUNT; ++j)
			{
				const J_SSAO_BLUR_SHADER type = (J_SSAO_BLUR_SHADER)i;
				const J_KERNEL_SIZE size = (J_KERNEL_SIZE)j;

				ssaoBlur[i][j] = std::make_unique<JDx12GraphicShaderDataHolder>();
				auto* holder = ssaoBlur[i][j].get();

				JGraphicShaderInitData initData;
				Private::StuffGraphicShaderMacro(initData, type, size);

				auto compileInfo = Private::GraphicShaderCompileInfo(type);
				holder->vs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro[0], L"FullScreenTriangleVS", L"vs_6_0");
				holder->ps = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro[0], compileInfo.functionName, L"ps_6_0");

				const DXGI_FORMAT format = type == J_SSAO_BLUR_SHADER::BILATERAL_X ? DXGI_FORMAT_R16G16_UNORM : DXGI_FORMAT_R16_UNORM;
				Private::BuildSsaoPso(holder, device, ssaoBlurRootSignature.Get(), format, 1, "SsaoBlurPso");
			}
		}
	}
	void JDx12Ssao::BuildSsaoCombineRootSignature(ID3D12Device* device)
	{
		JDx12RootSignatureBuilder2<SsaoCombine::slotCount, 1> builder;
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);		//aoMap
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);		//depthMap
		builder.PushConstantsBuffer(0);									//passCBIndex		 
		std::vector< CD3DX12_STATIC_SAMPLER_DESC> sam
		{
			CD3DX12_STATIC_SAMPLER_DESC(0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP) // addressW 
		};
		for (const auto& data : sam)
			builder.PushSampler(data);

		builder.Create(device, L"SsaoCombineRootSignature", ssaoCombineRootSignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);
	}
	void JDx12Ssao::BuildSsaoCombinePso(ID3D12Device* device, const JGraphicInfo& info)
	{
		for (uint i = 0; i < (uint)J_SSAO_FUNCTION::COUNT; ++i)
		{
			const J_SSAO_FUNCTION func = (J_SSAO_FUNCTION)i;

			ssaoCombine[i] = std::make_unique<JDx12GraphicShaderDataHolder>();
			auto* holder = ssaoCombine[i].get();

			JGraphicShaderInitData initData;
			Private::StuffGraphicShaderMacro(initData, func);
			initData.macro[0].push_back({ USE_HBAO, std::to_wstring(1) });

			auto compileInfo = JCompileInfo(L"SsaoCombine.hlsl", L"SsaoCombine");
			holder->vs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro[0], L"FullScreenTriangleVS", L"vs_6_0");
			holder->ps = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro[0], compileInfo.functionName, L"ps_6_0");

			const DXGI_FORMAT format = func == J_SSAO_FUNCTION::BLUR ? DXGI_FORMAT_R16G16_UNORM : DXGI_FORMAT_R16_UNORM;
			Private::BuildSsaoPso(holder, device, ssaoCombineRootSignature.Get(), format, 1, "SsaoCombinePso");
		}
	}
	void JDx12Ssao::BuildSsaoDepthLinearizeRootSignature(ID3D12Device* device)
	{
		JDx12RootSignatureBuilder<SsaoDepthLinearlize::slotCount> builder; 
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);		//depthMap
		builder.PushConstantsBuffer(0);									//passCBIndex
		builder.Create(device, L"SsaoDepthLinearizeRootSignature", ssaoDepthLinearizeRootSignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);
	}
	void JDx12Ssao::BuildSsaoDepthLinearizePso(ID3D12Device* device, const JGraphicInfo& info)
	{
		ssaoDepthLinearize = std::make_unique<JDx12GraphicShaderDataHolder>();
		auto* holder = ssaoDepthLinearize.get();

		JGraphicShaderInitData initData;
		auto compileInfo = JCompileInfo(L"DepthLinearize.hlsl", L"DepthLinearize");
		holder->vs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro[0], L"FullScreenTriangleVS", L"vs_6_0");
		holder->ps = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro[0], compileInfo.functionName, L"ps_6_0");

		Private::BuildSsaoPso(holder, device, ssaoDepthLinearizeRootSignature.Get(), DXGI_FORMAT_R32_FLOAT, 1, "SsaoDepthLinearizePso");
	}
	void JDx12Ssao::BuildSsaoDepthInterleaveRootSignature(ID3D12Device* device)
	{
		JDx12RootSignatureBuilder2<SsaoDepthInterleave::slotCount, 1> builder;
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);		//depthMap
		builder.PushConstantsBuffer(0);									//passCBIndex
		builder.PushConstantsBuffer(1);									//sliceCBIndex
		std::vector< CD3DX12_STATIC_SAMPLER_DESC> sam
		{
			CD3DX12_STATIC_SAMPLER_DESC(0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP) // addressW 
		};
		for (const auto& data : sam)
			builder.PushSampler(data);
		builder.Create(device, L"SsaoDepthInterleaveRootSignature", ssaoDepthInterleaveRootSignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);
	}
	void JDx12Ssao::BuildSsaoDepthInterleavePso(ID3D12Device* device, const JGraphicInfo& info)
	{
		ssaoDepthInterleave = std::make_unique<JDx12GraphicShaderDataHolder>();
		auto* holder = ssaoDepthInterleave.get();

		JGraphicShaderInitData initData;
		auto compileInfo = JCompileInfo(L"DepthInterleave.hlsl", L"DepthInterleave");
		holder->vs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro[0], L"FullScreenTriangleVS", L"vs_6_0");
		holder->ps = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro[0], compileInfo.functionName, L"ps_6_0");

		Private::BuildSsaoPso(holder, device, ssaoDepthInterleaveRootSignature.Get(), DXGI_FORMAT_R32_FLOAT, Ssao::mrtCount, "ssaoDepthInterleavePso");
	}
	void JDx12Ssao::ClearResource()
	{
		ClearRootSignature();
		ClearPso();
		ClearBuffer(); 
	}
	void JDx12Ssao::ClearRootSignature()
	{ 
		ssaoRootSignature = nullptr;
		hbaoRootSignature = nullptr;
		ssaoBlurRootSignature = nullptr;
		ssaoCombineRootSignature = nullptr;
		ssaoDepthLinearizeRootSignature = nullptr;
		ssaoDepthInterleaveRootSignature = nullptr;
	}
	void JDx12Ssao::ClearPso()
	{ 
		for (uint i = 0; i < SIZE_OF_ARRAY(ssao); ++i)
			ssao[i] = nullptr;
		for (uint i = 0; i < (uint)J_SSAO_BLUR_SHADER::COUNT; ++i)
		{
			for (uint j = 0; j < (uint)J_KERNEL_SIZE::COUNT; ++j)
				ssaoBlur[i][j] = nullptr;
		}
		for (uint i = 0; i < SIZE_OF_ARRAY(ssaoCombine); ++i)
			ssaoCombine[i] = nullptr;
		ssaoDepthLinearize = nullptr;
		ssaoDepthInterleave = nullptr;
	}
	void JDx12Ssao::ClearBuffer()
	{ 
		if (ssaoSampleCB != nullptr)
			ssaoSampleCB->Clear();
		ssaoSampleCB = nullptr;

		for (uint i = 0; i < SIZE_OF_ARRAY(randomVecInfo); ++i)
			JGraphicResourceInfo::Destroy(randomVecInfo[i].Release());
		if (ssaoAoSliceCB != nullptr)
			ssaoAoSliceCB->Clear();
		ssaoAoSliceCB = nullptr;
	}
}

//Kenel stream for debugging
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