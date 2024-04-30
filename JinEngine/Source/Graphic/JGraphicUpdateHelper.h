#pragma once 
#include"GraphicResource/JGraphicResourceType.h" 
#include"GraphicResource/JGraphicResourceInterface.h"
#include"FrameResource/JFrameResourceEnum.h"
#include"Culling/JCullingInterface.h"   
#include"Culling/JCullingUserAccess.h"   
#include"FrameResource/JFrameIndexAccess.h"
#include"../Object/Component/JComponentType.h"
#include"../Object/Component/RenderItem/JRenderLayer.h"
#include"../Core/Geometry/Mesh/JMeshType.h"
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

		using GameObjectVec = std::vector<JUserPtr<JGameObject>>;
		class JUpdateHelper
		{
		public:
			using GetElementCountT = Core::JStaticCallableType<uint>; 
			using GetElementCapacityT = Core::JStaticCallableType<uint>;
			using ReBuildUploadDataT = Core::JStaticCallableType<uint>;
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
				//buffer count
				uint count = 0;
				//buffer capacity
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
			public: 
				uint uploadCountPerTarget = 0;
				uint uploadOffset = 0;
				uint setDirty = 0;
			public:   
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
			bool hasUploadDataDirty;
			bool hasBindingDataDirty; 
		public: 
			//std::vector<std::unique_ptr<GetElementMultiCountT::Callable>> getElementMultiCount;
		public:
			void BeginUpdatingDrawTarget();
			void EndUpdatingDrawTarget();
		public:
			void Clear();
			void RegisterCallable(J_UPLOAD_FRAME_RESOURCE_TYPE type, GetElementCountT::Ptr getCountPtr);
			void RegisterCallable(J_GRAPHIC_RESOURCE_TYPE type, GetElementCountT::Ptr* getCountPtr, GetElementCapacityT::Ptr* getCapaPtr);
			//void RegisterCallable(GetElementMultiCountT::Ptr getMultiCountPtr);
			void WriteGraphicInfo(JGraphicInfo& info)const noexcept; 
		}; 
		class JGameObjectBuffer
		{
		public:
			using OpaqueVec = std::vector<JUserPtr<JGameObject>>;
			using OpaqueVecPerCam = std::vector<OpaqueVec>;
		public:
			OpaqueVec common;
			OpaqueVecPerCam aligned;		//applied frustum culling
		public:
			void ClearAlignedVecElement();
		};
		//draw data
		class JDrawHelper : public JFrameIndexAccess
		{
		private:
			friend class JGraphic;
		public:
			enum class DRAW_TYPE
			{
				SCENE,
				SHADOW_MAP,
				FRUSTUM_CULLING,
				OCC,
				LIT_CULLING
			};
		public:
			const JGraphicInfo& info;
			const JGraphicOption& option;
			JGameObjectBuffer& objVec;	//for hd
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
			J_COMPONENT_TYPE cullingCompType; 
		public:
			bool allowDrawShadowMap = false;
			bool allowDrawDebugMap = false;
			bool allowDrawDebugObject = false;		//draw outline and debug layer object
			bool allowFrustumCulling = true;
			bool allowHzbOcclusionCulling = true;  
			bool allowHdOcclusionCulling = true;
			bool allowDrawOccDepthMap = false;
			bool allowMutilthreadDraw = false;
			bool allowLightCulling = false;
			bool allowLightCullingDebug = false; 
			bool allowSsao = false;
			bool allowPostProcess = false;
			bool allowRtGi = false;
			bool allowTemporalProcess = false;
		public:
			bool RefelectOtherCamCullig(const uint rItemIndex)const noexcept;  
		public:
			JGraphicResourceUserInterface GetOccGResourceInterface()const noexcept;
			JCullingUserInterface GetCullInterface()const noexcept;
			JCullingUserAccess* GetCullingUserAccess()const noexcept;
			DRAW_TYPE GetDrawType()const noexcept;
		public:
			int GetSceneFrameIndex()const noexcept;
			int GetCamFrameIndex(const uint frameLayerIndex)const noexcept;
			int GetLitFrameIndex(const uint frameLayerIndex)const noexcept;
			int GetLitShadowFrameIndex()const noexcept;
			const std::vector<JUserPtr<JGameObject>>& GetGameObjectCashVec(const J_RENDER_LAYER rLayer, const Core::J_MESHGEOMETRY_TYPE meshType)const noexcept;
		public:
			void SetDrawTarget(JGraphicDrawTarget* drawTarget)noexcept;
			void SetTheadInfo(const uint threadCount, const uint threadIndex)noexcept;
			void SetAllowMultithreadDraw(const bool value)noexcept;
		public: 
			void SettingDrawShadowMap(const JWeakPtr<JLight>& lit)noexcept;
			void SettingDrawScene(const JWeakPtr<JCamera>& cam)noexcept;
			void SettingFrustumCulling(const JWeakPtr<JComponent>& comp)noexcept;
			void SettingOccCulling(const JWeakPtr<JComponent>& comp)noexcept;
			void SettingLightCulling(const JWeakPtr<JCamera>& cam);
		public:  
			bool CanDispatchWorkIndex()const noexcept; 
			bool UsePerspectiveProjection()const noexcept;
		public: 
			void DispatchWorkIndex(const uint count, _Out_ uint& stIndex, _Out_ uint& edIndex)const noexcept;
		public:
			static JDrawHelper CreateDrawSceneHelper(const JDrawHelper& ori, const JWeakPtr<JCamera>& cam)noexcept;
			static JDrawHelper CreateDrawShadowMapHelper(const JDrawHelper& ori, const JWeakPtr<JLight>& lit)noexcept;
			static JDrawHelper CreateFrustumCullingHelper(const JDrawHelper& ori, const JWeakPtr<JComponent>& comp)noexcept;
			static JDrawHelper CreateOccCullingHelper(const JDrawHelper& ori, const JWeakPtr<JComponent>& comp)noexcept;
			static JDrawHelper CreateLitCullingHelper(const JDrawHelper& ori, const JWeakPtr<JCamera>& cam)noexcept;
		private:
			JDrawHelper(const JGraphicInfo& info, const JGraphicOption& option, JGameObjectBuffer& objVec);
		};
		//draw detail condition
		struct JDrawCondition
		{
		public:
			//for i = drawSt, i < drawEd, 
			uint drawSt = 0;
			uint drawEd = 0;
		public:
			bool restrictRange = false;
		public:
			bool allowAnimation = false;
		public: 
			bool allowCulling = false;
			bool allowOcclusionCulling = false; 
		public:
			bool allowOutline = false;
			bool allowAllCullingResult = false;	//for check other cam spacespatial    
		public:
			bool onlyDrawOccluder = false; //it is valid in hzb, hd occluder draw
		public: 
			JDrawCondition() = default;
			JDrawCondition(const JDrawHelper& helper,
				const bool newAllowAnimation,
				const bool newAllowCulling,
				const bool newAllowDebugOutline,
				const bool onlyDrawOccluder = false);
		public:
			void SetRestrictRange(const uint st, const uint count);
		public:
			bool IsValidDrawingIndex(const uint drawIndex)const noexcept;
		}; 
	}
}