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
#include"GraphicResource/JGraphicResourceType.h" 
#include"GraphicResource/JGraphicResourceInterface.h" 
#include"Culling/JCullingInterface.h"      
#include"DataSet/JGraphicObjectDataSet.h"
#include"FrameResource/JFrameResourceType.h"
#include"../Object/Component/JComponentType.h"
#include"../Object/Component/Light/JLightType.h"
#include"../Object/Component/RenderItem/JRenderLayer.h" 
#include"../Core/Geometry/Mesh/JMeshType.h" 

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

		enum class J_GRAPHIC_CAPACITY_CONDITION
		{
			KEEP,
			DOWN_CAPACITY,
			UP_CAPACITY
		};
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
				static constexpr uint defaultDownCapacityCountMax = 16;
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
				J_GRAPHIC_CAPACITY_CONDITION reAllocCondition;
			};
			struct UploadUpdateData : public UpdateDataBase
			{  
			};
			struct BindingTextureData : public UpdateDataBase
			{  
			};
		public:
			UploadUpdateData uData[(int)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT];	//frame upload resource 
			BindingTextureData bData[(int)J_GRAPHIC_RESOURCE_TYPE::COUNT];
			bool hasUploadDataDirty;
			bool hasBindingDataDirty; 
		public: 
			//std::vector<std::unique_ptr<GetElementMultiCountT::Callable>> getElementMultiCount;
		public:
			void Begin();
			void End();
		public:
			void Clear(); 
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
		class JDrawHelper  
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
			/*
			* Camera 혹은 Light는 Render target 그리고 Depth map이 될 수 있는 GraphicResource를
			* 소유할 수 있으며 패러미터를 조정해서 결과값에 반영할 수 있는 Component이다.
			* 따라서 Rasterize방식의 렌더링시 두 개의 컴포넌트중 하나가 반드시 필요하다.
			* (Light는 그 밖에 Shading연산에 사용되며 Render target 그리고 Depth map이 되는 경우는
			* 주로 Shadow map이나 Light culling시 필요한 Shape drawing pass이다)
			*/
			JWeakPtr<JScene> scene = nullptr;
			//can single occ(one cam)
			JWeakPtr<JCamera> cam = nullptr;
			//can multi occ(multi cam * one dirctional light)
			JWeakPtr<JLight> lit = nullptr;  
		private:
			JComponent* comp = nullptr;
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
			/**
			* @return cam or lit ResourceInterface
			*/
			JGraphicResourceInterface* GetResourceInterface()const noexcept;
			/**
			* @return cam or lit CullInterface
			*/
			JCullingInterface* GetCullInterface()const noexcept;
			/**
			* @return cam or lit FrameInterface
			*/
			JFrameUpdateInterface* GetFrameInterface()const noexcept;
			/**
			* @return cam or lit GpuAcceleatorInterface
			*/
			JGpuAcceleratorInterface* GetGpuAcceleratorInterface()const noexcept;
			JGraphicObjectDataSetBase* GetObjectDataSet()const noexcept;
			DRAW_TYPE GetDrawType()const noexcept;
		public:
			int GetSceneFrameIndex(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept;
			int GetCamFrameIndex(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept;
			int GetLitFrameIndex(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept;
			int GetLitShadowFrameIndex(const J_SHADOW_MAP_TYPE smType)const noexcept;
			const std::vector<JUserPtr<JGameObject>>& GetGameObjectCacheVec(const J_RENDER_LAYER rLayer, const Core::J_MESHGEOMETRY_TYPE meshType)const noexcept;
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
		private:
			void SetCamera(const JWeakPtr<JCamera>& newCam)noexcept;
			void SetLight(const JWeakPtr<JLight>& newLit)noexcept;
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