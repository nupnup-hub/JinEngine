#include"JEditorPopupNode.h" 
#include"../String/JEditorString.h" 
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../Core/Guid/GuidCreator.h"

namespace JinEngine
{
	namespace Editor
	{
		JEditorPopupNode::JEditorPopupNode(const std::string name,
			const J_EDITOR_POPUP_NODE_TYPE nodeType,
			JEditorPopupNode* parent,
			const bool hasTooltip,
			const bool hasShortCut)
			:name(name),
			nodeType(nodeType),
			parent(parent),
			hasTooltip(hasTooltip),
			hasShortCut(hasShortCut)
		{
			if (parent != nullptr)
				parent->children.push_back(this);
			nodeId = Core::MakeGuid();
			tooltipId = Core::MakeGuid();
			shortCutId = Core::MakeGuid();
		}
		JEditorPopupNode::~JEditorPopupNode() {}
		size_t JEditorPopupNode::GetNodeId()const noexcept
		{
			return nodeId;
		}
		size_t JEditorPopupNode::GetTooltipId()const noexcept
		{
			return tooltipId;
		}
		size_t JEditorPopupNode::GetShortCutId()const noexcept
		{
			return shortCutId;
		}
		J_EDITOR_POPUP_NODE_TYPE JEditorPopupNode::GetNodeType()const noexcept
		{
			return nodeType;
		}
		void JEditorPopupNode::PopupOnScreen(_In_ JEditorString* editorString, _Out_ J_EDITOR_POPUP_NODE_RES& res, _Out_ std::size_t& clickMenuGuid)
		{
			const uint childrenCount = (uint)children.size();
			switch (nodeType)
			{
			case J_EDITOR_POPUP_NODE_TYPE::ROOT:
				popupPos = JImGuiImpl::GetGuiWindowPos();
				popupSize = JImGuiImpl::GetGuiWindowSize();
				for (uint i = 0; i < childrenCount; ++i)
					children[i]->PopupOnScreen(editorString, res, clickMenuGuid);
				break;
			case J_EDITOR_POPUP_NODE_TYPE::INTERNAL:
				if (ImGui::BeginMenu(editorString->GetString(nodeId).c_str()))
				{ 
					isOpen = true;
					popupPos = JImGuiImpl::GetGuiWindowPos();
					popupSize = JImGuiImpl::GetGuiWindowSize();
					for (uint i = 0; i < childrenCount; ++i)
						children[i]->PopupOnScreen(editorString, res, clickMenuGuid);
					ImGui::EndMenu();
				}
				else
					isOpen = false;
				PrintTooltip(editorString);
				break;
			case J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT:
				if (hasShortCut)
				{
					if (ImGui::MenuItem(editorString->GetString(nodeId).c_str(), editorString->GetString(shortCutId).c_str(), false))
					{
						res = J_EDITOR_POPUP_NODE_RES::CLICK_SLELECT_NODE;
						clickMenuGuid = nodeId;
					}
					PrintTooltip(editorString);
				}
				else
				{
					if (ImGui::MenuItem(editorString->GetString(nodeId).c_str()))
					{
						res = J_EDITOR_POPUP_NODE_RES::CLICK_SLELECT_NODE;
						clickMenuGuid = nodeId;
					}
					PrintTooltip(editorString);
				}
				break;
			case J_EDITOR_POPUP_NODE_TYPE::LEAF_TOGGLE:
				if (hasShortCut)
				{
					if (ImGui::MenuItem(editorString->GetString(nodeId).c_str(), editorString->GetString(shortCutId).c_str(), &isActivated))
					{
						if (isActivated)
							res = J_EDITOR_POPUP_NODE_RES::CLICK_TOGGLE_TO_ON;
						else
							res = J_EDITOR_POPUP_NODE_RES::CLICK_TOGGLE_TO_OFF;
						clickMenuGuid = nodeId;
					}
					PrintTooltip(editorString);
				}
				else
				{
					if (ImGui::MenuItem(editorString->GetString(nodeId).c_str(), "", &isActivated))
					{
						if (isActivated)
							res = J_EDITOR_POPUP_NODE_RES::CLICK_TOGGLE_TO_ON;
						else
							res = J_EDITOR_POPUP_NODE_RES::CLICK_TOGGLE_TO_OFF;
						clickMenuGuid = nodeId;
					}
					PrintTooltip(editorString);
				}
				break;
			case J_EDITOR_POPUP_NODE_TYPE::LEAF_CHECK_BOX:
				break;
			default:
				break;
			}
		}
		void JEditorPopupNode::PrintTooltip(_In_ JEditorString* editorString)noexcept
		{
			if (hasTooltip)
			{
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip(editorString->GetString(tooltipId).c_str());
			}
		}
		bool JEditorPopupNode::IsOpen()const noexcept
		{
			return isOpen;
		}
		bool JEditorPopupNode::IsMouseInPopup()const noexcept
		{ 
			return JVector2<float>(ImGui::GetMousePos().x, ImGui::GetMousePos().y).Contain(popupPos, popupSize);
		}
	}
}