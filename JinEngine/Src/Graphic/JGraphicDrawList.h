#pragma once
#include"../Core/JDataType.h"
#include"../Object/Component/JComponentType.h" 
#include<deque> 
#include<vector>
#include<memory>

namespace JinEngine
{
	class JComponent;
	class JScene; 
	class JSceneManager;
	class JLight;
	class JCamera;  
	class JFrameUpdateUserAccess;

	namespace Graphic
	{
		enum class J_GRAPHIC_DRAW_FREQUENCY
		{
			ALWAYS,
			UPDATED
		};

		class JGraphicResourceInfo;  
		class JGraphicDrawList;
		class JGraphicResourceInterface;
		class JGraphic;

		struct JShadowMapDrawRequestor
		{
		public:
			JUserPtr<JComponent> jLight;
			JGraphicResourceInfo* handle;
		public:
			JShadowMapDrawRequestor(const JUserPtr<JComponent>& jLight, JGraphicResourceInfo* handle);
			~JShadowMapDrawRequestor();
		};
		struct JSceneDrawRequestor
		{
		public:
			JUserPtr<JComponent> jCamera;
			JGraphicResourceInfo* handle;
		public:
			JSceneDrawRequestor(const JUserPtr<JComponent>& jCamera, JGraphicResourceInfo* handle);
			~JSceneDrawRequestor();
		};

		struct JGraphicDrawTarget
		{
		public:
			struct UpdateInfo
			{
			private:
				friend class JGraphicDrawList;
			private:
				std::unique_ptr<JFrameUpdateUserAccess> observationFrame;		// for resource preview scene
				const J_GRAPHIC_DRAW_FREQUENCY updateFrequency;
				const bool isAllowOcclusionCulling;
			public:
				uint objUpdateCount = 0;
				uint aniUpdateCount = 0;
				uint camUpdateCount = 0;
				uint lightUpdateCount = 0;
			public:
				//frame dirty = gNumFrameDirty
				uint hotObjUpdateCount = 0;
				uint hotAniUpdateCount = 0;
				uint hotCamUpdateCount = 0;
				uint hotLitghtUpdateCount = 0;
			public:
				bool hasSceneUpdate = false;
				bool hasShadowUpdate = false;
				bool hasOcclusionUpdate = false;
			public:
				void UpdateStart();
				void UpdateEnd();
			public:
				UpdateInfo(const J_GRAPHIC_DRAW_FREQUENCY updateFrequency, const bool isAllowOcclusionCulling);
				~UpdateInfo();
			};
		private:
			friend class JGraphicDrawList;
		public:
			JUserPtr<JScene> scene;
			std::unique_ptr<UpdateInfo> updateInfo;
			std::vector<std::unique_ptr<JShadowMapDrawRequestor>> shadowRequestor;
			std::vector<std::unique_ptr<JSceneDrawRequestor>> sceneRequestor;
		public:
			JGraphicDrawTarget(const JUserPtr<JScene>& scene, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency, const bool isAllowOcclusionCulling);
			~JGraphicDrawTarget();
		};
		 
		class JGraphicDrawList 
		{ 
		private:
			friend class JGraphic;
			friend class JGraphicResourceInterface;
			friend class JSceneManager;
			friend class JScene;
		private:
			//drawList
			static bool AddDrawList(const JUserPtr<JScene>& scene, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency, const bool isAllowOcclusionCulling)noexcept;
			static bool PopDrawList(const JUserPtr<JScene>& scene)noexcept;
			static bool HasDrawList(const JUserPtr<JScene>& scene)noexcept;
			static void UpdateScene(const JUserPtr<JScene>& scene, const J_COMPONENT_TYPE cType)noexcept;
		private:
			//option
			static bool AddObservationFrame(const JUserPtr<JScene>& scene, const JFrameUpdateUserAccess& observationFrame)noexcept;
		private:
			//request
			static void AddDrawShadowRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jLight, JGraphicResourceInfo* handle)noexcept;
			static void AddDrawSceneRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jCamera, JGraphicResourceInfo* handle)noexcept;
			static void PopDrawRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept;
			static bool HasRequestor(const JUserPtr<JScene>& scene)noexcept;
		private:
			static uint GetListCount()noexcept;
			static JGraphicDrawTarget* GetDrawScene(const uint index)noexcept;
		private:
			//static std::vector<JGraphicDrawTarget*> GetAllDrawTarget()noexcept;
		//	static std::vector<JGraphicDrawTarget*> GetDrawableTarget()noexcept;
		};	 
	}
}