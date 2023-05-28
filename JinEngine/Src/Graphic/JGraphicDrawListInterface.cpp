#include"JGraphicDrawListInterface.h"
#include"JGraphicDrawList.h"

namespace JinEngine
{
	namespace Graphic
	{
		bool JGraphicDrawListSceneInterface::AddDrawList(const JUserPtr<JScene>& scene)noexcept
		{
			return Graphic::JGraphicDrawList::AddDrawList(scene);
		}
		bool JGraphicDrawListSceneInterface::PopDrawList(const JUserPtr<JScene>& scene)noexcept
		{
			return Graphic::JGraphicDrawList::PopDrawList(scene);
		}
		bool JGraphicDrawListSceneInterface::AddObservationFrame(const JUserPtr<JScene>& scene, const JFrameUpdateUserAccess& observationFrame)noexcept
		{
			return Graphic::JGraphicDrawList::AddObservationFrame(scene, observationFrame);
		}
		void JGraphicDrawListSceneInterface::UpdateScene(const JUserPtr<JScene>& scene, const J_COMPONENT_TYPE cType)noexcept
		{
			Graphic::JGraphicDrawList::UpdateScene(scene, cType);
		}

		void JGraphicDrawListCompInterface::AddDrawShadowRequest(const JUserPtr<JScene>& scene, const JUserPtr<JLight>& jLight)noexcept
		{
			Graphic::JGraphicDrawList::AddDrawShadowRequest(scene, jLight);
		}
		void JGraphicDrawListCompInterface::AddDrawSceneRequest(const JUserPtr<JScene>& scene, const JUserPtr<JCamera>& jCamera, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency)noexcept
		{
			Graphic::JGraphicDrawList::AddDrawSceneRequest(scene, jCamera, updateFrequency);
		}
		void JGraphicDrawListCompInterface::AddFrustumCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency)noexcept
		{
			Graphic::JGraphicDrawList::AddFrustumCullingRequest(scene, jComp, updateFrequency);
		}
		void JGraphicDrawListCompInterface::AddHzbOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency)noexcept
		{
			Graphic::JGraphicDrawList::AddHzbOccCullingRequest(scene, jComp, updateFrequency);
		}
		void JGraphicDrawListCompInterface::PopDrawRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept
		{
			Graphic::JGraphicDrawList::PopDrawRequest(scene, jComp);
		}
		void JGraphicDrawListCompInterface::PopFrustumCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept
		{
			Graphic::JGraphicDrawList::PopFrustumCullingRequest(scene, jComp);
		}
		void JGraphicDrawListCompInterface::PopHzbOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept
		{
			Graphic::JGraphicDrawList::PopHzbOccCullingRequest(scene, jComp);
		}
		bool JGraphicDrawListCompInterface::HasRequestor(const JUserPtr<JScene>& scene)noexcept
		{
			return Graphic::JGraphicDrawList::HasRequestor(scene);
		}
	}
}