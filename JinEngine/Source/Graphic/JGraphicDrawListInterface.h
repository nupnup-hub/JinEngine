#pragma once
#include"JGraphicDrawListType.h"
#include"../Core/JDataType.h"
#include"../Object/Component/JComponentType.h"  

namespace JinEngine
{
	class JComponent;
	class JCamera;
	class JLight;
	class JScene;
	namespace Graphic
	{
		class JFrameUpdateUserAccess;
		class JGraphicDrawListSceneInterface
		{
		protected:
			//drawList
			static bool AddDrawList(const JUserPtr<JScene>& scene)noexcept;
			static bool PopDrawList(const JUserPtr<JScene>& scene)noexcept;
		protected:
			//option
			static bool AddObservationFrame(const JUserPtr<JScene>& scene, const JFrameUpdateUserAccess& observationFrame)noexcept;
		protected:
			static void UpdateScene(const JUserPtr<JScene>& scene, const J_COMPONENT_TYPE cType)noexcept;
		};

		class JGraphicDrawListCompInterface
		{
		protected:
			//request
			static void AddDrawShadowRequest(const JUserPtr<JScene>& scene, const JUserPtr<JLight>& jLight)noexcept;
			static void AddDrawSceneRequest(const JUserPtr<JScene>& scene, const JUserPtr<JCamera>& jCamera, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency)noexcept;
			static void AddFrustumCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency)noexcept;
			static void AddHzbOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency)noexcept;
			static void PopDrawRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept;
			static void PopFrustumCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept;
			static void PopHzbOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept;
			static bool HasRequestor(const JUserPtr<JScene>& scene)noexcept; 
		};
	}
}