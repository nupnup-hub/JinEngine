#include"JDx12ImageProcessing.h"
#include"../JImageProcessingFilter.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceInfo.h" 
#include"../../Shader/Dx/JDx12ShaderDataHolder.h" 
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../Device/Dx/JDx12RootSignature.h"
#include"../../FrameResource/JCameraConstants.h" 
#include"../../FrameResource/Dx/JDx12FrameResource.h" 
#include"../../JGraphicUpdateHelper.h"
#include"../../../Application/JApplicationEngine.h"
#include"../../../Core/Platform/JHardwareInfo.h"
#include"../../../Core/Math/JVectorExtend.h"
#include"../../../Object/Component/Camera/JCamera.h" 
#include"../../../Object/Resource/Texture/JTexture.h"  
#include<random>
//#include"../../../Develop/Debug/JDevelopDebug.h"

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
#define KERNEL_MAX_SIZE L"KERNEL_MAX_SIZE"
#define KERNEL_COUNT_IS_EVEN_NUMBER L"KERNEL_COUNT_IS_EVEN_NUMBER"

#define WIDTH_HEIGHT_EVEN 1
#define WIDTH_EVEN_HEIGHT_ODD 2
#define WIDTH_ODD_HEIGHT_EVEN 3
#define WIDTH_HEIGHT_ODD 4

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
	namespace Blur
	{
		static constexpr uint srcTextureHandleIndex = 0;
		static constexpr uint destTextureHandleIndex = srcTextureHandleIndex + 1;
		static constexpr uint blurPassCBIndex = destTextureHandleIndex + 1;
		static constexpr uint slotCount = blurPassCBIndex + 1;
	}
	namespace DownSample
	{
		static constexpr uint srcTextureHandleIndex = 0;
		static constexpr uint destTextureHandleIndex = srcTextureHandleIndex + 1;
		static constexpr uint downSamplePassCBIndex = destTextureHandleIndex + 1;
		static constexpr uint kenelCBIndex = downSamplePassCBIndex + 1;
		static constexpr uint slotCount = kenelCBIndex + 1;
		static constexpr uint cbPass32BitCount = 8;
	}
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
			 JDx12ImageProcessing::ssaoMaxSampleCount / 8,
			 JDx12ImageProcessing::ssaoMaxSampleCount / 4,
			 JDx12ImageProcessing::ssaoMaxSampleCount / 2,
			 JDx12ImageProcessing::ssaoMaxSampleCount
		};
		static constexpr uint randomWidth = 4; 
		static constexpr uint randomDirCount = 8;
		static constexpr uint mrtCount = 8;
		static constexpr uint splitCount = 4;
		static constexpr uint sliceCount = JDx12ImageProcessing::ssaoSliceCount;
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
		static constexpr uint passCBIndex= depthMapIndex + 1;
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
		static void StuffComputeShaderMacro(_Inout_ JComputeShaderInitData& initHelper)
		{
			initHelper.macro.push_back({ THREAD_DIM_X_SYMBOL, std::to_wstring(initHelper.dispatchInfo.threadDim.x) });
			initHelper.macro.push_back({ THREAD_DIM_Y_SYMBOL, std::to_wstring(initHelper.dispatchInfo.threadDim.y) });
			initHelper.macro.push_back({ THREAD_DIM_Z_SYMBOL, std::to_wstring(initHelper.dispatchInfo.threadDim.z) });

			initHelper.macro.push_back({ THREAD_COUNT, std::to_wstring(1) });
		}
		static void StuffComputeShaderMacro(_Inout_ JComputeShaderInitData& initHelper,
			const J_KERNEL_SIZE size,
			const bool useKenealCB,
			const size_t kenelMaxSize = JKenelType::MaxSize())
		{
			StuffComputeShaderMacro(initHelper);
			initHelper.macro.push_back({ KERNEL_MAX_SIZE, std::to_wstring(kenelMaxSize) });
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
			if (useKenealCB)
				initHelper.macro.push_back({ USE_KERNEL_DATA, std::to_wstring(1) });
		}
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
		static JShaderType::CompileInfo GraphicShaderCompileInfo(const J_SSAO_TYPE type)
		{
			switch (type)
			{
			case J_SSAO_TYPE::DEFAULT:
				return JShaderType::CompileInfo(L"SsaoPs.hlsl", L"SsaoPs");
			case J_SSAO_TYPE::HORIZON_BASED:
				return JShaderType::CompileInfo(L"SsaoPs.hlsl", L"HbaoPs");
			default:
				return JShaderType::CompileInfo(L"Error", L"Error");
			}
		}
		static JShaderType::CompileInfo GraphicShaderCompileInfo(const J_SSAO_BLUR_SHADER type)
		{
			switch (type)
			{
			case J_SSAO_BLUR_SHADER::BILATERAL_X:
				return JShaderType::CompileInfo(L"SsaoBlur.hlsl", L"BilateralBlurX");
			case J_SSAO_BLUR_SHADER::BILATERAL_Y:
				return JShaderType::CompileInfo(L"SsaoBlur.hlsl", L"BilateralBlurY");
			default:
				return JShaderType::CompileInfo(L"Error", L"Error");
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
		static size_t CalSSaoMapKey(const uint width, const uint height)noexcept
		{  
			return std::hash<size_t>{}(width + ((size_t)height << 4));
		}
		static float GetRandomNumber(UINT Index)
		{
			// Mersenne-Twister random numbers in [0,1).
			static const float mersenneTwisterNumbers[1024] = {
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

			const UINT numElements = SIZE_OF_ARRAY(mersenneTwisterNumbers);
			return mersenneTwisterNumbers[Index % numElements];
		} 
		static void BuildSsaoPso(JDx12GraphicShaderDataHolder* holder, 
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
			 
			ThrowIfFailedG(device->CreateGraphicsPipelineState(&newShaderPso, IID_PPV_ARGS(holder->pso.GetAddressOf())));
			holder->pso->SetPrivateData(WKPDID_D3DDebugObjectName, name.size(), name.c_str());
		}
	} 

	JDx12ImageProcessing::SsaoIntermediate::SsaoIntermediate(JGraphicDevice* device, JGraphicResourceManager* gM, const uint width, const uint height)
	{
		JGraphicResourceCreationDesc desc;
		desc.width = width;
		desc.height = height;
	 
		aoIntermediate00 = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERMEDIATE_MAP);
		aoIntermediate01 = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERMEDIATE_MAP);
		depth = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::SSAO_DEPTH_MAP);

		const uint sliceWidth = (width + Ssao::splitCount - 1) / Ssao::splitCount;
		const uint sliceHeight = (height + Ssao::splitCount - 1) / Ssao::splitCount;
		
		desc.width = sliceWidth;
		desc.height = sliceHeight;
		desc.arraySize = Ssao::sliceCount;
		 
		aoInterleave = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERLEAVE_MAP);
		depthInterleave = gM->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::SSAO_DEPTH_INTERLEAVE_MAP);
		 
		sameResolutionInfoCount = 1; 
	}
	JDx12ImageProcessing::SsaoIntermediate::~SsaoIntermediate()
	{
		JGraphicResourceInfo::Destroy(aoIntermediate00.Release());
		JGraphicResourceInfo::Destroy(aoIntermediate01.Release());
		JGraphicResourceInfo::Destroy(aoInterleave.Release());
		JGraphicResourceInfo::Destroy(depth.Release());
		JGraphicResourceInfo::Destroy(depthInterleave.Release());
	}

	JDx12ImageProcessing::SSaoDrawDataSet::SSaoDrawDataSet(JGraphicSsaoComputeSet* computeSet,
		const JDrawHelper& helper,
		const std::unordered_map<size_t, std::unique_ptr<SsaoIntermediate>>& ssaoInterMap,
		const JUserPtr<JGraphicResourceInfo>(&randomInfoVec)[(uint)J_SSAO_TYPE::COUNT])
	{
		JDx12GraphicSsaoComputeSet* ssaoSet = static_cast<JDx12GraphicSsaoComputeSet*>(computeSet);
		dx12Device = static_cast<JDx12GraphicDevice*>(computeSet->device);
		dx12Gm = static_cast<JDx12GraphicResourceManager*>(computeSet->graphicResourceM);
		dx12Frame = static_cast<JDx12FrameResource*>(computeSet->currFrame);
		cmdList = ssaoSet->cmdList;

		ssaoDesc = helper.cam->GetSsaoDesc();
		canBlur = ssaoDesc.useBlur;

		const JUserPtr<JGraphicResourceInfo>& randomInfo = randomInfoVec[(uint)ssaoDesc.ssaoType];
		auto gInterface = helper.cam->GraphicResourceUserInterface();
		int rtDataIndex = gInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		int dsDataIndex = gInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		int aoDataIndex = gInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP, J_GRAPHIC_TASK_TYPE::APPLY_SSAO);
		 
		const JVector2<uint> size = gInterface.GetResourceSize(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, rtDataIndex);
		const size_t key = Private::CalSSaoMapKey(size.x, size.y);
		auto data = ssaoInterMap.find(key);
		if (data == ssaoInterMap.end())
			return;

		inter = data->second.get();
		int rtVecIndex = gInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, rtDataIndex);
		int dsVecIndex = gInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, dsDataIndex);
		int aoVecIndex = gInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP, aoDataIndex);
		 
		auto rtInfo = dx12Gm->GetDxInfo(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, rtVecIndex);
		auto depthInfo = dx12Gm->GetDxInfo(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, dsVecIndex);
		auto aoInfo = dx12Gm->GetDxInfo(J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP, aoVecIndex);
		 
		//16 .. 1, 1, 1, 1, ~
		depthInterleaveRtvHeapSt = inter->depthInterleave->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::RTV) + 1;
		depthInterleaveSrvHeapSt = inter->depthInterleave->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV) + 1;
		 
		depthSrvHandle = dx12Gm->GetGpuSrvDescriptorHandle(depthInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV));
		normalSrvHandle = dx12Gm->GetGpuSrvDescriptorHandle(rtInfo->GetOptionHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV, J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP));
		randomSrvHandle = dx12Gm->GetGpuSrvDescriptorHandle(randomInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV));
		aoRtvHandle = dx12Gm->GetCpuRtvDescriptorHandle(aoInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::RTV));
		
		aoIntermediate00RtvHandle = dx12Gm->GetCpuRtvDescriptorHandle(inter->aoIntermediate00->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::RTV));
		aoIntermediate00SrvHandle = dx12Gm->GetGpuSrvDescriptorHandle(inter->aoIntermediate00->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV));
		aoIntermediate01RtvHandle = dx12Gm->GetCpuRtvDescriptorHandle(inter->aoIntermediate01->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::RTV));
		aoIntermediate01SrvHandle = dx12Gm->GetGpuSrvDescriptorHandle(inter->aoIntermediate01->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV));		
		aoInterleaveRtvHandle = dx12Gm->GetCpuRtvDescriptorHandle(inter->aoInterleave->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::RTV));
		aoInterleaveSrvHandle = dx12Gm->GetGpuSrvDescriptorHandle(inter->aoInterleave->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV));
		 
		aoDepthRtvHandle = dx12Gm->GetCpuRtvDescriptorHandle(inter->depth->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::RTV));
		aoDepthSrvHandle = dx12Gm->GetGpuSrvDescriptorHandle(inter->depth->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV));
		aoDepthInterleaveRtvHandle = dx12Gm->GetCpuRtvDescriptorHandle(inter->depthInterleave->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::RTV));
		aoDepthInterleaveSrvHandle = dx12Gm->GetGpuSrvDescriptorHandle(inter->depthInterleave->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV));
 
		rtResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, rtVecIndex);
		aoResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP, aoVecIndex);
		aoInter00Resource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERMEDIATE_MAP, inter->aoIntermediate00->GetArrayIndex());
		aoInter01Resource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERMEDIATE_MAP, inter->aoIntermediate01->GetArrayIndex());
		aoInterleaveResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::SSAO_INTERLEAVE_MAP, inter->aoInterleave->GetArrayIndex());
		aoDepthResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::SSAO_DEPTH_MAP, inter->depth->GetArrayIndex());
		aoDepthInterleaveResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::SSAO_DEPTH_INTERLEAVE_MAP, inter->depthInterleave->GetArrayIndex());
		isValid = true;
	}

	JDx12ImageProcessing::JDx12ImageProcessing()
		:guid(Core::MakeGuid())
	{}
	void JDx12ImageProcessing::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicBaseDataSet& baseDataSet)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(gM);
		ID3D12Device* d3d12Device = dx12Device->GetDevice();

		BuildRootSingnature(d3d12Device, baseDataSet.info, baseDataSet.option);
		BuildPso(d3d12Device, baseDataSet.info, baseDataSet.option);
		BuildBuffer(dx12Device, dx12Gm);
	}
	void JDx12ImageProcessing::Clear()
	{ 
		ClearRootSignature();
		ClearPso();
		ClearBuffer();
		ssaoInterMap.clear();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12ImageProcessing::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	void JDx12ImageProcessing::ApplyBlur(JGraphicBlurComputeSet* computeSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(computeSet))
			return;

		JDx12GraphicBlurComputeSet* blurSet = static_cast<JDx12GraphicBlurComputeSet*>(computeSet);
		BlurConstants constants;
		constants.size = blurSet->desc->imageSize;
		constants.invSize = 1.0f / blurSet->desc->imageSize;
		constants.mipLevel = blurSet->desc->mipLevel;

		if (blurSet->desc->GetBlurType() == J_BLUR_TYPE::BOX)
		{
			auto boxDesc = static_cast<JBoxBlurDesc*>(blurSet->desc.get());
			switch (boxDesc->kernelSize)
			{
			case JinEngine::Graphic::J_KERNEL_SIZE::_3x3:
			{
				JFilter::ComputeFilter<3, JKenelType::MaxSize(), true, true>(constants.kernel, &JFilter::Box, 9);
				break;
			}
			case JinEngine::Graphic::J_KERNEL_SIZE::_5x5:
			{
				JFilter::ComputeFilter<5, JKenelType::MaxSize(), true, true>(constants.kernel, &JFilter::Box, 25);
				break;
			}
			case JinEngine::Graphic::J_KERNEL_SIZE::_7x7:
			{
				JFilter::ComputeFilter<7, JKenelType::MaxSize(), true, true>(constants.kernel, &JFilter::Box, 49);
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
			case JinEngine::Graphic::J_KERNEL_SIZE::_3x3:
			{
				JFilter::ComputeFilter<3, JKenelType::MaxSize(), true, true>(constants.kernel, &JFilter::Gaus, gausDesc->sigma);
				break;
			}
			case JinEngine::Graphic::J_KERNEL_SIZE::_5x5:
			{
				JFilter::ComputeFilter<5, JKenelType::MaxSize(), true, true>(constants.kernel, &JFilter::Gaus, gausDesc->sigma);
				break;
			}
			case JinEngine::Graphic::J_KERNEL_SIZE::_7x7:
			{
				JFilter::ComputeFilter<7, JKenelType::MaxSize(), true, true>(constants.kernel, &JFilter::Gaus, gausDesc->sigma);
				break;
			}
			default:
				break;
			}
		}
		blurCB->CopyData(helper.info.currFrameResourceIndex, constants);

		blurSet->cmdList->SetComputeRootSignature(blurRootSignature.Get());
		blurSet->cmdList->SetComputeRootDescriptorTable(Blur::srcTextureHandleIndex, blurSet->srcHandle);
		blurSet->cmdList->SetComputeRootDescriptorTable(Blur::destTextureHandleIndex, blurSet->destHandle);
		blurCB->SetComputeCBBufferView(blurSet->cmdList, Blur::blurPassCBIndex, helper.info.currFrameResourceIndex);

		auto shaderData = blur[(uint)blurSet->desc->GetBlurType()][(uint)blurSet->desc->kernelSize].get();
		auto groupDim = shaderData->dispatchInfo.groupDim;

		blurSet->cmdList->SetPipelineState(shaderData->pso.Get());
		blurSet->cmdList->Dispatch(groupDim.x, groupDim.y, groupDim.z);
	}
	void JDx12ImageProcessing::ApplyMipmapGeneration(JGraphicDownSampleComputeSet* computeSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(computeSet))
			return;

		JDx12GraphicDownSampleComputeSet* downSamSet = static_cast<JDx12GraphicDownSampleComputeSet*>(computeSet);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(downSamSet->graphicResourceM);
		DownSampleConstants constants;

		if (downSamSet->desc->GetDownSampleType() == J_DOWN_SAMPLING_TYPE::GAUSIAAN)
		{
			auto gausDesc = static_cast<JGaussianDownSampleDesc*>(downSamSet->desc.get());
			switch (gausDesc->kernelSize)
			{
			case JinEngine::Graphic::J_KERNEL_SIZE::_3x3:
			{
				JFilter::ComputeFilter<3, JKenelType::MaxSize(), true, true>(constants.kernel, &JFilter::Gaus, gausDesc->sigma);
				break;
			}
			case JinEngine::Graphic::J_KERNEL_SIZE::_5x5:
			{
				JFilter::ComputeFilter<5, JKenelType::MaxSize(), true, true>(constants.kernel, &JFilter::Gaus, gausDesc->sigma);
				break;
			}
			case JinEngine::Graphic::J_KERNEL_SIZE::_7x7:
			{
				JFilter::ComputeFilter<7, JKenelType::MaxSize(), true, true>(constants.kernel, &JFilter::Gaus, gausDesc->sigma);
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
			case JinEngine::Graphic::J_KERNEL_SIZE::_3x3:
			{
				JFilter::ComputeFilter<3, JKenelType::MaxSize(), false, true>(constants.kernel, &JFilter::Kaiser, kaiserDesc->beta, 2);
				break;
			}
			case JinEngine::Graphic::J_KERNEL_SIZE::_5x5:
			{
				JFilter::ComputeFilter<5, JKenelType::MaxSize(), false, true>(constants.kernel, &JFilter::Kaiser, kaiserDesc->beta, 4);
				break;
			}
			case JinEngine::Graphic::J_KERNEL_SIZE::_7x7:
			{
				JFilter::ComputeFilter<7, JKenelType::MaxSize(), false, true>(constants.kernel, &JFilter::Kaiser, kaiserDesc->beta, 6);
				break;
			}
			default:
				break;
			}
		}
		downSampleCB->CopyData(helper.info.currFrameResourceIndex, constants);

		auto shaderData = downSample[(uint)downSamSet->desc->GetDownSampleType()][(uint)downSamSet->desc->kernelSize].get();
		auto dispatchInfo = shaderData->dispatchInfo;
		auto dispatchDim = JVector2<uint>(dispatchInfo.groupDim.x * dispatchInfo.threadDim.x, dispatchInfo.groupDim.y * dispatchInfo.threadDim.y);

		CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle = downSamSet->srcHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle = downSamSet->destHandle;
		uint srvSize = dx12Gm->GetDescriptorSize(J_GRAPHIC_BIND_TYPE::SRV);
		uint uavSize = dx12Gm->GetDescriptorSize(J_GRAPHIC_BIND_TYPE::UAV);

		JVector2<uint> srcSize = downSamSet->desc->imageSize;
		JVector2<uint> destSize = srcSize / 2.0f;

		downSamSet->cmdList->SetPipelineState(shaderData->pso.Get());
		downSamSet->cmdList->SetComputeRootSignature(downSampleRootSignature.Get());
		downSampleCB->SetComputeCBBufferView(downSamSet->cmdList, DownSample::kenelCBIndex, helper.info.currFrameResourceIndex);

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

			JVector4<uint> dataSet;
			dataSet.x = mipLevel;
			dataSet.y = samplingState;
			dataSet.z = dispatchDim.x;
			dataSet.w = dispatchDim.y;

			downSamSet->cmdList->SetComputeRootDescriptorTable(DownSample::srcTextureHandleIndex, srcHandle);
			downSamSet->cmdList->SetComputeRootDescriptorTable(DownSample::destTextureHandleIndex, destHandle);
			downSamSet->cmdList->SetComputeRoot32BitConstants(DownSample::downSamplePassCBIndex, 2, &srcSize, 0);
			downSamSet->cmdList->SetComputeRoot32BitConstants(DownSample::downSamplePassCBIndex, 2, &destSize, 2);
			downSamSet->cmdList->SetComputeRoot32BitConstants(DownSample::downSamplePassCBIndex, 4, &dataSet, 4);

			downSamSet->cmdList->Dispatch(dispatchInfo.groupDim.x, dispatchInfo.groupDim.y, dispatchInfo.groupDim.z);
			srcHandle.Offset(srvSize);
			destHandle.Offset(uavSize);
			srcSize /= 2;
			destSize /= 2;

			srcSize = JVector2F::Clamp(srcSize, JVector2F(1.0f, 1.0f), srcSize);
			destSize = JVector2F::Clamp(destSize, JVector2F(1.0f, 1.0f), destSize);
		}
	}
	void JDx12ImageProcessing::ApplySsao(JGraphicSsaoComputeSet* computeSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(computeSet) || helper.cam == nullptr || !helper.allowSsao)
			return;

		JDx12GraphicSsaoComputeSet* ssaoSet = static_cast<JDx12GraphicSsaoComputeSet*>(computeSet);
		SSaoDrawDataSet set(computeSet, helper, ssaoInterMap, randomVecInfo);
		if (!set.isValid)
			return;

		set.canUseHbaoInterleave = helper.option.useSsaoInterleave && set.ssaoDesc.ssaoType == J_SSAO_TYPE::HORIZON_BASED;
		JD3DUtility::ResourceTransition(set.cmdList, set.aoResource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
		if (set.canBlur)
		{
			JD3DUtility::ResourceTransition(set.cmdList, set.aoInter00Resource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
			JD3DUtility::ResourceTransition(set.cmdList, set.aoInter01Resource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
		} 
		if (set.canUseHbaoInterleave)
		{
			//transition interleave resource and call draw func
			JD3DUtility::ResourceTransition(set.cmdList, set.aoDepthResource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
			JD3DUtility::ResourceTransition(set.cmdList, set.aoDepthInterleaveResource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
			JD3DUtility::ResourceTransition(set.cmdList, set.aoInterleaveResource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);

			DepthMapLinearize(set, helper);
			JD3DUtility::ResourceTransition(set.cmdList, set.aoDepthResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);

			DepthMapInetrleave(set, helper);
			JD3DUtility::ResourceTransition(set.cmdList, set.aoDepthInterleaveResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);

			DrawSsaoMap(set, helper);
			JD3DUtility::ResourceTransition(set.cmdList, set.aoInterleaveResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);

			CombineSsaoMap(set, helper); 
			BlurSsao(set, helper);
		}
		else
		{
			DrawSsaoMap(set, helper); 
			BlurSsao(set, helper); 
		}
		JD3DUtility::ResourceTransition(set.cmdList, set.aoResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	}
	void JDx12ImageProcessing::DepthMapLinearize(const SSaoDrawDataSet& set, const JDrawHelper& helper)
	{
		set.cmdList->OMSetRenderTargets(1, &set.aoDepthRtvHandle, true, nullptr);
		BindViewPortAndRect(set.dx12Device, set.cmdList, set.aoDepthResource);

		auto shaderData = ssaoDepthLinearize.get();
		set.cmdList->SetPipelineState(shaderData->pso.Get());

		set.cmdList->SetGraphicsRootSignature(ssaoDepthLinearizeRootSignature.Get());
		set.cmdList->SetGraphicsRootDescriptorTable(SsaoDepthLinearlize::depthMapIndex, set.depthSrvHandle);
		set.dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::SSAO_PASS)->SetGraphicCBBufferView(set.cmdList, SsaoDepthLinearlize::passCBIndex, helper.GetCamSsaoFrameIndex());
		 
		DrawFullScreenQuad(set.dx12Gm, set.cmdList, shaderData->pso.Get());
	}
	void JDx12ImageProcessing::DepthMapInetrleave(const SSaoDrawDataSet& set, const JDrawHelper& helper)
	{
		auto shaderData = ssaoDepthInterleave.get();
		set.cmdList->SetPipelineState(shaderData->pso.Get());

		set.cmdList->SetGraphicsRootSignature(ssaoDepthInterleaveRootSignature.Get());
		set.cmdList->SetGraphicsRootDescriptorTable(SsaoDepthInterleave::depthMapIndex, set.aoDepthSrvHandle);
		set.dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::SSAO_PASS)->SetGraphicCBBufferView(set.cmdList, SsaoDepthInterleave::passCBIndex, helper.GetCamSsaoFrameIndex());
		BindViewPortAndRect(set.dx12Device, set.cmdList, set.aoDepthInterleaveResource);	//quater

		for (uint i = 0; i < Ssao::sliceCount; i += Ssao::mrtCount)
		{
			ssaoAoSliceCB->SetGraphicCBBufferView(set.cmdList, SsaoDepthInterleave::sliceCBIndex, i); 
			auto rtvHandle = set.dx12Gm->GetCpuRtvDescriptorHandle(set.depthInterleaveRtvHeapSt + i);
			set.cmdList->OMSetRenderTargets(Ssao::mrtCount, &rtvHandle, true, nullptr);
			DrawFullScreenQuad(set.dx12Gm, set.cmdList, shaderData->pso.Get());
		} 	 
	}
	void JDx12ImageProcessing::DrawSsaoMap(const SSaoDrawDataSet& set, const JDrawHelper& helper)
	{
		const uint shaderInedx = Private::CalIndex(set.ssaoDesc.ssaoType, set.ssaoDesc.sampleType, set.canBlur ? J_SSAO_FUNCTION::BLUR : J_SSAO_FUNCTION::NONE);
		auto shaderData = ssao[shaderInedx].get();
		set.cmdList->SetPipelineState(shaderData->pso.Get());
		set.cmdList->SetGraphicsRootSignature(set.ssaoDesc.ssaoType == J_SSAO_TYPE::DEFAULT ? ssaoRootSignature.Get() : hbaoRootSignature.Get());
		set.dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::SSAO_PASS)->SetGraphicCBBufferView(set.cmdList, Ssao::passCBIndex, helper.GetCamSsaoFrameIndex());
 
		if (set.canUseHbaoInterleave)
		{
			set.cmdList->OMSetRenderTargets(1, &set.aoInterleaveRtvHandle, false, nullptr);
			BindViewPortAndRect(set.dx12Device, set.cmdList, set.aoInterleaveResource);

			set.cmdList->SetGraphicsRootDescriptorTable(Ssao::normalMapIndex, set.normalSrvHandle);
			for (uint i = 0; i < Ssao::sliceCount; ++i)
			{
				ssaoAoSliceCB->SetGraphicCBBufferView(set.cmdList, Ssao::typePerCBIndex, i);
				set.cmdList->SetGraphicsRootDescriptorTable(Ssao::depthMapIndex, set.dx12Gm->GetGpuSrvDescriptorHandle(set.depthInterleaveSrvHeapSt + i));
				DrawFullScreenQuad(set.dx12Gm, set.cmdList, shaderData->pso.Get());
			}
		}
		else
		{
			if (set.ssaoDesc.ssaoType == J_SSAO_TYPE::DEFAULT)
				ssaoSampleCB->SetGraphicCBBufferView(set.cmdList, Ssao::typePerCBIndex, (uint)set.ssaoDesc.sampleType);
			set.cmdList->SetGraphicsRootDescriptorTable(Ssao::depthMapIndex, set.depthSrvHandle);
			set.cmdList->SetGraphicsRootDescriptorTable(Ssao::normalMapIndex, set.normalSrvHandle);
			set.cmdList->SetGraphicsRootDescriptorTable(Ssao::randomMapIndex, set.randomSrvHandle);
			set.cmdList->OMSetRenderTargets(1, set.canBlur ? &set.aoIntermediate00RtvHandle : &set.aoRtvHandle, false, nullptr);
			BindViewPortAndRect(set.dx12Device, set.cmdList, set.canBlur ? set.aoInter00Resource : set.aoResource);
			DrawFullScreenQuad(set.dx12Gm, set.cmdList, shaderData->pso.Get());
		}
	}
	void JDx12ImageProcessing::CombineSsaoMap(const SSaoDrawDataSet& set, const JDrawHelper& helper)
	{
		auto shaderData = ssaoCombine[(uint)set.canBlur ? (uint)J_SSAO_FUNCTION::BLUR : (uint)J_SSAO_FUNCTION::NONE].get();
		set.cmdList->SetPipelineState(shaderData->pso.Get());
		set.cmdList->SetGraphicsRootSignature(ssaoCombineRootSignature.Get());

		set.cmdList->SetGraphicsRootDescriptorTable(SsaoCombine::aoMapIndex, set.aoInterleaveSrvHandle);
		set.cmdList->SetGraphicsRootDescriptorTable(SsaoCombine::depthMapIndex, set.aoDepthSrvHandle);
		set.dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::SSAO_PASS)->SetGraphicCBBufferView(set.cmdList, SsaoCombine::passCBIndex, helper.GetCamSsaoFrameIndex());
		
		if (set.canBlur)
			set.cmdList->OMSetRenderTargets(1, &set.aoIntermediate00RtvHandle, false, nullptr);
		else
			set.cmdList->OMSetRenderTargets(1, &set.aoRtvHandle, false, nullptr);
		BindViewPortAndRect(set.dx12Device, set.cmdList, set.aoInter00Resource);
		DrawFullScreenQuad(set.dx12Gm, set.cmdList, shaderData->pso.Get());
	}
	void JDx12ImageProcessing::BlurSsao(const SSaoDrawDataSet& set, const JDrawHelper& helper)
	{
		if (!set.canBlur)
			return;

		if (set.ssaoDesc.blurType == J_SSAO_BLUR_TYPE::BILATERAL)
		{
			JD3DUtility::ResourceTransition(set.cmdList, set.aoInter00Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);

			auto shaderData = ssaoBlur[(uint)J_SSAO_BLUR_SHADER::BILATERAL_X][(uint)set.ssaoDesc.blurKenelSize].get();
			set.cmdList->SetPipelineState(shaderData->pso.Get());
			set.cmdList->SetGraphicsRootSignature(ssaoBlurRootSignature.Get());
			set.cmdList->SetGraphicsRootDescriptorTable(SsaoBlur::srcMapIndex, set.aoIntermediate00SrvHandle);
			set.dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::SSAO_PASS)->SetGraphicCBBufferView(set.cmdList, SsaoBlur::passCBIndex, helper.GetCamSsaoFrameIndex());
		 
			set.cmdList->OMSetRenderTargets(1, &set.aoIntermediate01RtvHandle, false, nullptr);
			BindViewPortAndRect(set.dx12Device, set.cmdList, set.aoInter01Resource);
			DrawFullScreenQuad(set.dx12Gm, set.cmdList, shaderData->pso.Get());

			JD3DUtility::ResourceTransition(set.cmdList, set.aoInter01Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);

			shaderData = ssaoBlur[(uint)J_SSAO_BLUR_SHADER::BILATERAL_Y][(uint)set.ssaoDesc.blurKenelSize].get();
			set.cmdList->SetGraphicsRootDescriptorTable(SsaoBlur::srcMapIndex, set.aoIntermediate01SrvHandle);
			set.cmdList->OMSetRenderTargets(1, &set.aoRtvHandle, false, nullptr);
			BindViewPortAndRect(set.dx12Device, set.cmdList, set.aoResource);
			DrawFullScreenQuad(set.dx12Gm, set.cmdList, shaderData->pso.Get());
		}
		else
		{
			JD3DUtility::ResourceTransition(set.cmdList, set.aoInter00Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
			JD3DUtility::ResourceTransition(set.cmdList, set.aoInter01Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
		}
	}
	void JDx12ImageProcessing::BindViewPortAndRect(JDx12GraphicDevice* device, ID3D12GraphicsCommandList* cmdList, ID3D12Resource* rtResource)
	{
		D3D12_RESOURCE_DESC rtDesc = rtResource->GetDesc();
		D3D12_VIEWPORT viewPort;
		D3D12_RECT rect;
		device->CalViewportAndRect(JVector2F(rtDesc.Width, rtDesc.Height), true, viewPort, rect);
		cmdList->RSSetViewports(1, &viewPort);
		cmdList->RSSetScissorRects(1, &rect);
	}
	void JDx12ImageProcessing::DrawFullScreenQuad(JDx12GraphicResourceManager* gm, ID3D12GraphicsCommandList* cmdList, ID3D12PipelineState* pso)
	{
		//cmdList->IASetVertexBuffers(0, 1, &vertexPtr);
		//cmdList->IASetIndexBuffer(&indexPtr);
		cmdList->IASetVertexBuffers(0, 0, nullptr);
		cmdList->IASetIndexBuffer(nullptr);
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		cmdList->SetPipelineState(pso);
		cmdList->DrawInstanced(3, 1, 0, 0); 
	}
	void JDx12ImageProcessing::RecompileShader(const JGraphicShaderCompileSet& dataSet)
	{
		ClearRootSignature();
		ClearPso();

		ID3D12Device* d3d12Device = static_cast<JDx12GraphicDevice*>(dataSet.device)->GetDevice();
		BuildRootSingnature(d3d12Device, dataSet.base.info, dataSet.base.option);
		BuildPso(d3d12Device, dataSet.base.info, dataSet.base.option);
	}
	void JDx12ImageProcessing::TryCreationSsaoIntermediate(JGraphicDevice* device, JGraphicResourceManager* gM, JGraphicResourceInfo* newSsao)
	{
		if (newSsao == nullptr || !IsSameDevice(device) || !IsSameDevice(gM))
			return;

		const uint width = newSsao->GetWidth();
		const uint height = newSsao->GetHeight();
		const size_t key = Private::CalSSaoMapKey(width, height);
		auto data = ssaoInterMap.find(key);		 
		if (data == ssaoInterMap.end())
		{ 
			ssaoInterMap.emplace(key, std::make_unique<SsaoIntermediate>(device, gM, width, height));
			data = ssaoInterMap.find(key);
		}

		++data->second->sameResolutionInfoCount;
	}
	void JDx12ImageProcessing::TryDestructionSsaoIntermediate(JGraphicDevice* device, JGraphicResourceManager* gM, JGraphicResourceInfo* ssao)
	{
		if (ssao == nullptr || !IsSameDevice(device) || !IsSameDevice(device) || !IsSameDevice(gM))
			return;
		 
		const size_t key = Private::CalSSaoMapKey(ssao->GetWidth(), ssao->GetHeight());
		auto data = ssaoInterMap.find(key);
		if (data == ssaoInterMap.end())
			return;

		--data->second->sameResolutionInfoCount;
		if (data->second->sameResolutionInfoCount < 1)
			ssaoInterMap.erase(key);
	}
	void JDx12ImageProcessing::BuildRootSingnature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		BuildBlurRootSignature(device);
		BuildDownSampleRootSignature(device);
		BuildSsaoRootSignature(device, option);
		BuildSsaoBlurRootSignature(device);
		BuildSsaoCombineRootSignature(device);
		BuildSsaoDepthLinearizeRootSignature(device);
		BuildSsaoDepthInterleaveRootSignature(device);
	}
	void JDx12ImageProcessing::BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{ 
		BuildBlurPso(device, info);
		BuildDownSamplePso(device, info);
		BuildSsaoPso(device, info, option);
		BuildSsaoBlurPso(device, info);
		BuildSsaoCombinePso(device, info);
		BuildSsaoDepthLinearizePso(device, info);
		BuildSsaoDepthInterleavePso(device, info);
	}
	void JDx12ImageProcessing::BuildBuffer(JDx12GraphicDevice* device, JDx12GraphicResourceManager* gm)
	{
		if (blurCB == nullptr)
			blurCB = std::make_unique<JDx12GraphicBuffer<BlurConstants>>(L"BlurCB", J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT);
		blurCB->Clear();
		blurCB->Build(device, Constants::gNumFrameResources);

		if (downSampleCB == nullptr)
			downSampleCB = std::make_unique<JDx12GraphicBuffer<DownSampleConstants>>(L"DownSampleCB", J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT);
		downSampleCB->Clear();
		downSampleCB->Build(device, Constants::gNumFrameResources);

		std::random_device rd;	//use hardware
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> disSNorm(-1.0f, 1.0f);
		std::uniform_real_distribution<float> disUNorm(0.0f, 1.0f);

		if (ssaoSampleCB == nullptr)
			ssaoSampleCB = std::make_unique<JDx12GraphicBuffer<SsaoSampleConstants>>(L"SSAOSample", J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT);

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
		desc.textureDesc = std::make_unique<JTextureCreationDesc>(ssaoRandomVec);
		desc.textureDesc->mipMapDesc.type = J_GRAPHIC_MIP_MAP_TYPE::NONE;

		randomVecInfo[(uint)J_SSAO_TYPE::DEFAULT] = gm->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D);

		randomCount = Ssao::randomWidth * Ssao::randomWidth;
		std::vector<JVector4F> hbaoRandomVec(randomCount);
		uint k = 0;
		for (uint i = 0; i < randomCount; ++i)
		{
			float r1 = Private::GetRandomNumber(k++);
			float r2 = Private::GetRandomNumber(k++);
			float r3 = Private::GetRandomNumber(k++);

			// Use random rotation angles in [0,2PI/NUM_DIRECTIONS).
			float angle = 2.0f * JMathHelper::Pi * r1 / Ssao::randomDirCount;
			hbaoRandomVec[i].x = cosf(angle);
			hbaoRandomVec[i].y = sinf(angle);
			hbaoRandomVec[i].z = r2;
			hbaoRandomVec[i].w = r3;

			/*

			float r1 = disSNorm(gen);
			// Use random rotation angles in [0,2PI/NUM_DIRECTIONS).
			float angle = 2.0f * JMathHelper::Pi * r1 / Ssao::randomDirCount;
			hbaoRandomVec[i].x = cosf(angle);
			hbaoRandomVec[i].y = sinf(angle);
			hbaoRandomVec[i].z = disUNorm(gen);
			hbaoRandomVec[i].w = disUNorm(gen);
			*/
		}

		desc.width = Ssao::randomWidth;
		desc.height = Ssao::randomWidth;
		desc.textureDesc = std::make_unique<JTextureCreationDesc>(hbaoRandomVec);
		desc.textureDesc->mipMapDesc.type = J_GRAPHIC_MIP_MAP_TYPE::NONE;

		randomVecInfo[(uint)J_SSAO_TYPE::HORIZON_BASED] = gm->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D);

		ssaoAoSliceCB = std::make_unique<JDx12GraphicBuffer<SsaoAoSliceConstants>>(L"SsaoAoSliceCB", J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT);
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
	void JDx12ImageProcessing::BuildBlurRootSignature(ID3D12Device* device)
	{
		CD3DX12_DESCRIPTOR_RANGE srcTable;
		srcTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_DESCRIPTOR_RANGE destTable;
		destTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		CD3DX12_ROOT_PARAMETER slotRootParameter[Blur::slotCount];
		slotRootParameter[Blur::srcTextureHandleIndex].InitAsDescriptorTable(1, &srcTable);
		slotRootParameter[Blur::destTextureHandleIndex].InitAsDescriptorTable(1, &destTable);
		slotRootParameter[Blur::blurPassCBIndex].InitAsConstantBufferView(0);

		CD3DX12_STATIC_SAMPLER_DESC sampler(0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(Blur::slotCount, slotRootParameter, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_NONE);

		JDx12RootSignature::Create(device, &rootSigDesc, L"BlurRootSignature", blurRootSignature.GetAddressOf());
	}
	void JDx12ImageProcessing::BuildBlurPso(ID3D12Device* device, const JGraphicInfo& info)
	{
		for (uint i = 0; i < (uint)J_BLUR_TYPE::COUNT; ++i)
		{
			for (uint j = 0; j < (uint)J_KERNEL_SIZE::COUNT; ++j)
			{
				const J_BLUR_TYPE type = (J_BLUR_TYPE)i;
				const J_KERNEL_SIZE size = (J_KERNEL_SIZE)j;

				blur[i][j] = std::make_unique<JDx12ComputeShaderDataHolder>();
				JDx12ComputeShaderDataHolder* holder = blur[i][j].get();

				JComputeShaderInitData initData;
				Private::StuffComputeShaderDispatchInfo(initData, info);
				Private::StuffComputeShaderMacro(initData, size, true);

				auto compileInfo = Private::ComputeShaderCompileInfo(type);

				holder->cs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro, compileInfo.functionName, L"cs_6_0");
				holder->dispatchInfo = initData.dispatchInfo;

				D3D12_COMPUTE_PIPELINE_STATE_DESC newShaderPso;
				ZeroMemory(&newShaderPso, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
				newShaderPso.pRootSignature = blurRootSignature.Get();
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
		slotRootParameter[DownSample::downSamplePassCBIndex].InitAsConstants(DownSample::cbPass32BitCount, 0);
		slotRootParameter[DownSample::kenelCBIndex].InitAsConstantBufferView(1);

		CD3DX12_STATIC_SAMPLER_DESC sampler(0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(DownSample::slotCount, slotRootParameter, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_NONE);
		JDx12RootSignature::Create(device, &rootSigDesc, L"DownSampleRootSignature", downSampleRootSignature.GetAddressOf());
	}
	void JDx12ImageProcessing::BuildDownSamplePso(ID3D12Device* device, const JGraphicInfo& info)
	{
		for (uint i = 0; i < (uint)J_DOWN_SAMPLING_TYPE::COUNT; ++i)
		{
			for (uint j = 0; j < (uint)J_KERNEL_SIZE::COUNT; ++j)
			{
				const J_DOWN_SAMPLING_TYPE type = (J_DOWN_SAMPLING_TYPE)i;
				const J_KERNEL_SIZE size = (J_KERNEL_SIZE)j;

				downSample[i][j] = std::make_unique<JDx12ComputeShaderDataHolder>();
				JDx12ComputeShaderDataHolder* holder = downSample[i][j].get();

				JComputeShaderInitData initData;
				Private::StuffComputeShaderDispatchInfo(initData, info);
				Private::StuffComputeShaderMacro(initData, size, true);

				auto compileInfo = Private::ComputeShaderCompileInfo(type);

				holder->cs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro, compileInfo.functionName, L"cs_6_0");
				holder->dispatchInfo = initData.dispatchInfo;

				D3D12_COMPUTE_PIPELINE_STATE_DESC newShaderPso;
				ZeroMemory(&newShaderPso, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
				newShaderPso.pRootSignature = downSampleRootSignature.Get();

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
	void JDx12ImageProcessing::BuildSsaoRootSignature(ID3D12Device* device, const JGraphicOption& option)
	{
		CD3DX12_DESCRIPTOR_RANGE depthMapTable;
		depthMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_DESCRIPTOR_RANGE normalMapTable;
		normalMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

		CD3DX12_DESCRIPTOR_RANGE randomMapTable;
		randomMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);

		CD3DX12_ROOT_PARAMETER slotRootParameter[Ssao::ssaoSlotCount];
		slotRootParameter[Ssao::passCBIndex].InitAsConstantBufferView(0);
		slotRootParameter[Ssao::typePerCBIndex].InitAsConstantBufferView(1);
		slotRootParameter[Ssao::depthMapIndex].InitAsDescriptorTable(1, &depthMapTable);
		slotRootParameter[Ssao::normalMapIndex].InitAsDescriptorTable(1, &normalMapTable);
		slotRootParameter[Ssao::randomMapIndex].InitAsDescriptorTable(1, &randomMapTable);

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

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(Ssao::ssaoSlotCount,
			slotRootParameter,
			(uint)sam.size(), sam.data(),
			D3D12_ROOT_SIGNATURE_FLAG_NONE);

		JDx12RootSignature::Create(device, &rootSigDesc, L"SsaoRootSignature", ssaoRootSignature.GetAddressOf());
	
		const uint slotCount = option.useSsaoInterleave ? Ssao::hbaoSlotCount - 1 : Ssao::hbaoSlotCount;
		const uint samCount = option.useSsaoInterleave ? (uint)sam.size() - 1 : (uint)sam.size();

		rootSigDesc = CD3DX12_ROOT_SIGNATURE_DESC(slotCount,
			slotRootParameter,
			samCount, sam.data(),
			D3D12_ROOT_SIGNATURE_FLAG_NONE);
		//slotRootParameter[Ssao::sliceCBIndex].InitAsConstants(8, 1); 
		JDx12RootSignature::Create(device, &rootSigDesc, L"hbaoRootSignature", hbaoRootSignature.GetAddressOf());

	}
	void JDx12ImageProcessing::BuildSsaoPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
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
					const bool canUseInterleave = option.useSsaoInterleave && isHbao;

					const uint index = Private::CalIndex(type, sampleType, function); 
					ssao[index] = std::make_unique<JDx12GraphicShaderDataHolder>();
					auto* holder = ssao[index].get();

					JGraphicShaderInitData initData;
					Private::StuffGraphicShaderMacro(initData, type, sampleType, function);
					if(canUseInterleave)
						initData.macro[0].push_back({ USE_SSAO_INTERLEAVE, std::to_wstring(1) });

					auto compileInfo = Private::GraphicShaderCompileInfo(type);
					holder->vs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro[0], L"FullScreenTriangleVS", L"vs_6_0");
					holder->ps = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro[0], compileInfo.functionName, L"ps_6_0");
					if (canUseInterleave)
					{
						auto gsInfo = JShaderType::CompileInfo(L"SsaoGs.hlsl", L"SsaoGs");
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
	void JDx12ImageProcessing::BuildSsaoBlurRootSignature(ID3D12Device* device)
	{
		CD3DX12_DESCRIPTOR_RANGE srcMapTable;
		srcMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		  
		CD3DX12_ROOT_PARAMETER slotRootParameter[SsaoBlur::slotCount];
		slotRootParameter[SsaoBlur::srcMapIndex].InitAsDescriptorTable(1, &srcMapTable); 
		slotRootParameter[SsaoBlur::passCBIndex].InitAsConstantBufferView(0); 

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

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(SsaoBlur::slotCount,
			slotRootParameter,
			(uint)sam.size(), sam.data(),
			D3D12_ROOT_SIGNATURE_FLAG_NONE);

		JDx12RootSignature::Create(device, &rootSigDesc, L"SsaoBlurRootSignature", ssaoBlurRootSignature.GetAddressOf());
	}
	void JDx12ImageProcessing::BuildSsaoBlurPso(ID3D12Device* device, const JGraphicInfo& info)
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
	void JDx12ImageProcessing::BuildSsaoCombineRootSignature(ID3D12Device* device)
	{
		CD3DX12_DESCRIPTOR_RANGE aoMapTable;
		aoMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_DESCRIPTOR_RANGE depthMapTable;
		depthMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

		CD3DX12_ROOT_PARAMETER slotRootParameter[SsaoCombine::slotCount];
		slotRootParameter[SsaoCombine::aoMapIndex].InitAsDescriptorTable(1, &aoMapTable);
		slotRootParameter[SsaoCombine::depthMapIndex].InitAsDescriptorTable(1, &depthMapTable);
		slotRootParameter[SsaoCombine::passCBIndex].InitAsConstantBufferView(0);

		std::vector< CD3DX12_STATIC_SAMPLER_DESC> sam
		{
			CD3DX12_STATIC_SAMPLER_DESC(0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP) // addressW 
		};
		 
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(SsaoCombine::slotCount,
			slotRootParameter,
			(uint)sam.size(), sam.data(),
			D3D12_ROOT_SIGNATURE_FLAG_NONE);

		JDx12RootSignature::Create(device, &rootSigDesc, L"SsaoCombineRootSignature", ssaoCombineRootSignature.GetAddressOf());
	}
	void JDx12ImageProcessing::BuildSsaoCombinePso(ID3D12Device* device, const JGraphicInfo& info)
	{ 
		for (uint i = 0; i < (uint)J_SSAO_FUNCTION::COUNT; ++i)
		{
			const J_SSAO_FUNCTION func = (J_SSAO_FUNCTION)i; 

			ssaoCombine[i] = std::make_unique<JDx12GraphicShaderDataHolder>();
			auto* holder = ssaoCombine[i].get();

			JGraphicShaderInitData initData;
			Private::StuffGraphicShaderMacro(initData, func);
			initData.macro[0].push_back({ USE_HBAO, std::to_wstring(1) });

			auto compileInfo = JShaderType::CompileInfo(L"SsaoCombine.hlsl", L"SsaoCombine");
			holder->vs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro[0], L"FullScreenTriangleVS", L"vs_6_0");
			holder->ps = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro[0], compileInfo.functionName, L"ps_6_0");
			
			const DXGI_FORMAT format = func == J_SSAO_FUNCTION::BLUR ? DXGI_FORMAT_R16G16_UNORM : DXGI_FORMAT_R16_UNORM;
			Private::BuildSsaoPso(holder, device, ssaoCombineRootSignature.Get(), format, 1, "SsaoCombinePso");		 
		}
	}
	void JDx12ImageProcessing::BuildSsaoDepthLinearizeRootSignature(ID3D12Device* device)
	{
		CD3DX12_DESCRIPTOR_RANGE depthMapTable;
		depthMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_ROOT_PARAMETER slotRootParameter[SsaoDepthLinearlize::slotCount];
		slotRootParameter[SsaoDepthLinearlize::depthMapIndex].InitAsDescriptorTable(1, &depthMapTable);
		slotRootParameter[SsaoDepthLinearlize::passCBIndex].InitAsConstantBufferView(0);

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(SsaoDepthLinearlize::slotCount,
			slotRootParameter,
			0, nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_NONE);

		JDx12RootSignature::Create(device, &rootSigDesc, L"SsaoDepthLinearizeRootSignature", ssaoDepthLinearizeRootSignature.GetAddressOf());
	}
	void JDx12ImageProcessing::BuildSsaoDepthLinearizePso(ID3D12Device* device, const JGraphicInfo& info)
	{
		ssaoDepthLinearize = std::make_unique<JDx12GraphicShaderDataHolder>();
		auto* holder = ssaoDepthLinearize.get();

		JGraphicShaderInitData initData; 
		auto compileInfo = JShaderType::CompileInfo(L"DepthLinearize.hlsl", L"DepthLinearize");
		holder->vs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro[0], L"FullScreenTriangleVS", L"vs_6_0");
		holder->ps = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro[0], compileInfo.functionName, L"ps_6_0");
		 
		Private::BuildSsaoPso(holder, device, ssaoDepthLinearizeRootSignature.Get(), DXGI_FORMAT_R32_FLOAT, 1, "SsaoDepthLinearizePso");
	}
	void JDx12ImageProcessing::BuildSsaoDepthInterleaveRootSignature(ID3D12Device* device)
	{
		CD3DX12_DESCRIPTOR_RANGE depthMapTable;
		depthMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_ROOT_PARAMETER slotRootParameter[SsaoDepthInterleave::slotCount];
		slotRootParameter[SsaoDepthInterleave::depthMapIndex].InitAsDescriptorTable(1, &depthMapTable);
		slotRootParameter[SsaoDepthInterleave::passCBIndex].InitAsConstantBufferView(0);
		slotRootParameter[SsaoDepthInterleave::sliceCBIndex].InitAsConstantBufferView(1);

		std::vector< CD3DX12_STATIC_SAMPLER_DESC> sam
		{
			CD3DX12_STATIC_SAMPLER_DESC(0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP) // addressW 
		};

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(SsaoDepthInterleave::slotCount,
			slotRootParameter,
			(uint)sam.size(), sam.data(),
			D3D12_ROOT_SIGNATURE_FLAG_NONE);

		JDx12RootSignature::Create(device, &rootSigDesc, L"SsaoDepthInterleaveRootSignature", ssaoDepthInterleaveRootSignature.GetAddressOf());
	}
	void JDx12ImageProcessing::BuildSsaoDepthInterleavePso(ID3D12Device* device, const JGraphicInfo& info)
	{
		ssaoDepthInterleave = std::make_unique<JDx12GraphicShaderDataHolder>();
		auto* holder = ssaoDepthInterleave.get();

		JGraphicShaderInitData initData; 
		auto compileInfo = JShaderType::CompileInfo(L"DepthInterleave.hlsl", L"DepthInterleave");
		holder->vs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro[0], L"FullScreenTriangleVS", L"vs_6_0");
		holder->ps = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro[0], compileInfo.functionName, L"ps_6_0");
		 
		Private::BuildSsaoPso(holder, device, ssaoDepthInterleaveRootSignature.Get(), DXGI_FORMAT_R32_FLOAT, Ssao::mrtCount, "ssaoDepthInterleavePso");
	}
	void JDx12ImageProcessing::ClearRootSignature()
	{
		blurRootSignature = nullptr;
		downSampleRootSignature = nullptr;
		ssaoRootSignature = nullptr;
		hbaoRootSignature = nullptr;
		ssaoBlurRootSignature = nullptr;
		ssaoCombineRootSignature = nullptr;
		ssaoDepthLinearizeRootSignature = nullptr;
		ssaoDepthInterleaveRootSignature = nullptr; 
	}
	void JDx12ImageProcessing::ClearPso()
	{
		for (uint i = 0; i < (uint)J_BLUR_TYPE::COUNT; ++i)
		{
			for (uint j = 0; j < (uint)J_KERNEL_SIZE::COUNT; ++j)
				blur[i][j] = nullptr;
		}
		for (uint i = 0; i < (uint)J_DOWN_SAMPLING_TYPE::COUNT; ++i)
		{
			for (uint j = 0; j < (uint)J_KERNEL_SIZE::COUNT; ++j)
				downSample[i][j] = nullptr;
		}
		for(uint i = 0; i < SIZE_OF_ARRAY(ssao); ++i)
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
	void JDx12ImageProcessing::ClearBuffer()
	{
		if (blurCB != nullptr)
			blurCB->Clear();
		blurCB = nullptr;

		if (downSampleCB != nullptr)
			downSampleCB->Clear();
		downSampleCB = nullptr; 

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