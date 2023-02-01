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
	class JSceneManagerImpl;
	class JLight;
	class JCamera; 
	class IFrameDirty;

	namespace Graphic
	{
		enum class J_GRAPHIC_DRAW_FREQUENCY
		{
			ALWAYS,
			UPDATED
		};

		class JGraphicResourceHandle;  
		class JGraphicDrawList;
		class JGraphicResourceHandleInterface;

		struct JShadowMapDrawRequestor
		{
		public:
			JComponent* jLight;
			JGraphicResourceHandle* handle;
		public:
			JShadowMapDrawRequestor(JComponent* jLight, JGraphicResourceHandle* handle);
			~JShadowMapDrawRequestor();
		};
		struct JSceneDrawRequestor
		{
		public:
			JComponent* jCamera;
			JGraphicResourceHandle* handle;
		public:
			JSceneDrawRequestor(JComponent* jCamera, JGraphicResourceHandle* handle);
			~JSceneDrawRequestor();
		};

		struct JGraphicDrawTarget
		{
		public:
			struct UpdateInfo
			{
			private:
				Core::JUserPtr<IFrameDirty> observationFrame;		// for resource preview scene
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
				UpdateInfo(Core::JUserPtr<IFrameDirty> observationFrame, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency, const bool isAllowOcclusionCulling);
				~UpdateInfo();
			};
		private:
			friend class JGraphicDrawList;
		public:
			JScene* scene; 
			std::unique_ptr<UpdateInfo> updateInfo;
			std::vector<std::unique_ptr<JShadowMapDrawRequestor>> shadowRequestor;
			std::vector<std::unique_ptr<JSceneDrawRequestor>> sceneRequestor;
		public:
			JGraphicDrawTarget(JScene* scene, 
				Core::JUserPtr<IFrameDirty> observationFrame,
				const J_GRAPHIC_DRAW_FREQUENCY updateFrequency,
				const bool isAllowOcclusionCulling);
			~JGraphicDrawTarget();
		};
		 
		class JGraphicDrawList 
		{ 
		private:
			friend class JGraphicImpl;
			friend class JGraphicResourceHandleInterface;
			friend class JSceneManagerImpl;
			friend class JScene;
		private:
			static bool AddDrawList(JScene* scene, Core::JUserPtr<IFrameDirty> observationFrame, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency, const bool isAllowOcclusionCulling)noexcept;
			static bool PopDrawList(JScene* scene)noexcept;
			static bool HasDrawList(JScene* scene)noexcept;
			static void UpdateScene(JScene* scene, const J_COMPONENT_TYPE cType)noexcept;
		private:
			static void AddDrawShadowRequest(JScene* scene, JComponent* jLight, JGraphicResourceHandle* handle)noexcept;
			static void AddDrawSceneRequest(JScene* scene, JComponent* jCamera, JGraphicResourceHandle* handle)noexcept;
			static void PopDrawRequest(JScene* scene, JComponent* jComp)noexcept;
			static bool HasRequestor(JScene* scene)noexcept;
		private:
			static uint GetListCount()noexcept;
			static JGraphicDrawTarget* GetDrawScene(const uint index)noexcept;
		};	 
	}
}