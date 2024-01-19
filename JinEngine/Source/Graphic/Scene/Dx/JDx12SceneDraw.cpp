#include"JDx12SceneDraw.h"  
#include"../../JGraphicUpdateHelper.h"
#include"../../JGraphicInfo.h"
#include"../../JGraphicOption.h" 
#include"../../Device/Dx/JDx12GraphicDevice.h"
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h" 
#include"../../Culling/Dx/JDx12CullingManager.h"
#include"../../Culling/JCullingInterface.h"
#include"../../Culling/JCullingInfo.h"
#include"../../FrameResource/Dx/JDx12FrameResource.h"
#include"../../FrameResource/JObjectConstants.h" 
#include"../../FrameResource/JAnimationConstants.h" 
#include"../../FrameResource/JMaterialConstants.h" 
#include"../../FrameResource/JPassConstants.h" 
#include"../../FrameResource/JCameraConstants.h" 
#include"../../FrameResource/JLightConstants.h"     
#include"../../Shader/Dx/JDx12ShaderDataHolder.h"
#include"../../Utility/Dx/JD3DUtility.h"
#include"../../../Core/Exception/JExceptionMacro.h"
#include"../../../Object/Component/Animator/JAnimator.h" 
#include"../../../Object/Component/Camera/JCamera.h"  
#include"../../../Object/Component/Transform/JTransform.h" 
#include"../../../Object/Component/RenderItem/JRenderItem.h"
#include"../../../Object/Resource/JResourceManager.h" 
#include"../../../Object/Resource/Mesh/JMeshGeometry.h" 
#include"../../../Object/Resource/Material/JMaterial.h"
#include"../../../Object/Resource/Shader/JShader.h"
#include"../../../Object/Resource/Scene/JScene.h" 
#include"../../../Object/GameObject/JGameObject.h"
#include"../../../Application/JApplicationEngine.h"

//Graphic Shader Macro Symbol  
#define TEXTURE_2D_COUNT_SYMBOL L"TEXTURE_2D_COUNT"
#define TEXTURE_CUBE_COUNT_SYMBOL L"CUBE_MAP_COUNT"
#define SHADOW_MAP_COUNT_SYMBOL L"SHADOW_MAP_COUNT"
#define SHADOW_MAP_ARRAY_COUNT_SYMBOL L"SHADOW_MAP_ARRAY_COUNT"
#define SHADOW_MAP_CUBE_COUNT_SYMBOL L"SHADOW_MAP_CUBE_COUNT"

#define USE_DIRECTIONAL_LIGHT_PCF L"USE_DIRECTIONAL_LIGHT_PCF"
#define USE_DIRECTIONAL_LIGHT_PCSS L"USE_DIRECTIONAL_LIGHT_PCSS"  

#define USE_POINT_LIGHT_PCF L"USE_POINT_LIGHT_PCF"   
#define USE_POINT_LIGHT_PCSS L"USE_POINT_LIGHT_PCSS"   

#define USE_SPOT_LIGHT_PCF L"USE_SPOT_LIGHT_PCF"    
#define USE_SPOT_LIGHT_PCSS L"USE_SPOT_LIGHT_PCSS"   

#define USE_PCF_16_SAMPLE L"USE_PCF_16_SAMPLE" 
#define USE_PCF_32_SAMPLE L"USE_PCF_32_SAMPLE" 

#define USE_BRDF_SMITH_MASKING L"USE_BRDF_SMITH_MASKING"  
#define USE_BRDF_TORRANCE_MASKING L"USE_BRDF_TORRANCE_MASKING"  

#define USE_BRDF_GGX_NDF L"USE_BRDF_GGX_NDF"  
#define USE_BRDF_BECKMANN_NDF L"USE_BRDF_BECKMANN_NDF"  
#define USE_BRDF_BLINN_PHONG_NDF L"USE_BRDF_BLINN_PHONG_NDF"  
#define USE_BRDF_ISOTROPY_NDF L"USE_BRDF_ISOTROPY_NDF"  

#define USE_BRDF_DISNEY_DIFFUSE L"USE_BRDF_DISNEY_DIFFUSE"  
#define USE_BRDF_HAMMON_DIFFUSE L"USE_BRDF_HAMMON_DIFFUSE"  
#define USE_OREN_NAYAR_DIFFUSE L"USE_OREN_NAYAR_DIFFUSE"
#define USE_BRDF_SHIRELY_DIFFUSE L"USE_BRDF_SHIRELY_DIFFUSE"  
#define USE_BRDF_LAMBERTIAN_DIFFUSE L"USE_BRDF_LAMBERTIAN_DIFFUSE"  

#define USE_LIGHT_CLUSTER L"LIGHT_CLUSTER" 

#define USE_DEFERRED_GEOMETRY L"DEFERRED_GEOMETRY"
#define USE_DEFERRED_SHADING L"DEFERRED_SHADING" 

#define USE_FULL_SCREEN_QUAD L"FULL_SCREEN_QUAD"
//Compute Shader Macro Symbol ";
#define THREAD_DIM_X_SYMBOL L"DIMX"
#define THREAD_DIM_Y_SYMBOL L"DIMY"
#define THREAD_DIM_Z_SYMBOL L"DIMZ"

#define CLUSTER_DIM_X L"CLUSTER_DIM_X"
#define CLUSTER_DIM_Y L"CLUSTER_DIM_Y"
#define CLUSTER_DIM_Z L"CLUSTER_DIM_Z"

#define NEAR_CLUST L"NEAR_CLUST"

namespace JinEngine::Graphic
{ 
	namespace Private
	{
		static D3D12_PRIMITIVE_TOPOLOGY_TYPE ConvertD3d12PrimitiveType(const J_SHADER_PRIMITIVE_TYPE primitiveType)noexcept
		{
			switch (primitiveType)
			{
			case JinEngine::J_SHADER_PRIMITIVE_TYPE::TRIANGLE:
				return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	// defualt value
			case JinEngine::J_SHADER_PRIMITIVE_TYPE::LINE:
				return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			default:
				return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
			}
		}
		static D3D12_COMPARISON_FUNC ConvertD3d12Comparesion(const J_SHADER_DEPTH_COMPARISON_FUNC depthCompareFunc)noexcept
		{
			switch (depthCompareFunc)
			{
			case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::DEFAULT:
				return D3D12_COMPARISON_FUNC_LESS;	// defualt value
			case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::NEVER:
				return D3D12_COMPARISON_FUNC_NEVER;
			case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::LESS:
				return D3D12_COMPARISON_FUNC_LESS;
			case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::EQUAL:
				return D3D12_COMPARISON_FUNC_EQUAL;
			case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::LESS_EQUAL:
				return D3D12_COMPARISON_FUNC_LESS_EQUAL;
			case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::GREATER:
				return D3D12_COMPARISON_FUNC_GREATER;
			case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::NOT_EQUAL:
				return D3D12_COMPARISON_FUNC_NOT_EQUAL;
			case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::GREATER_EQUAL:
				return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
			case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::ALWAYS:
				return D3D12_COMPARISON_FUNC_ALWAYS;
			default:
				return D3D12_COMPARISON_FUNC_ALWAYS;
			}
		}
		static D3D12_CULL_MODE ConvertD3d12CullMode(const bool isCullModeNone)noexcept
		{
			if (isCullModeNone)
				return D3D12_CULL_MODE_NONE;
			else
				return D3D12_CULL_MODE_BACK;	//default value
		}
		static void ConvertMacroSet(const J_GRAPHIC_SHADER_FUNCTION f, _Inout_ std::vector<JMacroSet>& set)
		{ 
			if (Core::HasSQValueEnum(f, SHADER_FUNCTION_ALBEDO_MAP))
				set.push_back({ L"ALBEDO_MAP", L"1" });
			if (Core::HasSQValueEnum(f, SHADER_FUNCTION_ALBEDO_MAP_ONLY))
				set.push_back({ L"ALBEDO_MAP_ONLY", L"2" });
			if (Core::HasSQValueEnum(f, SHADER_FUNCTION_NORMAL_MAP))
				set.push_back({ L"NORMAL_MAP", L"3" });
			if (Core::HasSQValueEnum(f, SHADER_FUNCTION_HEIGHT_MAP))
				set.push_back({ L"HEIGHT_MAP", L"4" });
			if (Core::HasSQValueEnum(f, SHADER_FUNCTION_METALLIC_MAP))
				set.push_back({ L"METALLIC_MAP", L"5" });
			if (Core::HasSQValueEnum(f, SHADER_FUNCTION_ROUGHNESS_MAP))
				set.push_back({ L"ROUGHNESS_MAP", L"6" });
			if (Core::HasSQValueEnum(f, SHADER_FUNCTION_AMBIENT_OCCLUSION_MAP))
				set.push_back({ L"AMBIENT_OCCLUSION_MAP", L"7" });

			if (Core::HasSQValueEnum(f, SHADER_FUNCTION_SHADOW))
				set.push_back({ L"SHADOW", L"8" });
			if (Core::HasSQValueEnum(f, SHADER_FUNCTION_LIGHT))
				set.push_back({ L"LIGHT", L"9" });
			if (Core::HasSQValueEnum(f, SHADER_FUNCTION_SKY))
				set.push_back({ L"SKY", L"10" });
			if (Core::HasSQValueEnum(f, SHADER_FUNCTION_ALPHA_CLIP))
				set.push_back({ L"ALPHA_CLIP", L"11" });
			if (Core::HasSQValueEnum(f, SHADER_FUNCTION_DEBUG))
				set.push_back({ L"DEBUG", L"12" });
		} 
		static JMacroSet ConvertMacroSet(const J_GRAPHIC_SHADER_VERTEX_LAYOUT v)
		{
			if (v == J_GRAPHIC_SHADER_VERTEX_LAYOUT::STATIC)
				return  { L"STATIC", L"1" };
			else
				return  { L"SKINNED", L"2" };
		} 
		static void StuffMacro(_Inout_ JGraphicShaderInitData& initHelper, const JGraphicInfo& info, const JGraphicOption& option)noexcept
		{ 
			for (uint i = 0; i < (uint)J_GRAPHIC_SHADER_VERTEX_LAYOUT::COUNT; ++i)
			{
				J_GRAPHIC_SHADER_VERTEX_LAYOUT layout = (J_GRAPHIC_SHADER_VERTEX_LAYOUT)i;
				initHelper.macro[i].clear();
				initHelper.macro[i].push_back(ConvertMacroSet(layout));
				ConvertMacroSet(initHelper.gFunctionFlag, initHelper.macro[i]);

				initHelper.macro[i].push_back({ TEXTURE_2D_COUNT_SYMBOL, std::to_wstring(info.binding2DTextureCapacity) });
				initHelper.macro[i].push_back({ TEXTURE_CUBE_COUNT_SYMBOL, std::to_wstring(info.bindingCubeMapCapacity) });
				initHelper.macro[i].push_back({ SHADOW_MAP_COUNT_SYMBOL,std::to_wstring(info.bindingShadowTextureCapacity) });
				initHelper.macro[i].push_back({ SHADOW_MAP_ARRAY_COUNT_SYMBOL, std::to_wstring(info.bindingShadowTextureArrayCapacity) });
				initHelper.macro[i].push_back({ SHADOW_MAP_CUBE_COUNT_SYMBOL,std::to_wstring(info.bindingShadowTextureCubeCapacity) });

				//Shadow
				if (option.useHighQualityShadow)
				{
					initHelper.macro[i].push_back({ USE_DIRECTIONAL_LIGHT_PCSS, std::to_wstring(1) });
					initHelper.macro[i].push_back({ USE_POINT_LIGHT_PCF, std::to_wstring(1) });
					initHelper.macro[i].push_back({ USE_SPOT_LIGHT_PCF, std::to_wstring(1) });
					initHelper.macro[i].push_back({ USE_PCF_32_SAMPLE, std::to_wstring(1) });
				}
				else if (option.useMiddleQualityShadow)
				{
					initHelper.macro[i].push_back({ USE_DIRECTIONAL_LIGHT_PCF, std::to_wstring(1) });
					initHelper.macro[i].push_back({ USE_POINT_LIGHT_PCF, std::to_wstring(1) });
					initHelper.macro[i].push_back({ USE_SPOT_LIGHT_PCF, std::to_wstring(1) });
					initHelper.macro[i].push_back({ USE_PCF_32_SAMPLE, std::to_wstring(1) });
				}
				else if (option.useLowQualityShadow)
				{
					initHelper.macro[i].push_back({ USE_DIRECTIONAL_LIGHT_PCF, std::to_wstring(1) });
					initHelper.macro[i].push_back({ USE_POINT_LIGHT_PCF, std::to_wstring(1) });
					initHelper.macro[i].push_back({ USE_SPOT_LIGHT_PCF, std::to_wstring(1) });
					initHelper.macro[i].push_back({ USE_PCF_16_SAMPLE, std::to_wstring(1) });
				}

				//BRDF
				if (option.useSmithMasking)
					initHelper.macro[i].push_back({ USE_BRDF_SMITH_MASKING, std::to_wstring(1) });
				if (option.useTorranceMaskig)
					initHelper.macro[i].push_back({ USE_BRDF_TORRANCE_MASKING, std::to_wstring(1) });
				if (option.useGGXNDF)
					initHelper.macro[i].push_back({ USE_BRDF_GGX_NDF, std::to_wstring(1) });
				if (option.useBeckmannNDF)
					initHelper.macro[i].push_back({ USE_BRDF_BECKMANN_NDF, std::to_wstring(1) });
				if (option.useBlinnPhongNDF)
					initHelper.macro[i].push_back({ USE_BRDF_BLINN_PHONG_NDF, std::to_wstring(1) });
				if (option.useIsotropy)
					initHelper.macro[i].push_back({ USE_BRDF_ISOTROPY_NDF, std::to_wstring(1) });
				if (option.useDisneyDiffuse)
					initHelper.macro[i].push_back({ USE_BRDF_DISNEY_DIFFUSE, std::to_wstring(1) });
				if (option.useHammonDiffuse)
					initHelper.macro[i].push_back({ USE_BRDF_HAMMON_DIFFUSE, std::to_wstring(1) });
				if (option.useOrenNayarDiffuse)
					initHelper.macro[i].push_back({ USE_OREN_NAYAR_DIFFUSE, std::to_wstring(1) });
				if (option.useShirleyDiffuse)
					initHelper.macro[i].push_back({ USE_BRDF_SHIRELY_DIFFUSE, std::to_wstring(1) });
				if (option.useLambertianDiffuse)
					initHelper.macro[i].push_back({ USE_BRDF_LAMBERTIAN_DIFFUSE, std::to_wstring(1) });

				//Rendering
				if (option.allowLightCluster && initHelper.macroOption.allowLightCulling && initHelper.shaderType != J_GRAPHIC_SHADER_TYPE::PREVIEW)
					initHelper.macro[i].push_back({ USE_LIGHT_CLUSTER, std::to_wstring(1) });

				if (initHelper.processType == J_GRAPHIC_RENDERING_PROCESS::DEFERRED_GEOMETRY)
					initHelper.macro[i].push_back({ USE_DEFERRED_GEOMETRY, std::to_wstring(1) });
				else if (initHelper.processType == J_GRAPHIC_RENDERING_PROCESS::DEFERRED_SHADING)
				{
					//material per shader가 있는 forward, deferred geo와 달리
					//deferred shade는 scene class가 하나만 소유하므로
					//음영계산에서 material에 property에 종속된 macro값들중 일부(주로 음영 계산에 대한 분기)를 직접 setting 해줘야함					
					//ex) Shadow
					initHelper.macro[i].push_back({ USE_FULL_SCREEN_QUAD, std::to_wstring(1) });
					initHelper.macro[i].push_back({ USE_DEFERRED_SHADING, std::to_wstring(1) }); 
					ConvertMacroSet(J_GRAPHIC_SHADER_FUNCTION::SHADER_FUNCTION_SHADOW, initHelper.macro[i]);
				}

				switch (JinEngine::Graphic::Constants::litClusterXRange[option.clusterXIndex])
				{
				case JinEngine::Graphic::Constants::litClusterXRange[0]:
				case JinEngine::Graphic::Constants::litClusterXRange[1]:
				{
					initHelper.macro[i].push_back({ CLUSTER_DIM_X, std::to_wstring(option.GetClusterXCount()) });
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
					initHelper.macro[i].push_back({ CLUSTER_DIM_Y, std::to_wstring(option.GetClusterYCount()) });
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
					initHelper.macro[i].push_back({ CLUSTER_DIM_Z, std::to_wstring(option.GetClusterZCount()) });
					break;
				}
				default:
					break;
				}
				initHelper.macro[i].push_back({ NEAR_CLUST, std::to_wstring(option.clusterNear) });
				//if(option.useSsao  || option.useHbao)
				//	initHelper.macro[i].push_back({ USE_SSAO, std::to_wstring(1) });
				//initHelper.macro[i].push_back(shaderFuncMacroMap.find(SHADER_FUNCTION_NONE)->second);
			} 
		}
	}
	namespace Forward
	{
		//graphic root
		static constexpr int enginePassCBIndex = 0;
		static constexpr int scenePassCBIndex = enginePassCBIndex + 1;
		static constexpr int camCBIndex = scenePassCBIndex + 1;
		static constexpr int objCBIndex = camCBIndex + 1;
		static constexpr int skinCBIndex = objCBIndex + 1;
		//static constexpr int litIndexCBIndex = camCBIndex + 1;
		//static constexpr int boundObjCBIndex = litIndexCBIndex + 1;

		static constexpr int dLitBuffIndex = skinCBIndex + 1;
		static constexpr int pLitBuffIndex = dLitBuffIndex + 1;
		static constexpr int sLitBuffIndex = pLitBuffIndex + 1;
		static constexpr int rLitBuffIndex = sLitBuffIndex + 1;
		static constexpr int csmBuffIndex = rLitBuffIndex + 1;
		static constexpr int matBuffIndex = csmBuffIndex + 1;

		static constexpr int texture2DBuffIndex = matBuffIndex + 1;
		static constexpr int textureCubeBuffIndex = texture2DBuffIndex + 1;
		static constexpr int textureShadowMapBuffIndex = textureCubeBuffIndex + 1;
		static constexpr int textureShadowMapArrayBuffIndex = textureShadowMapBuffIndex + 1;
		static constexpr int textureShadowMapCubeBuffIndex = textureShadowMapArrayBuffIndex + 1;

		static constexpr int depthMapBufferIndex = textureShadowMapCubeBuffIndex + 1;
		static constexpr int cluserOffsetBufferIndex = depthMapBufferIndex + 1;
		static constexpr int clusterLinkBufferIndex = cluserOffsetBufferIndex + 1;
		//static constexpr int textureNormalMapIndex = textureShadowMapCubeBuffIndex + 1;

		static constexpr int rootSlotCount = textureShadowMapCubeBuffIndex + 1;
		static constexpr int rootSlotCountIncCluster = clusterLinkBufferIndex + 1;

		static const std::vector<CD3DX12_STATIC_SAMPLER_DESC> Sampler()noexcept
		{
			return std::vector<CD3DX12_STATIC_SAMPLER_DESC>
			{
				//pointClamp
				CD3DX12_STATIC_SAMPLER_DESC(0, // shaderRegister
					D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
					D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
					D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
					D3D12_TEXTURE_ADDRESS_MODE_CLAMP), // addressW

					//pointWrap
					CD3DX12_STATIC_SAMPLER_DESC(1, // shaderRegister
						D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
						D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
						D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
						D3D12_TEXTURE_ADDRESS_MODE_WRAP), // addressW

					//linearWrap
					CD3DX12_STATIC_SAMPLER_DESC(2, // shaderRegister
						D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
						D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
						D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
						D3D12_TEXTURE_ADDRESS_MODE_WRAP), // addressW

						//anisotropicWrap
					CD3DX12_STATIC_SAMPLER_DESC(3, // shaderRegister
						D3D12_FILTER_ANISOTROPIC, // filter
						D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
						D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
						D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
						0.0f,                             // mipLODBias
						8),				                  // maxAnisotropy

					//shadow pcss find bloker
					CD3DX12_STATIC_SAMPLER_DESC(4, // shaderRegister
						D3D12_FILTER_MIN_MAG_MIP_LINEAR,
						//D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
						D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
						D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
						D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
						0.0f,                               // mipLODBias
						16,                                 // maxAnisotropy
						D3D12_COMPARISON_FUNC_LESS_EQUAL),


					//LTC
					CD3DX12_STATIC_SAMPLER_DESC(5, // shaderRegister
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
					CD3DX12_STATIC_SAMPLER_DESC(6, // shaderRegister
						D3D12_FILTER_MIN_MAG_MIP_LINEAR,
						//D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP  // addressW
					),

					//shadow linear point cmp
					CD3DX12_STATIC_SAMPLER_DESC(7, // shaderRegister
						D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
						D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
						D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
						D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
						0.0f,                               // mipLODBias
						16,                                 // maxAnisotropy
						D3D12_COMPARISON_FUNC_LESS_EQUAL,
						D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK)
			};
		}
	}
	namespace Deferred
	{
		static constexpr int layerCount = 4;
		namespace Geometry
		{
			//graphic root
			static constexpr int enginePassCBIndex = 0;
			static constexpr int scenePassCBIndex = enginePassCBIndex + 1;
			static constexpr int camCBIndex = scenePassCBIndex + 1;
			static constexpr int objCBIndex = camCBIndex + 1;
			static constexpr int skinCBIndex = objCBIndex + 1;
			//static constexpr int litIndexCBIndex = camCBIndex + 1;
			//static constexpr int boundObjCBIndex = litIndexCBIndex + 1;

			static constexpr int matBuffIndex = skinCBIndex + 1;

			static constexpr int texture2DBuffIndex = matBuffIndex + 1;
			static constexpr int textureCubeBuffIndex = texture2DBuffIndex + 1;
			static constexpr int rootSlotCount = textureCubeBuffIndex + 1;

			static const std::vector<CD3DX12_STATIC_SAMPLER_DESC> Sampler()noexcept
			{
				return std::vector<CD3DX12_STATIC_SAMPLER_DESC>
				{
					//pointClamp
					CD3DX12_STATIC_SAMPLER_DESC(0, // shaderRegister
						D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP), // addressW

						//pointWrap
						CD3DX12_STATIC_SAMPLER_DESC(1, // shaderRegister
							D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
							D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
							D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
							D3D12_TEXTURE_ADDRESS_MODE_WRAP), // addressW

						//linearWrap
						CD3DX12_STATIC_SAMPLER_DESC(2, // shaderRegister
							D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
							D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
							D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
							D3D12_TEXTURE_ADDRESS_MODE_WRAP), // addressW

							//anisotropicWrap
						CD3DX12_STATIC_SAMPLER_DESC(3, // shaderRegister
							D3D12_FILTER_ANISOTROPIC, // filter
							D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
							D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
							D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
							0.0f,                             // mipLODBias
							8),				                  // maxAnisotropy
				};
			}
		}
		namespace Shading
		{
			//graphic root 
			static constexpr int enginePassCBIndex = 0;
			static constexpr int scenePassCBIndex = enginePassCBIndex + 1;
			static constexpr int camCBIndex = scenePassCBIndex + 1;

			static constexpr int dLitBuffIndex = camCBIndex + 1;
			static constexpr int pLitBuffIndex = dLitBuffIndex + 1;
			static constexpr int sLitBuffIndex = pLitBuffIndex + 1;
			static constexpr int rLitBuffIndex = sLitBuffIndex + 1;
			static constexpr int csmBuffIndex = rLitBuffIndex + 1;

			static constexpr int texture2DBuffIndex = csmBuffIndex + 1;
			static constexpr int textureCubeBuffIndex = texture2DBuffIndex + 1;
			static constexpr int textureShadowMapBuffIndex = textureCubeBuffIndex + 1;
			static constexpr int textureShadowMapArrayBuffIndex = textureShadowMapBuffIndex + 1;
			static constexpr int textureShadowMapCubeBuffIndex = textureShadowMapArrayBuffIndex + 1;

			static constexpr int gBufferIndex = textureShadowMapCubeBuffIndex + 1;

			static constexpr int depthMapBufferIndex = gBufferIndex + 1;
			static constexpr int cluserOffsetBufferIndex = depthMapBufferIndex + 1;
			static constexpr int clusterLinkBufferIndex = cluserOffsetBufferIndex + 1;
			//static constexpr int textureNormalMapIndex = textureShadowMapCubeBuffIndex + 1;
			 
			static constexpr int rootSlotCount = depthMapBufferIndex + 1;
			static constexpr int rootSlotCountIncCluster = clusterLinkBufferIndex + 1;

			//static constexpr int textureNormalMapIndex = textureShadowMapCubeBuffIndex + 1;
			 
			static const std::vector<CD3DX12_STATIC_SAMPLER_DESC> Sampler()
			{
				return Forward::Sampler();
			}
		}
	}

	JDx12SceneDraw::GraphicDataSet::GraphicDataSet(const JDx12GraphicBindSet* dx12BindSet)
		:dx12Frame(static_cast<JDx12FrameResource*>(dx12BindSet->currFrame)),
		dx12Gm(static_cast<JDx12GraphicResourceManager*>(dx12BindSet->graphicResourceM)),
		cmdList(dx12BindSet->cmdList)
	{}
	JDx12SceneDraw::GraphicDataSet::GraphicDataSet(const JDx12GraphicSceneDrawSet* dx12DrawSet)
		: dx12Device(static_cast<JDx12GraphicDevice*>(dx12DrawSet->device)),
		dx12Frame(static_cast<JDx12FrameResource*>(dx12DrawSet->currFrame)),
		dx12Gm(static_cast<JDx12GraphicResourceManager*>(dx12DrawSet->graphicResourceM)),
		dx12Cm(static_cast<JDx12CullingManager*>(dx12DrawSet->cullingM)),
		cmdList(dx12DrawSet->cmdList)
	{}

	JDx12SceneDraw::ResourceDataSet::ResourceDataSet(const GraphicDataSet& set, const JDrawHelper& helper)
	{
		auto gRInterface = helper.cam->GraphicResourceUserInterface();
		auto cInterface = helper.GetCullInterface();

		scenePassCBIndex = helper.option.allowDeferred ? Deferred::Geometry::scenePassCBIndex : Forward::scenePassCBIndex;
		camCBIndex = helper.option.allowDeferred ? Deferred::Geometry::camCBIndex : Forward::camCBIndex;

		camFrameIndex = helper.GetCamFrameIndex();
		sceneFrameIndex = helper.GetPassFrameIndex();

		int rtDataIndex = gRInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		int dsDataIndex = gRInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);

		int rtvVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, rtDataIndex);
		int rtvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_BIND_TYPE::RTV, rtDataIndex);

		int dsvVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, dsDataIndex);
		int dsvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_BIND_TYPE::DSV, dsDataIndex);

		rtResource = set.dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, rtvVecIndex);
		dsResource = set.dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, dsvVecIndex);

		rtv = set.dx12Gm->GetCpuRtvDescriptorHandle(rtvHeapIndex);
		dsv = set.dx12Gm->GetCpuDsvDescriptorHandle(dsvHeapIndex);

		dsSrvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_BIND_TYPE::SRV, dsDataIndex);
		depthMapSrv = set.dx12Gm->GetGpuSrvDescriptorHandle(dsSrvHeapIndex);

		if (helper.option.allowDeferred)
		{
			albedoResource = set.dx12Gm->GetOptionResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP, rtvVecIndex);
			normalResource = set.dx12Gm->GetOptionResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP, rtvVecIndex);
			tangentResource = set.dx12Gm->GetOptionResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_RESOURCE_OPTION_TYPE::TANGENT_MAP, rtvVecIndex);
			lightPropResource = set.dx12Gm->GetOptionResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY, rtvVecIndex);

			int albedoRtvHeapIndex = gRInterface.GetOptionHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_BIND_TYPE::RTV, J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP, rtDataIndex);
			int normalRtvHeapIndex = gRInterface.GetOptionHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_BIND_TYPE::RTV, J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP, rtDataIndex);
			int tangetRtvHeapIndex = gRInterface.GetOptionHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_BIND_TYPE::RTV, J_GRAPHIC_RESOURCE_OPTION_TYPE::TANGENT_MAP, rtDataIndex);
			int lightPropRtvHeapIndex = gRInterface.GetOptionHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_BIND_TYPE::RTV, J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY, rtDataIndex);
			 
			albedoRtv = set.dx12Gm->GetCpuRtvDescriptorHandle(albedoRtvHeapIndex);
			normalRtv = set.dx12Gm->GetCpuRtvDescriptorHandle(normalRtvHeapIndex);
			tangentRtv = set.dx12Gm->GetCpuRtvDescriptorHandle(tangetRtvHeapIndex);
			lightPropRtv = set.dx12Gm->GetCpuRtvDescriptorHandle(lightPropRtvHeapIndex);

			int albedoSrvIndex = gRInterface.GetOptionHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_BIND_TYPE::SRV, J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP, rtDataIndex);
			albedoSrv = set.dx12Gm->GetGpuSrvDescriptorHandle(albedoSrvIndex);
		}

		canUseLightCulling = helper.cam->AllowLightCulling() && helper.option.allowLightCluster;
		canUseLightCluster = canUseLightCulling && helper.option.allowLightCluster;
	}
	void JDx12SceneDraw::ResourceDataSet::SettingCluster(const GraphicDataSet& set, const JDrawHelper& helper)
	{
		auto gRInterface = helper.cam->GraphicResourceUserInterface();
		const uint offsetBufferDataIndex = gRInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET, J_GRAPHIC_TASK_TYPE::LIGHT_CULLING);
		const uint linkedDataIndex = gRInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST, J_GRAPHIC_TASK_TYPE::LIGHT_CULLING);
	 
		offsetSrvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET, J_GRAPHIC_BIND_TYPE::SRV, offsetBufferDataIndex);
		linkedSrvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST, J_GRAPHIC_BIND_TYPE::SRV, linkedDataIndex);
 
		clusterLinkOffsetSrv = set.dx12Gm->GetGpuSrvDescriptorHandle(offsetSrvHeapIndex);
		clusterLinkListSrv = set.dx12Gm->GetGpuSrvDescriptorHandle(linkedSrvHeapIndex);
	}
	void JDx12SceneDraw::ResourceDataSet::SettingDebugging(const GraphicDataSet& set, const JDrawHelper& helper)
	{
		auto gRInterface = helper.cam->GraphicResourceUserInterface();

		debugVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL, 0);
		debugHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL, J_GRAPHIC_BIND_TYPE::DSV, 0);
		debugResource = set.dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL, debugVecIndex);	 
		debugDsv = set.dx12Gm->GetCpuDsvDescriptorHandle(debugHeapIndex);
	}

	void JDx12SceneDraw::Initialize(JGraphicDevice* device, JGraphicResourceManager* gm, const JGraphicBaseDataSet& baseDataSet)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gm))
			return;
	 
		if (baseDataSet.option.allowDeferred)
		{
			BuildForwardRootSignature(static_cast<JDx12GraphicDevice*>(device)->GetDevice(), baseDataSet.info, baseDataSet.option);
			BuildDeferredGeometryRootSignature(static_cast<JDx12GraphicDevice*>(device)->GetDevice(), baseDataSet.info, baseDataSet.option);
			BuildDeferredShadingRootSignature(static_cast<JDx12GraphicDevice*>(device)->GetDevice(), baseDataSet.info, baseDataSet.option);
			BuildDeferredShader(JGraphicShaderCompileSet(device, baseDataSet.info, baseDataSet.option));
		}
		else
			BuildForwardRootSignature(static_cast<JDx12GraphicDevice*>(device)->GetDevice(), baseDataSet.info, baseDataSet.option);
	}
	void JDx12SceneDraw::Clear()
	{
		forwardRootSignature = deferredGeometryRootSignature = deferredShadingRootSignature = nullptr;
		for (uint i = 0; i < (uint)INNER_DEFERRED_SHADER_TYPE::COUNT; ++i)
			deferredShadingHolder[i] = nullptr;
	}
	J_GRAPHIC_DEVICE_TYPE JDx12SceneDraw::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	bool JDx12SceneDraw::HasPreprocessing()const noexcept
	{
		return true;
	}
	bool JDx12SceneDraw::HasPostprocessing()const noexcept
	{
		return true;
	}
	void JDx12SceneDraw::BindResource(const J_GRAPHIC_RENDERING_PROCESS process, const JGraphicBindSet* bindSet)
	{
		if (!IsSameDevice(bindSet))
			return;

		GraphicDataSet set(static_cast<const JDx12GraphicBindSet*>(bindSet));
		if (process == J_GRAPHIC_RENDERING_PROCESS::DEFERRED_GEOMETRY)
			BindDeferredGeometryRootAndResource(set.dx12Frame, set.dx12Gm, set.cmdList);
		else if (process == J_GRAPHIC_RENDERING_PROCESS::DEFERRED_SHADING)
			BindDeferredShadingRootAndResource(set.dx12Frame, set.dx12Gm, set.cmdList);
		else
			BindForwardRootAndResource(set.dx12Frame, set.dx12Gm, set.cmdList);
	}
	void JDx12SceneDraw::BindForwardRootAndResource(JDx12FrameResource* dx12Frame, JDx12GraphicResourceManager* dx12Gm, ID3D12GraphicsCommandList* cmdList)
	{
		cmdList->SetGraphicsRootSignature(forwardRootSignature.Get());
		dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::ENGINE_PASS)->SetGraphicCBBufferView(cmdList, Forward::enginePassCBIndex, 0);
		dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT)->SetGraphicsRootShaderResourceView(cmdList, Forward::dLitBuffIndex);
		dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT)->SetGraphicsRootShaderResourceView(cmdList, Forward::pLitBuffIndex);
		dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT)->SetGraphicsRootShaderResourceView(cmdList, Forward::sLitBuffIndex);
		dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT)->SetGraphicsRootShaderResourceView(cmdList, Forward::rLitBuffIndex);
		dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::CASCADE_SHADOW_MAP_INFO)->SetGraphicsRootShaderResourceView(cmdList, Forward::csmBuffIndex);
		dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL)->SetGraphicsRootShaderResourceView(cmdList, Forward::matBuffIndex);

		cmdList->SetGraphicsRootDescriptorTable(Forward::texture2DBuffIndex, dx12Gm->GetFirstGpuSrvDescriptorHandle(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D));
		cmdList->SetGraphicsRootDescriptorTable(Forward::textureCubeBuffIndex, dx12Gm->GetFirstGpuSrvDescriptorHandle(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE));
		cmdList->SetGraphicsRootDescriptorTable(Forward::textureShadowMapBuffIndex, dx12Gm->GetFirstGpuSrvDescriptorHandle(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP));
		cmdList->SetGraphicsRootDescriptorTable(Forward::textureShadowMapArrayBuffIndex, dx12Gm->GetFirstGpuSrvDescriptorHandle(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY));
		cmdList->SetGraphicsRootDescriptorTable(Forward::textureShadowMapCubeBuffIndex, dx12Gm->GetFirstGpuSrvDescriptorHandle(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE));
	}
	void JDx12SceneDraw::BindDeferredGeometryRootAndResource(JDx12FrameResource* dx12Frame, JDx12GraphicResourceManager* dx12Gm, ID3D12GraphicsCommandList* cmdList)
	{
		using namespace Deferred;
		cmdList->SetGraphicsRootSignature(deferredGeometryRootSignature.Get());
		dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::ENGINE_PASS)->SetGraphicCBBufferView(cmdList, Geometry::enginePassCBIndex, 0);
		dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL)->SetGraphicsRootShaderResourceView(cmdList, Geometry::matBuffIndex);

		cmdList->SetGraphicsRootDescriptorTable(Geometry::texture2DBuffIndex, dx12Gm->GetFirstGpuSrvDescriptorHandle(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D));
		cmdList->SetGraphicsRootDescriptorTable(Geometry::textureCubeBuffIndex, dx12Gm->GetFirstGpuSrvDescriptorHandle(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE));
	}
	void JDx12SceneDraw::BindDeferredShadingRootAndResource(JDx12FrameResource* dx12Frame, JDx12GraphicResourceManager* dx12Gm, ID3D12GraphicsCommandList* cmdList)
	{
		using namespace Deferred;
		cmdList->SetGraphicsRootSignature(deferredShadingRootSignature.Get());
		dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::ENGINE_PASS)->SetGraphicCBBufferView(cmdList, Shading::enginePassCBIndex, 0);
		dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT)->SetGraphicsRootShaderResourceView(cmdList, Shading::dLitBuffIndex);
		dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT)->SetGraphicsRootShaderResourceView(cmdList, Shading::pLitBuffIndex);
		dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT)->SetGraphicsRootShaderResourceView(cmdList, Shading::sLitBuffIndex);
		dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT)->SetGraphicsRootShaderResourceView(cmdList, Shading::rLitBuffIndex);
		dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::CASCADE_SHADOW_MAP_INFO)->SetGraphicsRootShaderResourceView(cmdList, Shading::csmBuffIndex);

		cmdList->SetGraphicsRootDescriptorTable(Shading::texture2DBuffIndex, dx12Gm->GetFirstGpuSrvDescriptorHandle(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D));
		cmdList->SetGraphicsRootDescriptorTable(Shading::textureCubeBuffIndex, dx12Gm->GetFirstGpuSrvDescriptorHandle(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE));
		cmdList->SetGraphicsRootDescriptorTable(Shading::textureShadowMapBuffIndex, dx12Gm->GetFirstGpuSrvDescriptorHandle(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP));
		cmdList->SetGraphicsRootDescriptorTable(Shading::textureShadowMapArrayBuffIndex, dx12Gm->GetFirstGpuSrvDescriptorHandle(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY));
		cmdList->SetGraphicsRootDescriptorTable(Shading::textureShadowMapCubeBuffIndex, dx12Gm->GetFirstGpuSrvDescriptorHandle(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE));
	}
	void JDx12SceneDraw::ClearResource(const GraphicDataSet& gSet, const ResourceDataSet& rSet, const JDrawHelper& helper)
	{
		gSet.cmdList->ClearDepthStencilView(rSet.dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		gSet.cmdList->ClearRenderTargetView(rSet.rtv, gSet.dx12Gm->GetBackBufferClearColor(), 0, nullptr);

		if (helper.option.allowDeferred)
		{
			gSet.cmdList->ClearRenderTargetView(rSet.albedoRtv, gSet.dx12Gm->GetBackBufferClearColor(), 0, nullptr);
			gSet.cmdList->ClearRenderTargetView(rSet.normalRtv, gSet.dx12Gm->GetBlackColor(), 0, nullptr);
			gSet.cmdList->ClearRenderTargetView(rSet.tangentRtv, gSet.dx12Gm->GetBlackColor(), 0, nullptr);
			gSet.cmdList->ClearRenderTargetView(rSet.lightPropRtv, gSet.dx12Gm->GetBlackColor(), 0, nullptr);
		}
	}
	void JDx12SceneDraw::SwitchResourceStateForDrawing(const GraphicDataSet& gSet, const ResourceDataSet& rSet, const JDrawHelper& helper)
	{
		JD3DUtility::ResourceTransition(gSet.cmdList, rSet.dsResource, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		JD3DUtility::ResourceTransition(gSet.cmdList, rSet.rtResource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
		if (helper.option.allowDeferred)
		{
			JD3DUtility::ResourceTransition(gSet.cmdList, rSet.albedoResource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
			JD3DUtility::ResourceTransition(gSet.cmdList, rSet.normalResource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
			JD3DUtility::ResourceTransition(gSet.cmdList, rSet.tangentResource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
			JD3DUtility::ResourceTransition(gSet.cmdList, rSet.lightPropResource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}
	} 
	void JDx12SceneDraw::SwitchResourceStateForDeferredShade(const GraphicDataSet& gSet, const ResourceDataSet& rSet, const JDrawHelper& helper)
	{
		using namespace Deferred;
		JD3DUtility::ResourceTransition(gSet.cmdList, rSet.dsResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
		JD3DUtility::ResourceTransition(gSet.cmdList, rSet.albedoResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
		JD3DUtility::ResourceTransition(gSet.cmdList, rSet.normalResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
		JD3DUtility::ResourceTransition(gSet.cmdList, rSet.tangentResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
		JD3DUtility::ResourceTransition(gSet.cmdList, rSet.lightPropResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	}
	void JDx12SceneDraw::ReturnResourceState(const GraphicDataSet& gSet, const ResourceDataSet& rSet, const JDrawHelper& helper)
	{
		//Deferred는 shading step이전에 depth map에서 값을 읽기위해 
		//D3D12_RESOURCE_STATE_RENDER_TARGET->D3D12_RESOURCE_STATE_GENERIC_READ로 상태를 변경한다.
		//D3D12_RESOURCE_STATE_RENDER_TARGET는 scene draw중에만 필요하므로 SettingResourceAfterDrawing 호출 이전에
		//scene draw가 완료되었으면 미리 상태변경해도 상관없다.
		if (!helper.option.allowDeferred)
			JD3DUtility::ResourceTransition(gSet.cmdList, rSet.dsResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
		JD3DUtility::ResourceTransition(gSet.cmdList, rSet.rtResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ); 
	}
	void JDx12SceneDraw::BindRenderTarget(const GraphicDataSet& gSet, const ResourceDataSet& rSet, const JDrawHelper& helper, const bool bindDs)
	{
		if (bindDs)
			gSet.cmdList->OMSetRenderTargets(1, &rSet.rtv, true, &rSet.dsv);
		else
			gSet.cmdList->OMSetRenderTargets(1, &rSet.rtv, true, nullptr);
	}
	void JDx12SceneDraw::BindGBufferByRenderTarget(const GraphicDataSet& gSet, const ResourceDataSet& rSet, const JDrawHelper& helper)
	{
		using namespace Deferred;
		gSet.cmdList->OMSetRenderTargets(layerCount, &rSet.albedoRtv, true, &rSet.dsv);
	}
	void JDx12SceneDraw::BindCommonCB(const GraphicDataSet& gSet, const ResourceDataSet& rSet, const JDrawHelper& helper)
	{
		gSet.dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::SCENE_PASS)->SetGraphicCBBufferView(gSet.cmdList, rSet.scenePassCBIndex, rSet.sceneFrameIndex);
		gSet.dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA)->SetGraphicCBBufferView(gSet.cmdList, rSet.camCBIndex, rSet.camFrameIndex);
	}
	void JDx12SceneDraw::BindLightListResource(const GraphicDataSet& gSet, const ResourceDataSet& rSet, const JDrawHelper& helper)
	{ 
		if (helper.option.allowDeferred)
		{
			using namespace Deferred;
			gSet.cmdList->SetGraphicsRootDescriptorTable(Shading::depthMapBufferIndex, rSet.depthMapSrv);
			gSet.cmdList->SetGraphicsRootDescriptorTable(Shading::cluserOffsetBufferIndex, rSet.clusterLinkOffsetSrv);
			gSet.cmdList->SetGraphicsRootDescriptorTable(Shading::clusterLinkBufferIndex, rSet.clusterLinkListSrv);
		}
		else
		{
			gSet.cmdList->SetGraphicsRootDescriptorTable(Forward::depthMapBufferIndex, rSet.depthMapSrv);
			gSet.cmdList->SetGraphicsRootDescriptorTable(Forward::cluserOffsetBufferIndex, rSet.clusterLinkOffsetSrv);
			gSet.cmdList->SetGraphicsRootDescriptorTable(Forward::clusterLinkBufferIndex, rSet.clusterLinkListSrv);
		} 
	}
	void JDx12SceneDraw::BindGBufferResource(const GraphicDataSet& gSet, const ResourceDataSet& rSet, const JDrawHelper& helper)
	{
		using namespace Deferred;
		gSet.cmdList->SetGraphicsRootDescriptorTable(Shading::gBufferIndex, rSet.albedoSrv);
		if(!rSet.canUseLightCluster)
			gSet.cmdList->SetGraphicsRootDescriptorTable(Shading::depthMapBufferIndex, rSet.depthMapSrv);
	}
	void JDx12SceneDraw::BindViewPortAndRect(const GraphicDataSet& gSet, const ResourceDataSet& rSet)
	{
		D3D12_RESOURCE_DESC rtDesc = rSet.rtResource->GetDesc();
		D3D12_VIEWPORT viewPort;
		D3D12_RECT rect;
		gSet.dx12Device->CalViewportAndRect(JVector2F(rtDesc.Width, rtDesc.Height), true, viewPort, rect);
		gSet.cmdList->RSSetViewports(1, &viewPort);
		gSet.cmdList->RSSetScissorRects(1, &rect);
	}
	void JDx12SceneDraw::DrawSceneGameObject(const GraphicDataSet& gSet, const JDrawHelper& helper)
	{
		const std::vector<JUserPtr<JGameObject>>& objVec00 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::STATIC);
		const std::vector<JUserPtr<JGameObject>>& objVec01 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::SKINNED);
		const std::vector<JUserPtr<JGameObject>>& objVec02 = helper.GetGameObjectCashVec(J_RENDER_LAYER::DEBUG_OBJECT, Core::J_MESHGEOMETRY_TYPE::STATIC);
		const std::vector<JUserPtr<JGameObject>>& objVec03 = helper.GetGameObjectCashVec(J_RENDER_LAYER::SKY, Core::J_MESHGEOMETRY_TYPE::STATIC);

		DrawGameObject(gSet, objVec00, helper, JDrawCondition(helper, false, true, helper.allowDrawDebugObject), helper.option.allowDeferred);
		DrawGameObject(gSet, objVec01, helper, JDrawCondition(helper, helper.scene->IsActivatedSceneTime(), true, helper.allowDrawDebugObject), helper.option.allowDeferred);
		if (helper.allowDrawDebugObject)
			DrawGameObject(gSet, objVec02, helper, JDrawCondition(), helper.option.allowDeferred);
		DrawGameObject(gSet, objVec03, helper, JDrawCondition(), helper.option.allowDeferred);
	}
	void JDx12SceneDraw::DrawQuadForDeferredShading(const GraphicDataSet& gSet, const ResourceDataSet& rSet, const JDrawHelper& helper)
	{
		JUserPtr<JMeshGeometry> quad = _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::FULL_SCREEN_QUAD);
		const D3D12_VERTEX_BUFFER_VIEW vertexPtr = gSet.dx12Gm->VertexBufferView(quad);
		const D3D12_INDEX_BUFFER_VIEW indexPtr = gSet.dx12Gm->IndexBufferView(quad);

		gSet.cmdList->IASetVertexBuffers(0, 1, &vertexPtr);
		gSet.cmdList->IASetIndexBuffer(&indexPtr);
		gSet.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		if (!rSet.canUseLightCluster)
			gSet.cmdList->SetPipelineState(deferredShadingHolder[(uint)INNER_DEFERRED_SHADER_TYPE::EXCEPT_LIGHT_CULLING]->pso.Get());
		else
			gSet.cmdList->SetPipelineState(deferredShadingHolder[(uint)INNER_DEFERRED_SHADER_TYPE::STANDARD]->pso.Get());
		gSet.cmdList->DrawIndexedInstanced(quad->GetSubmeshIndexCount(0), 1, quad->GetSubmeshStartIndexLocation(0), quad->GetSubmeshBaseVertexLocation(0), 0);
	}
	void JDx12SceneDraw::BeginDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(bindSet))
			return;

		GraphicDataSet gSet(static_cast<const JDx12GraphicBindSet*>(bindSet));
		ResourceDataSet rSet(gSet, helper);
		
		SwitchResourceStateForDrawing(gSet, rSet, helper);
		ClearResource(gSet, rSet, helper);
	}
	void JDx12SceneDraw::EndDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(bindSet))
			return;

		GraphicDataSet gSet(static_cast<const JDx12GraphicBindSet*>(bindSet));
		ResourceDataSet rSet(gSet, helper);	 
		ReturnResourceState(gSet, rSet, helper);
	}
	void JDx12SceneDraw::DrawSceneRenderTarget(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)
	{ 
		if (!IsSameDevice(drawSet))
			return;

		GraphicDataSet gSet(static_cast<const JDx12GraphicSceneDrawSet*>(drawSet));
		ResourceDataSet rSet(gSet, helper);
		const bool canUseCluster = rSet.canUseLightCluster && !helper.option.allowDeferred;
		if (canUseCluster)
			rSet.SettingCluster(gSet, helper);

		SwitchResourceStateForDrawing(gSet, rSet, helper);
		ClearResource(gSet, rSet, helper);

		if (helper.option.allowDeferred)
			BindGBufferByRenderTarget(gSet, rSet, helper);
		else
			BindRenderTarget(gSet, rSet, helper, true);
		BindCommonCB(gSet, rSet, helper);
		BindViewPortAndRect(gSet, rSet);
		if (canUseCluster)
			BindLightListResource(gSet, rSet, helper);
		DrawSceneGameObject(gSet, helper);

		//deferred일 경우 ReturnResourceState 호출에 대한 책임을 DrawSceneShade에게 넘긴다.
		if (!helper.option.allowDeferred)
			ReturnResourceState(gSet, rSet, helper);		 
	}
	void JDx12SceneDraw::DrawSceneRenderTargetMultiThread(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(drawSet))
			return;

		GraphicDataSet gSet(static_cast<const JDx12GraphicSceneDrawSet*>(drawSet));
		ResourceDataSet rSet(gSet, helper);
		const bool canUseCluster = rSet.canUseLightCluster && !helper.option.allowDeferred;
		if (canUseCluster)
			rSet.SettingCluster(gSet, helper);

		if (helper.option.allowDeferred)
			BindGBufferByRenderTarget(gSet, rSet, helper);
		else
			BindRenderTarget(gSet, rSet, helper, true);
		BindCommonCB(gSet, rSet, helper);
		if (canUseCluster)
			BindLightListResource(gSet, rSet, helper);
		BindViewPortAndRect(gSet, rSet); (gSet, rSet);
		DrawSceneGameObject(gSet, helper);
	}
	void JDx12SceneDraw::DrawSceneDebugUI(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(drawSet) || !helper.allowDrawDebugObject)
			return;

		GraphicDataSet gSet(static_cast<const JDx12GraphicSceneDrawSet*>(drawSet));
		ResourceDataSet rSet(gSet, helper);
		rSet.SettingDebugging(gSet, helper);

		JD3DUtility::ResourceTransition(gSet.cmdList, rSet.rtResource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
		JD3DUtility::ResourceTransition(gSet.cmdList, rSet.debugResource, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		gSet.cmdList->ClearDepthStencilView(rSet.debugDsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		gSet.cmdList->OMSetRenderTargets(1, &rSet.rtv, true, &rSet.debugDsv);

		BindCommonCB(gSet, rSet, helper);
		BindViewPortAndRect(gSet, rSet);

		const std::vector<JUserPtr<JGameObject>>& objVec = helper.GetGameObjectCashVec(J_RENDER_LAYER::DEBUG_UI, Core::J_MESHGEOMETRY_TYPE::STATIC);
		DrawGameObject(gSet, objVec, helper, JDrawCondition(), false);

		JD3DUtility::ResourceTransition(gSet.cmdList, rSet.debugResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
		JD3DUtility::ResourceTransition(gSet.cmdList, rSet.rtResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	}
	void JDx12SceneDraw::DrawSceneDebugUIMultiThread(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(drawSet) || !helper.allowDrawDebugObject)
			return;

		GraphicDataSet gSet(static_cast<const JDx12GraphicSceneDrawSet*>(drawSet));
		ResourceDataSet rSet(gSet, helper);
		rSet.SettingDebugging(gSet, helper);

		JD3DUtility::ResourceTransition(gSet.cmdList, rSet.debugResource, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		gSet.cmdList->ClearDepthStencilView(rSet.debugDsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		gSet.cmdList->OMSetRenderTargets(1, &rSet.rtv, true, &rSet.debugDsv);

		BindCommonCB(gSet, rSet, helper);
		BindViewPortAndRect(gSet, rSet);

		const std::vector<JUserPtr<JGameObject>>& objVec = helper.GetGameObjectCashVec(J_RENDER_LAYER::DEBUG_UI, Core::J_MESHGEOMETRY_TYPE::STATIC);
		DrawGameObject(gSet, objVec, helper, JDrawCondition(), false);

		JD3DUtility::ResourceTransition(gSet.cmdList, rSet.debugResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
	}
	void JDx12SceneDraw::DrawSceneShade(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(drawSet))
			return;

		GraphicDataSet gSet(static_cast<const JDx12GraphicSceneDrawSet*>(drawSet));
		ResourceDataSet rSet(gSet, helper);
		if (rSet.canUseLightCluster)
			rSet.SettingCluster(gSet, helper);
		  
		SwitchResourceStateForDeferredShade(gSet, rSet, helper);
		BindRenderTarget(gSet, rSet, helper, false);
		BindCommonCB(gSet, rSet, helper);
		if (rSet.canUseLightCluster)
			BindLightListResource(gSet, rSet, helper);
		BindGBufferResource(gSet, rSet, helper);
		BindViewPortAndRect(gSet, rSet);
		DrawQuadForDeferredShading(gSet, rSet, helper);
		ReturnResourceState(gSet, rSet, helper);
	}
	void JDx12SceneDraw::DrawSceneShadeMultiThread(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(drawSet))
			return;

		GraphicDataSet gSet(static_cast<const JDx12GraphicSceneDrawSet*>(drawSet));
		ResourceDataSet rSet(gSet, helper);
		if (rSet.canUseLightCluster)
			rSet.SettingCluster(gSet, helper);

		SwitchResourceStateForDeferredShade(gSet, rSet, helper);
		BindRenderTarget(gSet, rSet, helper, false);
		BindCommonCB(gSet, rSet, helper);
		if (rSet.canUseLightCluster)
			BindLightListResource(gSet, rSet, helper);
		BindGBufferResource(gSet, rSet, helper);
		BindViewPortAndRect(gSet, rSet);
		DrawQuadForDeferredShading(gSet, rSet, helper);
	}
	void JDx12SceneDraw::DrawGameObject(const GraphicDataSet& set,
		const std::vector<JUserPtr<JGameObject>>& gameObject,
		const JDrawHelper& helper,
		const JDrawCondition& condition,
		const bool isDeferred)
	{
		using GetShaderDataPtr = JGraphicShaderDataHolderBase * (*)(const JShader*, const J_GRAPHIC_SHADER_TYPE type, const J_GRAPHIC_SHADER_VERTEX_LAYOUT);
		auto getForwardShaderDataLam = [](const JShader* shader, const J_GRAPHIC_SHADER_TYPE type, const J_GRAPHIC_SHADER_VERTEX_LAYOUT layout) {return shader->GetGraphicForwardData(type, layout).Get(); };
		auto getDeferredShaderDataLam = [](const JShader* shader, const J_GRAPHIC_SHADER_TYPE type, const J_GRAPHIC_SHADER_VERTEX_LAYOUT layout) {return shader->GetGraphicDeferredData(type, layout).Get(); };
		GetShaderDataPtr getShader = nullptr;
		if (isDeferred)
			getShader = getDeferredShaderDataLam;
		else
			getShader = getForwardShaderDataLam;
		uint objectCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JObjectConstants));
		uint skinCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JAnimationConstants));

		uint objCBIndex = isDeferred ? Deferred::Geometry::objCBIndex : Forward::objCBIndex;
		uint skinCBIndex = isDeferred ? Deferred::Geometry::skinCBIndex : Forward::skinCBIndex;

		auto objectCB = set.dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT)->GetResource();
		auto skinCB = set.dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION)->GetResource();

		const J_SCENE_USE_CASE_TYPE useCase = helper.scene->GetUseCaseType();
		const J_GRAPHIC_SHADER_TYPE shaderType = useCase == J_SCENE_USE_CASE_TYPE::MAIN ?
			J_GRAPHIC_SHADER_TYPE::STANDARD : J_GRAPHIC_SHADER_TYPE::PREVIEW;

		const uint gameObjCount = (uint)gameObject.size();
		uint st = 0;
		uint ed = gameObjCount;
		if (helper.CanDispatchWorkIndex())
			helper.DispatchWorkIndex(gameObjCount, st, ed);

		auto cullUser = helper.GetCullInterface();
		for (uint i = st; i < ed; ++i)
		{
			JRenderItem* renderItem = gameObject[i]->GetRenderItem().Get();
			const uint objFrameIndex = helper.GetObjectFrameIndex(renderItem);
			const uint boundFrameIndex = helper.GetBoundingFrameIndex(renderItem);

			if (condition.allowCulling && !renderItem->IsIgnoreCullingResult() && cullUser.IsCulled(J_CULLING_TARGET::RENDERITEM, boundFrameIndex))
				continue;

			if (condition.allowAllCullingResult && helper.RefelectOtherCamCullig(boundFrameIndex))
				continue;

			JUserPtr<JMeshGeometry> mesh = renderItem->GetMesh();
			const D3D12_VERTEX_BUFFER_VIEW vertexPtr = set.dx12Gm->VertexBufferView(mesh);
			const D3D12_INDEX_BUFFER_VIEW indexPtr = set.dx12Gm->IndexBufferView(mesh);

			set.cmdList->IASetVertexBuffers(0, 1, &vertexPtr);
			set.cmdList->IASetIndexBuffer(&indexPtr);
			set.cmdList->IASetPrimitiveTopology(JD3DUtility::ConvertRenderPrimitive(renderItem->GetPrimitiveType()));

			JAnimator* animator = gameObject[i]->GetComponentWithParent<JAnimator>().Get();
			const uint submeshCount = (uint)mesh->GetTotalSubmeshCount();

			if (condition.allowDebugOutline && gameObject[i]->IsSelected())
				set.cmdList->OMSetStencilRef(Constants::outlineStencilRef);

			for (uint j = 0; j < submeshCount; ++j)
			{
				const JShader* shader = renderItem->GetValidMaterial(j)->GetShader().Get();
				const bool onSkinned = animator != nullptr && condition.allowAnimation;
				const Core::J_MESHGEOMETRY_TYPE meshType = onSkinned ? Core::J_MESHGEOMETRY_TYPE::SKINNED : Core::J_MESHGEOMETRY_TYPE::STATIC;
				const J_GRAPHIC_SHADER_VERTEX_LAYOUT shaderLayout = JShaderType::ConvertToVertexLayout(meshType);

				JDx12GraphicShaderDataHolder* dx12ShaderData = static_cast<JDx12GraphicShaderDataHolder*>((getShader)(shader, shaderType, shaderLayout));
				if (condition.allowDebugOutline && gameObject[i]->IsSelected())
					set.cmdList->SetPipelineState(dx12ShaderData->extraPso[(uint)J_GRAPHIC_SHADER_EXTRA_FUNCTION::STENCIL_WRITE_ALWAYS].Get());
				else
					set.cmdList->SetPipelineState(dx12ShaderData->pso.Get());
			 
				D3D12_GPU_VIRTUAL_ADDRESS objectCBAddress = objectCB->GetGPUVirtualAddress() + (objFrameIndex + j) * objectCBByteSize;
				set.cmdList->SetGraphicsRootConstantBufferView(objCBIndex, objectCBAddress);
				if (onSkinned && meshType == Core::J_MESHGEOMETRY_TYPE::SKINNED)
				{
					D3D12_GPU_VIRTUAL_ADDRESS skinObjCBAddress = skinCB->GetGPUVirtualAddress() + helper.GetAnimationFrameIndex(animator) * skinCBByteSize;
					set.cmdList->SetGraphicsRootConstantBufferView(skinCBIndex, skinObjCBAddress);
				}
				set.cmdList->DrawIndexedInstanced(mesh->GetSubmeshIndexCount(j), 1, mesh->GetSubmeshStartIndexLocation(j), mesh->GetSubmeshBaseVertexLocation(j), 0);
			}
			if (condition.allowDebugOutline && gameObject[i]->IsSelected())
				set.cmdList->OMSetStencilRef(Constants::commonStencilRef);
		}
	}
	void JDx12SceneDraw::RecompileShader(const JGraphicShaderCompileSet& dataSet)
	{
		if (!IsSameDevice(dataSet.device))
			return;

		ReBuildRootSignature(static_cast<JDx12GraphicDevice*>(dataSet.device)->GetDevice(), dataSet.base.info, dataSet.base.option);
		BuildDeferredShader(dataSet);

		auto shaderVec = JShader::StaticTypeInfo().GetInstanceRawPtrVec();
		for (auto& data : shaderVec)
		{
			JShader* shader = static_cast<JShader*>(data);
			if (!shader->IsComputeShader())
				RecompileUserShader(shader);
		}
	}
	JOwnerPtr<JGraphicShaderDataHolderBase> JDx12SceneDraw::CreateGraphicShader(const JGraphicShaderCompileSet& dataSet, JGraphicShaderInitData initData)
	{
		if (!IsSameDevice(dataSet.device))
			return nullptr;
		 
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(dataSet.device);
		auto holder = Core::JPtrUtil::MakeOwnerPtr<JDx12GraphicShaderDataHolder>();

		PsoBuildData psoBuildData;
		psoBuildData.holder = holder.Get();
		psoBuildData.dx12Device = dx12Device;
		psoBuildData.shaderType = initData.shaderType;
		psoBuildData.gFunctionFlag = initData.gFunctionFlag;
		psoBuildData.extraType = J_GRAPHIC_SHADER_EXTRA_FUNCTION::NONE;
		psoBuildData.processType = initData.processType;
		psoBuildData.condition = initData.condition;

		Private::StuffMacro(initData, dataSet.base.info, dataSet.base.option);
		CompileShader(holder.Get(), initData);
		StuffInputLayout(holder->inputLayout, initData.layoutType);
		StuffPso(psoBuildData, dataSet.base.option);

		for (uint i = 0; i < (uint)J_GRAPHIC_SHADER_EXTRA_FUNCTION::COUNT; ++i)
		{
			psoBuildData.extraType = (J_GRAPHIC_SHADER_EXTRA_FUNCTION)i;
			StuffPso(psoBuildData, dataSet.base.option);
		}

		return std::move(holder);
	}
	void JDx12SceneDraw::CompileShader(JDx12GraphicShaderDataHolder* holder, const JGraphicShaderInitData& initData)
	{
		std::wstring vertexShaderPath = JApplicationEngine::ShaderPath() + L"\\VertexShader.hlsl";
		std::wstring pixelShaderPath = JApplicationEngine::ShaderPath();
		if (initData.processType == J_GRAPHIC_RENDERING_PROCESS::DEFERRED_GEOMETRY)
			pixelShaderPath += L"\\PixelStuffGBuffer.hlsl";
		else if (initData.processType == J_GRAPHIC_RENDERING_PROCESS::DEFERRED_SHADING)
			pixelShaderPath += L"\\PixelDeferredShader.hlsl";
		else
			pixelShaderPath += L"\\PixelForwardShader.hlsl";

		holder->vs = JDxShaderDataUtil::CompileShader(vertexShaderPath, initData.macro[(uint)initData.layoutType], L"VS", L"vs_6_0");
		holder->ps = JDxShaderDataUtil::CompileShader(pixelShaderPath, initData.macro[(uint)initData.layoutType], L"PS", L"ps_6_0");
	}
	void JDx12SceneDraw::StuffInputLayout(_Out_ std::vector<D3D12_INPUT_ELEMENT_DESC>& outInputLayout, const J_GRAPHIC_SHADER_VERTEX_LAYOUT vertexLayout)
	{
		switch (vertexLayout)
		{
		case JinEngine::J_GRAPHIC_SHADER_VERTEX_LAYOUT::STATIC:
		{
			outInputLayout =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};
			break;
		}
		case JinEngine::J_GRAPHIC_SHADER_VERTEX_LAYOUT::SKINNED:
		{
			outInputLayout =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "BONEINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 56, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};
			break;
		}
		default:
			break;
		}
	}
	void JDx12SceneDraw::StuffPso(const PsoBuildData& data, const JGraphicOption& option)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC newShaderPso;
		ZeroMemory(&newShaderPso, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		newShaderPso.InputLayout = { data.holder->inputLayout.data(), (uint)data.holder->inputLayout.size() };
		if (data.processType == J_GRAPHIC_RENDERING_PROCESS::DEFERRED_GEOMETRY)
			newShaderPso.pRootSignature = deferredGeometryRootSignature.Get();
		else if (data.processType == J_GRAPHIC_RENDERING_PROCESS::DEFERRED_SHADING)
			newShaderPso.pRootSignature = deferredShadingRootSignature.Get();
		else
			newShaderPso.pRootSignature = forwardRootSignature.Get();
		newShaderPso.VS =
		{
			reinterpret_cast<BYTE*>(data.holder->vs->GetBufferPointer()),
			data.holder->vs->GetBufferSize()
		};
		newShaderPso.PS =
		{
			reinterpret_cast<BYTE*>(data.holder->ps->GetBufferPointer()),
			data.holder->ps->GetBufferSize()
		};
		if (data.holder->hs != nullptr)
		{
			newShaderPso.HS =
			{
				reinterpret_cast<BYTE*>(data.holder->hs->GetBufferPointer()),
				data.holder->hs->GetBufferSize()
			};
		}
		if (data.holder->ds != nullptr)
		{
			newShaderPso.DS =
			{
				reinterpret_cast<BYTE*>(data.holder->ds->GetBufferPointer()),
				data.holder->ds->GetBufferSize()
			};
		}
		if (data.holder->gs != nullptr)
		{
			newShaderPso.GS =
			{
				reinterpret_cast<BYTE*>(data.holder->gs->GetBufferPointer()),
				data.holder->gs->GetBufferSize()
			};
		}

		newShaderPso.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		newShaderPso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		if (data.extraType == J_GRAPHIC_SHADER_EXTRA_FUNCTION::STENCIL_WRITE_ALWAYS)
		{
			newShaderPso.DepthStencilState.StencilEnable = true;
			newShaderPso.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
			newShaderPso.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_REPLACE;
			newShaderPso.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_REPLACE;
			newShaderPso.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
			newShaderPso.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_REPLACE;
			newShaderPso.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_REPLACE;
		}

		if ((data.gFunctionFlag & SHADER_FUNCTION_SKY) > 0)
		{
			newShaderPso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			//newShaderPso.DepthStencilState.StencilWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; 
			newShaderPso.DepthStencilState.StencilEnable = false;
		}

		newShaderPso.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		newShaderPso.SampleMask = UINT_MAX;
		newShaderPso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		newShaderPso.NumRenderTargets = 1;
		newShaderPso.RTVFormats[0] = JDx12GraphicResourceManager::GetBackBufferFormat();
		newShaderPso.SampleDesc.Count = data.dx12Device->GetM4xMsaaState() ? 4 : 1;
		newShaderPso.SampleDesc.Quality = data.dx12Device->GetM4xMsaaState() ? (data.dx12Device->GetM4xMsaaQuality() - 1) : 0;

		newShaderPso.DSVFormat = JDx12GraphicResourceManager::GetDepthStencilFormat();
		if ((data.gFunctionFlag & SHADER_FUNCTION_DEBUG) > 0)
		{
			//newShaderPso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			newShaderPso.DepthStencilState.StencilEnable = false;
		}
		//Debug
		//For Occ
		/*
		if (((gFunctionFlag & SHADER_FUNCTION_LIGHT) > 0) && ((gFunctionFlag & SHADER_FUNCTION_SHADOW) > 0))
		{
			CD3DX12_BLEND_DESC blendDesc(D3D12_DEFAULT);
			blendDesc.RenderTarget[0] =
			{
				TRUE, FALSE,
				D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_OP_ADD,
				D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
				D3D12_LOGIC_OP_NOOP,
				D3D12_COLOR_WRITE_ENABLE_ALL,
			};
			newShaderPso.BlendState = blendDesc;
		}
		*/

		if (data.processType == J_GRAPHIC_RENDERING_PROCESS::DEFERRED_SHADING)
		{
			newShaderPso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			newShaderPso.DepthStencilState.StencilWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			newShaderPso.DepthStencilState.DepthEnable = false;
		}
		else if (data.processType == J_GRAPHIC_RENDERING_PROCESS::DEFERRED_GEOMETRY)
		{
			newShaderPso.NumRenderTargets = Deferred::layerCount;
			newShaderPso.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			newShaderPso.RTVFormats[1] = DXGI_FORMAT_R10G10B10A2_UNORM;
			newShaderPso.RTVFormats[2] = DXGI_FORMAT_R10G10B10A2_UNORM;
			newShaderPso.RTVFormats[3] = DXGI_FORMAT_R8G8B8A8_UNORM; 
		}

		if (data.condition.primitiveCondition == J_SHADER_APPLIY_CONDITION::APPLY)
			newShaderPso.PrimitiveTopologyType = Private::ConvertD3d12PrimitiveType(data.condition.primitiveType);
		if (data.condition.depthCompareCondition == J_SHADER_APPLIY_CONDITION::APPLY)
			newShaderPso.DepthStencilState.DepthFunc = Private::ConvertD3d12Comparesion(data.condition.depthCompareFunc);
		if (data.condition.cullModeCondition == J_SHADER_APPLIY_CONDITION::APPLY)
			newShaderPso.RasterizerState.CullMode = Private::ConvertD3d12CullMode(data.condition.isCullModeNone);

		if (option.useMSAA)
		{
			newShaderPso.RasterizerState.MultisampleEnable = true;
		}

		if (data.extraType == J_GRAPHIC_SHADER_EXTRA_FUNCTION::NONE)
		{
			ThrowIfFailedG(data.dx12Device->GetDevice()->CreateGraphicsPipelineState(&newShaderPso, IID_PPV_ARGS(data.holder->pso.GetAddressOf())));
			data.holder->pso->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Pso") - 1, "Pso");
		}
		else
		{
			ThrowIfFailedG(data.dx12Device->GetDevice()->CreateGraphicsPipelineState(&newShaderPso, IID_PPV_ARGS(data.holder->extraPso[(int)data.extraType].GetAddressOf())));
			data.holder->extraPso[(int)data.extraType]->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Extra Pso") - 1, "Extra Pso");
		}
	}
	void JDx12SceneDraw::BuildForwardRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_DESCRIPTOR_RANGE tex2DTable;
		tex2DTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.binding2DTextureCapacity, 2, 0);

		CD3DX12_DESCRIPTOR_RANGE cubeMapTable;
		cubeMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.bindingCubeMapCapacity, 2, 1);

		CD3DX12_DESCRIPTOR_RANGE shadowMapTable;
		shadowMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.bindingShadowTextureCapacity, 2, 2);

		CD3DX12_DESCRIPTOR_RANGE shadowMapArryTable;
		shadowMapArryTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.bindingShadowTextureArrayCapacity, 2, 3);

		CD3DX12_DESCRIPTOR_RANGE shadowMapCubeTable;
		shadowMapCubeTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.bindingShadowTextureCubeCapacity, 2, 4);

		CD3DX12_DESCRIPTOR_RANGE depthMapTable;
		depthMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 6);

		CD3DX12_DESCRIPTOR_RANGE offsetBufferTable;
		offsetBufferTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 7);

		CD3DX12_DESCRIPTOR_RANGE linkedListBuffertable;
		linkedListBuffertable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 8);
		  
		const int slocCount = option.allowLightCluster ? Forward::rootSlotCountIncCluster : Forward::rootSlotCount;
		CD3DX12_ROOT_PARAMETER* slotRootParameter = new CD3DX12_ROOT_PARAMETER[slocCount]();

		// Create root CBV.
		slotRootParameter[Forward::enginePassCBIndex].InitAsConstantBufferView(Forward::enginePassCBIndex);
		slotRootParameter[Forward::scenePassCBIndex].InitAsConstantBufferView(Forward::scenePassCBIndex);
		slotRootParameter[Forward::camCBIndex].InitAsConstantBufferView(Forward::camCBIndex);
		slotRootParameter[Forward::objCBIndex].InitAsConstantBufferView(Forward::objCBIndex);
		slotRootParameter[Forward::skinCBIndex].InitAsConstantBufferView(Forward::skinCBIndex); 

		//Shading Buff
		slotRootParameter[Forward::dLitBuffIndex].InitAsShaderResourceView(0, 0);
		slotRootParameter[Forward::pLitBuffIndex].InitAsShaderResourceView(0, 1);
		slotRootParameter[Forward::sLitBuffIndex].InitAsShaderResourceView(0, 2);
		slotRootParameter[Forward::rLitBuffIndex].InitAsShaderResourceView(0, 3);
		slotRootParameter[Forward::csmBuffIndex].InitAsShaderResourceView(0, 4);
		//Material Buff
		slotRootParameter[Forward::matBuffIndex].InitAsShaderResourceView(1);
		//Texture Buff
		slotRootParameter[Forward::texture2DBuffIndex].InitAsDescriptorTable(1, &tex2DTable, D3D12_SHADER_VISIBILITY_ALL);
		slotRootParameter[Forward::textureCubeBuffIndex].InitAsDescriptorTable(1, &cubeMapTable, D3D12_SHADER_VISIBILITY_ALL);
		slotRootParameter[Forward::textureShadowMapBuffIndex].InitAsDescriptorTable(1, &shadowMapTable, D3D12_SHADER_VISIBILITY_ALL);
		slotRootParameter[Forward::textureShadowMapArrayBuffIndex].InitAsDescriptorTable(1, &shadowMapArryTable, D3D12_SHADER_VISIBILITY_ALL);
		slotRootParameter[Forward::textureShadowMapCubeBuffIndex].InitAsDescriptorTable(1, &shadowMapCubeTable, D3D12_SHADER_VISIBILITY_ALL);
		//slotRootParameter[Forward::textureNormalMapIndex].InitAsDescriptorTable(1, &normalMapTable, D3D12_SHADER_VISIBILITY_ALL);
		if (option.allowLightCluster)
		{
			slotRootParameter[Forward::depthMapBufferIndex].InitAsDescriptorTable(1, &depthMapTable, D3D12_SHADER_VISIBILITY_ALL);
			slotRootParameter[Forward::cluserOffsetBufferIndex].InitAsDescriptorTable(1, &offsetBufferTable, D3D12_SHADER_VISIBILITY_ALL);
			slotRootParameter[Forward::clusterLinkBufferIndex].InitAsDescriptorTable(1, &linkedListBuffertable, D3D12_SHADER_VISIBILITY_ALL);
		}

		const std::vector<CD3DX12_STATIC_SAMPLER_DESC> sam = Forward::Sampler();
		// A root signature is an array of root parameters.
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(slocCount,
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
			IID_PPV_ARGS(forwardRootSignature.GetAddressOf())));

		forwardRootSignature->SetName(L"Main Forward RootSignature");
		delete[] slotRootParameter;
	}
	void JDx12SceneDraw::BuildDeferredGeometryRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		using namespace Deferred;
		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_DESCRIPTOR_RANGE tex2DTable;
		tex2DTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.binding2DTextureCapacity, 2, 0);

		CD3DX12_DESCRIPTOR_RANGE cubeMapTable;
		cubeMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.bindingCubeMapCapacity, 2, 1);

		CD3DX12_ROOT_PARAMETER slotRootParameter[Geometry::rootSlotCount];

		// Create root CBV.
		slotRootParameter[Geometry::enginePassCBIndex].InitAsConstantBufferView(Geometry::enginePassCBIndex);
		slotRootParameter[Geometry::scenePassCBIndex].InitAsConstantBufferView(Geometry::scenePassCBIndex);
		slotRootParameter[Geometry::camCBIndex].InitAsConstantBufferView(Geometry::camCBIndex);
		slotRootParameter[Geometry::objCBIndex].InitAsConstantBufferView(Geometry::objCBIndex);
		slotRootParameter[Geometry::skinCBIndex].InitAsConstantBufferView(Geometry::skinCBIndex);

		//Material Buff
		slotRootParameter[Geometry::matBuffIndex].InitAsShaderResourceView(1);
		//Texture Buff
		slotRootParameter[Geometry::texture2DBuffIndex].InitAsDescriptorTable(1, &tex2DTable, D3D12_SHADER_VISIBILITY_ALL);
		slotRootParameter[Geometry::textureCubeBuffIndex].InitAsDescriptorTable(1, &cubeMapTable, D3D12_SHADER_VISIBILITY_ALL);
		//slotRootParameter[Constants::textureNormalMapIndex].InitAsDescriptorTable(1, &normalMapTable, D3D12_SHADER_VISIBILITY_ALL);

		const std::vector<CD3DX12_STATIC_SAMPLER_DESC> sam = Geometry::Sampler();

		// A root signature is an array of root parameters.
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(Geometry::rootSlotCount,
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
			IID_PPV_ARGS(deferredGeometryRootSignature.GetAddressOf())));

		deferredGeometryRootSignature->SetName(L"Main Deferred Geometry RootSignature");
	}
	void JDx12SceneDraw::BuildDeferredShadingRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		using namespace Deferred;
		CD3DX12_DESCRIPTOR_RANGE tex2DTable;
		tex2DTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.binding2DTextureCapacity, 2, 0);

		CD3DX12_DESCRIPTOR_RANGE cubeMapTable;
		cubeMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.bindingCubeMapCapacity, 2, 1);

		CD3DX12_DESCRIPTOR_RANGE shadowMapTable;
		shadowMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.bindingShadowTextureCapacity, 2, 2);

		CD3DX12_DESCRIPTOR_RANGE shadowMapArryTable;
		shadowMapArryTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.bindingShadowTextureArrayCapacity, 2, 3);

		CD3DX12_DESCRIPTOR_RANGE shadowMapCubeTable;
		shadowMapCubeTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.bindingShadowTextureCubeCapacity, 2, 4);

		CD3DX12_DESCRIPTOR_RANGE gBufferTable;
		gBufferTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, layerCount, 2, 5);

		CD3DX12_DESCRIPTOR_RANGE depthMapTable;
		depthMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 6);

		CD3DX12_DESCRIPTOR_RANGE offsetBufferTable;
		offsetBufferTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 7);

		CD3DX12_DESCRIPTOR_RANGE linkedListBuffertable;
		linkedListBuffertable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 8);

		const int slocCount = option.allowLightCluster ? Shading::rootSlotCountIncCluster : Shading::rootSlotCount;
		CD3DX12_ROOT_PARAMETER* slotRootParameter = new CD3DX12_ROOT_PARAMETER[slocCount]();
 
		// Create root CBV. 
		slotRootParameter[Shading::enginePassCBIndex].InitAsConstantBufferView(Shading::enginePassCBIndex);
		slotRootParameter[Shading::scenePassCBIndex].InitAsConstantBufferView(Shading::scenePassCBIndex);
		slotRootParameter[Shading::camCBIndex].InitAsConstantBufferView(Shading::camCBIndex);

		//Shading Buff
		slotRootParameter[Shading::dLitBuffIndex].InitAsShaderResourceView(0, 0);
		slotRootParameter[Shading::pLitBuffIndex].InitAsShaderResourceView(0, 1);
		slotRootParameter[Shading::sLitBuffIndex].InitAsShaderResourceView(0, 2);
		slotRootParameter[Shading::rLitBuffIndex].InitAsShaderResourceView(0, 3);
		slotRootParameter[Shading::csmBuffIndex].InitAsShaderResourceView(0, 4);
		//Texture Buff
		slotRootParameter[Shading::texture2DBuffIndex].InitAsDescriptorTable(1, &tex2DTable, D3D12_SHADER_VISIBILITY_ALL);
		slotRootParameter[Shading::textureCubeBuffIndex].InitAsDescriptorTable(1, &cubeMapTable, D3D12_SHADER_VISIBILITY_ALL);
		slotRootParameter[Shading::textureShadowMapBuffIndex].InitAsDescriptorTable(1, &shadowMapTable, D3D12_SHADER_VISIBILITY_ALL);
		slotRootParameter[Shading::textureShadowMapArrayBuffIndex].InitAsDescriptorTable(1, &shadowMapArryTable, D3D12_SHADER_VISIBILITY_ALL);
		slotRootParameter[Shading::textureShadowMapCubeBuffIndex].InitAsDescriptorTable(1, &shadowMapCubeTable, D3D12_SHADER_VISIBILITY_ALL);

		slotRootParameter[Shading::gBufferIndex].InitAsDescriptorTable(1, &gBufferTable, D3D12_SHADER_VISIBILITY_ALL);
		slotRootParameter[Shading::depthMapBufferIndex].InitAsDescriptorTable(1, &depthMapTable, D3D12_SHADER_VISIBILITY_ALL);
		if (option.allowLightCluster)
		{ 
			slotRootParameter[Shading::cluserOffsetBufferIndex].InitAsDescriptorTable(1, &offsetBufferTable, D3D12_SHADER_VISIBILITY_ALL);
			slotRootParameter[Shading::clusterLinkBufferIndex].InitAsDescriptorTable(1, &linkedListBuffertable, D3D12_SHADER_VISIBILITY_ALL);
		}

		const std::vector<CD3DX12_STATIC_SAMPLER_DESC> sam = Shading::Sampler();

		// A root signature is an array of root parameters.
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(slocCount,
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
			IID_PPV_ARGS(deferredShadingRootSignature.GetAddressOf())));

		deferredShadingRootSignature->SetName(L"Main Deferred Shading RootSignature");
		delete[] slotRootParameter;
	}
	void JDx12SceneDraw::ReBuildRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		if (option.allowDeferred)
		{
			forwardRootSignature = nullptr;
			BuildForwardRootSignature(device, info, option);

			deferredGeometryRootSignature = nullptr;
			BuildDeferredGeometryRootSignature(device, info, option);

			deferredShadingRootSignature = nullptr;
			BuildDeferredShadingRootSignature(device, info, option);
		}
		else
		{
			if (deferredGeometryRootSignature != nullptr)
				deferredGeometryRootSignature = nullptr;
			if (deferredShadingRootSignature != nullptr)
				deferredShadingRootSignature = nullptr;

			forwardRootSignature = nullptr;
			BuildForwardRootSignature(device, info, option);
		}
	}
	void JDx12SceneDraw::BuildDeferredShader(const JGraphicShaderCompileSet& dataSet)
	{
		for(uint i =0; i < (uint)INNER_DEFERRED_SHADER_TYPE::COUNT; ++i)
			deferredShadingHolder[i] = nullptr;

		if (!dataSet.base.option.allowDeferred)
			return;

		JGraphicShaderInitData initData;
		initData.gFunctionFlag = SHADER_FUNCTION_NONE;		//flag에 따른 data(Material 종속)는 모두 geometry step에서 계산되고 필요하면 gBuffer에 저장된다.
		initData.layoutType = J_GRAPHIC_SHADER_VERTEX_LAYOUT::STATIC;
		initData.processType = J_GRAPHIC_RENDERING_PROCESS::DEFERRED_SHADING;
		initData.macroOption.allowLightCulling = true;

		auto holder = CreateGraphicShader(dataSet, initData); 
		deferredShadingHolder[(uint)INNER_DEFERRED_SHADER_TYPE::STANDARD] = static_cast<JDx12GraphicShaderDataHolder*>(holder.Release());
	
		initData.macroOption.allowLightCulling = false;
		holder = CreateGraphicShader(dataSet, initData);
		deferredShadingHolder[(uint)INNER_DEFERRED_SHADER_TYPE::EXCEPT_LIGHT_CULLING] = static_cast<JDx12GraphicShaderDataHolder*>(holder.Release());

	}
}