#include"JProjectSelectorPage.h"
#include"Window/JProjectSelectorHub.h"
#include"../JEditorAttribute.h"
#include"../JEditorPageShareData.h" 
#include"../../GuiLibEx/ImGuiEx/JImGuiImpl.h" 
#include"../../../Object/Resource/JResourceManager.h"
#include"../../../Object/Resource/Texture/JTexture.h"

namespace JinEngine
{
	namespace Editor
	{
		JProjectSelectorPage::JProjectSelectorPage()
			: JEditorPage("ProjectSelectorPage",
				std::make_unique<JEditorAttribute>(),
				J_EDITOR_PAGE_NONE),
			ResourceEvListener(GetGuid())
		{
			projectHub = std::make_unique<JProjectSelectorHub>("Project Selector",
				std::make_unique<JEditorAttribute>(),
				GetPageType(), 
				J_EDITOR_WINDOW_NONE);

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
			ImGui::SetWindowFontScale(2);
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowPos(viewport->WorkPos);

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			//ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

			JImGuiImpl::SetFont(J_EDITOR_FONT_TYPE::MEDIUM); 
			ImGuiWindowFlags guiWindowFlag = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;
			guiWindowFlag |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoInputs;

			EnterPage(guiWindowFlag); 
			ImGui::PopStyleVar(2);

			//JImGuiImpl::AddImage(*(backgroundTexture.Get()), wPos, wSize, false, IM_COL32(255, 255, 255, 50));
			uint currOpWndCount = GetOpenWindowCount();
			for (uint i = 0; i < currOpWndCount; ++i)
				GetOpenWindow(i)->UpdateWindow();
			ClosePage();
			ImGui::SetWindowFontScale(1);
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
			JEditorPage::DoDeActivate();
			backgroundTexture.Clear();
		}
		void JProjectSelectorPage::OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
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