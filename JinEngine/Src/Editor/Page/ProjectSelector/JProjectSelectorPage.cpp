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
				std::make_unique<JEditorAttribute>(0.0f, 0.0f, 1.0f, 1.0f, false, false),
				J_EDITOR_PAGE_NONE)
		{
			projectHub = std::make_unique<JProjectSelectorHub>("Project Selector",
				std::make_unique<JEditorAttribute>(0.0f, 0.0f, 1.0f, 1.0f, true, true),
				GetPageType());

			windows.push_back(projectHub.get());
			opendWindow.push_back(projectHub.get());
			JEditorPageShareData::RegisterPage(GetPageType(), pageFlag);
		}
		JProjectSelectorPage::~JProjectSelectorPage()
		{
			JEditorPageShareData::UnRegisterPage(GetPageType());
		}
		J_EDITOR_PAGE_TYPE JProjectSelectorPage::GetPageType()const noexcept
		{
			return J_EDITOR_PAGE_TYPE::PROJECT_SELECTOR;
		}
		void JProjectSelectorPage::Initialize()
		{

		}
		void JProjectSelectorPage::UpdatePage()
		{
			ImGui::SetWindowFontScale(2);
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowPos(viewport->WorkPos);

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

			JImGuiImpl::SetFont(J_EDITOR_FONT_TYPE::BOLD); 
			ImGuiWindowFlags windowFlag = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;
			windowFlag |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoInputs;

			EnterPage(windowFlag); 
			ImGui::PopStyleVar(3);

			//JImGuiImpl::AddImage(*(backgroundTexture.Get()), wPos, wSize, false, IM_COL32(255, 255, 255, 50));
			uint8 opendWindowCount = (uint8)opendWindow.size();
			for (uint8 i = 0; i < opendWindowCount; ++i)
				opendWindow[i]->UpdateWindow();
			ClosePage();
			ImGui::SetWindowFontScale(1);
		}
		bool JProjectSelectorPage::IsValidOpenRequest(const Core::JUserPtr<JObject>& selectedObj) noexcept
		{
			return true;
		}
		void JProjectSelectorPage::DoActivate()noexcept
		{
			JEditorPage::DoActivate();
			backgroundTexture = Core::GetUserPtr(JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::MISSING));
			CallOnResourceReference(backgroundTexture.Get());
		}
		void JProjectSelectorPage::DoDeActivate()noexcept
		{
			JEditorPage::DoDeActivate();
			CallOffResourceReference(backgroundTexture.Get());
			backgroundTexture.Clear();
		}
		void JProjectSelectorPage::OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
		{
			if (iden == GetGuid())
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
			{
				if (backgroundTexture.IsValid() && jRobj->GetGuid() == backgroundTexture->GetGuid())
				{
					CallOffResourceReference(backgroundTexture.Get());
					backgroundTexture.Clear();
				}
			}
		}
	}
}