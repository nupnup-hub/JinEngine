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