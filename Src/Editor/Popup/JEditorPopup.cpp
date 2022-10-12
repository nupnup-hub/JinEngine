#include"JEditorPopup.h"
#include"JEditorPopupNode.h"
#include"../../Core/JDataType.h"  
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"

namespace JinEngine
{
	namespace Editor
	{
		JEditorPopup::JEditorPopup(const std::string& name, std::unique_ptr<JEditorPopupNode> popupRoot)
			:name(name), popupRoot(popupRoot.get()), isOpen(false)
		{
			allPopupNode.push_back(std::move(popupRoot));
		}
		JEditorPopup::~JEditorPopup() {}
		void JEditorPopup::Update()
		{
			if (JImGuiImpl::IsRightMouseClicked() && !IsOpen() && JImGuiImpl::IsMouseInWindow(JImGuiImpl::GetGuiWindowPos(), JImGuiImpl::GetGuiWindowSize()))
				SetOpen(true);
			else if (JImGuiImpl::IsRightMouseClicked() && IsOpen())
				SetOpen(false);
			else if (JImGuiImpl::IsLeftMouseClicked() && IsOpen() && !IsMouseInPopup())
				SetOpen(false);

			if(IsOpen() && !JImGuiImpl::IsEnablePopup())
				SetOpen(false);
		}
		void JEditorPopup::AddPopupNode(std::unique_ptr<JEditorPopupNode> child)noexcept
		{
			if (child != nullptr)
				allPopupNode.push_back(std::move(child));
		}
		void JEditorPopup::ExecutePopup(_In_ JEditorString* editorString, _Out_ J_EDITOR_POPUP_NODE_RES& res, _Out_ size_t& clickedPopupGuid)noexcept
		{
			res = J_EDITOR_POPUP_NODE_RES::NON_CLICK;
			clickedPopupGuid = 0;
			if (isOpen)
			{
				ImGui::OpenPopup(name.c_str());
				if (ImGui::BeginPopup(name.c_str()))
				{
					popupRoot->PopupOnScreen(editorString, res, clickedPopupGuid);
					ImGui::Separator();
					ImGui::EndPopup();
				}
			}
		}
		void JEditorPopup::SetOpen(bool value)noexcept
		{
			isOpen = value;
		}
		bool JEditorPopup::IsOpen()const noexcept
		{
			return isOpen;
		}
		bool JEditorPopup::IsMouseInPopup()noexcept
		{
			if (!IsOpen())
				return false;
			else
			{
				const uint allNodeCount = (uint)allPopupNode.size();
				for (uint i = 0; i < allNodeCount; ++i)
				{
					const J_EDITOR_POPUP_NODE_TYPE type = allPopupNode[i]->GetNodeType();
					if (type == J_EDITOR_POPUP_NODE_TYPE::ROOT)
					{
						bool res = allPopupNode[i]->IsMouseInPopup();
						if (res)
							return true;
					}
					else if (type == J_EDITOR_POPUP_NODE_TYPE::INTERNAL && allPopupNode[i]->IsOpen())
					{
						bool res = allPopupNode[i]->IsMouseInPopup();
						if (res)
							return true;
					}
				}
				return false;
			}
		}
	}
}