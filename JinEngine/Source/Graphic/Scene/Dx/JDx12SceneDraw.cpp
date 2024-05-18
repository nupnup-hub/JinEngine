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


#include"JDx12SceneDraw.h"  
#include"../../JGraphicUpdateHelper.h"
#include"../../JGraphicInfo.h"
#include"../../JGraphicOption.h" 
#include"../../Device/Dx/JDx12GraphicDevice.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceInfo.h" 
#include"../../Culling/Dx/JDx12CullingManager.h"
#include"../../Culling/JCullingInterface.h"
#include"../../Culling/JCullingInfo.h"
#include"../../Command/Dx/JDx12CommandContext.h"
#include"../../FrameResource/Dx/JDx12FrameResource.h"
#include"../../FrameResource/JObjectConstants.h" 
#include"../../FrameResource/JAnimationConstants.h" 
#include"../../FrameResource/JMaterialConstants.h" 
#include"../../FrameResource/JSceneConstants.h" 
#include"../../FrameResource/JCameraConstants.h" 
#include"../../FrameResource/JLightConstants.h"      
#include"../../Utility/Dx/JDx12Utility.h"
#include"../../Utility/Dx/JDx12ObjectCreation.h"
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
#include"../../../Application/Engine/JApplicationEngine.h"

//Graphic Shader Macro Symbol  
#define TEXTURE_2D_COUNT_SYMBOL L"TEXTURE_2D_COUNT"
#define TEXTURE_CUBE_COUNT_SYMBOL L"CUBE_MAP_COUNT"
#define SHADOW_MAP_COUNT_SYMBOL L"SHADOW_MAP_COUNT"
#define SHADOW_MAP_ARRAY_COUNT_SYMBOL L"SHADOW_MAP_ARRAY_COUNT"
#define SHADOW_MAP_CUBE_COUNT_SYMBOL L"SHADOW_MAP_CUBE_COUNT"

#define G_BUFFER_LAYER_COUNT L"G_BUFFER_LAYER_COUNT"

#define USE_DIRECTIONAL_LIGHT_PCF L"USE_DIRECTIONAL_LIGHT_PCF"
#define USE_DIRECTIONAL_LIGHT_PCSS L"USE_DIRECTIONAL_LIGHT_PCSS"  

#define USE_POINT_LIGHT_PCF L"USE_POINT_LIGHT_PCF"   
#define USE_POINT_LIGHT_PCSS L"USE_POINT_LIGHT_PCSS"   

#define USE_SPOT_LIGHT_PCF L"USE_SPOT_LIGHT_PCF"    
#define USE_SPOT_LIGHT_PCSS L"USE_SPOT_LIGHT_PCSS"   

#define USE_PCF_16_SAMPLE L"USE_PCF_16_SAMPLE" 
#define USE_PCF_32_SAMPLE L"USE_PCF_32_SAMPLE" 

#define USE_BRDF_GGX_MICROFACET L"USE_BRDF_GGX_MICROFACET"  
#define USE_BRDF_BECKMANN_MICROFACET L"USE_BRDF_BECKMANN_MICROFACET"  
#define USE_BRDF_BLINN_PHONG_MICROFACET L"USE_BRDF_BLINN_PHONG_MICROFACET"  
#define USE_BRDF_ISOTROPY_NDF L"USE_BRDF_ISOTROPY_NDF"  

#define USE_BRDF_DISNEY_DIFFUSE L"USE_BRDF_DISNEY_DIFFUSE"  
#define USE_FROST_BITE_DISNEY_DIFFUSE L"USE_FROST_BITE_DISNEY_DIFFUSE"  
#define USE_BRDF_HAMMON_DIFFUSE L"USE_BRDF_HAMMON_DIFFUSE"  
#define USE_OREN_NAYAR_DIFFUSE L"USE_OREN_NAYAR_DIFFUSE"
#define USE_BRDF_SHIRELY_DIFFUSE L"USE_BRDF_SHIRELY_DIFFUSE"  
#define USE_BRDF_LAMBERTIAN_DIFFUSE L"USE_BRDF_LAMBERTIAN_DIFFUSE"  

#define USE_LIGHT_CLUSTER L"LIGHT_CLUSTER" 
#define USE_GLOBAL_ILLUMINATION L"GLOBAL_ILLUMINATION" 

#define USE_DEFERRED_GEOMETRY L"DEFERRED_GEOMETRY"
#define USE_DEFERRED_SHADING L"DEFERRED_SHADING" 

#define USE_FULL_SCREEN_QUAD L"FULL_SCREEN_QUAD"
#define USE_FULL_SCREEN_TRIANGLE L"FULL_SCREEN_TRIANGLE" 

#define CLUSTER_DIM_X L"CLUSTER_DIM_X"
#define CLUSTER_DIM_Y L"CLUSTER_DIM_Y"
#define CLUSTER_DIM_Z L"CLUSTER_DIM_Z"

#define NEAR_CLUST L"NEAR_CLUST"

#define G_BUFFER_ALBEDO_COLOR L"G_BUFFER_ALBEDO_COLOR"
#define G_BUFFER_SPECULAR_COLOR L"G_BUFFER_SPECULAR_COLOR"
#define G_BUFFER_NORMAL_AND_TANGENT L"G_BUFFER_NORMAL_AND_TANGENT"
#define G_BUFFER_LIGHT_PROP L"G_BUFFER_LIGHT_PROP"
#define G_BUFFER_VELOCITY L"G_BUFFER_VELOCITY" 

namespace JinEngine::Graphic
{
	namespace Private
	{
		static constexpr uint useFullscreenQuad = false;

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
			if (Core::HasSQValueEnum(f, SHADER_FUNCTION_SPECULAR_MAP))
				set.push_back({ L"SPECULAR_MAP", L"8" });

			if (Core::HasSQValueEnum(f, SHADER_FUNCTION_SHADOW))
				set.push_back({ L"SHADOW", L"9" });
			if (Core::HasSQValueEnum(f, SHADER_FUNCTION_LIGHT))
				set.push_back({ L"LIGHT", L"10" });
			if (Core::HasSQValueEnum(f, SHADER_FUNCTION_SKY))
				set.push_back({ L"SKY", L"11" });
			if (Core::HasSQValueEnum(f, SHADER_FUNCTION_ALPHA_CLIP))
				set.push_back({ L"ALPHA_CLIP", L"12" });
			if (Core::HasSQValueEnum(f, SHADER_FUNCTION_DEBUG))
				set.push_back({ L"DEBUG", L"13" });
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

				initHelper.macro[i].push_back({ TEXTURE_2D_COUNT_SYMBOL, std::to_wstring(info.resource.binding2DTextureCapacity) });
				initHelper.macro[i].push_back({ TEXTURE_CUBE_COUNT_SYMBOL, std::to_wstring(info.resource.bindingCubeMapCapacity) });
				initHelper.macro[i].push_back({ SHADOW_MAP_COUNT_SYMBOL,std::to_wstring(info.resource.bindingShadowTextureCapacity) });
				initHelper.macro[i].push_back({ SHADOW_MAP_ARRAY_COUNT_SYMBOL, std::to_wstring(info.resource.bindingShadowTextureArrayCapacity) });
				initHelper.macro[i].push_back({ SHADOW_MAP_CUBE_COUNT_SYMBOL,std::to_wstring(info.resource.bindingShadowTextureCubeCapacity) });

				initHelper.macro[i].push_back({ G_BUFFER_ALBEDO_COLOR, std::to_wstring(Constants::gBufferAlbedoLayer) });
				initHelper.macro[i].push_back({ G_BUFFER_NORMAL_AND_TANGENT,std::to_wstring(Constants::gBufferNormalAndTangentLayer) });
				initHelper.macro[i].push_back({ G_BUFFER_LIGHT_PROP,std::to_wstring(Constants::gBufferLightPropertyLayer) });

				//initHelper.macro[i].push_back({ G_BUFFER_VELOCITY, std::to_wstring(Constants::gBufferVelocityLayer) });
				//initHelper.macro[i].push_back({ G_BUFFER_VELOCITY,std::to_wstring(Constants::gBufferVelocityLayer) });
				initHelper.macro[i].push_back({ G_BUFFER_LAYER_COUNT,std::to_wstring(Constants::gBufferLayerCount) });

				//Shadow
				if (option.shadow.useHighQualityShadow)
				{
					initHelper.macro[i].push_back({ USE_DIRECTIONAL_LIGHT_PCSS, std::to_wstring(1) });
					initHelper.macro[i].push_back({ USE_POINT_LIGHT_PCF, std::to_wstring(1) });
					initHelper.macro[i].push_back({ USE_SPOT_LIGHT_PCF, std::to_wstring(1) });
					initHelper.macro[i].push_back({ USE_PCF_32_SAMPLE, std::to_wstring(1) });
				}
				else if (option.shadow.useMiddleQualityShadow)
				{
					initHelper.macro[i].push_back({ USE_DIRECTIONAL_LIGHT_PCF, std::to_wstring(1) });
					initHelper.macro[i].push_back({ USE_POINT_LIGHT_PCF, std::to_wstring(1) });
					initHelper.macro[i].push_back({ USE_SPOT_LIGHT_PCF, std::to_wstring(1) });
					initHelper.macro[i].push_back({ USE_PCF_32_SAMPLE, std::to_wstring(1) });
				}
				else if (option.shadow.useLowQualityShadow)
				{
					initHelper.macro[i].push_back({ USE_DIRECTIONAL_LIGHT_PCF, std::to_wstring(1) });
					initHelper.macro[i].push_back({ USE_POINT_LIGHT_PCF, std::to_wstring(1) });
					initHelper.macro[i].push_back({ USE_SPOT_LIGHT_PCF, std::to_wstring(1) });
					initHelper.macro[i].push_back({ USE_PCF_16_SAMPLE, std::to_wstring(1) });
				}

				//BRDF 
				if (option.rendering.useGGXMicrofacet)
					initHelper.macro[i].push_back({ USE_BRDF_GGX_MICROFACET, std::to_wstring(1) });
				if (option.rendering.useBeckmannMicrofacet)
					initHelper.macro[i].push_back({ USE_BRDF_BECKMANN_MICROFACET, std::to_wstring(1) });
				if (option.rendering.useBlinnPhongMicrofacet)
					initHelper.macro[i].push_back({ USE_BRDF_BLINN_PHONG_MICROFACET, std::to_wstring(1) });
				if (option.rendering.useIsotropy)
					initHelper.macro[i].push_back({ USE_BRDF_ISOTROPY_NDF, std::to_wstring(1) });
				if (option.rendering.useDisneyDiffuse)
					initHelper.macro[i].push_back({ USE_BRDF_DISNEY_DIFFUSE, std::to_wstring(1) });
				if (option.rendering.useFrostBiteDiffuse)
					initHelper.macro[i].push_back({ USE_FROST_BITE_DISNEY_DIFFUSE, std::to_wstring(1) });
				if (option.rendering.useHammonDiffuse)
					initHelper.macro[i].push_back({ USE_BRDF_HAMMON_DIFFUSE, std::to_wstring(1) });
				if (option.rendering.useOrenNayarDiffuse)
					initHelper.macro[i].push_back({ USE_OREN_NAYAR_DIFFUSE, std::to_wstring(1) });
				if (option.rendering.useShirleyDiffuse)
					initHelper.macro[i].push_back({ USE_BRDF_SHIRELY_DIFFUSE, std::to_wstring(1) });
				if (option.rendering.useLambertianDiffuse)
					initHelper.macro[i].push_back({ USE_BRDF_LAMBERTIAN_DIFFUSE, std::to_wstring(1) });

				//Rendering
				if (option.culling.allowLightCluster && initHelper.shaderType != J_GRAPHIC_SHADER_TYPE::PREVIEW &&
					Core::HasSQValueEnum(initHelper.privateFlag, SHADER_FUNCTION_PRIVATE_LIGHT_CULLING))
					initHelper.macro[i].push_back({ USE_LIGHT_CLUSTER, std::to_wstring(1) });

				if (option.rendering.allowRaytracing && initHelper.shaderType != J_GRAPHIC_SHADER_TYPE::PREVIEW &&
					Core::HasSQValueEnum(initHelper.privateFlag, SHADER_FUNCTION_PRIVATE_GLOBAL_ILLUMINATION))
					initHelper.macro[i].push_back({ USE_GLOBAL_ILLUMINATION, std::to_wstring(1) });


				if (initHelper.processType == J_GRAPHIC_RENDERING_PROCESS::DEFERRED_GEOMETRY)
					initHelper.macro[i].push_back({ USE_DEFERRED_GEOMETRY, std::to_wstring(1) });
				else if (initHelper.processType == J_GRAPHIC_RENDERING_PROCESS::DEFERRED_SHADING)
				{
					//material per shader가 있는 forward, deferred geo와 달리
					//deferred shade는 scene class가 하나만 소유하므로
					//음영계산에서 material에 property에 종속된 macro값들중 일부(주로 음영 계산에 대한 분기)를 직접 setting 해줘야함					
					//ex) Shadow
					if constexpr (useFullscreenQuad)
						initHelper.macro[i].push_back({ USE_FULL_SCREEN_QUAD, std::to_wstring(1) });
					else
						initHelper.macro[i].push_back({ USE_FULL_SCREEN_TRIANGLE, std::to_wstring(1) });
					initHelper.macro[i].push_back({ USE_DEFERRED_SHADING, std::to_wstring(1) });
					ConvertMacroSet(J_GRAPHIC_SHADER_FUNCTION::SHADER_FUNCTION_SHADOW, initHelper.macro[i]);
				}

				if (option.debugging.testTrigger00)
					initHelper.macro[i].push_back({ L"TEST", std::to_wstring(1) });

				switch (JinEngine::Graphic::Constants::litClusterXRange[option.culling.clusterXIndex])
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
				switch (JinEngine::Graphic::Constants::litClusterYRange[option.culling.clusterYIndex])
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
				switch (JinEngine::Graphic::Constants::litClusterZRange[option.culling.clusterZIndex])
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
				initHelper.macro[i].push_back({ NEAR_CLUST, std::to_wstring(option.culling.clusterNear) });
				//if(option.postProcess.useSsao  || option.useHbao)
				//	initHelper.macro[i].push_back({ USE_SSAO, std::to_wstring(1) });
				//initHelper.macro[i].push_back(shaderFuncMacroMap.find(SHADER_FUNCTION_NONE)->second);
			}
		}
	}
	namespace Forward
	{
		//graphic root
		static constexpr int sceneCBIndex = 0;
		static constexpr int camCBIndex = sceneCBIndex + 1;
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

		static constexpr int ambientOcclusionMapIndex = textureShadowMapCubeBuffIndex + 1;
		static constexpr int depthMapBufferIndex = ambientOcclusionMapIndex + 1;
		static constexpr int cluserOffsetBufferIndex = depthMapBufferIndex + 1;
		static constexpr int clusterLinkBufferIndex = cluserOffsetBufferIndex + 1;
		//static constexpr int textureNormalMapIndex = textureShadowMapCubeBuffIndex + 1;

		static constexpr int rootSlotCount = ambientOcclusionMapIndex + 1;
		static constexpr int rootSlotCountIncCluster = clusterLinkBufferIndex + 1;

		static constexpr int samplerCount = 8;
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
		static constexpr uint layerCount = Constants::gBufferLayerCount;
		namespace Geometry
		{
			//graphic root 
			static constexpr int sceneCBIndex = 0;
			static constexpr int camCBIndex = sceneCBIndex + 1;
			static constexpr int objCBIndex = camCBIndex + 1;
			static constexpr int skinCBIndex = objCBIndex + 1;
			//static constexpr int litIndexCBIndex = camCBIndex + 1;
			//static constexpr int boundObjCBIndex = litIndexCBIndex + 1;

			static constexpr int matBuffIndex = skinCBIndex + 1;

			static constexpr int texture2DBuffIndex = matBuffIndex + 1;
			static constexpr int textureCubeBuffIndex = texture2DBuffIndex + 1;
			static constexpr int rootSlotCount = textureCubeBuffIndex + 1;

			static constexpr int samplerCount = 4;
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
			static constexpr int sceneCBIndex = 0;
			static constexpr int camCBIndex = sceneCBIndex + 1;

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

			static constexpr int ambientOcclusionMapIndex = textureShadowMapCubeBuffIndex + 1;

			static constexpr int gBufferIndex = ambientOcclusionMapIndex + 1;
			static constexpr int depthMapBufferIndex = gBufferIndex + 1;

			static constexpr int cluserOffsetBufferIndex = depthMapBufferIndex + 1;
			static constexpr int clusterLinkBufferIndex = cluserOffsetBufferIndex + 1;

			static constexpr int giBufferIndex = clusterLinkBufferIndex + 1;

			static constexpr int rootSlotCount = giBufferIndex + 1;

			//static constexpr int exceptAllRootSlotCount = depthMapBufferIndex + 1;

			//static constexpr int cluserOffsetBufferIndex = depthMapBufferIndex + 1;
			//static constexpr int clusterLinkBufferIndex = cluserOffsetBufferIndex + 1; 	  
			//static constexpr int includeClusterRootSlotCount = clusterLinkBufferIndex + 1;

			//static constexpr int textureNormalMapIndex = textureShadowMapCubeBuffIndex + 1;
			static constexpr int samplerCount = Forward::samplerCount;
			static const std::vector<CD3DX12_STATIC_SAMPLER_DESC> Sampler()
			{
				return Forward::Sampler();
			}
		}
	}
	namespace Velocity
	{
		static constexpr int passCBIndex = 0;
		static constexpr int depthMapIndex = passCBIndex + 1;
		static constexpr int velocityMapIndex = depthMapIndex + 1;
		static constexpr int rootSlotCount = velocityMapIndex + 1;

		static JVector3<uint> GetThreadDim()
		{
			return JVector3<uint>(16, 16, 1);
		}
	}

	JDx12SceneDraw::ResourceDataSet::ResourceDataSet(JDx12CommandContext* context, const JDrawHelper& helper)
	{
		gRInterface = helper.cam->GraphicResourceUserInterface();
		auto cInterface = helper.GetCullInterface();

		sceneCBIndex = helper.option.rendering.allowDeferred ? Deferred::Geometry::sceneCBIndex : Forward::sceneCBIndex;
		camCBIndex = helper.option.rendering.allowDeferred ? Deferred::Geometry::camCBIndex : Forward::camCBIndex;

		camFrameIndex = helper.GetCamFrameIndex(CameraFrameLayer::drawScene);
		sceneFrameIndex = helper.GetSceneFrameIndex();

		rtSet = context->ComputeSet(gRInterface, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		dsSet = context->ComputeSet(gRInterface, J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);

		if (helper.option.rendering.allowDeferred)
		{
			gBufferSet[Constants::gBufferAlbedoLayer] = context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP);
			gBufferSet[Constants::gBufferLightPropertyLayer] = context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY);
			gBufferSet[Constants::gBufferNormalAndTangentLayer] = context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP);
			//gBufferSet[Constants::gBufferVelocityLayer] = context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY);
		}

		if (helper.allowTemporalProcess)
		{
			velocitySet = context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY);
			preRsSet = context->ComputeSet(gRInterface, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::RAYTRACING_GI);
			preDsSet = context->ComputeSet(gRInterface, J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::RAYTRACING_GI);
			preLightPropSet = context->ComputeSet(preRsSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY);
			preNormalSet = context->ComputeSet(preRsSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP);
			preVelocitySet = context->ComputeSet(preRsSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY);
		}

		aoSet = context->ComputeSet(gRInterface, J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP, J_GRAPHIC_TASK_TYPE::APPLY_SSAO);

		canUseAo = aoSet.IsValid();
		canUseLightCulling = helper.cam->AllowLightCulling() && helper.option.culling.allowLightCluster;
		canUseLightCluster = canUseLightCulling && helper.option.culling.allowLightCluster;
		canUseGi = helper.option.rendering.allowRaytracing && helper.cam->AllowRaytracingGI();
	}
	void JDx12SceneDraw::ResourceDataSet::SettingCluster(JDx12CommandContext* context, const JDrawHelper& helper)
	{
		clusterOffsetSet = context->ComputeSet(gRInterface, J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET, J_GRAPHIC_TASK_TYPE::LIGHT_CULLING);
		clusterListSet = context->ComputeSet(gRInterface, J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST, J_GRAPHIC_TASK_TYPE::LIGHT_CULLING);
	}
	void JDx12SceneDraw::ResourceDataSet::SettingGi(JDx12CommandContext* context, const JDrawHelper& helper)
	{
		giColorSet = context->ComputeSet(gRInterface, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::RAYTRACING_GI);
	}
	void JDx12SceneDraw::ResourceDataSet::SettingDebugging(JDx12CommandContext* context, const JDrawHelper& helper)
	{
		auto gRInterface = helper.cam->GraphicResourceUserInterface();
		debugSet = context->ComputeSet(gRInterface, J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
	}
	JDx12SceneDraw::INNER_DEFERRED_SHADER_TYPE JDx12SceneDraw::ResourceDataSet::GetDeferredType()const noexcept
	{
		INNER_DEFERRED_SHADER_TYPE type = INNER_DEFERRED_SHADER_EXCEPT_ALL;
		if (canUseLightCluster)
			type = Core::AddSQValueEnum(type, INNER_DEFERRED_SHADER_LIGHT_CULLING);
		if (canUseGi)
			type = Core::AddSQValueEnum(type, INNER_DEFERRED_SHADER_GI);
		return type;
	}
	bool JDx12SceneDraw::ResourceDataSet::IsValid()const noexcept
	{
		return rtSet.IsValid() && dsSet.IsValid();
	}

	JDx12SceneDraw::~JDx12SceneDraw()
	{
		ClearResource();
	}
	void JDx12SceneDraw::Initialize(JGraphicDevice* device, JGraphicResourceManager* gm)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gm))
			return;

		BuildResource(device, gm);
	}
	void JDx12SceneDraw::Clear()
	{
		ClearResource();
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
	bool JDx12SceneDraw::HasDependency(const JGraphicInfo::TYPE type)const noexcept
	{
		if (type == JGraphicInfo::TYPE::RESOURCE)
			return true;
		else
			return false;
	}
	bool JDx12SceneDraw::HasDependency(const JGraphicOption::TYPE type)const noexcept
	{
		if (type == JGraphicOption::TYPE::CULLING || type == JGraphicOption::TYPE::RENDERING ||
			type == JGraphicOption::TYPE::SHAODW || type == JGraphicOption::TYPE::POST_PROCESS ||
			type == JGraphicOption::TYPE::DEBUGGING)
			return true;
		else
			return false;
	}
	void JDx12SceneDraw::NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set)
	{
		RecompileShader(JGraphicShaderCompileSet(static_cast<const JDx12GraphicInfoChangedSet&>(set).device));
	}
	void JDx12SceneDraw::NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)
	{
		if (set.changedPart == JGraphicOption::TYPE::RENDERING)
			RecompileShader(JGraphicShaderCompileSet(static_cast<const JDx12GraphicOptionChangedSet&>(set).device));
		if (set.changedPart == JGraphicOption::TYPE::CULLING && set.newOption.culling.LightCullingDependencyChanged(set.preOption))
			RecompileShader(JGraphicShaderCompileSet(static_cast<const JDx12GraphicOptionChangedSet&>(set).device));
		if (set.changedPart == JGraphicOption::TYPE::SHAODW)
			RecompileShader(JGraphicShaderCompileSet(static_cast<const JDx12GraphicOptionChangedSet&>(set).device));
		if (set.changedPart == JGraphicOption::TYPE::POST_PROCESS && set.newOption.postProcess.useSsao != set.preOption.postProcess.useSsao)
			RecompileShader(JGraphicShaderCompileSet(static_cast<const JDx12GraphicOptionChangedSet&>(set).device));
		if (set.changedPart == JGraphicOption::TYPE::DEBUGGING && set.newOption.debugging.requestRecompileGraphicShader)
			RecompileShader(JGraphicShaderCompileSet(static_cast<const JDx12GraphicOptionChangedSet&>(set).device));
	}
	void JDx12SceneDraw::BindResource(const J_GRAPHIC_RENDERING_PROCESS process, const JGraphicBindSet* bindSet)
	{
		if (!IsSameDevice(bindSet))
			return;

		const JDx12GraphicBindSet* dx12BindSet = static_cast<const JDx12GraphicBindSet*>(bindSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12BindSet->context);
		if (process == J_GRAPHIC_RENDERING_PROCESS::DEFERRED_GEOMETRY)
			BindDeferredGeometryRootAndResource(context);
		else if (process == J_GRAPHIC_RENDERING_PROCESS::DEFERRED_SHADING)
			BindDeferredShadingRootAndResource(context);
		else
			BindForwardRootAndResource(context);
	}
	void JDx12SceneDraw::BindForwardRootAndResource(JDx12CommandContext* context)
	{
		context->SetGraphicsRootSignature(forwardRootSignature.Get());
		context->SetGraphicsRootShaderResourceView(Forward::dLitBuffIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT);
		context->SetGraphicsRootShaderResourceView(Forward::pLitBuffIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT);
		context->SetGraphicsRootShaderResourceView(Forward::sLitBuffIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT);
		context->SetGraphicsRootShaderResourceView(Forward::rLitBuffIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT);
		context->SetGraphicsRootShaderResourceView(Forward::csmBuffIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::CASCADE_SHADOW_MAP_INFO);
		context->SetGraphicsRootShaderResourceView(Forward::matBuffIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL);

		context->SetGraphicsRootDescriptorTable(Forward::texture2DBuffIndex, J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D);
		context->SetGraphicsRootDescriptorTable(Forward::textureCubeBuffIndex, J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE);
		context->SetGraphicsRootDescriptorTable(Forward::textureShadowMapBuffIndex, J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP);
		context->SetGraphicsRootDescriptorTable(Forward::textureShadowMapArrayBuffIndex, J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY);
		context->SetGraphicsRootDescriptorTable(Forward::textureShadowMapCubeBuffIndex, J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE);

		context->SetGraphicsRootDescriptorTable(Forward::ambientOcclusionMapIndex, J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE);
	}
	void JDx12SceneDraw::BindDeferredGeometryRootAndResource(JDx12CommandContext* context)
	{
		using namespace Deferred;
		context->SetGraphicsRootSignature(deferredGeometryRootSignature.Get());
		context->SetGraphicsRootShaderResourceView(Geometry::matBuffIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL);

		context->SetGraphicsRootDescriptorTable(Geometry::texture2DBuffIndex, J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D);
		context->SetGraphicsRootDescriptorTable(Geometry::textureCubeBuffIndex, J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE);
	}
	void JDx12SceneDraw::BindDeferredShadingRootAndResource(JDx12CommandContext* context)
	{
		using namespace Deferred;
		context->SetGraphicsRootSignature(deferredShadingRootSignature.Get());
		context->SetGraphicsRootShaderResourceView(Shading::dLitBuffIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT);
		context->SetGraphicsRootShaderResourceView(Shading::pLitBuffIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT);
		context->SetGraphicsRootShaderResourceView(Shading::sLitBuffIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT);
		context->SetGraphicsRootShaderResourceView(Shading::rLitBuffIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT);
		context->SetGraphicsRootShaderResourceView(Shading::csmBuffIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::CASCADE_SHADOW_MAP_INFO);

		context->SetGraphicsRootDescriptorTable(Shading::texture2DBuffIndex, J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D);
		context->SetGraphicsRootDescriptorTable(Shading::textureCubeBuffIndex, J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE);
		context->SetGraphicsRootDescriptorTable(Shading::textureShadowMapBuffIndex, J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP);
		context->SetGraphicsRootDescriptorTable(Shading::textureShadowMapArrayBuffIndex, J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY);
		context->SetGraphicsRootDescriptorTable(Shading::textureShadowMapCubeBuffIndex, J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE);
	}
	void JDx12SceneDraw::ClearResource(JDx12CommandContext* context, ResourceDataSet& rSet, const JDrawHelper& helper)
	{
		context->ClearDepthStencilView(rSet.dsSet);
		context->ClearRenderTargetView(rSet.rtSet, Constants::GetBackBufferClearColor());
		if (helper.option.rendering.allowDeferred)
		{
			context->ClearRenderTargetView(rSet.gBufferSet[Constants::gBufferAlbedoLayer], Constants::GetBackBufferClearColor());
			context->Apply(&rSet.gBufferSet[Constants::gBufferAlbedoLayer + 1], &JDx12CommandContext::ClearRenderTargetView, std::make_index_sequence<Constants::gBufferLayerCount - 1>(), Constants::GetBlackClearColor());
		}
	}
	void JDx12SceneDraw::SwitchResourceStateForDrawing(JDx12CommandContext* context, ResourceDataSet& rSet, const JDrawHelper& helper)
	{
		context->Transition(rSet.dsSet.holder, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		context->Transition(rSet.rtSet.holder, D3D12_RESOURCE_STATE_RENDER_TARGET);
		if (helper.option.rendering.allowDeferred)
			context->Transition(rSet.gBufferSet, D3D12_RESOURCE_STATE_RENDER_TARGET, false, std::make_index_sequence<Constants::gBufferLayerCount>());
		if (rSet.canUseLightCluster)
		{
			context->Transition(rSet.clusterOffsetSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			context->Transition(rSet.clusterListSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}
		context->FlushResourceBarriers();
	}
	void JDx12SceneDraw::SwitchResourceStateForDeferredShade(JDx12CommandContext* context, ResourceDataSet& rSet, const JDrawHelper& helper)
	{
		using namespace Deferred;
		context->Transition(rSet.dsSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		context->Transition(rSet.gBufferSet, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, false, std::make_index_sequence<Constants::gBufferLayerCount>());
		if (rSet.canUseLightCluster)
		{
			context->Transition(rSet.clusterOffsetSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			context->Transition(rSet.clusterListSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}
		context->FlushResourceBarriers();
	}
	void JDx12SceneDraw::ReturnResourceState(JDx12CommandContext* context, const ResourceDataSet& rSet, const JDrawHelper& helper)
	{
		//Deferred는 shading step이전에 depth map에서 값을 읽기위해 
		//D3D12_RESOURCE_STATE_RENDER_TARGET->D3D12_RESOURCE_STATE_GENERIC_READ로 상태를 변경한다.
		//D3D12_RESOURCE_STATE_RENDER_TARGET는 scene draw중에만 필요하므로 SettingResourceAfterDrawing 호출 이전에
		//scene draw가 완료되었으면 미리 상태변경해도 상관없다.
		// 
		if (!helper.option.rendering.allowDeferred)
			context->Transition(rSet.dsSet.holder, D3D12_RESOURCE_STATE_DEPTH_READ);
		context->Transition(rSet.rtSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
	}
	void JDx12SceneDraw::BindRenderTarget(JDx12CommandContext* context, const ResourceDataSet& rSet, const JDrawHelper& helper, const bool bindDs)
	{
		if (bindDs)
			context->SetRenderTargetView(rSet.rtSet, rSet.dsSet);
		else
			context->SetRenderTargetView(rSet.rtSet);
	}
	void JDx12SceneDraw::BindGBufferByRenderTarget(JDx12CommandContext* context, const ResourceDataSet& rSet, const JDrawHelper& helper)
	{
		using namespace Deferred;
		context->SetRenderTargetView(rSet.gBufferSet[0], rSet.dsSet, layerCount, true);
	}
	void JDx12SceneDraw::BindCommonCB(JDx12CommandContext* context, const ResourceDataSet& rSet, const JDrawHelper& helper)
	{
		context->SetGraphicsRootConstantBufferView(rSet.sceneCBIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::SCENE_PASS, rSet.sceneFrameIndex);
		context->SetGraphicsRootConstantBufferView(rSet.camCBIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA, rSet.camFrameIndex);
	}
	void JDx12SceneDraw::BindAoResource(JDx12CommandContext* context, const ResourceDataSet& rSet, const JDrawHelper& helper)
	{
		if (helper.option.rendering.allowDeferred)
			context->SetGraphicsRootDescriptorTable(Deferred::Shading::ambientOcclusionMapIndex, rSet.aoSet.GetGpuSrvHandle());
		else
			context->SetGraphicsRootDescriptorTable(Forward::ambientOcclusionMapIndex, rSet.aoSet.GetGpuSrvHandle());
	}
	void JDx12SceneDraw::BindLightListResource(JDx12CommandContext* context, const ResourceDataSet& rSet, const JDrawHelper& helper)
	{
		if (helper.option.rendering.allowDeferred)
		{
			using namespace Deferred;
			context->SetGraphicsRootDescriptorTable(Shading::depthMapBufferIndex, rSet.dsSet.GetGpuSrvHandle());
			context->SetGraphicsRootDescriptorTable(Shading::cluserOffsetBufferIndex, rSet.clusterOffsetSet.GetGpuSrvHandle());
			context->SetGraphicsRootDescriptorTable(Shading::clusterLinkBufferIndex, rSet.clusterListSet.GetGpuSrvHandle());
		}
		else
		{
			//context->SetGraphicsRootDescriptorTable(Forward::depthMapBufferIndex, rSet.dsSet.GetGpuSrvHandle());
			context->SetGraphicsRootDescriptorTable(Forward::cluserOffsetBufferIndex, rSet.clusterOffsetSet.GetGpuSrvHandle());
			context->SetGraphicsRootDescriptorTable(Forward::clusterLinkBufferIndex, rSet.clusterListSet.GetGpuSrvHandle());
		}
	}
	void JDx12SceneDraw::BindGiResource(JDx12CommandContext* context, const ResourceDataSet& rSet, const JDrawHelper& helper)
	{
		if (helper.option.rendering.allowDeferred)
		{
			using namespace Deferred;
			context->SetGraphicsRootDescriptorTable(Shading::giBufferIndex, rSet.giColorSet.GetGpuSrvHandle());
		}
	}
	void JDx12SceneDraw::BindGBufferResource(JDx12CommandContext* context, const ResourceDataSet& rSet, const JDrawHelper& helper)
	{
		using namespace Deferred;
		context->SetGraphicsRootDescriptorTable(Shading::gBufferIndex, rSet.gBufferSet[0].GetGpuSrvHandle());
		if (!rSet.canUseLightCluster)
			context->SetGraphicsRootDescriptorTable(Shading::depthMapBufferIndex, rSet.dsSet.GetGpuSrvHandle());
	}
	void JDx12SceneDraw::BindViewPortAndRect(JDx12CommandContext* context, const ResourceDataSet& rSet)
	{
		context->SetViewportAndRect(rSet.rtSet.info->GetResourceSize());
	}
	void JDx12SceneDraw::DrawSceneGameObject(JDx12CommandContext* context, const JDrawHelper& helper)
	{
		const std::vector<JUserPtr<JGameObject>>& objVec00 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::STATIC);
		const std::vector<JUserPtr<JGameObject>>& objVec01 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::SKINNED);
		const std::vector<JUserPtr<JGameObject>>& objVec02 = helper.GetGameObjectCashVec(J_RENDER_LAYER::DEBUG_OBJECT, Core::J_MESHGEOMETRY_TYPE::STATIC);
		const std::vector<JUserPtr<JGameObject>>& objVec03 = helper.GetGameObjectCashVec(J_RENDER_LAYER::SKY, Core::J_MESHGEOMETRY_TYPE::STATIC);

		DrawGameObject(context, objVec00, helper, JDrawCondition(helper, false, true, helper.allowDrawDebugObject), helper.option.rendering.allowDeferred);
		DrawGameObject(context, objVec01, helper, JDrawCondition(helper, helper.scene->IsActivatedSceneTime(), true, helper.allowDrawDebugObject), helper.option.rendering.allowDeferred);
		if (helper.allowDrawDebugObject)
			DrawGameObject(context, objVec02, helper, JDrawCondition(), helper.option.rendering.allowDeferred);
		DrawGameObject(context, objVec03, helper, JDrawCondition(), helper.option.rendering.allowDeferred);
	}
	void JDx12SceneDraw::DrawFullScreenGeometry(JDx12CommandContext* context, const ResourceDataSet& rSet, const JDrawHelper& helper)
	{
		context->SetPipelineState(deferredShadingHolder[rSet.GetDeferredType()].Get());
		if constexpr (Private::useFullscreenQuad)
		{
			JUserPtr<JMeshGeometry> quad = _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::FULL_SCREEN_QUAD);
			context->SetMeshGeometryData(quad);
			context->DrawIndexedInstanced(quad);
		}
		else
			context->DrawFullScreenTriangle();
	}
	void JDx12SceneDraw::CopyPreTemporalResource(JDx12CommandContext* context, ResourceDataSet& rSet, const JDrawHelper& helper)
	{
		if (helper.allowTemporalProcess)
		{
			//Velocity는 Restir이외에 pass에서 추가적으로 사용시 Scene에서 Compute
			//지금은 Restir pass에서 픽셀별로 계산
			static constexpr uint copyTarget = 3;
			JDx12GraphicResourceComputeSet* fromSet[copyTarget]
			{
				&rSet.gBufferSet[Constants::gBufferLightPropertyLayer],
				&rSet.gBufferSet[Constants::gBufferNormalAndTangentLayer],
				//&rSet.velocitySet,
				&rSet.dsSet
			};
			JDx12GraphicResourceComputeSet* toSet[copyTarget]
			{
				&rSet.preLightPropSet,
				&rSet.preNormalSet,
				//&rSet.preVelocitySet,
				&rSet.preDsSet
			};
			context->CopyResource(fromSet, toSet, std::make_index_sequence<copyTarget>());
		}
	}
	void JDx12SceneDraw::ComputeVelocity(JDx12CommandContext* context, const ResourceDataSet& set, const JDrawHelper& helper)
	{
		context->Transition(set.dsSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context->Transition(set.velocitySet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		context->FlushResourceBarriers();

		context->SetComputeRootSignature(velocityRootsignature.Get());
		context->SetPipelineState(velocityShader.get());

		context->SetComputeRootConstantBufferView(Velocity::passCBIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA, set.camFrameIndex);
		context->SetComputeRootDescriptorTable(Velocity::depthMapIndex, set.dsSet.GetGpuSrvHandle());
		context->SetComputeRootDescriptorTable(Velocity::velocityMapIndex, set.velocitySet.GetGpuUavHandle());

		context->Dispatch2D(set.rtSet.info->GetResourceSize(), velocityShader->dispatchInfo.threadDim.XY());
	}
	void JDx12SceneDraw::BeginDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(bindSet))
			return;

		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(bindSet->context);
		ResourceDataSet rSet(context, helper);
		if (!rSet.IsValid())
			return;

		CopyPreTemporalResource(context, rSet, helper);
		SwitchResourceStateForDrawing(context, rSet, helper);
		ClearResource(context, rSet, helper);
	}
	void JDx12SceneDraw::EndDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(bindSet))
			return;

		const JDx12GraphicBindSet* dx12BindSet = static_cast<const JDx12GraphicBindSet*>(bindSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12BindSet->context);
		ResourceDataSet rSet(context, helper);
		if (!rSet.IsValid())
			return;

		ReturnResourceState(context, rSet, helper);
	}
	void JDx12SceneDraw::DrawSceneRenderTarget(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(drawSet))
			return;

		const JDx12GraphicSceneDrawSet* dx12DrawSet = static_cast<const JDx12GraphicSceneDrawSet*>(drawSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12DrawSet->context);
		ResourceDataSet rSet(context, helper);
		if (!rSet.IsValid())
			return;

		rSet.canUseAo &= !helper.option.rendering.allowDeferred;
		rSet.canUseLightCluster &= !helper.option.rendering.allowDeferred;
		if (rSet.canUseLightCluster)
			rSet.SettingCluster(context, helper);

		CopyPreTemporalResource(context, rSet, helper);
		SwitchResourceStateForDrawing(context, rSet, helper);
		ClearResource(context, rSet, helper);

		if (helper.option.rendering.allowDeferred)
			BindGBufferByRenderTarget(context, rSet, helper);
		else
			BindRenderTarget(context, rSet, helper, true);
		BindCommonCB(context, rSet, helper);
		BindViewPortAndRect(context, rSet);
		if (rSet.canUseAo)
			BindAoResource(context, rSet, helper);
		if (rSet.canUseLightCluster)
			BindLightListResource(context, rSet, helper);
		DrawSceneGameObject(context, helper);

		//deferred일 경우 ReturnResourceState 호출에 대한 책임을 DrawSceneShade에게 넘긴다.
		if (!helper.option.rendering.allowDeferred)
			ReturnResourceState(context, rSet, helper);
	}
	void JDx12SceneDraw::DrawSceneRenderTargetMultiThread(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(drawSet))
			return;

		const JDx12GraphicSceneDrawSet* dx12DrawSet = static_cast<const JDx12GraphicSceneDrawSet*>(drawSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12DrawSet->context);
		ResourceDataSet rSet(context, helper);
		if (!rSet.IsValid())
			return;

		rSet.canUseAo &= !helper.option.rendering.allowDeferred;
		rSet.canUseLightCluster &= !helper.option.rendering.allowDeferred;
		if (rSet.canUseLightCluster)
			rSet.SettingCluster(context, helper);

		if (helper.option.rendering.allowDeferred)
			BindGBufferByRenderTarget(context, rSet, helper);
		else
			BindRenderTarget(context, rSet, helper, true);
		BindCommonCB(context, rSet, helper);
		if (rSet.canUseAo)
			BindAoResource(context, rSet, helper);
		if (rSet.canUseLightCluster)
			BindLightListResource(context, rSet, helper);
		BindViewPortAndRect(context, rSet);
		DrawSceneGameObject(context, helper);
	}
	void JDx12SceneDraw::DrawSceneDebugUI(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(drawSet) || !helper.allowDrawDebugObject)
			return;

		const JDx12GraphicSceneDrawSet* dx12DrawSet = static_cast<const JDx12GraphicSceneDrawSet*>(drawSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12DrawSet->context);
		ResourceDataSet rSet(context, helper);
		if (!rSet.IsValid())
			return;

		rSet.SettingDebugging(context, helper);

		context->Transition(rSet.rtSet.holder, D3D12_RESOURCE_STATE_RENDER_TARGET);
		context->Transition(rSet.debugSet.holder, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
		context->ClearDepthStencilView(rSet.debugSet);
		context->SetRenderTargetView(rSet.rtSet, rSet.debugSet);

		BindCommonCB(context, rSet, helper);
		BindViewPortAndRect(context, rSet);

		const std::vector<JUserPtr<JGameObject>>& objVec = helper.GetGameObjectCashVec(J_RENDER_LAYER::DEBUG_UI, Core::J_MESHGEOMETRY_TYPE::STATIC);
		DrawGameObject(context, objVec, helper, JDrawCondition(), false);

		//context->Transition(rSet.debugSet.holder, D3D12_RESOURCE_STATE_DEPTH_READ);
		context->Transition(rSet.rtSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
	}
	void JDx12SceneDraw::DrawSceneDebugUIMultiThread(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(drawSet) || !helper.allowDrawDebugObject)
			return;

		const JDx12GraphicSceneDrawSet* dx12DrawSet = static_cast<const JDx12GraphicSceneDrawSet*>(drawSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12DrawSet->context);
		ResourceDataSet rSet(context, helper);
		if (!rSet.IsValid())
			return;

		rSet.SettingDebugging(context, helper);

		context->Transition(rSet.debugSet.holder, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
		context->ClearDepthStencilView(rSet.debugSet);
		context->SetRenderTargetView(rSet.rtSet, rSet.debugSet);

		BindCommonCB(context, rSet, helper);
		BindViewPortAndRect(context, rSet);

		const std::vector<JUserPtr<JGameObject>>& objVec = helper.GetGameObjectCashVec(J_RENDER_LAYER::DEBUG_UI, Core::J_MESHGEOMETRY_TYPE::STATIC);
		DrawGameObject(context, objVec, helper, JDrawCondition(), false);

		//context->Transition(rSet.debugSet.holder, D3D12_RESOURCE_STATE_DEPTH_READ);
	}
	void JDx12SceneDraw::DrawSceneShade(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(drawSet))
			return;

		const JDx12GraphicSceneDrawSet* dx12DrawSet = static_cast<const JDx12GraphicSceneDrawSet*>(drawSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12DrawSet->context);
		ResourceDataSet rSet(context, helper);
		if (!rSet.IsValid())
			return;

		if (rSet.canUseLightCluster)
			rSet.SettingCluster(context, helper);
		if (rSet.canUseGi)
			rSet.SettingGi(context, helper);

		SwitchResourceStateForDeferredShade(context, rSet, helper);
		BindRenderTarget(context, rSet, helper, false);
		BindCommonCB(context, rSet, helper);
		if (rSet.canUseAo)
			BindAoResource(context, rSet, helper);
		BindGBufferResource(context, rSet, helper);
		if (rSet.canUseLightCluster)
			BindLightListResource(context, rSet, helper);
		if (rSet.canUseGi)
			BindGiResource(context, rSet, helper);
		BindViewPortAndRect(context, rSet);
		DrawFullScreenGeometry(context, rSet, helper);
		ReturnResourceState(context, rSet, helper);
	}
	void JDx12SceneDraw::DrawSceneShadeMultiThread(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(drawSet))
			return;

		const JDx12GraphicSceneDrawSet* dx12DrawSet = static_cast<const JDx12GraphicSceneDrawSet*>(drawSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12DrawSet->context);
		ResourceDataSet rSet(context, helper);
		if (!rSet.IsValid())
			return;

		if (rSet.canUseLightCluster)
			rSet.SettingCluster(context, helper);
		if (rSet.canUseGi)
			rSet.SettingGi(context, helper);

		SwitchResourceStateForDeferredShade(context, rSet, helper);
		BindRenderTarget(context, rSet, helper, false);
		BindCommonCB(context, rSet, helper);
		if (rSet.canUseAo)
			BindAoResource(context, rSet, helper);
		BindGBufferResource(context, rSet, helper);
		if (rSet.canUseLightCluster)
			BindLightListResource(context, rSet, helper);
		if (rSet.canUseGi)
			BindGiResource(context, rSet, helper);
		BindViewPortAndRect(context, rSet);
		DrawFullScreenGeometry(context, rSet, helper);
	}
	void JDx12SceneDraw::ComputeSceneDependencyTemporalResource(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(drawSet) || !helper.allowTemporalProcess)
			return;

		const JDx12GraphicSceneDrawSet* dx12DrawSet = static_cast<const JDx12GraphicSceneDrawSet*>(drawSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12DrawSet->context);
		ResourceDataSet rSet(context, helper);
		if (!rSet.IsValid() || !rSet.velocitySet.IsValid())
			return;

		ComputeVelocity(context, rSet, helper);
	}
	void JDx12SceneDraw::DrawGameObject(JDx12CommandContext* context,
		const std::vector<JUserPtr<JGameObject>>& gameObject,
		const JDrawHelper& helper,
		const JDrawCondition& condition,
		const bool isDeferred)
	{
		using GetShaderDataPtr = JShaderDataHolder * (*)(const JShader*, const J_GRAPHIC_SHADER_TYPE type, const J_GRAPHIC_SHADER_VERTEX_LAYOUT);
		auto getForwardShaderDataLam = [](const JShader* shader, const J_GRAPHIC_SHADER_TYPE type, const J_GRAPHIC_SHADER_VERTEX_LAYOUT layout) {return shader->GetGraphicForwardData(type, layout).Get(); };
		auto getDeferredShaderDataLam = [](const JShader* shader, const J_GRAPHIC_SHADER_TYPE type, const J_GRAPHIC_SHADER_VERTEX_LAYOUT layout) {return shader->GetGraphicDeferredData(type, layout).Get(); };
		GetShaderDataPtr getShader = nullptr;
		if (isDeferred)
			getShader = getDeferredShaderDataLam;
		else
			getShader = getForwardShaderDataLam;

		uint objCBIndex = isDeferred ? Deferred::Geometry::objCBIndex : Forward::objCBIndex;
		uint skinCBIndex = isDeferred ? Deferred::Geometry::skinCBIndex : Forward::skinCBIndex;

		const J_SCENE_USE_CASE_TYPE useCase = helper.scene->GetUseCaseType();
		const J_GRAPHIC_SHADER_TYPE shaderType = useCase == J_SCENE_USE_CASE_TYPE::MAIN ?
			J_GRAPHIC_SHADER_TYPE::STANDARD : J_GRAPHIC_SHADER_TYPE::PREVIEW;

		auto cullUser = helper.GetCullInterface();

		uint st, ed = 0;
		helper.DispatchWorkIndex((uint)gameObject.size(), st, ed);
		for (uint i = st; i < ed; ++i)
		{
			JUserPtr<JRenderItem> renderItem = gameObject[i]->GetRenderItem();
			const uint objFrameIndex = helper.GetObjectFrameIndex(renderItem.Get());
			const uint boundFrameIndex = helper.GetBoundingFrameIndex(renderItem.Get());

			if (condition.allowCulling && !renderItem->IsIgnoreCullingResult() && cullUser.IsCulled(J_CULLING_TARGET::RENDERITEM, boundFrameIndex))
				continue;

			if (condition.allowAllCullingResult && helper.RefelectOtherCamCullig(boundFrameIndex))
				continue;

			JUserPtr<JMeshGeometry> mesh = renderItem->GetMesh();
			JUserPtr<JAnimator> animator = gameObject[i]->GetComponentWithParent<JAnimator>();

			const bool onSkinned = animator != nullptr && condition.allowAnimation;
			const Core::J_MESHGEOMETRY_TYPE meshType = onSkinned ? Core::J_MESHGEOMETRY_TYPE::SKINNED : Core::J_MESHGEOMETRY_TYPE::STATIC;
			const J_GRAPHIC_SHADER_VERTEX_LAYOUT shaderLayout = JShaderType::ConvertToVertexLayout(meshType);

			context->SetMeshGeometryData(renderItem);
			if (condition.allowOutline && gameObject[i]->IsSelected())
				context->SetStencilRef(Constants::outlineStencilRef);
			if (meshType == Core::J_MESHGEOMETRY_TYPE::SKINNED)
				context->SetGraphicsRootConstantBufferView(skinCBIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION, helper.GetAnimationFrameIndex(animator.Get()));

			const uint submeshCount = (uint)mesh->GetTotalSubmeshCount();

			for (uint j = 0; j < submeshCount; ++j)
			{
				const JShader* shader = renderItem->GetValidMaterial(j)->GetShader().Get();
				JDx12GraphicShaderDataHolder* dx12ShaderData = static_cast<JDx12GraphicShaderDataHolder*>((getShader)(shader, shaderType, shaderLayout));
				if (condition.allowOutline && gameObject[i]->IsSelected())
					context->SetPipelineState(dx12ShaderData, (uint)J_GRAPHIC_SHADER_EXTRA_FUNCTION::STENCIL_WRITE_ALWAYS);
				else
					context->SetPipelineState(dx12ShaderData, 0);

				context->SetGraphicsRootConstantBufferView(objCBIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT, objFrameIndex + j);
				context->DrawIndexedInstanced(mesh, j);
			}
			if (condition.allowOutline && gameObject[i]->IsSelected())
				context->SetStencilRef(Constants::commonStencilRef);
		}
	}
	void JDx12SceneDraw::RecompileShader(const JGraphicShaderCompileSet& dataSet)
	{
		if (!IsSameDevice(dataSet.device))
			return;

		ReBuildRootSignature(static_cast<JDx12GraphicDevice*>(dataSet.device)->GetDevice(), GetGraphicInfo(), GetGraphicOption());
		BuildDeferredShader(dataSet);
		BuildVelocityShader(static_cast<JDx12GraphicDevice*>(dataSet.device)->GetDevice(), GetGraphicInfo(), GetGraphicOption());

		auto shaderVec = JShader::StaticTypeInfo().GetInstanceRawPtrVec();
		for (auto& data : shaderVec)
		{
			JShader* shader = static_cast<JShader*>(data);
			if (!shader->IsComputeShader())
				RecompileUserShader(shader);
		}
	}
	JOwnerPtr<JShaderDataHolder> JDx12SceneDraw::CreateShader(const JGraphicShaderCompileSet& dataSet, JGraphicShaderInitData initData)
	{
		if (!IsSameDevice(dataSet.device))
			return nullptr;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(dataSet.device);
		auto gOption = GetGraphicOption();
		auto gInfo = GetGraphicInfo();
		auto holder = Core::JPtrUtil::MakeOwnerPtr<JDx12GraphicShaderDataHolder>();

		PsoBuildData psoBuildData;
		psoBuildData.holder = holder.Get();
		psoBuildData.dx12Device = dx12Device;
		psoBuildData.shaderType = initData.shaderType;
		psoBuildData.gFunctionFlag = initData.gFunctionFlag;
		psoBuildData.processType = initData.processType;
		psoBuildData.condition = initData.condition;
		psoBuildData.deferredType = INNER_DEFERRED_SHADER_EXCEPT_ALL;
		if (Core::HasSQValueEnum(initData.privateFlag, SHADER_FUNCTION_PRIVATE_LIGHT_CULLING))
			psoBuildData.deferredType = Core::AddSQValueEnum(psoBuildData.deferredType, INNER_DEFERRED_SHADER_LIGHT_CULLING);
		if (Core::HasSQValueEnum(initData.privateFlag, SHADER_FUNCTION_PRIVATE_GLOBAL_ILLUMINATION))
			psoBuildData.deferredType = Core::AddSQValueEnum(psoBuildData.deferredType, INNER_DEFERRED_SHADER_GI);

		Private::StuffMacro(initData, gInfo, gOption);
		CompileShader(holder.Get(), initData);
		StuffInputLayout(holder->inputLayout, initData.layoutType);

		for (uint i = 0; i < (uint)J_GRAPHIC_SHADER_EXTRA_FUNCTION::COUNT; ++i)
		{
			psoBuildData.extraType = (J_GRAPHIC_SHADER_EXTRA_FUNCTION)i;
			StuffPso(psoBuildData, gOption);
		}

		return std::move(holder);
	}
	void JDx12SceneDraw::CompileShader(JDx12GraphicShaderDataHolder* holder, const JGraphicShaderInitData& initData)
	{
		std::wstring vertexShaderPath = JApplicationEngine::ShaderPath() + L"\\" + ShaderRelativePath::SceneRasterize(L"VertexShader.hlsl");
		std::wstring pixelShaderPath = JApplicationEngine::ShaderPath() + L"\\";
		if (initData.processType == J_GRAPHIC_RENDERING_PROCESS::DEFERRED_GEOMETRY)
			pixelShaderPath += ShaderRelativePath::SceneRasterize(L"PixelStuffGBuffer.hlsl");
		else if (initData.processType == J_GRAPHIC_RENDERING_PROCESS::DEFERRED_SHADING)
			pixelShaderPath += ShaderRelativePath::SceneRasterize(L"PixelDeferredShader.hlsl");
		else
			pixelShaderPath += ShaderRelativePath::SceneRasterize(L"PixelForwardShader.hlsl");

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
		if (data.processType == J_GRAPHIC_RENDERING_PROCESS::DEFERRED_SHADING && !Private::useFullscreenQuad)
			newShaderPso.InputLayout = { nullptr, 0 };
		else
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
		newShaderPso.RTVFormats[0] = Constants::GetRenderTargetFormat(option.rendering.renderTargetFormat);
		newShaderPso.SampleDesc.Count = data.dx12Device->GetM4xMsaaState() ? 4 : 1;
		newShaderPso.SampleDesc.Quality = data.dx12Device->GetM4xMsaaState() ? (data.dx12Device->GetM4xMsaaQuality() - 1) : 0;

		newShaderPso.DSVFormat = Constants::GetDepthStencilFormat();
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
			newShaderPso.RTVFormats[0] = Constants::GetGBufferFormat(Constants::gBufferAlbedoLayer);
			newShaderPso.RTVFormats[1] = Constants::GetGBufferFormat(Constants::gBufferLightPropertyLayer);
			newShaderPso.RTVFormats[2] = Constants::GetGBufferFormat(Constants::gBufferNormalAndTangentLayer);
			//newShaderPso.RTVFormats[3] = Constants::GetGBufferFormat(Constants::gBufferVelocityLayer);
			//newShaderPso.RTVFormats[4] = Constants::GetGBufferFormat(Constants::gBufferVelocityLayer);
		}

		if (data.condition.primitiveCondition == J_SHADER_APPLIY_CONDITION::APPLY)
			newShaderPso.PrimitiveTopologyType = Private::ConvertD3d12PrimitiveType(data.condition.primitiveType);
		if (data.condition.depthCompareCondition == J_SHADER_APPLIY_CONDITION::APPLY)
			newShaderPso.DepthStencilState.DepthFunc = Private::ConvertD3d12Comparesion(data.condition.depthCompareFunc);
		if (data.condition.cullModeCondition == J_SHADER_APPLIY_CONDITION::APPLY)
			newShaderPso.RasterizerState.CullMode = Private::ConvertD3d12CullMode(data.condition.isCullModeNone);

		if (option.rendering.useMSAA)
			newShaderPso.RasterizerState.MultisampleEnable = true;

		const uint psoIndex = (uint)data.extraType;

		ThrowIfFailedG(data.dx12Device->GetDevice()->CreateGraphicsPipelineState(&newShaderPso, IID_PPV_ARGS(data.holder->GetPsoAddress(psoIndex))));
		data.holder->GetPso(psoIndex)->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Pso") - 1, "Pso");
	}
	void JDx12SceneDraw::BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		const JGraphicOption& gOption = GetGraphicOption();
		const JGraphicInfo& gInfo = GetGraphicInfo();
		if (gOption.rendering.allowDeferred)
		{
			BuildForwardRootSignature(static_cast<JDx12GraphicDevice*>(device)->GetDevice(), gInfo, gOption);
			BuildDeferredGeometryRootSignature(static_cast<JDx12GraphicDevice*>(device)->GetDevice(), gInfo, gOption);
			BuildDeferredShadingRootSignature(static_cast<JDx12GraphicDevice*>(device)->GetDevice(), gInfo, gOption);
			BuildDeferredShader(JGraphicShaderCompileSet(device));
		}
		else
			BuildForwardRootSignature(static_cast<JDx12GraphicDevice*>(device)->GetDevice(), gInfo, gOption);

		BuildVelocityRootSignature(static_cast<JDx12GraphicDevice*>(device)->GetDevice(), gInfo, gOption);
		BuildVelocityShader(static_cast<JDx12GraphicDevice*>(device)->GetDevice(), gInfo, gOption);
	}
	void JDx12SceneDraw::BuildForwardRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		JDx12RootSignatureBuilder2<Forward::rootSlotCountIncCluster, Forward::samplerCount> builder;
		builder.PushConstantsBuffer(Forward::sceneCBIndex);
		builder.PushConstantsBuffer(Forward::camCBIndex);
		builder.PushConstantsBuffer(Forward::objCBIndex);
		builder.PushConstantsBuffer(Forward::skinCBIndex);

		builder.PushShaderResource(0, 0);		//dLitBuffIndex
		builder.PushShaderResource(0, 1);		//pLitBuffIndex
		builder.PushShaderResource(0, 2);		//sLitBuffIndex
		builder.PushShaderResource(0, 3);		//rLitBuffIndex
		builder.PushShaderResource(0, 4);		//csmBuffIndex
		builder.PushShaderResource(1);			//material

		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.resource.binding2DTextureCapacity, 2, 0);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.resource.bindingCubeMapCapacity, 2, 1);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.resource.bindingShadowTextureCapacity, 2, 2);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.resource.bindingShadowTextureArrayCapacity, 2, 3);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.resource.bindingShadowTextureCubeCapacity, 2, 4);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 5);			//ambientOcclusion 
		if (option.culling.allowLightCluster)
		{
			builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 7);		//depthMap
			builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 8);		//light cluster offset
			builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 9);		//light cluster link
		}
		const std::vector<CD3DX12_STATIC_SAMPLER_DESC> sam = Forward::Sampler();
		for (const auto& data : sam)
			builder.PushSampler(data);

		builder.Create(device, L"Main Forwawrd Geometry RootSignature", forwardRootSignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	}
	void JDx12SceneDraw::BuildDeferredGeometryRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		using namespace Deferred;
		JDx12RootSignatureBuilder2<Geometry::rootSlotCount, Geometry::samplerCount> builder;
		builder.PushConstantsBuffer(Geometry::sceneCBIndex);
		builder.PushConstantsBuffer(Geometry::camCBIndex);
		builder.PushConstantsBuffer(Geometry::objCBIndex);
		builder.PushConstantsBuffer(Geometry::skinCBIndex);

		builder.PushShaderResource(1);		//material

		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.resource.binding2DTextureCapacity, 2, 0);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.resource.bindingCubeMapCapacity, 2, 1);

		const std::vector<CD3DX12_STATIC_SAMPLER_DESC> sam = Geometry::Sampler();
		for (const auto& data : sam)
			builder.PushSampler(data);

		builder.Create(device, L"Main Deferred Geometry RootSignature", deferredGeometryRootSignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	}
	void JDx12SceneDraw::BuildDeferredShadingRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		using namespace Deferred;

		JDx12RootSignatureBuilder2<Shading::rootSlotCount, Shading::samplerCount> builder;
		builder.PushConstantsBuffer(Shading::sceneCBIndex);
		builder.PushConstantsBuffer(Shading::camCBIndex);

		builder.PushShaderResource(0, 0);		//dLitBuffIndex
		builder.PushShaderResource(0, 1);		//pLitBuffIndex
		builder.PushShaderResource(0, 2);		//sLitBuffIndex
		builder.PushShaderResource(0, 3);		//rLitBuffIndex
		builder.PushShaderResource(0, 4);		//csmBuffIndex

		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.resource.binding2DTextureCapacity, 2, 0);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.resource.bindingCubeMapCapacity, 2, 1);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.resource.bindingShadowTextureCapacity, 2, 2);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.resource.bindingShadowTextureArrayCapacity, 2, 3);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.resource.bindingShadowTextureCubeCapacity, 2, 4);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 5);			//ambientOcclusion 
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, layerCount, 2, 6);	//gBuffer
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 7);			//depth 

		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 8);		//light cluster offset
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 9);		//light cluster link
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 10);		// gI
		const std::vector<CD3DX12_STATIC_SAMPLER_DESC> sam = Shading::Sampler();
		for (const auto& data : sam)
			builder.PushSampler(data);

		D3D12_ROOT_SIGNATURE_FLAGS flag = Private::useFullscreenQuad ? D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT : D3D12_ROOT_SIGNATURE_FLAG_NONE;
		builder.Create(device, L"Main Deferred Shading RootSignature", deferredShadingRootSignature.GetAddressOf(), flag);
	}
	void JDx12SceneDraw::BuildVelocityRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		JDx12RootSignatureBuilder<Velocity::rootSlotCount> builder;
		builder.PushConstantsBuffer(Velocity::passCBIndex);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		builder.Create(device, L"Velocity RootSignature", velocityRootsignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);
	}
	void JDx12SceneDraw::ReBuildRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		if (option.rendering.allowDeferred)
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
			deferredGeometryRootSignature = nullptr;
			deferredShadingRootSignature = nullptr;

			forwardRootSignature = nullptr;
			BuildForwardRootSignature(device, info, option);
		}
		velocityRootsignature = nullptr;
		BuildVelocityRootSignature(device, info, option);
	}
	void JDx12SceneDraw::BuildDeferredShader(const JGraphicShaderCompileSet& dataSet)
	{
		for (uint i = 0; i < SIZE_OF_ARRAY(deferredShadingHolder); ++i)
			deferredShadingHolder[i] = nullptr;

		if (!GetGraphicOption().rendering.allowDeferred)
			return;

		JGraphicShaderInitData initData;
		initData.gFunctionFlag = SHADER_FUNCTION_NONE;		//flag에 따른 data(Material 종속)는 모두 geometry step에서 계산되고 필요하면 gBuffer에 저장된다.
		initData.layoutType = J_GRAPHIC_SHADER_VERTEX_LAYOUT::STATIC;
		initData.processType = J_GRAPHIC_RENDERING_PROCESS::DEFERRED_SHADING;
		initData.privateFlag = SHADER_FUNCTION_PRIVATE_NONE;

		auto holder = CreateShader(dataSet, initData);
		deferredShadingHolder[INNER_DEFERRED_SHADER_EXCEPT_ALL] = (static_cast<JDx12GraphicShaderDataHolder*>(holder.Release()));

		initData.privateFlag = SHADER_FUNCTION_PRIVATE_LIGHT_CULLING;
		holder = CreateShader(dataSet, initData);
		deferredShadingHolder[INNER_DEFERRED_SHADER_LIGHT_CULLING] = (static_cast<JDx12GraphicShaderDataHolder*>(holder.Release()));

		initData.privateFlag = SHADER_FUNCTION_PRIVATE_GLOBAL_ILLUMINATION;
		holder = CreateShader(dataSet, initData);
		deferredShadingHolder[INNER_DEFERRED_SHADER_GI] = (static_cast<JDx12GraphicShaderDataHolder*>(holder.Release()));

		initData.privateFlag = Core::AddSQValueEnum(SHADER_FUNCTION_PRIVATE_LIGHT_CULLING, SHADER_FUNCTION_PRIVATE_GLOBAL_ILLUMINATION);
		holder = CreateShader(dataSet, initData);
		deferredShadingHolder[INNER_DEFERRED_SHADER_INCLUDE_ALL] = (static_cast<JDx12GraphicShaderDataHolder*>(holder.Release()));
	}
	void JDx12SceneDraw::BuildVelocityShader(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		velocityShader = nullptr;
		velocityShader = std::make_unique<JDx12ComputeShaderDataHolder>();

		constexpr uint shaderCount = 1;
		JDx12ComputePsoBulder<shaderCount> psoBuilder("JDx12SceneDraw");

		psoBuilder.PushHolder(velocityShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::SceneRasterize(L"VelocityBuffer.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Velocity::GetThreadDim());
		psoBuilder.PushRootSignature(velocityRootsignature.Get());
		psoBuilder.Create(device);
	}
	void JDx12SceneDraw::ClearResource()
	{
		forwardRootSignature = deferredGeometryRootSignature = nullptr;
		deferredShadingRootSignature = nullptr;
		velocityRootsignature = nullptr;

		for (uint i = 0; i < SIZE_OF_ARRAY(deferredShadingHolder); ++i)
			deferredShadingHolder[i] = nullptr;
		velocityShader = nullptr;
	}
}