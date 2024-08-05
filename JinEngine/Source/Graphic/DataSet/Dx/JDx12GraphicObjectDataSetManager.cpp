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


#pragma once  
#include"JDx12GraphicObjectDataSetManager.h"

#include"../../../Object/Component/JComponentHint.h"
#include"../../../Object/Component/JComponent.h" 
#include"../../../Object/Component/Light/JLight.h" 
#include"../../../Object/Resource/JResourceObjectHint.h" 
#include"../../../Object/Resource/JResourceObject.h"  
#include"../../../Core/Utility/JTypeSequence.h"

namespace JinEngine::Graphic
{    
	namespace
	{
		using TYPE_PER_INDEX = JDx12GraphicObjectDataSetManager::TYPE_PER_INDEX;
		static constexpr TYPE_PER_INDEX ConvertT(const J_COMPONENT_TYPE type)
		{
			return (TYPE_PER_INDEX)type;
		}
		static constexpr TYPE_PER_INDEX ConvertT(const J_RESOURCE_TYPE type)
		{
			return (TYPE_PER_INDEX)((uint)type + (uint)TYPE_PER_INDEX::RESOURCE_BEGIN);
		}
		static constexpr uint ConvertU(const J_COMPONENT_TYPE type)
		{
			return (uint)type;
		}
		static constexpr uint ConvertU(const J_RESOURCE_TYPE type)
		{
			return (uint)((uint)type + (uint)TYPE_PER_INDEX::RESOURCE_BEGIN);
		}
		static constexpr J_COMPONENT_TYPE ConvertC(const TYPE_PER_INDEX type)
		{
			return (J_COMPONENT_TYPE)type;
		}
		static constexpr J_RESOURCE_TYPE ConvertR(const TYPE_PER_INDEX type)
		{
			return (J_RESOURCE_TYPE)((uint)type - (uint)TYPE_PER_INDEX::RESOURCE_BEGIN);
		}
		static constexpr bool IsComponentType(const TYPE_PER_INDEX type)
		{
			return type < TYPE_PER_INDEX::RESOURCE_BEGIN;
		}
		static constexpr bool IsResourceType(const TYPE_PER_INDEX type)
		{
			return type > TYPE_PER_INDEX::COMP_END;
		}
	}
	//Helper structure
	namespace
	{
		//Graphic resource interface define type helper class
		//Same as Core::JStaticTuple
		template<J_GRAPHIC_RESOURCE_TYPE _T, uint _U>
		struct JStaticTupleGraphicUint
		{
		public:
			using FirstType = J_GRAPHIC_RESOURCE_TYPE;
			using SecondType = uint;
		public:
			static constexpr FirstType first = _T;
			static constexpr SecondType second = _U;
		};
		 
		//Metadata creation helper
		//Set parameter automatically by interface property
		template<typename CullingInterface, typename FrameInterface, typename GpuAcceleratorInterface, typename GraphicResourceInterface, TYPE_PER_INDEX _typePerIndex>
		struct JTypePerInterfaceMetadata
		{
		private:
			template<typename T, typename = void>
			struct HasFrameUploadHint
			{
			public:
				static constexpr bool value = true;
			};
			template<typename T>
			struct HasFrameUploadHint<T, std::void_t<decltype(&T::isSupportedFrameUpload)>>
			{
			public:
				static constexpr bool value = T::isSupportedFrameUpload;
			};
			template<typename T, typename = void>
			struct HasFrameDirtyHint
			{
			public:
				static constexpr bool value = true;
			};
			template<typename T>
			struct HasFrameDirtyHint<T, std::void_t<decltype(&T::isSupportedFrameDirty)>>
			{
			public:
				static constexpr bool value = T::isSupportedFrameDirty;
			};
		public:
			//basicalliy
			static constexpr bool hasCullingInterface = !std::is_same_v<CullingInterface, Core::JEmptyType>;
			static constexpr bool hasFrameInterface = !std::is_same_v<FrameInterface, Core::JEmptyType>;
			static constexpr bool hasGpuAcceleratorInterface = !std::is_same_v<GpuAcceleratorInterface, Core::JEmptyType>;
			static constexpr bool hasGraphicResourceInterface = !std::is_same_v<GraphicResourceInterface, Core::JEmptyType>;
			static constexpr TYPE_PER_INDEX typePerIndex = _typePerIndex;
		public:
			//detail  
			static constexpr bool isSupportedFrameUpload = hasFrameInterface && HasFrameUploadHint<FrameInterface>::value;
			static constexpr bool isSupportedFrameDirty = hasFrameInterface && HasFrameDirtyHint<FrameInterface>::value;
		public:
			static void Register(JObjectDataSetMetadata& data)
			{
				if (IsComponentType(typePerIndex))
				{
					data.typeGuid = CTypeCommonCall::CallGetTypeInfo(ConvertC(typePerIndex)).TypeGuid();
					data.componentTypeValue = (uint)ConvertC(typePerIndex);
				}
				else
				{
					data.typeGuid = RTypeCommonCall::CallGetTypeInfo(ConvertR(typePerIndex)).TypeGuid();
					data.resourceTypeValue = (uint)ConvertR(typePerIndex);
				}
				 
				data.isSupportedCulling = hasCullingInterface;
				data.isSupportedGpuAccelerator = hasGpuAcceleratorInterface;
				data.isSupportedGraphicResource = hasGraphicResourceInterface;

				data.isSupportedFrameDirty = isSupportedFrameDirty;
				data.isSupportedFrameResourceUpload = isSupportedFrameUpload;
			}
		}; 
  
		//Set data manually
		template<bool _hasCullingInterface, bool _hasFrameInterface, bool _hasGpuAcceleratorInterface, bool _hasGraphicResourceInterface, TYPE_PER_INDEX _typePerIndex>
		struct JTypePerInterfaceMetadata2
		{
		public:
			//basicalliy
			static constexpr bool hasCullingInterface = _hasCullingInterface;
			static constexpr bool hasFrameInterface = _hasFrameInterface;
			static constexpr bool hasGpuAcceleratorInterface = _hasGpuAcceleratorInterface;
			static constexpr bool hasGraphicResourceInterface = _hasGraphicResourceInterface; 
			static constexpr TYPE_PER_INDEX typePerIndex = _typePerIndex;
		public:
			//detail
			static constexpr bool isSupportedFrameUpload = hasFrameInterface;
			static constexpr bool isSupportedFrameDirty= hasFrameInterface;
		public:
			static void Register(JObjectDataSetMetadata& data)
			{
				if (IsComponentType(typePerIndex))
				{
					data.typeGuid = CTypeCommonCall::CallGetTypeInfo(ConvertC(typePerIndex)).TypeGuid();
					data.componentTypeValue = (uint)ConvertC(typePerIndex);
				}
				else
				{
					data.typeGuid = RTypeCommonCall::CallGetTypeInfo(ConvertR(typePerIndex)).TypeGuid();
					data.resourceTypeValue = (uint)ConvertR(typePerIndex);
				}
				 
				data.isSupportedCulling = hasCullingInterface;
				data.isSupportedGpuAccelerator = hasGpuAcceleratorInterface;
				data.isSupportedGraphicResource = hasGraphicResourceInterface;

				data.isSupportedFrameDirty = isSupportedFrameDirty;
				data.isSupportedFrameResourceUpload = isSupportedFrameUpload;
			}
		};
	}
	//CompInterfaceType
	namespace
	{
		namespace Animator
		{
			using FrameUpload = Core::JDefinedTypeSequence< J_FRAME_RESOURCE_UPLOAD_TYPE,
				J_FRAME_RESOURCE_UPLOAD_TYPE::ANIMATION>;

			using FrameInterface = JFrameUpdateTypePerSingleHolder<JFrameDirtyTrigger, FrameUpload>;
		}
		namespace Behavior
		{
			//Behavior
			/*
			* Behavior의 파생클래스가 특정한 조건하에 설정가능하도록
			* Interface를 마련할 필요가 있다.
			*/
			//using BehaviorFrameInterface = JFrameUpdateTypePerSingleHolder<(Requested dirty type), (Custom type)>;
		}
		namespace Camera
		{
			static constexpr uint mainRtNumber = 0;
			static constexpr uint afterPostProcesseNumber = 1;
			static constexpr uint giRtNumber = 2;
			static constexpr uint litCullDebugRtNumber = 3;

			static constexpr uint mainDsNumber = 0;
			static constexpr uint previousDsNumber = 1;

			static constexpr uint depthDebuggingNumber = 0;
			static constexpr uint albedoDebuggingNumber = depthDebuggingNumber + 1;
			static constexpr uint specularDebuggingNumber = albedoDebuggingNumber + 1;
			static constexpr uint normalDebuggingNumber = specularDebuggingNumber + 1;
			static constexpr uint tangentDebuggingNumber = normalDebuggingNumber + 1;
			//static constexpr uint velocityDebuggingNumber = 4;
			static constexpr uint ssaoDebuggingNumber = tangentDebuggingNumber + 1;
			static constexpr uint debuggingMapCount = ssaoDebuggingNumber + 1;

			static constexpr uint rtCount = 4;
			static constexpr uint dsCount = 2;
			static constexpr uint debugCount = 6;
			static constexpr uint reserviorCount = 4;

			using FrameUpload = Core::JDefinedTypeSequence< J_FRAME_RESOURCE_UPLOAD_TYPE,
				J_FRAME_RESOURCE_UPLOAD_TYPE::CAMERA,
				J_FRAME_RESOURCE_UPLOAD_TYPE::DEPTH_TEST_PASS,
				J_FRAME_RESOURCE_UPLOAD_TYPE::HZB_OCC_COMPUTE_PASS,
				J_FRAME_RESOURCE_UPLOAD_TYPE::LIGHT_CULLING_PASS,
				J_FRAME_RESOURCE_UPLOAD_TYPE::SSAO_PASS,
				J_FRAME_RESOURCE_UPLOAD_TYPE::RAYTRACING_GI,
				J_FRAME_RESOURCE_UPLOAD_TYPE::RAYTRACING_SHADOW,
				J_FRAME_RESOURCE_UPLOAD_TYPE::RAYTRACING_DENOISE>;

			class CameraFrameDirty final : public JFrameDirty
			{
			public:
				int GetFrameDirtyMax()const noexcept final
				{
					//첫번째 frame에 update된 constants에  mPreViewProj이 유효한 값을 갖게 하기위해 +1
					//(1: delta), (2: zero), (3: zero), (1: zero)
					return Graphic::Constants::gNumFrameResources + 1;
				}
			};
			using FrameInterface = JFrameUpdateTypePerSingleHolder<CameraFrameDirty, FrameUpload>;
			 
			using GraphicResource = Core::JStaticTupleSequence<JStaticTupleGraphicUint<J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, dsCount>,
				JStaticTupleGraphicUint<J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL, 1>,
				JStaticTupleGraphicUint<J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP, debugCount>,
				JStaticTupleGraphicUint<J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, 1>,
				JStaticTupleGraphicUint<J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP, 1>,
				JStaticTupleGraphicUint<J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG, 1>,
				JStaticTupleGraphicUint<J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, rtCount>,
				JStaticTupleGraphicUint<J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST, 1>,
				JStaticTupleGraphicUint<J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET, 1>,
				JStaticTupleGraphicUint<J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP, 1>,
				JStaticTupleGraphicUint<J_GRAPHIC_RESOURCE_TYPE::IMAGE_PROCESSING, 1>,
				JStaticTupleGraphicUint<J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_LUMA, 1>,
				JStaticTupleGraphicUint<J_GRAPHIC_RESOURCE_TYPE::RESTIR_INITIAL_SAMPLE, 1>,
				JStaticTupleGraphicUint<J_GRAPHIC_RESOURCE_TYPE::RESTIR_RESERVOIR, reserviorCount>>;
			 
			class GraphicResourceInterface final: public JGraphicRestrictMultiResourceHolder<Camera::GraphicResource>
			{
			public:
				uint GetAllocableResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept final
				{
					switch (rType)
					{
					case J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL:
					{
						if (taskType == J_GRAPHIC_TASK_TYPE::RAYTRACING_GI || taskType == J_GRAPHIC_TASK_TYPE::STORE_PREVIOUS_FRAME_DATA)
							return 1;
						else
							return 1;
					}
					case J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL:
						return 1;
					case J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP:
					{
						if (taskType == J_GRAPHIC_TASK_TYPE::DEPTH_MAP_VISUALIZE)
							return 1;
						else if (taskType == J_GRAPHIC_TASK_TYPE::ALBEDO_MAP_VISUALIZE)
							return 1;
						else if (taskType == J_GRAPHIC_TASK_TYPE::SPECULAR_MAP_VISUALIZE)
							return 1;
						else if (taskType == J_GRAPHIC_TASK_TYPE::NORMAL_MAP_VISUALIZE)
							return 1;
						else if (taskType == J_GRAPHIC_TASK_TYPE::TANGENT_MAP_VISUALIZE)
							return 1;
						//else if (taskType == J_GRAPHIC_TASK_TYPE::VELOCITY_MAP_VISUALIZE)
						//	return Private::velocityDebuggingNumber;
						else if (taskType == J_GRAPHIC_TASK_TYPE::SSAO_VISUALIZE)
							return 1;
						else
							return 0;
					}
					case J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP:
						return 1;
					case J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP:
						return 1;
					case J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG:
						return 1;
					case J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
					{
						if (taskType == J_GRAPHIC_TASK_TYPE::SCENE_DRAW)
							return 1;
						else if (taskType == J_GRAPHIC_TASK_TYPE::APPLY_POST_PROCESS_RESULT)
							return 1;
						else if (taskType == J_GRAPHIC_TASK_TYPE::RAYTRACING_GI || taskType == J_GRAPHIC_TASK_TYPE::STORE_PREVIOUS_FRAME_DATA)
							return 1;
						else if (taskType == J_GRAPHIC_TASK_TYPE::LIGHT_LIST_DRAW)
							return 1;
						else
							return 0;
					}
					case J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST:
						return 1;
					case J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET:
						return 1;
					case J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP:
						return 1;
					case J_GRAPHIC_RESOURCE_TYPE::IMAGE_PROCESSING:
						return 1;
					case J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_EXPOSURE:
						return 1;
					case J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_LUMA:
						return 1;
					case J_GRAPHIC_RESOURCE_TYPE::RESTIR_RESERVOIR:
						return 4;
					default:
						return 0;
					}
				}
			public:
				int GetResourceIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept final
				{ 
					switch (rType)
					{
					case J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL:
					{
						if (taskType == J_GRAPHIC_TASK_TYPE::RAYTRACING_GI || taskType == J_GRAPHIC_TASK_TYPE::STORE_PREVIOUS_FRAME_DATA)
							return previousDsNumber;
						else
							return mainDsNumber;
					}
					case J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL:
						return 0;
					case J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP:
					{
						if (taskType == J_GRAPHIC_TASK_TYPE::DEPTH_MAP_VISUALIZE)
							return depthDebuggingNumber;
						else if (taskType == J_GRAPHIC_TASK_TYPE::ALBEDO_MAP_VISUALIZE)
							return albedoDebuggingNumber;
						else if (taskType == J_GRAPHIC_TASK_TYPE::SPECULAR_MAP_VISUALIZE)
							return specularDebuggingNumber;
						else if (taskType == J_GRAPHIC_TASK_TYPE::NORMAL_MAP_VISUALIZE)
							return normalDebuggingNumber;
						else if (taskType == J_GRAPHIC_TASK_TYPE::TANGENT_MAP_VISUALIZE)
							return tangentDebuggingNumber;
						//else if (taskType == J_GRAPHIC_TASK_TYPE::VELOCITY_MAP_VISUALIZE)
						//	return Private::velocityDebuggingNumber;
						else if (taskType == J_GRAPHIC_TASK_TYPE::SSAO_VISUALIZE)
							return ssaoDebuggingNumber;
						else
							return invalidIndex;
					}
					case J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP:
						return 0;
					case J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP:
						return 0;
					case J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG:
						return 0;
					case J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
					{
						if (taskType == J_GRAPHIC_TASK_TYPE::SCENE_DRAW)
							return mainRtNumber;
						else if (taskType == J_GRAPHIC_TASK_TYPE::APPLY_POST_PROCESS_RESULT)
							return afterPostProcesseNumber;
						else if (taskType == J_GRAPHIC_TASK_TYPE::RAYTRACING_GI || taskType == J_GRAPHIC_TASK_TYPE::STORE_PREVIOUS_FRAME_DATA)
							return giRtNumber;
						else if (taskType == J_GRAPHIC_TASK_TYPE::LIGHT_LIST_DRAW)
							return litCullDebugRtNumber;
						else
							return invalidIndex;
					}
					case J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST:
						return 0;
					case J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET:
						return 0;
					case J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP:
						return 0;
					case J_GRAPHIC_RESOURCE_TYPE::IMAGE_PROCESSING:
						return 0;
					case J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_EXPOSURE:
						return 0;
					case J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_LUMA:
						return 0;
					case J_GRAPHIC_RESOURCE_TYPE::RESTIR_RESERVOIR:
					{
						if (taskType == J_GRAPHIC_TASK_TYPE::RAYTRACING_GI)
							return 0;
						else
							return invalidIndex;
					}
					default:
						return invalidIndex;
					}
				}
			};

			using CullingInterface = Graphic::JCullingTypePerSingleTargetHolder;

			using CsmTargetInterface = Graphic::JCsmTargetInterface;
		}
		namespace Light
		{ 
			namespace Directional
			{
				using FrameUpload = Core::JDefinedTypeSequence< J_FRAME_RESOURCE_UPLOAD_TYPE,
					J_FRAME_RESOURCE_UPLOAD_TYPE::DIRECTIONAL_LIGHT,
					J_FRAME_RESOURCE_UPLOAD_TYPE::CASCADE_SHADOW_MAP_INFO,
					J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_ARRAY_DRAW,
					J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_DRAW,
					J_FRAME_RESOURCE_UPLOAD_TYPE::DEPTH_TEST_PASS,
					J_FRAME_RESOURCE_UPLOAD_TYPE::HZB_OCC_COMPUTE_PASS>;

				using FrameInterface = JFrameUpdateTypePerSingleHolder<JFrameDirty, FrameUpload>;
 
				using SingleGraphicResource = Core::JDefinedTypeSequence<J_GRAPHIC_RESOURCE_TYPE,
					J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP,
					J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP,
					J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP,
					J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG>;

				using MultiGraphicResource = Core::JDefinedTypeSequence<J_GRAPHIC_RESOURCE_TYPE,
					J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY,
					J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP>;
				 
				class GraphucResourceInterafce final : public Graphic::JGraphicWideSingleAndMultiResourceHolder<SingleGraphicResource,
					MultiGraphicResource,
					true>
				{
				public:
					int GetResourceIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept final
					{
						switch (rType)
						{
						case JinEngine::J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP:
							return 0;
						case JinEngine::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP:
							return 0;
						case JinEngine::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY:
							return 0;
						case JinEngine::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP:
							return 0;
						case JinEngine::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP:
							return 0;
						case JinEngine::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG:
							return 0;
						default:
							return invalidIndex;
						}
					}
				};

				using CullingInterface = Graphic::JCullingSingleTargetHolder<J_CULLING_TARGET::RENDERITEM>;

				using CsmHandlerInterface = Graphic::JCsmHandlerInterface;
			}
			namespace Point
			{
				using FrameUpload = Core::JDefinedTypeSequence< J_FRAME_RESOURCE_UPLOAD_TYPE,
					J_FRAME_RESOURCE_UPLOAD_TYPE::POINT_LIGHT,
					J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_CUBE_DRAW>;

				using FrameInterface = JFrameUpdateTypePerSingleHolder<JFrameDirty, FrameUpload>;

				using GraphicResource = Core::JDefinedTypeSequence<J_GRAPHIC_RESOURCE_TYPE,
					J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP,
					J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE>;

				class GraphucResourceInterafce : public Graphic::JGraphicWideSingleResourceHolder<GraphicResource>
				{
				public:
					int GetResourceIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept
					{
						/*
						*switch (rType)
						{
						case JinEngine::J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP:
							return 0;
						case JinEngine::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE:
							return 0;
						default:
							return invalidIndex;
						}
						*/
						return (rType == J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP || rType == J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE) ? 0 : invalidIndex;
					}
				};

				using CullingInterface = Graphic::JCullingSingleHolder<J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::RENDERITEM>;
			}	
			namespace Spot
			{
				using FrameUpload = Core::JDefinedTypeSequence< J_FRAME_RESOURCE_UPLOAD_TYPE,
					J_FRAME_RESOURCE_UPLOAD_TYPE::SPOT_LIGHT,
					J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_DRAW>;

				using FrameInterface = JFrameUpdateTypePerSingleHolder<JFrameDirty, FrameUpload>;

				using GraphicResource = Core::JDefinedTypeSequence<J_GRAPHIC_RESOURCE_TYPE,
					J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP,
					J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP>;

				class GraphucResourceInterafce : public Graphic::JGraphicWideSingleResourceHolder<GraphicResource>
				{
				public:
					int GetResourceIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept
					{
						/*
						*switch (rType)
						{
						case JinEngine::J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP:
							return 0;
						case JinEngine::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP:
							return 0;
						default:
							return invalidIndex;
						}
						*/
						return (rType == J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP || rType == J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP) ? 0 : invalidIndex;
					}
				};

				using CullingInterface = Graphic::JCullingSingleHolder<J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::RENDERITEM>;
			}  
			namespace Rect
			{
				using FrameUpload = Core::JDefinedTypeSequence< J_FRAME_RESOURCE_UPLOAD_TYPE,
					J_FRAME_RESOURCE_UPLOAD_TYPE::RECT_LIGHT,
					J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_DRAW>;

				using FrameInterface = JFrameUpdateTypePerSingleHolder<JFrameDirty, FrameUpload>;

				using GraphicResource = Core::JDefinedTypeSequence<J_GRAPHIC_RESOURCE_TYPE,
					J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP,
					J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP>;

				class GraphucResourceInterafce : public Graphic::JGraphicWideSingleResourceHolder<GraphicResource>
				{
				public:
					int GetResourceIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept
					{
						/*
						*switch (rType)
						{
						case JinEngine::J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP:
							return 0;
						case JinEngine::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP:
							return 0;
						default:
							return invalidIndex;
						} 
						*/
						return (rType == J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP || rType == J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP) ? 0 : invalidIndex;
					}
				};

				using CullingInterface = Graphic::JCullingSingleHolder<J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::RENDERITEM>;
			}	 
		}
		namespace RenderItem
		{
			using FrameUpload = Core::JDefinedTypeSequence< J_FRAME_RESOURCE_UPLOAD_TYPE,
				J_FRAME_RESOURCE_UPLOAD_TYPE::OBJECT,
				J_FRAME_RESOURCE_UPLOAD_TYPE::BOUNDING_OBJECT,
				J_FRAME_RESOURCE_UPLOAD_TYPE::HZB_OCC_OBJECT,
				J_FRAME_RESOURCE_UPLOAD_TYPE::OBJECT_REF_INFO>;

			using FrameInterface = JFrameUpdateTypePerSingleHolder<JFrameDirty, FrameUpload>;
		} 
		namespace Transform
		{
			using FrameInterface = JFrameUpdateOnlyDirty<JFrameDirtyChain<JFrameDirtyTrigger>>;
		}
	} 
	//ResourceInterfaceType
	namespace
	{
		namespace Material
		{
			using FrameUpload = Core::JDefinedTypeSequence< J_FRAME_RESOURCE_UPLOAD_TYPE,
				J_FRAME_RESOURCE_UPLOAD_TYPE::MATERIAL>;

			using FrameInterface = JFrameUpdateTypePerSingleHolder<JFrameDirty, FrameUpload>;
		}
		namespace Scene
		{
			using FrameUpload = Core::JDefinedTypeSequence< J_FRAME_RESOURCE_UPLOAD_TYPE,
				J_FRAME_RESOURCE_UPLOAD_TYPE::SCENE_PASS>;

			using FrameInterface = JFrameUpdateTypePerSingleHolder<JFrameDirty, FrameUpload>;
		}
		namespace Texture
		{
			class GraphucResourceInterafce : public JGraphicSingleResourceHolder
			{
			public: 
				uint GetAllocableResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept final
				{
					int index = GetResourceIndex(rType, taskType);
					return index != invalidIndex ? 1 : 0;
				}
				int GetResourceIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept
				{  
					/*
					switch (rType)
					{
					case JinEngine::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D:
						return 0;
					case JinEngine::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE:
						return 0;
					default:
						return invalidIndex;
					}
					*/
					return (rType == J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D || rType == J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE) ? 0 : invalidIndex;
				}
			}; 
		}  
	}

	//CompSetType
	namespace
	{
		using AnimatorDataSet = JGraphicObjectDataOneSocket<Animator::FrameInterface>;
		using CameraDataSet = JGraphicObjectDataQuadrupleSocket<Camera::CsmTargetInterface, Camera::CullingInterface, Camera::FrameInterface, Camera::GraphicResourceInterface>;
		using DirectionalLightDataSet = JGraphicObjectDataQuadrupleSocket< Light::Directional::CsmHandlerInterface, Light::Directional::CullingInterface, Light::Directional::FrameInterface, Light::Directional::GraphucResourceInterafce>;
		using PointLightDataSet = JGraphicObjectDataTripleSocket<Light::Point::CullingInterface, Light::Point::FrameInterface, Light::Point::GraphucResourceInterafce>;
		using SpotLightDataSet = JGraphicObjectDataTripleSocket<Light::Spot::CullingInterface, Light::Spot::FrameInterface, Light::Spot::GraphucResourceInterafce>;
		using RectLightDataSet = JGraphicObjectDataTripleSocket<Light::Rect::CullingInterface, Light::Rect::FrameInterface, Light::Rect::GraphucResourceInterafce>;
		using RenderItemDataSet = JGraphicObjectDataOneSocket<RenderItem::FrameInterface>;
		using TransformDataSet = JGraphicObjectDataOneSocket<Transform::FrameInterface>;
	}
	//ResourceSetType
	namespace
	{
		using MaterialDataSet = JGraphicObjectDataOneSocket<Material::FrameInterface>;
		using SceneDataSet = JGraphicObjectDataOneSocket<Scene::FrameInterface>;
		using TextureDataSet = JGraphicObjectDataOneSocket<Texture::GraphucResourceInterafce>;
	} 
 
	void JDx12GraphicObjectDataSetManager::Initialize(JGraphicDevice* device)
	{
		BuildResource(device);
		CreateMetadata();
	}
	void JDx12GraphicObjectDataSetManager::Clear()
	{
		ClearResource();
	}
	const JDx12GraphicObjectDataSetManager::DataVec& JDx12GraphicObjectDataSetManager::GetDataVec(const J_COMPONENT_TYPE type)const noexcept
	{
		return set[ConvertU(type)];
	}
	const JDx12GraphicObjectDataSetManager::DataVec& JDx12GraphicObjectDataSetManager::GetDataVec(const J_RESOURCE_TYPE type)const noexcept
	{
		return set[ConvertU(type)];
	}
	JObjectDataSetMetadata JDx12GraphicObjectDataSetManager::GetMetadata(const J_COMPONENT_TYPE type)const noexcept
	{
		return metadata[ConvertU(type)];
	}
	JObjectDataSetMetadata JDx12GraphicObjectDataSetManager::GetMetadata(const J_RESOURCE_TYPE type)const noexcept
	{
		return metadata[ConvertU(type)];
	}
	bool JDx12GraphicObjectDataSetManager::Add(const JUserPtr<JObject>& obj)
	{
		const J_OBJECT_TYPE objType = obj->GetObjectType();
		if (objType == J_OBJECT_TYPE::COMPONENT_OBJECT)
		{
			const J_COMPONENT_TYPE compType = Core::ConnectChildUserPtr<JComponent>(obj)->GetComponentType();
			switch (compType)
			{
			case JinEngine::J_COMPONENT_TYPE::USER_DEFIENED_BEHAVIOR:
			{
				//추가필요
				break;
			}
			case JinEngine::J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR:
			{
				set[ConvertU(compType)].Add(Core::JPtrUtil::MakeOwnerPtr<AnimatorDataSet>(obj, std::make_unique<Animator::FrameInterface>()));
				break;
			}
			case JinEngine::J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA:
			{
				set[ConvertU(compType)].Add(Core::JPtrUtil::MakeOwnerPtr<CameraDataSet>(obj,
					std::make_unique<Camera::CsmTargetInterface>(),
					std::make_unique<Camera::CullingInterface>(),
					std::make_unique<Camera::FrameInterface>(),
					std::make_unique<Camera::GraphicResourceInterface>()));
				break;
			}
			case JinEngine::J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT:
			{
				const J_LIGHT_TYPE litType = Core::ConnectChildUserPtr<JLight>(obj)->GetLightType();
				switch (litType)
				{
				case JinEngine::J_LIGHT_TYPE::DIRECTIONAL:
				{
					set[ConvertU(compType)].Add(Core::JPtrUtil::MakeOwnerPtr<DirectionalLightDataSet>(obj,
						std::make_unique<Light::Directional::CsmHandlerInterface>(),
						std::make_unique<Light::Directional::CullingInterface>(),
						std::make_unique<Light::Directional::FrameInterface>(),
						std::make_unique<Light::Directional::GraphucResourceInterafce>()));
					break;
				}
				case JinEngine::J_LIGHT_TYPE::POINT:
				{
					set[ConvertU(compType)].Add(Core::JPtrUtil::MakeOwnerPtr<PointLightDataSet>(obj,
						std::make_unique<Light::Point::CullingInterface>(),
						std::make_unique<Light::Point::FrameInterface>(),
						std::make_unique<Light::Point::GraphucResourceInterafce>()));
					break;
				}
				case JinEngine::J_LIGHT_TYPE::SPOT:
				{
					set[ConvertU(compType)].Add(Core::JPtrUtil::MakeOwnerPtr<SpotLightDataSet>(obj,
						std::make_unique<Light::Spot::CullingInterface>(),
						std::make_unique<Light::Spot::FrameInterface>(),
						std::make_unique<Light::Spot::GraphucResourceInterafce>()));
					break;
				}
				case JinEngine::J_LIGHT_TYPE::RECT:
				{
					set[ConvertU(compType)].Add(Core::JPtrUtil::MakeOwnerPtr<RectLightDataSet>(obj,
						std::make_unique<Light::Rect::CullingInterface>(),
						std::make_unique<Light::Rect::FrameInterface>(),
						std::make_unique<Light::Rect::GraphucResourceInterafce>()));
					break;
				}
				default:
					break;
				}
				break;
			}
			case JinEngine::J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM:
			{
				set[ConvertU(compType)].Add(Core::JPtrUtil::MakeOwnerPtr<RenderItemDataSet>(obj, std::make_unique<RenderItem::FrameInterface>()));
				break;
			}
			case JinEngine::J_COMPONENT_TYPE::ENGINE_DEFIENED_TRANSFORM:
			{
				set[ConvertU(compType)].Add(Core::JPtrUtil::MakeOwnerPtr<TransformDataSet>(obj, std::make_unique<Transform::FrameInterface>()));
				break;
			}
			default:
				break;
			}
			return true;
		}
		else if (objType == J_OBJECT_TYPE::RESOURCE_OBJECT)
		{
			const J_RESOURCE_TYPE resourceType = Core::ConnectChildUserPtr<JResourceObject>(obj)->GetResourceType();
			switch (resourceType)
			{
			case JinEngine::J_RESOURCE_TYPE::MATERIAL:
			{
				set[ConvertU(resourceType)].Add(Core::JPtrUtil::MakeOwnerPtr<MaterialDataSet>(obj, std::make_unique<Material::FrameInterface>()));
				break;
			}
			case JinEngine::J_RESOURCE_TYPE::TEXTURE:
			{
				set[ConvertU(resourceType)].Add(Core::JPtrUtil::MakeOwnerPtr<TextureDataSet>(obj, std::make_unique<Texture::GraphucResourceInterafce>()));
				break;
			}
			case JinEngine::J_RESOURCE_TYPE::SCENE:
			{
				set[ConvertU(resourceType)].Add(Core::JPtrUtil::MakeOwnerPtr<SceneDataSet>(obj, std::make_unique<Scene::FrameInterface>()));
				break;
			}
			case JinEngine::J_RESOURCE_TYPE::SHADER:
			case JinEngine::J_RESOURCE_TYPE::SCRIPT:
			case JinEngine::J_RESOURCE_TYPE::SKELETON:
			case JinEngine::J_RESOURCE_TYPE::ANIMATION_CLIP:
			case JinEngine::J_RESOURCE_TYPE::ANIMATION_CONTROLLER:
			default:
				break;
			}
			return true;
		}
		else
		{
			//not supported
			return false;
		}
	}
	bool JDx12GraphicObjectDataSetManager::Remove(JUserPtr<JGraphicModuleManagedDataFrame>& data)
	{
		auto removeLam = [](JOwnerPtr<JGraphicObjectDataSetBase>* owner, const size_t guid)
		{
			return (*owner)->Object()->GetGuid() == guid;
		};
		bool(*removePtr)(JOwnerPtr<JGraphicObjectDataSetBase>*, size_t) = removeLam;

		const JWeakPtr<JObject> obj = data->Object();
		const J_OBJECT_TYPE objType = obj->GetObjectType();

		data.Release();
		if (objType == J_OBJECT_TYPE::COMPONENT_OBJECT)
		{
			const J_COMPONENT_TYPE compType = Core::ConnectChildUserPtr<JComponent>(obj)->GetComponentType();
			const int index = set[ConvertU(compType)].GetIndex(removePtr, obj->GetGuid());
			set[ConvertU(compType)].Remove(index);
			return true;
		}
		else if (objType == J_OBJECT_TYPE::RESOURCE_OBJECT)
		{
			const J_RESOURCE_TYPE resourceType = Core::ConnectChildUserPtr<JResourceObject>(obj)->GetResourceType();
			const int index = set[ConvertU(resourceType)].GetIndex(removePtr, obj->GetGuid());
			set[ConvertU(resourceType)].Remove(index);
			return true;
		}
		else
		{
			//not supported
			return false;
		}
	}
	void JDx12GraphicObjectDataSetManager::BuildResource(JGraphicDevice* device)
	{

	}
	void JDx12GraphicObjectDataSetManager::CreateMetadata()
	{  
		using AnimatorMetadata = JTypePerInterfaceMetadata<Core::JEmptyType, Animator::FrameInterface, Core::JEmptyType, Core::JEmptyType, ConvertT(J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR)>;
		using BehaviorMetadata = JTypePerInterfaceMetadata2<true, true, true, true, ConvertT(J_COMPONENT_TYPE::USER_DEFIENED_BEHAVIOR)>;
		using CameraMetadata = JTypePerInterfaceMetadata<Camera::CullingInterface, Camera::FrameInterface, Core::JEmptyType, Camera::GraphicResourceInterface, ConvertT(J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)>;
		using LightMetadata = JTypePerInterfaceMetadata2<true, true, false, true, ConvertT(J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)>;
		using RenderItemMetadata = JTypePerInterfaceMetadata<Core::JEmptyType, RenderItem::FrameInterface, Core::JEmptyType, Core::JEmptyType, ConvertT(J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM)>;
		using TransformMetadata = JTypePerInterfaceMetadata<Core::JEmptyType, Transform::FrameInterface, Core::JEmptyType, Core::JEmptyType, ConvertT(J_COMPONENT_TYPE::ENGINE_DEFIENED_TRANSFORM)>;

		using MaterialMetadata = JTypePerInterfaceMetadata<Core::JEmptyType, Material::FrameInterface, Core::JEmptyType, Core::JEmptyType, ConvertT(J_RESOURCE_TYPE::MATERIAL)>;
		using SceneMetadata = JTypePerInterfaceMetadata<Core::JEmptyType, Scene::FrameInterface, Core::JEmptyType, Core::JEmptyType, ConvertT(J_RESOURCE_TYPE::SCENE)>;
		using TextureMetadata = JTypePerInterfaceMetadata<Core::JEmptyType, Core::JEmptyType, Core::JEmptyType, Texture::GraphucResourceInterafce, ConvertT(J_RESOURCE_TYPE::TEXTURE)>;
		  
		AnimatorMetadata::Register(metadata[ConvertU(J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR)]);
		BehaviorMetadata::Register(metadata[ConvertU(J_COMPONENT_TYPE::USER_DEFIENED_BEHAVIOR)]);
		CameraMetadata::Register(metadata[ConvertU(J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)]);
		LightMetadata::Register(metadata[ConvertU(J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)]);
		RenderItemMetadata::Register(metadata[ConvertU(J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM)]);
		TransformMetadata::Register(metadata[ConvertU(J_COMPONENT_TYPE::ENGINE_DEFIENED_TRANSFORM)]);
		MaterialMetadata::Register(metadata[ConvertU(J_RESOURCE_TYPE::MATERIAL)]);
		SceneMetadata::Register(metadata[ConvertU(J_RESOURCE_TYPE::SCENE)]);
		TextureMetadata::Register(metadata[ConvertU(J_RESOURCE_TYPE::TEXTURE)]);
	}
	void JDx12GraphicObjectDataSetManager::ClearResource()
	{
		for (uint i = 0; i < SIZE_OF_ARRAY(set); ++i)
			set[i].Clear(); 
	}
}
/*
*	unuse
	const JDx12GraphicObjectDataSetManager::DataVec* JDx12GraphicObjectDataSetManager::GetDataVec(const Core::JTypeInfo& info)const noexcept
	{
		J_COMPONENT_TYPE compType = CTypeCommonCall::ConvertCompType(info);
		if ((int)compType != invalidIndex)
			return &set[ConvertU(compType)];

		J_RESOURCE_TYPE resourceType = RTypeCommonCall::ConvertCompType(info);
		if ((int)resourceType != invalidIndex)
			return &set[ConvertU(resourceType)];

		return nullptr;
	}
*/