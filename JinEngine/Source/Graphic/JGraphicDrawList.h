#pragma once
#include"JGraphicDrawListType.h"
#include"../Core/JCoreEssential.h"
#include"../Object/Component/JComponentType.h"  
#include<deque> 
#include<vector>
#include<memory>

struct ID3D12Device;
namespace JinEngine
{
	class JComponent;
	class JScene; 
	class JSceneManager;
	class JLight;
	class JCamera;  

	namespace Graphic
	{
		class JGraphicResourceInfo;  
		class JGraphicDrawList;
		class JGraphicSingleResourceInterface;
		class JGraphicMultoResourceInterface;
		class JGraphic;
		class JFrameUpdateUserAccess;

		class JShadowMapDrawRequestor
		{
		public:
			JUserPtr<JLight> jLight;
			const J_GRAPHIC_DRAW_FREQUENCY updateFrequency;
		public:
			bool isUpdated = false;
			bool canDrawThisFrame = false; 
		public:
			JShadowMapDrawRequestor(const JUserPtr<JLight>& jLight, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency);
			~JShadowMapDrawRequestor();
		};
		class JSceneDrawRequestor
		{
		public:
			JUserPtr<JCamera> jCamera;
			const J_GRAPHIC_DRAW_FREQUENCY updateFrequency;
		public:
			bool isUpdated = false;
			bool canDrawThisFrame = false; 
		public:
			JSceneDrawRequestor(const JUserPtr<JCamera>& jCamera, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency);
			~JSceneDrawRequestor();
		};
		class JSceneFrustumCullingRequestor
		{
		public:
			JUserPtr<JComponent> comp;
			const J_GRAPHIC_DRAW_FREQUENCY updateFrequency;
		public:
			bool isUpdated = false;
			bool canDrawThisFrame = false;
			bool keepCanDrawTrigger = false; 
		public:
			JSceneFrustumCullingRequestor(const JUserPtr<JComponent>& comp, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency);
			~JSceneFrustumCullingRequestor();
		};
		class JSceneHzbOccCullingRequestor
		{
		public:
			JUserPtr<JComponent> comp;
			const J_GRAPHIC_DRAW_FREQUENCY updateFrequency;
		public:
			bool isUpdated = false;
			bool canDrawThisFrame = false;
			bool keepCanDrawTrigger = false; 
		public:
			JSceneHzbOccCullingRequestor(const JUserPtr<JComponent>& comp, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency);
			~JSceneHzbOccCullingRequestor();
		};
		class JSceneHdOccCullingRequestor
		{
		public:
			JUserPtr<JComponent> comp;
			const J_GRAPHIC_DRAW_FREQUENCY updateFrequency;
		public:
			bool isUpdated = false;
			bool canDrawThisFrame = false;
			bool keepCanDrawTrigger = false;
		public:
			JSceneHdOccCullingRequestor(const JUserPtr<JComponent>& comp, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency);
			~JSceneHdOccCullingRequestor();
		};

		class JGraphicDrawTarget
		{
		public:
			struct UpdateInfo
			{
			private:
				friend class JGraphicDrawList;
				friend class JGraphicDrawTarget;
			private:
				JUserPtr<JFrameUpdateUserAccess> observationFrame;		// for resource preview scene
			public:
				uint objUpdateCount = 0;
				uint aniUpdateCount = 0;
				uint camUpdateCount = 0;
				uint lightUpdateCount = 0;
				uint shadowMapUpdateCount = 0;
				uint hzbOccUpdateCount = 0;
				uint hdOccUpdateCount = 0;
			public:
				//frame dirty = gNumFrameDirty
				uint hotObjUpdateCount = 0;
				uint hotAniUpdateCount = 0;
				uint hotCamUpdateCount = 0;
				uint hotLitghtUpdateCount = 0;
			public:
				bool hasObjRecopy = false;
			public:
				bool sceneUpdated = false;
				bool shadowUpdated = false;  
				bool occCullingUpdated = false;
			public:
				bool nextSceneUpdate = false;
			private:
				void UpdateStart();
				void UpdateEnd();
			};
		private:
			friend class JGraphicDrawList;
		public:
			JUserPtr<JScene> scene;
			std::unique_ptr<UpdateInfo> updateInfo;
			std::vector<std::unique_ptr<JShadowMapDrawRequestor>> shadowRequestor;
			std::vector<std::unique_ptr<JSceneDrawRequestor>> sceneRequestor;
			std::vector<std::unique_ptr<JSceneFrustumCullingRequestor>> frustumCullingRequestor;
			std::vector<std::unique_ptr<JSceneHzbOccCullingRequestor>> hzbOccCullingRequestor;
			std::vector<std::unique_ptr<JSceneHdOccCullingRequestor>> hdOccCullingRequestor;
		public:
			JGraphicDrawTarget(const JUserPtr<JScene>& scene);
			~JGraphicDrawTarget();
		public:
			void UpdateStart();
			void UpdateEnd();
		};
		 
		class JGraphicDrawList 
		{ 
		private:
			friend class JGraphic;
			friend class JGraphicDrawListSceneInterface;
			friend class JGraphicDrawListCompInterface; 
		private:
			//drawList
			static bool AddDrawList(const JUserPtr<JScene>& scene)noexcept;
			static bool PopDrawList(const JUserPtr<JScene>& scene)noexcept;
			static bool HasDrawList(const JUserPtr<JScene>& scene)noexcept;
			static void UpdateScene(const JUserPtr<JScene>& scene, const J_COMPONENT_TYPE cType)noexcept;
		private:
			//option
			static bool AddObservationFrame(const JUserPtr<JScene>& scene, const JUserPtr<JFrameUpdateUserAccess>& observationFrame)noexcept;
		private:
			//request
			static void AddDrawShadowRequest(const JUserPtr<JScene>& scene, const JUserPtr<JLight>& jLight)noexcept;
			static void AddDrawSceneRequest(const JUserPtr<JScene>& scene, const JUserPtr<JCamera>& jCamera, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency)noexcept;
			static void AddFrustumCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency)noexcept;
			static void AddHzbOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency)noexcept;
			static void AddHdOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency)noexcept;
			static void PopDrawRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept;
			static void PopFrustumCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept;
			static void PopHzbOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept;
			static void PopHdOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept;
			static bool HasRequestor(const JUserPtr<JScene>& scene)noexcept;
		private:
			static uint GetListCount()noexcept;
			static JGraphicDrawTarget* GetDrawScene(const uint index)noexcept; 
			static JSceneDrawRequestor* GetSceneDrawRequestor(const size_t guid)noexcept;
		};	 
	}
}