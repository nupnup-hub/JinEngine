#include"JEditorPopupWindow.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"

namespace JinEngine
{
	namespace Editor
	{
		void JEditorPopupWindow::RegisterBind(std::unique_ptr<Core::JBindHandleBase> newConfirmBind,
			std::unique_ptr<Core::JBindHandleBase> newOpenBind,
			std::unique_ptr<Core::JBindHandleBase> newCloseBind)noexcept
		{
			confirmBind = std::move(newConfirmBind);
			openBind = std::move(newOpenBind);
			closeBind = std::move(newCloseBind);
		}
		bool JEditorPopupWindow::IsOpen()const noexcept
		{
			return isOpen;
		}
		bool JEditorPopupWindow::IsIgnoreConfirm()const noexcept
		{
			return isIgnoreConfirm;
		}
		bool JEditorPopupWindow::IsPressConfirm()const noexcept
		{
			return isPressConfirm;
		}
		void JEditorPopupWindow::SetOpen() noexcept
		{
			if (!isOpen)
			{
				isOpen = true;
				SetConfirm(false);
				if (openBind != nullptr)
					openBind->InvokeCompletelyBind();
			}
		}
		void JEditorPopupWindow::SetClose() noexcept
		{
			if (isOpen)
			{
				isOpen = false;
				if (closeBind != nullptr)
					closeBind->InvokeCompletelyBind();
			}
		}
		void JEditorPopupWindow::SetIgnore(const bool value)noexcept
		{
			isIgnoreConfirm = value;
		}
		void JEditorPopupWindow::SetConfirm(const bool value)noexcept
		{
			if (value != isPressConfirm)
			{
				isPressConfirm = value;
				if (isPressConfirm && confirmBind != nullptr)
					confirmBind->InvokeCompletelyBind();
			}
		}

		void JEditorCautionPopup::Update(const std::string& uniqueLabel,
			const std::string& desc,
			const JVector2<float> pagePos,
			const JVector2<float> pageSize)
		{
			const uint widthFactor = pageSize.x / 4;
			const uint innerXPadding = widthFactor / 10;
			const uint heightFactor = pageSize.y / 3;
			const uint innerYPadding = heightFactor / 10;

			const uint descCount = (uint)desc.size();
			const uint descLength = JImGuiImpl::GetAlphabetSize().x * descCount;
			uint divCount = descLength / widthFactor;
			uint divIndex = descCount / divCount;
			uint modCount = descLength % widthFactor;
			uint nowIndex = 0;
			std::string finalDesc;

			if (descLength <= widthFactor)
				finalDesc = desc;
			else
			{
				while (nowIndex < descCount)
				{
					finalDesc += desc.substr(nowIndex, divIndex);
					nowIndex += divIndex;
				}
				if (modCount > 0)
					finalDesc += desc.substr(descCount - modCount, modCount);
			}

			ImVec2 pos = ImVec2(pagePos.x + (pageSize.x / 2) - ((widthFactor * 0.5f)),
				pagePos.y + (pageSize.y * 0.5f) - ((heightFactor * 0.5f)));

			ImGui::SetNextWindowPos(pos);
			ImGui::SetNextWindowSize(ImVec2(widthFactor, heightFactor));
			ImGuiWindowFlags flag = ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoDocking |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoScrollbar;

			JImGuiImpl::BeginWindow("##CloseConfirmPopup" + uniqueLabel, nullptr, flag);
			JVector2<float> textLength = ImGui::CalcTextSize("Close JinEngine!");

			JImGuiImpl::SetFont(J_EDITOR_FONT_TYPE::SEMI_BOLD);
			JImGuiImpl::PushFont();
			ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.5f - (textLength.x * 0.5f));
			JImGuiImpl::Text("Close JinEngine");
			ImGui::Separator();
			ImGui::SetCursorPosX(innerXPadding);
			JImGuiImpl::Text(desc);

			auto style = ImGui::GetStyle();
			textLength = ImGui::CalcTextSize("Confirm!Cancel");
			ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.5f - textLength.x - (style.ItemSpacing.x / 2));
			ImGui::SetCursorPosY(heightFactor - innerYPadding - textLength.y);
			if (JImGuiImpl::Button("Confirm##CloseConfirmPopup"))
				SetConfirm(true); 
			JImGuiImpl::EndWindow();
			JImGuiImpl::PopFont();
		}
		J_EDITOR_POPUP_WINDOW_TYPE JEditorCautionPopup::GetPopupType()const noexcept
		{
			return J_EDITOR_POPUP_WINDOW_TYPE::CAUTION;
		}

		void JEditorCloseConfirmPopup::RegisterBind(std::unique_ptr<Core::JBindHandleBase> newConfirmBind,
			std::unique_ptr<Core::JBindHandleBase> newOpenBind,
			std::unique_ptr<Core::JBindHandleBase> newCloseBind,
			std::unique_ptr<Core::JBindHandleBase> newCancelBind)
		{
			JEditorPopupWindow::RegisterBind(std::move(newConfirmBind), std::move(newOpenBind), std::move(newCloseBind));
			if (newCancelBind != nullptr)
				cancelBind = std::move(newCancelBind);
		}
		void JEditorCloseConfirmPopup::Update(const std::string& uniqueLabel,
			const std::string& desc,
			const JVector2<float> pagePos,
			const JVector2<float> pageSize)
		{
			const uint widthFactor = pageSize.x / 4;
			const uint innerXPadding = widthFactor / 10;
			const uint heightFactor = pageSize.y / 3;
			const uint innerYPadding = heightFactor / 10;

			const uint descCount = (uint)desc.size();
			const uint descLength = JImGuiImpl::GetAlphabetSize().x * descCount;
			std::string finalDesc;

			if (descLength <= widthFactor)
				finalDesc = desc;
			else
			{
				uint divCount = descLength / widthFactor;
				uint divIndex = descCount / divCount;
				uint modCount = descLength % widthFactor;
				uint nowIndex = 0;

				while (nowIndex < descCount)
				{
					finalDesc += desc.substr(nowIndex, divIndex);
					nowIndex += divIndex;
				}
				if (modCount > 0)
					finalDesc += desc.substr(descCount - modCount, modCount);
			}

			ImVec2 pos = ImVec2(pagePos.x + (pageSize.x / 2)  - ((widthFactor * 0.5f)),
				pagePos.y + (pageSize.y * 0.5f) -((heightFactor * 0.5f)));
			  
			ImGui::SetNextWindowPos(pos);
			ImGui::SetNextWindowSize(ImVec2(widthFactor, heightFactor));
			ImGuiWindowFlags flag = ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoDocking |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoScrollbar;

			JImGuiImpl::BeginWindow("##CloseConfirmPopup" + uniqueLabel, nullptr, flag);
			JVector2<float> textLength = ImGui::CalcTextSize("Close JinEngine!");

			JImGuiImpl::SetFont(J_EDITOR_FONT_TYPE::SEMI_BOLD);
			JImGuiImpl::PushFont();
			ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.5f - (textLength.x * 0.5f));
			JImGuiImpl::Text("Close JinEngine");	
			ImGui::Separator();
			ImGui::SetCursorPosX(innerXPadding); 
			JImGuiImpl::Text(desc); 

			auto style = ImGui::GetStyle();
			textLength = ImGui::CalcTextSize("Confirm!Cancel");
			ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.5f - textLength.x - (style.ItemSpacing.x / 2));
			ImGui::SetCursorPosY(heightFactor - innerYPadding - textLength.y);
			if (JImGuiImpl::Button("Confirm##CloseConfirmPopup"))
				SetConfirm(true);
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textLength.x);
			if (JImGuiImpl::Button("Cancel##CloseConfirmPopup"))
				SetCancel(true);
			JImGuiImpl::EndWindow();
			JImGuiImpl::PopFont();
		}
		J_EDITOR_POPUP_WINDOW_TYPE JEditorCloseConfirmPopup::GetPopupType()const noexcept
		{
			return J_EDITOR_POPUP_WINDOW_TYPE::CLOSE_CONFIRM;
		}
		void JEditorCloseConfirmPopup::SetOpen() noexcept
		{
			JEditorPopupWindow::SetOpen();
			SetCancel(false);
		}
		void JEditorCloseConfirmPopup::SetCancel(const bool value)noexcept
		{
			isPressCancel = value;
			if (isPressCancel && cancelBind != nullptr)
				cancelBind->InvokeCompletelyBind();
		}
	}
}