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


#include"JProjectSelectorPage.h"
#include"Window/JProjectSelectorHub.h"
#include"../JEditorAttribute.h"
#include"../JEditorPageShareData.h" 
#include"../../Gui/JGui.h" 
#include"../../../Object/Resource/JResourceManager.h"
#include"../../../Object/Resource/Texture/JTexture.h" 

namespace JinEngine
{
	namespace Editor
	{
		JProjectSelectorPage::JProjectSelectorPage(std::unique_ptr<JEditorProjectInterface>&& pInterface)
			: JEditorPage("ProjectSelectorPage",
				std::make_unique<JEditorAttribute>(),
				J_EDITOR_PAGE_NONE)
		{
			projectHub = std::make_unique<JProjectSelectorHub>("Project Selector",
				std::make_unique<JEditorAttribute>(),
				GetPageType(), 
				J_EDITOR_WINDOW_NONE,
				std::move(pInterface));

			std::vector<JEditorWindow*> windows
			{
				projectHub.get()
			};
			AddWindow(windows);
			ResourceEvListener::AddEventListener(*JResourceObject::EvInterface(), GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
		}
		JProjectSelectorPage::~JProjectSelectorPage()
		{
			ResourceEvListener::RemoveListener(*JResourceObject::EvInterface(), GetGuid());
		}
		J_EDITOR_PAGE_TYPE JProjectSelectorPage::GetPageType()const noexcept
		{
			return J_EDITOR_PAGE_TYPE::PROJECT_SELECTOR;
		}
		void JProjectSelectorPage::SetInitWindow()
		{
			uint currOpWndCount = GetOpenWindowCount();
			for (uint i = 0; i < currOpWndCount; ++i)
				CloseWindow(GetOpenWindow(i));

			OpenWindow(projectHub.get());

			currOpWndCount = GetOpenWindowCount();
			for (uint i = 0; i < currOpWndCount; ++i)
				GetOpenWindow(i)->SetLastActivated(true);		 
		}
		void JProjectSelectorPage::UpdatePage()
		{
			JGui::SetCurrentWindowFontScale(2); 
			JGui::SetNextWindowSize(JGui::GetMainWorkSize());
			JGui::SetNextWindowPos(JGui::GetMainWorkPos());
 
			J_GUI_WINDOW_FLAG_ guiWindowFlag = J_GUI_WINDOW_FLAG_NO_DOCKING | J_GUI_WINDOW_FLAG_NO_MOVE | J_GUI_WINDOW_FLAG_NO_TITLE_BAR;
			guiWindowFlag |= J_GUI_WINDOW_FLAG_NO_COLLAPSE | J_GUI_WINDOW_FLAG_NO_NAV_INPUT;

			EnterPage(guiWindowFlag); 
			JGui::SetFont(J_GUI_FONT_TYPE::MEDIUM);
			JGui::PushFont();

			//JGui::AddImage(*(backgroundTexture.Get()), wPos, wSize, false, IM_COL32(255, 255, 255, 50));
			UpdateOpenWindow();
			JGui::PopFont(); 
			ClosePage();
			JGui::SetCurrentWindowFontScale(1);
		}
		bool JProjectSelectorPage::IsValidOpenRequest(const JUserPtr<Core::JIdentifier>& selectedObj) noexcept
		{
			return true;
		}
		void JProjectSelectorPage::DoActivate()noexcept
		{
			JEditorPage::DoActivate();
			backgroundTexture = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::MISSING);
		}
		void JProjectSelectorPage::DoDeActivate()noexcept
		{
			backgroundTexture.Clear();
			JEditorPage::DoDeActivate();
		}
		void JProjectSelectorPage::OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj, JResourceEventDesc* desc)
		{
			if (iden == GetGuid())
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
			{
				if (backgroundTexture.IsValid() && jRobj->GetGuid() == backgroundTexture->GetGuid())
					backgroundTexture.Clear();
			}
		}
	}
}