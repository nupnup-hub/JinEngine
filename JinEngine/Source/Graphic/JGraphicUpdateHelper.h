#pragma once 
#include"GraphicResource/JGraphicResourceType.h" 
#include"GraphicResource/JGraphicResourceInterface.h"
#include"FrameResource/JFrameResourceEnum.h"
#include"Culling/JCullingInterface.h"   
#include"Culling/JCullingUserAccess.h"   
#include"../Object/Component/JComponentType.h"
#include"../Object/Component/RenderItem/JRenderLayer.h"
#include"../Object/Resource/Mesh/JMeshType.h"
#include<vector>
#include<memory>

namespace JinEngine
{
	class JScene;
	class JComponent;
	class JGameObject;
	class JCamera;
	class JLight;  
	class JRenderItem;
	class JAnimator;
	namespace Core
	{
		class JTypeInfo;
	}
	namespace Graphic
	{
		struct JGraphicInfo;
		struct JGraphicOption;
		class JGraphicDrawTarget;
		class JGraphic; 
		struct JUpdateHelper
		{
		public:
			using GetElementCountT = Core::JStaticCallableType<uint>;
			using GetElementMultiCountT = Core::JStaticCallableType<void>;
			using GetElementCapacityT = Core::JStaticCallableType<uint>;
			using ReBuildUploadDataT = Core::JStaticCallableType<uint>;
			using NotifyUpdateCapacityT = Core::JStaticCallableType<void>; 
		public:
			using SetCapacityT = Core::JStaticCallableType<void>;
		public:
			struct UpdateDataBase
			{
			private:
				static constexpr float defaultUpCapacityFactor = 2.0f;
				static constexpr float defaultDownCapacityFactor = 2.0f;
				static constexpr uint defaultDownCapacityCountMax = 3600;
			public:
				uint count = 0;
				uint capacity = 0;
			public:
				float upCapacityFactor = defaultUpCapacityFactor;
			public:
				uint downCapacityCountMax = defaultDownCapacityCountMax;	//
				uint downCapacityCount = 0;		//count per frame if count over (downCapacityFactor * capacity) count is zero
				float downCapacityFactor = defaultDownCapacityFactor;		//if count uder capacity / downCapacityFactor  start counting
			public:
				J_UPLOAD_CAPACITY_CONDITION reAllocCondition;
			};
			struct UploadUpdateData : public UpdateDataBase
			{
			public:
				std::unique_ptr<GetElementCountT::Callable> getElement = nullptr;
				std::vector<std::unique_ptr<NotifyUpdateCapacityT::Callable>> notifyUpdateCapacity;
			public: 
				//accumulated upload count
				//필요한 upload data만 사용
				uint offset = 0;
				uint setDirty = 0; 
				bool useGetMultiCount = true;
			};
			struct BindingTextureData : public UpdateDataBase
			{
			public:
				std::unique_ptr<GetElementCountT::Callable> getTextureCount = nullptr;
				std::unique_ptr<GetElementCapacityT::Callable> getTextureCapacity = nullptr; 
			public: 
				bool hasCallable = false;
			public:
				bool HasCallable()const noexcept;
			};
		public:
			UploadUpdateData uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT];	//frame upload resource 
			BindingTextureData bData[(int)J_GRAPHIC_RESOURCE_TYPE::COUNT];
			bool hasRebuildCondition;
			bool hasRecompileShader;
		public:
			std::vector<std::unique_ptr<GetElementMultiCountT::Callable>> getElementMultiCount;
		public:
			void Clear();
			void RegisterCallable(J_UPLOAD_FRAME_RESOURCE_TYPE type, GetElementCountT::Ptr getCountPtr);
			void RegisterCallable(J_GRAPHIC_RESOURCE_TYPE type, GetElementCountT::Ptr* getCountPtr, GetElementCapacityT::Ptr* getCapaPtr);
			void RegisterCallable(GetElementMultiCountT::Ptr getMultiCountPtr);
			void RegisterListener(J_UPLOAD_FRAME_RESOURCE_TYPE type, std::unique_ptr<NotifyUpdateCapacityT::Callable>&& listner);
			void WriteGraphicInfo(JGraphicInfo& info)const noexcept;
			void NotifyUpdateFrameCapacity(JGraphic& grpahic);
		}; 
		struct JAlignedObject
		{
		public:
			using ObjectVec = std::vector<JUserPtr<JGameObject>>;
		public:
			std::vector<ObjectVec>opaqueVec;
		};
		//draw data
		struct JDrawHelper
		{
		private:
			friend class JGraphic;
		public:
			enum class DRAW_TYPE
			{
				SCENE,
				SHADOW_MAP,
				OCC
			};
		public:
			const JGraphicInfo& info;
			const JGraphicOption& option;
			const JAlignedObject& alignedObj;	//for hd
		public:
			JGraphicDrawTarget* drawTarget = nullptr;  
		public:
			JWeakPtr<JScene> scene = nullptr;
			//can single occ(one cam)
			JWeakPtr<JCamera> cam = nullptr;
			//can multi occ(multi cam * one dirctional light)
			JWeakPtr<JLight> lit = nullptr;    
		public:
			int threadCount = -1;
			int threadIndex = -1;
		private:
			DRAW_TYPE drawType = DRAW_TYPE::SCENE;
		public:  
			J_COMPONENT_TYPE occCompType; 
		public:
			bool allowDrawDepthMap = false;
			bool allowDrawDebug = false;		//draw outline and debug layer object
			bool allowFrustumCulling = true;
			bool allowOcclusionCulling = true;
			bool allowDrawOccDepthMap= false;	//for debug
			bool allowMutilthreadDraw = false;
		public:
			bool RefelectOtherCamCullig(const uint rItemIndex)const noexcept;  
		public:
			JGraphicResourceUserInterface GetOccGResourceInterface()const noexcept;
			JCullingUserInterface GetCullInterface()const noexcept;
			JCullingUserAccess* GetCullingUserAccess()const noexcept;
			DRAW_TYPE GetDrawType()const noexcept;
		public:
			int GetPassFrameIndex()const noexcept;
			int GetCamFrameIndex()const noexcept;
			int GetCamDepthTestPassFrameIndex()const noexcept;
			int GetCamHzbOccComputeFrameIndex()const noexcept;
			int GetShadowMapDrawFrameIndex()const noexcept;
			int GetLitDepthTestPassFrameIndex()const noexcept;
			int GetLitHzbOccComputeFrameIndex()const noexcept;
			const std::vector<JUserPtr<JGameObject>>& GetGameObjectCashVec(const J_RENDER_LAYER rLayer, const J_MESHGEOMETRY_TYPE meshType)const noexcept;
		public:
			static int GetObjectFrameIndex(JRenderItem* rItem)noexcept;
			static int GetBoundingFrameIndex(JRenderItem* rItem)noexcept;
			static int GetAnimationFrameIndex(JAnimator* ani)noexcept;
		public:
			void SetDrawTarget(JGraphicDrawTarget* drawTarget)noexcept;
			void SetTheadInfo(const uint threadCount, const uint threadIndex)noexcept;
			void SetAllowMultithreadDraw(const bool value)noexcept;
		public: 
			void SettingOccCulling(const JWeakPtr<JComponent>& comp)noexcept;
			void SettingDrawShadowMap(const JWeakPtr<JLight>& lit)noexcept;
			void SettingDrawScene(const JWeakPtr<JCamera>& cam)noexcept;
		public:
			bool CanDrawShadowMap()const noexcept;
			bool CanOccCulling()const noexcept;
			bool CanDispatchWorkIndex()const noexcept;  
		public:
			void DispatchWorkIndex(const uint count, _Out_ uint& stIndex, _Out_ uint& edIndex)const noexcept;
		public:
			static JDrawHelper CreateDrawSceneHelper(const JDrawHelper& ori, const JWeakPtr<JCamera>& cam)noexcept;
			static JDrawHelper CreateDrawShadowMapHelper(const JDrawHelper& ori, const JWeakPtr<JLight>& lit)noexcept;
			static JDrawHelper CreateOccCullingHelper(const JDrawHelper& ori, const JWeakPtr<JComponent>& comp)noexcept;
		private:
			JDrawHelper(const JGraphicInfo& info, const JGraphicOption& option, const JAlignedObject& alignedObj);
		};
		//draw detail condition
		struct JDrawCondition
		{
		public:
			bool allowAnimation = false;
		public: 
			bool allowCulling = false;
			bool allowHzbOcclusionCulling = false;
			bool allowHDOcclusionCulling = false;
		public:
			bool allowDebugOutline = false;
			bool allowAllCullingResult = false;	//for check other cam spacespatial   
		public: 
			JDrawCondition() = default;
			JDrawCondition(const JDrawHelper& helper,
				const bool newAllowAnimation,
				const bool newAllowCulling,
				const bool newAllowDebugOutline);
		}; 
	}
}