#include"JEditorPopupMenu.h"
#include"JEditorPopupNode.h"
#include"../../Core/JDataType.h"  
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"

namespace JinEngine
{
	namespace Editor
	{
		JEditorPopupMenu::JEditorPopupMenu(const std::string& name, std::unique_ptr<JEditorPopupNode> popupRoot)
			:name(name), popupRoot(popupRoot.get()), isOpen(false)
		{
			allPopupNode.push_back(std::move(popupRoot));
		}
		JEditorPopupMenu::~JEditorPopupMenu() {}
		void JEditorPopupMenu::Update()
		{
			if (IsOpen())
			{
				if (JImGuiImpl::IsRightMouseClicked())
					SetOpen(false);
				else if (JImGuiImpl::IsLeftMouseClicked() && !IsMouseInPopup())
					SetOpen(false);
			}
			else
			{ 
				if (JImGuiImpl::IsRightMouseClicked() && ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) && JImGuiImpl::IsMouseInRect(JImGuiImpl::GetGuiWindowPos(), JImGuiImpl::GetGuiWindowSize()))
					SetOpen(true);
				if (!JImGuiImpl::IsEnablePopup())
					SetOpen(false);
			}
		}
		void JEditorPopupMenu::AddPopupNode(std::unique_ptr<JEditorPopupNode> child)noexcept
		{
			if (child != nullptr)
				allPopupNode.push_back(std::move(child));
		}
		void JEditorPopupMenu::ExecutePopup(_In_ JEditorStringMap* editorString)noexcept
		{
			if (isOpen)
			{
				ImGui::OpenPopup(name.c_str());
				if (ImGui::BeginPopup(name.c_str()))
				{
					 JEditorPopupNode* selected = popupRoot->PopupOnScreen(editorString);
					ImGui::Separator();
					ImGui::EndPopup();

					if (selected != nullptr)
					{
						selected->InvokeSelectBind();
						SetOpen(false);
					}
				}
			}
		}
		void JEditorPopupMenu::SetOpen(bool value)noexcept
		{
			isOpen = value;
		}
		bool JEditorPopupMenu::IsOpen()const noexcept
		{
			return isOpen;
		}
		bool JEditorPopupMenu::IsMouseInPopup()noexcept
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