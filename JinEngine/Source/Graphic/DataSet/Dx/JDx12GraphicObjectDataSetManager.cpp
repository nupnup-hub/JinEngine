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
#include"../../../Object/Component/Camera/JCamera.h"
#include"../../../Object/Resource/JResourceObjectHint.h" 
#include"../../../Object/Resource/JResourceObject.h" 
#include"../../../Object/Resource/Mesh/JMeshGeometry.h" 
#include"../../../Object/GraphicRule/JGraphicModuleInterfaceHolder.h"
#include"../../../Object/JObjectTypeStatistics.h"

#include"../../../Core/Utility/JTypeSequence.h"
#include"../../../Core/Log/JLogMacro.h" 

/*
Lambda를 매개변수가 해당클래스에 포인터인 경우
private까지 Access 가능 부모의 protected도 가능
but 부모클래스인 경우 public 만가능
*/
#define REGISTER_ALLOC_OPTION(type)														\
public:																					\
static void RegisterTypeData()															\
{																						\
	using JAllocationDesc = JinEngine::Core::JAllocationDesc;							\
	using NotifyReAllocPtr = JAllocationDesc::NotifyReAllocF::Ptr;						\
	using NotifyReAllocF = JAllocationDesc::NotifyReAllocF::Functor;					\
	using ReceiverPtr = JAllocationDesc::ReceiverPtr;									\
	using ReAllocatedPtr = JAllocationDesc::ReAllocatedPtr;								\
	using MemIndex = JAllocationDesc::MemIndex;											\
																						\
	NotifyReAllocPtr notifyPtr = [](ReceiverPtr receiver, ReAllocatedPtr movedPtr, MemIndex index)	\
	{																								\
		CorrectType* movedInfo = static_cast<CorrectType*>(movedPtr);	\
		JGraphicObjectDataSetManager* manager = movedInfo->manager;									\
																									\
		auto obj = movedInfo->Object();																\
		const J_OBJECT_TYPE objType = obj->GetObjectType();											\
		const JDx12GraphicObjectDataSetManager::DataVec* dataVec = nullptr;							\
																									\
		if (objType == J_OBJECT_TYPE::COMPONENT_OBJECT)												\
			dataVec = &manager->GetDataVec(GetUniqueIndex(obj));									\
		else if (objType == J_OBJECT_TYPE::RESOURCE_OBJECT)											\
			dataVec = &manager->GetDataVec(GetUniqueIndex(obj));									\
		else                                                                                        \
		{																							\
			J_LOG_PRINT_OUT("ReAllocError in" + type::StaticTypeInfo().Name(), "Invalid type: " + Core::GetName(objType));				\
			return;																					\
		}																							\
																									\
		const uint count = dataVec->Count();														\
		auto objRaw = obj.Get();																	\
		for(uint i = 0; i < count; ++i)																\
		{																							\
			auto& data = *(dataVec->Get(i));														\
			if (data->Object().Get() == objRaw)														\
			{																						\
				data.Swap(movedInfo);																\
				type::NotifyReAlloc(data);															\
				break;																				\
			}																						\
		}																							\
		/*(*dataVec).Get(index)->Swap(movedInfo);*/													\
	};																								\
	auto reAllocF = std::make_unique<JAllocationDesc::NotifyReAllocF::Functor>(notifyPtr);			\
	std::unique_ptr<JAllocationDesc> desc = std::make_unique<JAllocationDesc>();					\
	desc->notifyReAllocB = UniqueBind(std::move(reAllocF), static_cast<ReceiverPtr>(nullptr), JinEngine::Core::empty, JinEngine::Core::empty);\
	StaticTypeInfo().SetAllocationOption(std::move(desc));									\
}\



namespace JinEngine::Graphic
{       
	//Helper structure
	namespace
	{
		static UniqueIndex GetUniqueIndex(const JWeakPtr<JObject>& obj)
		{  
			const J_OBJECT_TYPE objType = obj->GetObjectType(); 
			if (objType == J_OBJECT_TYPE::COMPONENT_OBJECT)
			{
				const J_COMPONENT_TYPE compType = Core::ConnectChildUserPtr<JComponent>(obj)->GetComponentType();
				return ConvertUniqueIndex(compType, obj->GetSubTypeIndex());
			}
			else if (objType == J_OBJECT_TYPE::RESOURCE_OBJECT)
			{
				const J_RESOURCE_TYPE resourceType = Core::ConnectChildUserPtr<JResourceObject>(obj)->GetResourceType();
				return ConvertUniqueIndex(resourceType, obj->GetSubTypeIndex()) + totalCompVariation;
			}
			else
				return invalidIndex;
		}
	}
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
		template<typename CullingInterface, 
			typename FrameInterface, 
			typename GpuAcceleratorInterface, 
			typename GraphicResourceInterface>
		struct JTypePerInterfaceMetadata
		{
		private:
			template<typename T, typename = void>
			struct HasFrameUploadHint
			{
			public:
				static constexpr bool isSupportedFrameUpload = false;
				static constexpr bool isSupportedFrameDirty = false;
				static constexpr bool isNeedToUpdateEveryFrame = false;
			};
			template<typename T>
			struct HasFrameUploadHint<T, std::void_t<decltype(&T::isSupportedFrameUpload)>>
			{
			public:
				static constexpr bool isSupportedFrameUpload = T::isSupportedFrameUpload;
				static constexpr bool isSupportedFrameDirty = T::isSupportedFrameDirty;
				static constexpr bool isNeedToUpdateEveryFrame = T::isNeedToUpdateEveryFrame;
			};  
		public:
			//basicalliy
			static constexpr bool hasCullingInterface = !std::is_same_v<CullingInterface, Core::JEmptyType>;
			static constexpr bool hasFrameInterface = !std::is_same_v<FrameInterface, Core::JEmptyType>;
			static constexpr bool hasGpuAcceleratorInterface = !std::is_same_v<GpuAcceleratorInterface, Core::JEmptyType>;
			static constexpr bool hasGraphicResourceInterface = !std::is_same_v<GraphicResourceInterface, Core::JEmptyType>;
		public:
			//detail  
			static constexpr bool isSupportedFrameUpload = hasFrameInterface && HasFrameUploadHint<FrameInterface>::isSupportedFrameUpload;
			static constexpr bool isSupportedFrameDirty = hasFrameInterface && HasFrameUploadHint<FrameInterface>::isSupportedFrameDirty;
			static constexpr bool isNeedToUpdateEveryFrame = hasFrameInterface && HasFrameUploadHint<FrameInterface>::isNeedToUpdateEveryFrame;
		protected:
			static void Register(JObjectDataSetMetadata& data)
			{			  
				//CanUse 
				data.isSupportedCulling = hasCullingInterface;
				data.isSupportedGpuAccelerator = hasGpuAcceleratorInterface;
				data.isSupportedGraphicResource = hasGraphicResourceInterface;

				data.isSupportedFrameResourceUpload = isSupportedFrameUpload;
				data.isSupportedFrameDirty = isSupportedFrameDirty;
				data.isNeedToUpdateEveryFrame = isNeedToUpdateEveryFrame;

				if constexpr (hasFrameInterface)
				{  
					for (uint i = 0; i < (uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT; ++i)
					{
						const J_FRAME_RESOURCE_UPLOAD_TYPE type = (J_FRAME_RESOURCE_UPLOAD_TYPE)i;
						data.supportedFrameType.set(i, FrameInterface::IsSupported(type)); 
					} 
				}
			}
		}; 
		
		template<J_COMPONENT_TYPE compType,
			uint localTypeIndex,
			typename CullingInterface,
			typename FrameInterface,
			typename GpuAcceleratorInterface,
			typename GraphicResourceInterface>
			struct JCompTypePerInterfaceMetadata :  public JTypePerInterfaceMetadata<CullingInterface,
			FrameInterface,
			GpuAcceleratorInterface,
			GpuAcceleratorInterface>
		{
		private:
			using Parent = JTypePerInterfaceMetadata<CullingInterface, FrameInterface, GpuAcceleratorInterface, GpuAcceleratorInterface>;
		public:
			static void Register(JObjectDataSetMetadata* dataArray)
			{
				UniqueIndex uniqueIndex = ConvertCompUniqueIndex<compType>(localTypeIndex);
				JObjectDataSetMetadata& data = dataArray[uniqueIndex];
				Parent::Register(data);
				data.uniqueIndex = uniqueIndex;
				data.tag = Core::GetName(compType) + " " + std::to_string(data.uniqueIndex);
			}
		};
		template<J_RESOURCE_TYPE resourceType,
			uint localTypeIndex,
			typename CullingInterface,
			typename FrameInterface,
			typename GpuAcceleratorInterface,
			typename GraphicResourceInterface>
			struct JResourceTypePerInterfaceMetadata : public JTypePerInterfaceMetadata<CullingInterface,
			FrameInterface,
			GpuAcceleratorInterface,
			GpuAcceleratorInterface>
		{
		private:
			using Parent = JTypePerInterfaceMetadata<CullingInterface, FrameInterface, GpuAcceleratorInterface, GpuAcceleratorInterface>;
		public:
			static void Register(JObjectDataSetMetadata* dataArray)
			{
			    UniqueIndex uniqueIndex = ConvertResourceUniqueIndex<resourceType>(localTypeIndex) + totalCompVariation;
				JObjectDataSetMetadata& data = dataArray[uniqueIndex];
				Parent::Register(data);
				data.uniqueIndex = uniqueIndex;
				data.tag = Core::GetName(resourceType) + " " + std::to_string(data.uniqueIndex);
			}
		}; 	 
	}
	//CompInterfaceType
	namespace
	{
		namespace Animator
		{
			using FrameUpload = Core::JDefinedTypeSequence<J_FRAME_RESOURCE_UPLOAD_TYPE,
				J_FRAME_RESOURCE_UPLOAD_TYPE::ANIMATION>;

			using FrameInterface = JFrameUpdateTypePerSingleHolder<JFrameAlwaysDirty, FrameUpload>;
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
			static constexpr uint storePreviousDataNumber = mainRtNumber + 1;
			static constexpr uint afterPostProcesseNumber = storePreviousDataNumber + 1;
			static constexpr uint raytracingRtNumber = afterPostProcesseNumber + 1;
			static constexpr uint litCullDebugRtNumber = raytracingRtNumber + 1;
			static constexpr uint rtCount = litCullDebugRtNumber + 1;

			static constexpr uint mainDsNumber = 0;
			static constexpr uint previousDsNumber = 1;
			static constexpr uint dsCount = previousDsNumber + 1;

			static constexpr uint depthDebuggingNumber = 0;
			static constexpr uint albedoDebuggingNumber = depthDebuggingNumber + 1;
			static constexpr uint specularDebuggingNumber = albedoDebuggingNumber + 1;
			static constexpr uint normalDebuggingNumber = specularDebuggingNumber + 1;
			static constexpr uint tangentDebuggingNumber = normalDebuggingNumber + 1;
			static constexpr uint ssaoDebuggingNumber = tangentDebuggingNumber + 1;
			//static constexpr uint velocityDebuggingNumber = ssaoDebuggingNumber + 1;
			//static constexpr uint debuggingMapCount = velocityDebuggingNumber + 1;
			static constexpr uint debuggingMapCount = ssaoDebuggingNumber + 1;
			static constexpr uint debugCount = debuggingMapCount + 1;
 
			static constexpr uint reserviorCount = 4;

			using FrameUpload = Core::JDefinedTypeSequence<J_FRAME_RESOURCE_UPLOAD_TYPE,
				J_FRAME_RESOURCE_UPLOAD_TYPE::CAMERA,
				J_FRAME_RESOURCE_UPLOAD_TYPE::DEPTH_TEST_PASS,
				J_FRAME_RESOURCE_UPLOAD_TYPE::HZB_OCC_COMPUTE_PASS,
				J_FRAME_RESOURCE_UPLOAD_TYPE::LIGHT_CULLING_PASS,
				J_FRAME_RESOURCE_UPLOAD_TYPE::SSAO_PASS>;
				//J_FRAME_RESOURCE_UPLOAD_TYPE::RAYTRACING_GI,
				//J_FRAME_RESOURCE_UPLOAD_TYPE::RAYTRACING_SHADOW,
				//J_FRAME_RESOURCE_UPLOAD_TYPE::RAYTRACING_DENOISE>;

			class CameraFrameDirty final : public JFrameDirty
			{
			public:
				/*
				int GetFrameDirtyMax()const noexcept final
				{
					//첫번째 frame에 update된 constants에  mPreViewProj이 유효한 값을 갖게 하기위해 +1
					//(1: delta), (2: zero), (3: zero), (1: zero)
					return Graphic::Constants::gNumFrameResources + 1;
				}
				-> 수정 mPreViewProj와 같은 matrix값들은 frame dirty가 3일시 6번을 update 해줘야한다.
				preframe - curframe delta를 0 ~ 2 그리고 3 ~ 5를 변동이 없는 값들로 채워줘야 옳바른
				Velocity 값을 구할 수 있다 허나 mPreViewProj은 제한적인 상황에 사용됨으로
				해당하는 Subclass에서 자체적으로 계산해서 upload하도록 하자.
				2024-08-19
				*/
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
				JStaticTupleGraphicUint<J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_EXPOSURE, 1>,
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
						if (taskType == J_GRAPHIC_TASK_TYPE::STORE_PREVIOUS_FRAME_DATA)
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
						else if (taskType == J_GRAPHIC_TASK_TYPE::SSAO_VISUALIZE)
							return 1;
						//else if (taskType == J_GRAPHIC_TASK_TYPE::VELOCITY_MAP_VISUALIZE)
						//	return 1;
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
						else if (taskType == J_GRAPHIC_TASK_TYPE::STORE_PREVIOUS_FRAME_DATA)
							return 1;
						else if (taskType == J_GRAPHIC_TASK_TYPE::APPLY_POST_PROCESS_RESULT)
							return 1;
						else if (taskType == J_GRAPHIC_TASK_TYPE::RAYTRACING_GI)
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
					case J_GRAPHIC_RESOURCE_TYPE::RESTIR_INITIAL_SAMPLE:
						return 1;
					case J_GRAPHIC_RESOURCE_TYPE::RESTIR_RESERVOIR:
						return 4;
					default:
						return 0;
					}
				}
				int GetResourceIndexOffset(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept final
				{ 
					switch (rType)
					{
					case J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL:
					{
						if (taskType == J_GRAPHIC_TASK_TYPE::STORE_PREVIOUS_FRAME_DATA)
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
						else if (taskType == J_GRAPHIC_TASK_TYPE::SSAO_VISUALIZE)
							return ssaoDebuggingNumber;
						//else if (taskType == J_GRAPHIC_TASK_TYPE::VELOCITY_MAP_VISUALIZE)
						//	return velocityDebuggingNumber;
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
						else if (taskType == J_GRAPHIC_TASK_TYPE::STORE_PREVIOUS_FRAME_DATA)
							return storePreviousDataNumber;
						else if (taskType == J_GRAPHIC_TASK_TYPE::APPLY_POST_PROCESS_RESULT)
							return afterPostProcesseNumber;
						else if (taskType == J_GRAPHIC_TASK_TYPE::RAYTRACING_GI)
							return raytracingRtNumber;
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
					case J_GRAPHIC_RESOURCE_TYPE::RESTIR_INITIAL_SAMPLE:
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
					int GetResourceIndexOffset(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept final
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
					int GetResourceIndexOffset(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept
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
					int GetResourceIndexOffset(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept
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
					int GetResourceIndexOffset(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept
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
			using FrameInterface = JFrameUpdateOnlyDirty<JFrameDirtyTrigger>;
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
		namespace Mesh
		{
			namespace Static
			{
				using GraphicResource = Core::JDefinedTypeSequence<J_GRAPHIC_RESOURCE_TYPE,
					J_GRAPHIC_RESOURCE_TYPE::VERTEX,
					J_GRAPHIC_RESOURCE_TYPE::INDEX>;

				class GraphucResourceInterafce : public Graphic::JGraphicWideSingleResourceHolder<GraphicResource>
				{
				public:
					int GetResourceIndexOffset(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept
					{ 
						return (rType == J_GRAPHIC_RESOURCE_TYPE::VERTEX || rType == J_GRAPHIC_RESOURCE_TYPE::INDEX) ? 0 : invalidIndex;
					}
				};
			}
			namespace Skinned
			{
				using GraphicResource = Core::JDefinedTypeSequence<J_GRAPHIC_RESOURCE_TYPE,
					J_GRAPHIC_RESOURCE_TYPE::VERTEX,
					J_GRAPHIC_RESOURCE_TYPE::INDEX>;

				class GraphucResourceInterafce : public Graphic::JGraphicWideSingleResourceHolder<GraphicResource>
				{
				public:
					int GetResourceIndexOffset(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept
					{
						return (rType == J_GRAPHIC_RESOURCE_TYPE::VERTEX || rType == J_GRAPHIC_RESOURCE_TYPE::INDEX) ? 0 : invalidIndex;
					}
				};
			}
		}
		namespace Scene
		{
			using FrameUpload = Core::JDefinedTypeSequence<J_FRAME_RESOURCE_UPLOAD_TYPE,
				J_FRAME_RESOURCE_UPLOAD_TYPE::SCENE_PASS>;

			using FrameInterface = JFrameUpdateTypePerSingleHolder<JFrameDirty, FrameUpload>;

			using GpuAcceleratorInterface = JGpuAcceleratorInterface;
		}
		namespace Texture
		{
			class GraphucResourceInterafce : public JGraphicSingleResourceHolder
			{
			public: 
				uint GetAllocableResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept final
				{
					int index = GetResourceIndexOffset(rType, taskType);
					return index != invalidIndex ? 1 : 0;
				}
				int GetResourceIndexOffset(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept
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
		template<typename Type>
		class JDx12GraphicObjectDataOneSocket : public JGraphicObjectDataOneSocket<Type>
		{
		private:
			using CorrectType = JDx12GraphicObjectDataOneSocket<Type>;
		private:
			REGISTER_ALLOC_OPTION(JDx12GraphicObjectDataOneSocket)
			REGISTER_CLASS_USE_ALLOCATOR(JDx12GraphicObjectDataOneSocket)
		private:
			JDx12GraphicObjectDataSetManager* manager;
		public:
			JDx12GraphicObjectDataOneSocket(const JGraphicModuleManagedDataCreationDesc& desc,
				std::unique_ptr<Type>&& firstInterface,
				JDx12GraphicObjectDataSetManager* manager)
				:JGraphicObjectDataOneSocket<Type>(desc, std::move(firstInterface)), manager(manager)
			{  
			}
		};
		template<typename FirstType, typename SecondType>
		class JDx12GraphicObjectDataDoubleSocket : public JGraphicObjectDataDoubleSocket<FirstType, SecondType>
		{
		private:
			using CorrectType = JDx12GraphicObjectDataDoubleSocket<FirstType, SecondType>;
		private:
			REGISTER_ALLOC_OPTION(JDx12GraphicObjectDataDoubleSocket)
			REGISTER_CLASS_USE_ALLOCATOR(JDx12GraphicObjectDataDoubleSocket)
		private:
			JDx12GraphicObjectDataSetManager* manager;
		public:
			JDx12GraphicObjectDataDoubleSocket(const JGraphicModuleManagedDataCreationDesc& desc,
				std::unique_ptr<FirstType>&& firstInterface,
				std::unique_ptr<SecondType>&& secondInterface,
				JDx12GraphicObjectDataSetManager* manager)
				:JGraphicObjectDataDoubleSocket<FirstType, SecondType>(desc,
					std::move(firstInterface),
					std::move(secondInterface)), manager(manager)
			{}
		};
		template<typename FirstType, typename SecondType, typename ThirdType>
		class JDx12GraphicObjectDataTripleSocket : public JGraphicObjectDataTripleSocket<FirstType, SecondType, ThirdType>
		{
		private:
			using CorrectType = JDx12GraphicObjectDataTripleSocket<FirstType, SecondType, ThirdType>;
		private:
			REGISTER_ALLOC_OPTION(JDx12GraphicObjectDataTripleSocket)
			REGISTER_CLASS_USE_ALLOCATOR(JDx12GraphicObjectDataTripleSocket)
		private:
			JDx12GraphicObjectDataSetManager* manager;
		public:
			JDx12GraphicObjectDataTripleSocket(const JGraphicModuleManagedDataCreationDesc& desc,
				std::unique_ptr<FirstType>&& firstInterface,
				std::unique_ptr<SecondType>&& secondInterface,
				std::unique_ptr<ThirdType>&& thirdInterface,
				JDx12GraphicObjectDataSetManager* manager)
				:JGraphicObjectDataTripleSocket<FirstType, SecondType, ThirdType>(desc,
					std::move(firstInterface),
					std::move(secondInterface),
					std::move(thirdInterface)), manager(manager)
			{}
		};
		template<typename FirstType, typename SecondType, typename ThirdType, typename ForthType>
		class JDx12GraphicObjectDataQuadrupleSocket : public JGraphicObjectDataQuadrupleSocket<FirstType, SecondType, ThirdType, ForthType>
		{
		private:
			using CorrectType = JDx12GraphicObjectDataQuadrupleSocket<FirstType, SecondType, ThirdType, ForthType>;
		private:
			REGISTER_ALLOC_OPTION(JDx12GraphicObjectDataQuadrupleSocket)
			REGISTER_CLASS_USE_ALLOCATOR(JDx12GraphicObjectDataQuadrupleSocket)
		private:
			JDx12GraphicObjectDataSetManager* manager;
		public:
			JDx12GraphicObjectDataQuadrupleSocket(const JGraphicModuleManagedDataCreationDesc& desc,
				std::unique_ptr<FirstType>&& firstInterface,
				std::unique_ptr<SecondType>&& secondInterface,
				std::unique_ptr<ThirdType>&& thirdInterface,
				std::unique_ptr<ForthType>&& forthInterface,
				JDx12GraphicObjectDataSetManager* manager)
				:JGraphicObjectDataQuadrupleSocket<FirstType, SecondType, ThirdType, ForthType>(desc,
					std::move(firstInterface),
					std::move(secondInterface),
					std::move(thirdInterface),
					std::move(forthInterface)), manager(manager)
			{ 
			}
		};

		using AnimatorDataSet = JDx12GraphicObjectDataOneSocket<Animator::FrameInterface>;
		using CameraDataSet = JDx12GraphicObjectDataQuadrupleSocket<Camera::CsmTargetInterface, Camera::CullingInterface, Camera::FrameInterface, Camera::GraphicResourceInterface>;
		using DirectionalLightDataSet = JDx12GraphicObjectDataQuadrupleSocket<Light::Directional::CsmHandlerInterface, Light::Directional::CullingInterface, Light::Directional::FrameInterface, Light::Directional::GraphucResourceInterafce>;
		using PointLightDataSet = JDx12GraphicObjectDataTripleSocket<Light::Point::CullingInterface, Light::Point::FrameInterface, Light::Point::GraphucResourceInterafce>;
		using SpotLightDataSet = JDx12GraphicObjectDataTripleSocket<Light::Spot::CullingInterface, Light::Spot::FrameInterface, Light::Spot::GraphucResourceInterafce>;
		using RectLightDataSet = JDx12GraphicObjectDataTripleSocket<Light::Rect::CullingInterface, Light::Rect::FrameInterface, Light::Rect::GraphucResourceInterafce>;
		using RenderItemDataSet = JDx12GraphicObjectDataOneSocket<RenderItem::FrameInterface>;
		using TransformDataSet = JDx12GraphicObjectDataOneSocket<Transform::FrameInterface>;
	}
	//ResourceSetType
	namespace
	{
		using MaterialDataSet = JDx12GraphicObjectDataOneSocket<Material::FrameInterface>;
		using StaticMeshDataSet = JDx12GraphicObjectDataOneSocket<Mesh::Static::GraphucResourceInterafce>;
		using SkinnedMeshDataSet = JDx12GraphicObjectDataOneSocket<Mesh::Skinned::GraphucResourceInterafce>;
		using SceneDataSet = JDx12GraphicObjectDataDoubleSocket<Scene::FrameInterface, Scene::GpuAcceleratorInterface>;
		using TextureDataSet = JDx12GraphicObjectDataOneSocket<Texture::GraphucResourceInterafce>;
	} 
 
	//Additional execution with grahic reosurce creation
	namespace
	{
		void ApplyDeferred(JCamera* cam, const bool isAct)
		{ 
			auto graphicData = cam->ModuleManagedData();

			JGraphicResourceTypeSet typeSet(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
			const int resourceIndex = graphicData->GetGraphicResourceUserInterface()->GetResourceIndexOffset(typeSet.resouce, typeSet.task);
			if (!graphicData->GetGraphicResourceUserInterface()->IsValidHandle(typeSet.resouce, resourceIndex))
				return;

			if (isAct)
			{
				//typeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP;
				//if (graphicData->GetGraphicResourceUserInterface()->HasOption(typeSet.resouce, typeSet.option, resourceIndex))
				//	continue;

				typeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP;
				GMI()->CreateGraphicResourceOption(graphicData, typeSet);

				typeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY;
				GMI()->CreateGraphicResourceOption(graphicData, typeSet);

				typeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP;
				GMI()->CreateGraphicResourceOption(graphicData, typeSet);
			}
			else
			{
				typeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP;
				GMI()->DestroyGraphicResourceOption(graphicData, typeSet);

				typeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY;
				GMI()->DestroyGraphicResourceOption(graphicData, typeSet);

				typeSet.option = J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP;
				GMI()->DestroyGraphicResourceOption(graphicData, typeSet);
			}
			graphicData->GetFrameUpdateUserInterface()->SetFrameDirty();
		}
		void ApplyDeferred(const JGraphicOptionChangedSet& set, const ObjectDataSetVec& camVec)
		{
			if (set.preOption.rendering.allowDeferred == set.newOption.rendering.allowDeferred &&
				set.preOption.rendering.allowRaytracing == set.newOption.rendering.allowRaytracing)
				return;

			const bool isAct = set.newOption.rendering.allowDeferred || set.newOption.rendering.allowRaytracing;
			const uint count = camVec.Count();
			for (uint i = 0; i < count; ++i)
			{
				JUniquePtr<JGraphicObjectDataSetBase>& objSet = *camVec.Get(i);
				JCamera* cam = static_cast<JCamera*>(objSet->Object().Get());
				ApplyDeferred(cam, isAct);
			}
		}
		namespace Camera
		{
			void AdditionalExecution(JCamera* cam, const JGraphicOption& option)
			{
				ApplyDeferred(cam, option.rendering.allowDeferred || option.rendering.allowRaytracing);
			}
		}
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
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicObjectDataSetManager::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	const JDx12GraphicObjectDataSetManager::DataVec& JDx12GraphicObjectDataSetManager::GetDataVec(const UniqueIndex index)const noexcept
	{
		return set[index];
	}
	JObjectDataSetMetadata JDx12GraphicObjectDataSetManager::GetMetadata(const UniqueIndex index)const noexcept
	{
		return metadata[index];
	}  
	bool JDx12GraphicObjectDataSetManager::HasDependency(const JGraphicOption::TYPE type)const noexcept
	{
		if (type == JGraphicOption::TYPE::RENDERING || type == JGraphicOption::TYPE::POST_PROCESS)
			return true;
		else
			return false;
	}
	JFastPtr<JGraphicModuleManagedDataFrame> JDx12GraphicObjectDataSetManager::Add(const JGraphicModuleManagedDataCreationDesc& desc)
	{
		JUniquePtr<JGraphicObjectDataSetBase> newUnique;
		JFastPtr<JGraphicObjectDataSetBase> result;

		const JWeakPtr<JObject> obj = desc.object;
		const J_OBJECT_TYPE objType = obj->GetObjectType();
		UniqueIndex index = invalidIndex;

		if (objType == J_OBJECT_TYPE::COMPONENT_OBJECT)
		{
			const J_COMPONENT_TYPE compType = Core::ConnectChildUserPtr<JComponent>(obj)->GetComponentType();
			switch (compType)
			{
			case JinEngine::J_COMPONENT_TYPE::USER_BEHAVIOR:
			{
				//추가필요
				break;
			}
			case JinEngine::J_COMPONENT_TYPE::ENGINE_ANIMATOR:
			{
				newUnique = Core::JPtrUtil::MakeUniquePtr<AnimatorDataSet>(desc, std::make_unique<Animator::FrameInterface>(), this);
				index = ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_ANIMATOR>();
				break;
			}
			case JinEngine::J_COMPONENT_TYPE::ENGINE_CAMERA:
			{
				newUnique = Core::JPtrUtil::MakeUniquePtr<CameraDataSet>(desc,
					std::make_unique<Camera::CsmTargetInterface>(),
					std::make_unique<Camera::CullingInterface>(),
					std::make_unique<Camera::FrameInterface>(),
					std::make_unique<Camera::GraphicResourceInterface>(),
					this);
				index = ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_CAMERA>();
				break;
			}
			case JinEngine::J_COMPONENT_TYPE::ENGINE_LIGHT:
			{
				const J_LIGHT_TYPE litType = Core::ConnectChildUserPtr<JLight>(obj)->GetLightType();
				switch (litType)
				{
				case JinEngine::J_LIGHT_TYPE::DIRECTIONAL:
				{
					newUnique = Core::JPtrUtil::MakeUniquePtr<DirectionalLightDataSet>(desc,
						std::make_unique<Light::Directional::CsmHandlerInterface>(),
						std::make_unique<Light::Directional::CullingInterface>(),
						std::make_unique<Light::Directional::FrameInterface>(),
						std::make_unique<Light::Directional::GraphucResourceInterafce>(),
						this);
					index = ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_LIGHT>(J_LIGHT_TYPE::DIRECTIONAL);
					break;
				}
				case JinEngine::J_LIGHT_TYPE::POINT:
				{
					newUnique = Core::JPtrUtil::MakeUniquePtr<PointLightDataSet>(desc,
						std::make_unique<Light::Point::CullingInterface>(),
						std::make_unique<Light::Point::FrameInterface>(),
						std::make_unique<Light::Point::GraphucResourceInterafce>(),
						this);
					index = ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_LIGHT>(J_LIGHT_TYPE::POINT);
					break;
				}
				case JinEngine::J_LIGHT_TYPE::SPOT:
				{
					newUnique = Core::JPtrUtil::MakeUniquePtr<SpotLightDataSet>(desc,
						std::make_unique<Light::Spot::CullingInterface>(),
						std::make_unique<Light::Spot::FrameInterface>(),
						std::make_unique<Light::Spot::GraphucResourceInterafce>(),
						this);
					index = ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_LIGHT>(J_LIGHT_TYPE::SPOT);
					break;
				}
				case JinEngine::J_LIGHT_TYPE::RECT:
				{
					newUnique = Core::JPtrUtil::MakeUniquePtr<RectLightDataSet>(desc,
						std::make_unique<Light::Rect::CullingInterface>(),
						std::make_unique<Light::Rect::FrameInterface>(),
						std::make_unique<Light::Rect::GraphucResourceInterafce>(),
						this);
					index = ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_LIGHT>(J_LIGHT_TYPE::RECT);
					break;
				}
				default:
					break;
				}
				break;
			}
			case JinEngine::J_COMPONENT_TYPE::ENGINE_RENDERITEM:
			{
				newUnique = Core::JPtrUtil::MakeUniquePtr<RenderItemDataSet>(desc, std::make_unique<RenderItem::FrameInterface>(), this);
				index = ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_RENDERITEM>();
				break;
			}
			case JinEngine::J_COMPONENT_TYPE::ENGINE_TRANSFORM:
			{
				newUnique = Core::JPtrUtil::MakeUniquePtr<TransformDataSet>(desc, std::make_unique<Transform::FrameInterface>(), this);
				index = ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_TRANSFORM>();
				break;
			}
			default:
				break;
			} 
			result = newUnique;
			if (newUnique != nullptr)
				set[index].Add(std::move(newUnique));
		}
		else if (objType == J_OBJECT_TYPE::RESOURCE_OBJECT)
		{
			const J_RESOURCE_TYPE resourceType = Core::ConnectChildUserPtr<JResourceObject>(obj)->GetResourceType();
			switch (resourceType)
			{
			case JinEngine::J_RESOURCE_TYPE::MATERIAL:
			{
				newUnique = Core::JPtrUtil::MakeUniquePtr<MaterialDataSet>(desc, std::make_unique<Material::FrameInterface>(), this);
				index = ConvertResourceUniqueIndex<J_RESOURCE_TYPE::MATERIAL>();
				break;
			}
			case JinEngine::J_RESOURCE_TYPE::MESH:
			{
				const Core::J_MESHGEOMETRY_TYPE meshType = Core::ConnectChildUserPtr<JMeshGeometry>(obj)->GetMeshGeometryType();
				if (meshType == Core::J_MESHGEOMETRY_TYPE::STATIC)
				{
					newUnique = Core::JPtrUtil::MakeUniquePtr<StaticMeshDataSet>(desc, std::make_unique<Mesh::Static::GraphucResourceInterafce>(), this);
					index = ConvertResourceUniqueIndex<J_RESOURCE_TYPE::MESH>(Core::J_MESHGEOMETRY_TYPE::STATIC);
				}
				else if (meshType == Core::J_MESHGEOMETRY_TYPE::SKINNED)
				{
					newUnique = Core::JPtrUtil::MakeUniquePtr<SkinnedMeshDataSet>(desc, std::make_unique<Mesh::Skinned::GraphucResourceInterafce>(), this);
					index = ConvertResourceUniqueIndex<J_RESOURCE_TYPE::MESH>(Core::J_MESHGEOMETRY_TYPE::SKINNED);
				}
				break;
			}
			case JinEngine::J_RESOURCE_TYPE::TEXTURE:
			{
				newUnique = Core::JPtrUtil::MakeUniquePtr<TextureDataSet>(desc, std::make_unique<Texture::GraphucResourceInterafce>(), this);
				index = ConvertResourceUniqueIndex<J_RESOURCE_TYPE::TEXTURE>();
				break;
			}
			case JinEngine::J_RESOURCE_TYPE::SCENE:
			{
				newUnique = Core::JPtrUtil::MakeUniquePtr<SceneDataSet>(desc,
					std::make_unique<Scene::FrameInterface>(),
					std::make_unique<Scene::GpuAcceleratorInterface>(),
					this);
				index = ConvertResourceUniqueIndex<J_RESOURCE_TYPE::SCENE>();
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
			result = newUnique;
			if (newUnique != nullptr)
				set[index + totalCompVariation].Add(std::move(newUnique));
		} 

		//Develop::JDevelopDebug::PushLog(obj->GetTypeInfo().Name() + " Add");
		//Develop::JDevelopDebug::Write();
		return result;
	}
	bool JDx12GraphicObjectDataSetManager::Remove(JFastPtr<JGraphicModuleManagedDataFrame>& data)
	{
		if (data == nullptr)
			return false;

		auto removeLam = [](JUniquePtr<JGraphicObjectDataSetBase>* owner, const size_t guid)
		{
			return (*owner)->Object()->GetGuid() == guid;
		};
		bool(*removePtr)(JUniquePtr<JGraphicObjectDataSetBase>*, size_t) = removeLam;

		const JWeakPtr<JObject> obj = data->Object();
		const J_OBJECT_TYPE objType = obj->GetObjectType();
		data.Release();
	 
		//Develop::JDevelopDebug::PushLog(obj->GetTypeInfo().Name() + " Remove");
		//Develop::JDevelopDebug::Write();

		const UniqueIndex typeIndex = GetUniqueIndex(obj);  
		return set[typeIndex].Remove(set[typeIndex].GetIndex(removePtr, obj->GetGuid()));
	}
	void JDx12GraphicObjectDataSetManager::NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& optionChangedSet)
	{
		for (uint i = 0; i < (uint)J_GRAPHIC_OPTIONAL_FEATURE::COUNT; ++i)
		{
			const J_GRAPHIC_OPTIONAL_FEATURE type = (J_GRAPHIC_OPTIONAL_FEATURE)i;
			const bool* preValuePtr = optionChangedSet.preOption.GetOptionalFeatureValuePtr(type);
			const bool* newValuePtr = optionChangedSet.newOption.GetOptionalFeatureValuePtr(type);

			if (preValuePtr == nullptr || *preValuePtr == *newValuePtr)
				continue;

			auto funcMap = GMI()->GetObserverFuncMap(type);
			for (const auto& funcData : funcMap)
			{
				if (funcData.second == nullptr)
					continue;
				 
				const uint count = set[funcData.first].Count();
				for (uint i = 0; i < count; ++i)
					funcData.second((*set[funcData.first].Get(i))->Object().Get(), *newValuePtr);
			}
		}  
	}
	void JDx12GraphicObjectDataSetManager::NotifyGraphicResourceCreation(JGraphicObjectDataSetBase* base, const JUserPtr<JGraphicResourceInfo>& newInfo, const J_GRAPHIC_TASK_TYPE task)
	{
	}
	void JDx12GraphicObjectDataSetManager::BuildResource(JGraphicDevice* device)
	{

	}
	void JDx12GraphicObjectDataSetManager::CreateMetadata()
	{  
		using AnimatorMetadata = JCompTypePerInterfaceMetadata<J_COMPONENT_TYPE::ENGINE_ANIMATOR, 0, Core::JEmptyType, Animator::FrameInterface, Core::JEmptyType, Core::JEmptyType>;
		using BehaviorMetadata = JCompTypePerInterfaceMetadata< J_COMPONENT_TYPE::USER_BEHAVIOR, 0, Core::JEmptyType, Core::JEmptyType, Core::JEmptyType, Core::JEmptyType>;
		using CameraMetadata = JCompTypePerInterfaceMetadata<J_COMPONENT_TYPE::ENGINE_CAMERA, 0, Camera::CullingInterface, Camera::FrameInterface, Core::JEmptyType, Camera::GraphicResourceInterface>;
		using DLightMetadata = JCompTypePerInterfaceMetadata<J_COMPONENT_TYPE::ENGINE_LIGHT, (uint)J_LIGHT_TYPE::DIRECTIONAL, Light::Directional::CullingInterface, Light::Directional::FrameInterface, Core::JEmptyType, Light::Directional::GraphucResourceInterafce>;
		using PLightMetadata = JCompTypePerInterfaceMetadata<J_COMPONENT_TYPE::ENGINE_LIGHT, (uint)J_LIGHT_TYPE::POINT, Light::Point::CullingInterface, Light::Point::FrameInterface, Core::JEmptyType, Light::Point::GraphucResourceInterafce>;
		using SLightMetadata = JCompTypePerInterfaceMetadata<J_COMPONENT_TYPE::ENGINE_LIGHT, (uint)J_LIGHT_TYPE::SPOT, Light::Spot::CullingInterface, Light::Spot::FrameInterface, Core::JEmptyType, Light::Spot::GraphucResourceInterafce>;
		using RLightMetadata = JCompTypePerInterfaceMetadata<J_COMPONENT_TYPE::ENGINE_LIGHT, (uint)J_LIGHT_TYPE::RECT, Light::Rect::CullingInterface, Light::Rect::FrameInterface, Core::JEmptyType, Light::Rect::GraphucResourceInterafce>;
		using RenderItemMetadata = JCompTypePerInterfaceMetadata<J_COMPONENT_TYPE::ENGINE_RENDERITEM, 0, Core::JEmptyType, RenderItem::FrameInterface, Core::JEmptyType, Core::JEmptyType>;
		using TransformMetadata = JCompTypePerInterfaceMetadata<J_COMPONENT_TYPE::ENGINE_TRANSFORM, 0, Core::JEmptyType, Transform::FrameInterface, Core::JEmptyType, Core::JEmptyType>;

		using MaterialMetadata = JResourceTypePerInterfaceMetadata<J_RESOURCE_TYPE::MATERIAL, 0, Core::JEmptyType, Material::FrameInterface, Core::JEmptyType, Core::JEmptyType>;
		using StaticMeshMetadata = JResourceTypePerInterfaceMetadata<J_RESOURCE_TYPE::MESH, (uint)Core::J_MESHGEOMETRY_TYPE::STATIC, Core::JEmptyType, Core::JEmptyType, Core::JEmptyType, Mesh::Static::GraphucResourceInterafce>;
		using SkinnedMeshMetadata = JResourceTypePerInterfaceMetadata<J_RESOURCE_TYPE::MESH, (uint)Core::J_MESHGEOMETRY_TYPE::SKINNED, Core::JEmptyType, Core::JEmptyType, Core::JEmptyType, Mesh::Skinned::GraphucResourceInterafce>;

		using SceneMetadata = JResourceTypePerInterfaceMetadata<J_RESOURCE_TYPE::SCENE, 0, Core::JEmptyType, Scene::FrameInterface, Core::JEmptyType, Core::JEmptyType>;
		using TextureMetadata = JResourceTypePerInterfaceMetadata<J_RESOURCE_TYPE::TEXTURE, 0, Core::JEmptyType, Core::JEmptyType, Core::JEmptyType, Texture::GraphucResourceInterafce>;
	 
		AnimatorMetadata::Register(metadata);
		BehaviorMetadata::Register(metadata);
		CameraMetadata::Register(metadata);
		DLightMetadata::Register(metadata);
		PLightMetadata::Register(metadata);
		SLightMetadata::Register(metadata);
		RLightMetadata::Register(metadata);
		RenderItemMetadata::Register(metadata);
		TransformMetadata::Register(metadata);

		MaterialMetadata::Register(metadata);
		StaticMeshMetadata::Register(metadata);
		SkinnedMeshMetadata::Register(metadata);
		SceneMetadata::Register(metadata);
		TextureMetadata::Register(metadata);
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