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
		bool JGraphicDrawListSceneInterface::AddObservationFrame(const JUserPtr<JScene>& scene, const JUserPtr<JFrameUpdateUserAccess>& observationFrame)noexcept
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
		void JGraphicDrawListCompInterface::AddHdOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency)noexcept
		{
			Graphic::JGraphicDrawList::AddHdOccCullingRequest(scene, jComp, updateFrequency);
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
		void JGraphicDrawListCompInterface::PopHdOccCullingRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept
		{
			Graphic::JGraphicDrawList::PopHdOccCullingRequest(scene, jComp);
		}
		bool JGraphicDrawListCompInterface::HasRequestor(const JUserPtr<JScene>& scene)noexcept
		{
			return Graphic::JGraphicDrawList::HasRequestor(scene);
		}
	}
}