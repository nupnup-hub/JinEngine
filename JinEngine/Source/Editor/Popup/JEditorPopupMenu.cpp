#include"JEditorPopupMenu.h"
#include"JEditorPopupNode.h"
#include"../Gui/JGui.h"
#include"../../Core/JCoreEssential.h"  

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
			isLeafPopupContentsClicked = false;
			isPopupContentsClicked = false; 

			if (IsOpen())
			{
				if (JGui::IsMouseClicked(Core::J_MOUSE_BUTTON::RIGHT))
					SetOpen(false);
				else if (JGui::IsMouseClicked(Core::J_MOUSE_BUTTON::LEFT) && !IsMouseInPopup())
					SetOpen(false);

				if (JGui::AnyMouseClicked(false) && IsMouseInPopup())
					isPopupContentsClicked = true;
			}
			else
			{ 

				if (JGui::IsMouseClicked(Core::J_MOUSE_BUTTON::RIGHT) &&
					JGui::IsCurrentWindowFocused(J_GUI_FOCUS_FLAG_CHILD_WINDOW) &&
					JGui::IsMouseInRect(JGui::GetWindowPos(), JGui::GetWindowSize()))
					SetOpen(true);
				if (!JGui::IsEnablePopup())
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
				JGui::OpenPopup(name);
				if (JGui::BeginPopup(name))
				{
					 JEditorPopupNode* selected = popupRoot->PopupOnScreen(editorString);
					 JGui::Separator();
					 JGui::EndPopup();

					if (selected != nullptr)
					{
						selected->InvokeSelectBind();
						SetOpen(false);
						isLeafPopupContentsClicked = true;
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
		bool JEditorPopupMenu::IsMouseInPopup()const noexcept
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
		bool JEditorPopupMenu::IsPopupContentsClicked()const noexcept
		{
			return isPopupContentsClicked;
		}
		bool JEditorPopupMenu::IsLeafPopupContentsClicked()const noexcept
		{
			return isLeafPopupContentsClicked;
		}
	}
}