#include"JEditorPopupWindow.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"

namespace JinEngine
{
	namespace Editor
	{
		void JEditorPopupWindow::RegisterBind(const J_EDITOR_POPUP_WINDOW_FUNC_TYPE type, std::unique_ptr<Core::JBindHandleBase>&& bind)noexcept
		{
			switch (type)
			{
			case JinEngine::Editor::J_EDITOR_POPUP_WINDOW_FUNC_TYPE::OPEN_POPUP:
			{
				openBind = std::move(bind);
				break;
			}
			case JinEngine::Editor::J_EDITOR_POPUP_WINDOW_FUNC_TYPE::CLOSE_POPUP:
			{
				closeBind = std::move(bind);
				break;
			}
			case JinEngine::Editor::J_EDITOR_POPUP_WINDOW_FUNC_TYPE::CONFIRM:
			{
				confirmBind = std::move(bind);
				break;
			}
			default:
				break;
			}	
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
		bool JEditorPopupWindow::IsDefaultFuncType(const J_EDITOR_POPUP_WINDOW_FUNC_TYPE type)const noexcept
		{
			switch (type)
			{
			case JinEngine::Editor::J_EDITOR_POPUP_WINDOW_FUNC_TYPE::OPEN_POPUP:
				return true;
			case JinEngine::Editor::J_EDITOR_POPUP_WINDOW_FUNC_TYPE::CLOSE_POPUP:
				return true;
			case JinEngine::Editor::J_EDITOR_POPUP_WINDOW_FUNC_TYPE::CONFIRM:
				return true; 
			default:
				return false;
			}
		}
		std::string JEditorPopupWindow::GetDesc()const noexcept
		{
			return desc;
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
		void JEditorPopupWindow::SetDesc(const std::string& newDesc)noexcept
		{
			if (newDesc.empty())
				return;
			desc = newDesc;
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
 
		void JEditorCautionPopup::Update(const std::string& uniqueLabel, const JVector2<float> pagePos, const JVector2<float> pageSize)
		{
			std::string desc = GetDesc();
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

			JVector2<float> pos = JVector2<float>(pagePos.x + (pageSize.x / 2) - ((widthFactor * 0.5f)),
				pagePos.y + (pageSize.y * 0.5f) - ((heightFactor * 0.5f)));

			ImGui::SetNextWindowPos(pos);
			ImGui::SetNextWindowSize(JVector2<float>(widthFactor, heightFactor));
			ImGuiWindowFlags flag = ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoDocking |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoScrollbar;

			JImGuiImpl::BeginWindow("##CloseConfirmPopup" + uniqueLabel, 0, flag);
			JVector2<float> textLength = ImGui::CalcTextSize("Close JinEngine!");

			JImGuiImpl::SetFont(J_EDITOR_FONT_TYPE::SEMI_BOLD);
			JImGuiImpl::PushFont();
			ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.5f - (textLength.x * 0.5f));
			JImGuiImpl::Text("Close JinEngine");
			ImGui::Separator();
			ImGui::SetCursorPosX(innerXPadding);
	 
			auto style = ImGui::GetStyle();
			textLength = ImGui::CalcTextSize("Confirm!Cancel");
			ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.5f - textLength.x - (style.ItemSpacing.x / 2));
			ImGui::SetCursorPosY(heightFactor - innerYPadding - textLength.y);
			if (JImGuiImpl::Button("Confirm##CloseConfirmPopup"))
				SetConfirm(true); 
			JImGuiImpl::PopFont();
			JImGuiImpl::EndWindow();
		}
		bool JEditorCautionPopup::IsSupportedFuncType(const J_EDITOR_POPUP_WINDOW_FUNC_TYPE type)const noexcept
		{
			return IsDefaultFuncType(type);
		}
		J_EDITOR_POPUP_WINDOW_TYPE JEditorCautionPopup::GetPopupType()const noexcept
		{
			return J_EDITOR_POPUP_WINDOW_TYPE::CAUTION;
		}

		void JEditorCloseConfirmPopup::RegisterBind(const J_EDITOR_POPUP_WINDOW_FUNC_TYPE type, std::unique_ptr<Core::JBindHandleBase>&& bind)noexcept
		{
			if (IsDefaultFuncType(type))
				JEditorPopupWindow::RegisterBind(type, std::move(bind));
			else
			{
				switch (type)
				{ 
				case JinEngine::Editor::J_EDITOR_POPUP_WINDOW_FUNC_TYPE::CANCEL:
				{
					cancelBind = std::move(bind);
					break;
				}
				case JinEngine::Editor::J_EDITOR_POPUP_WINDOW_FUNC_TYPE::CONTENTS:
				{
					contentsBind = std::move(bind);
					break;
				}
				default:
					break;
				} 
			} 
		}
		void JEditorCloseConfirmPopup::Update(const std::string& uniqueLabel, const JVector2<float> pagePos, const JVector2<float> pageSize)
		{
			std::string desc = GetDesc();
			const uint widthFactor = pageSize.x / 2.5f;
			const uint innerXPadding = widthFactor / 10;
			const uint heightFactor = pageSize.y / 2.5f;
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

			JVector2<float> pos = JVector2<float>(pagePos.x + (pageSize.x / 2)  - ((widthFactor * 0.5f)),
				pagePos.y + (pageSize.y * 0.5f) -((heightFactor * 0.5f)));
			  
			ImGui::SetNextWindowPos(pos);
			ImGui::SetNextWindowSize(JVector2<float>(widthFactor, heightFactor));
			ImGuiWindowFlags flag = ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoDocking |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoScrollbar;
			 
			JImGuiImpl::BeginWindow("##CloseConfirmPopup" + uniqueLabel, 0, flag);
			JVector2<float> textLength = ImGui::CalcTextSize("Close JinEngine!");

			JImGuiImpl::SetFont(J_EDITOR_FONT_TYPE::SEMI_BOLD);
			JImGuiImpl::PushFont();
			ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.5f - (textLength.x * 0.5f));
			JImGuiImpl::Text("Close JinEngine");	
			ImGui::Separator();
			ImGui::SetCursorPosX(innerXPadding); 
			JImGuiImpl::Text(desc); 

			if (contentsBind != nullptr)
				contentsBind->InvokeCompletelyBind();

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
			JImGuiImpl::PopFont();
			JImGuiImpl::EndWindow();
		}
		bool JEditorCloseConfirmPopup::IsSupportedFuncType(const J_EDITOR_POPUP_WINDOW_FUNC_TYPE type)const noexcept
		{
			switch (type)
			{
			case JinEngine::Editor::J_EDITOR_POPUP_WINDOW_FUNC_TYPE::CANCEL:
				return true;
			case JinEngine::Editor::J_EDITOR_POPUP_WINDOW_FUNC_TYPE::CONTENTS:
				return true;
			default:
				return IsDefaultFuncType(type);
			}
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