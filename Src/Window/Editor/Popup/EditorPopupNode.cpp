#include"EditorPopupNode.h" 
#include"../String/EditorString.h"
#include"../../../Utility/JCommonUtility.h"

namespace JinEngine
{
	EditorPopupNode::EditorPopupNode(const std::string name,
		const EDITOR_POPUP_NODE_TYPE nodeType,
		EditorPopupNode* parent,
		const bool hasTooltip ,
		const bool hasShortCut)
		:name(name), 
		nodeType(nodeType),
		parent(parent),
		hasTooltip(hasTooltip), 
		hasShortCut(hasShortCut)
	{
		if (parent != nullptr)
			parent->children.push_back(this);
		MakeNodeId(parent, name, nodeId, tooltipId, shortCutId);
	}
	EditorPopupNode::~EditorPopupNode() {}
	size_t EditorPopupNode::GetNodeId()const noexcept
	{
		return nodeId;
	}
	size_t EditorPopupNode::GetTooltipId()const noexcept
	{
		return tooltipId;
	}
	size_t EditorPopupNode::GetShortCutId()const noexcept
	{
		return shortCutId;
	}
	EDITOR_POPUP_NODE_TYPE EditorPopupNode::GetNodeType()const noexcept
	{
		return nodeType;
	}
	void EditorPopupNode::PopupOnScreen(_In_ EditorString* editorString, _Out_ EDITOR_POPUP_NODE_RES& res, _Out_ std::size_t& clickMenuGuid)
	{
		const uint childrenCount = (uint)children.size();
		switch (nodeType)
		{
		case JinEngine::EDITOR_POPUP_NODE_TYPE::ROOT:
			popupPos = ImGui::GetWindowPos();	
			popupSize = ImGui::GetWindowSize();
			for (uint i = 0; i < childrenCount; ++i)
				children[i]->PopupOnScreen(editorString, res, clickMenuGuid);
			break;	
		case JinEngine::EDITOR_POPUP_NODE_TYPE::INTERNAL:		
			if (ImGui::BeginMenu(editorString->GetString(nodeId).c_str()))
			{
				isOpen = true;
				popupPos = ImGui::GetWindowPos();
				popupSize = ImGui::GetWindowSize();
				for (uint i = 0; i < childrenCount; ++i)
					children[i]->PopupOnScreen(editorString, res, clickMenuGuid);
				ImGui::EndMenu();
			}
			else
				isOpen = false;
			PrintTooltip(editorString);
			break;		
		case JinEngine::EDITOR_POPUP_NODE_TYPE::LEAF_SELECT:	
			if (hasShortCut)
			{
				if (ImGui::MenuItem(editorString->GetString(nodeId).c_str(), editorString->GetString(shortCutId).c_str(), false))
				{
					res = EDITOR_POPUP_NODE_RES::CLICK_SLELECT_NODE;
					clickMenuGuid = nodeId;
				}
				PrintTooltip(editorString);
			}
			else
			{
				if (ImGui::MenuItem(editorString->GetString(nodeId).c_str()))
				{
					res = EDITOR_POPUP_NODE_RES::CLICK_SLELECT_NODE;
					clickMenuGuid = nodeId;
				}
				PrintTooltip(editorString);
			}
			break;	
		case JinEngine::EDITOR_POPUP_NODE_TYPE::LEAF_TOGGLE:		
			if (hasShortCut)
			{
				if (ImGui::MenuItem(editorString->GetString(nodeId).c_str(), editorString->GetString(shortCutId).c_str(), &isActivated))
				{
					if (isActivated)
						res = EDITOR_POPUP_NODE_RES::CLICK_TOGGLE_TO_ON;
					else
						res = EDITOR_POPUP_NODE_RES::CLICK_TOGGLE_TO_OFF;
					clickMenuGuid = nodeId;
				}
				PrintTooltip(editorString);
			}
			else
			{
				if (ImGui::MenuItem(editorString->GetString(nodeId).c_str(), "", &isActivated))
				{
					if (isActivated)
						res = EDITOR_POPUP_NODE_RES::CLICK_TOGGLE_TO_ON;
					else
						res = EDITOR_POPUP_NODE_RES::CLICK_TOGGLE_TO_OFF;
					clickMenuGuid = nodeId;
				}
				PrintTooltip(editorString);
			}
			break;
		case JinEngine::EDITOR_POPUP_NODE_TYPE::LEAF_CHECK_BOX:
			break;
		default:
			break;
		}
	}
	void EditorPopupNode::PrintTooltip(_In_ EditorString* editorString)noexcept
	{
		if (hasTooltip)
		{
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(editorString->GetString(tooltipId).c_str());
		}
	}
	bool EditorPopupNode::IsOpen()const noexcept
	{
		return isOpen;
	}
	bool EditorPopupNode::IsMouseInPopup()const noexcept
	{
		return ImGuiEx::Contain(popupPos, popupSize, ImGui::GetMousePos());
	}
	void EditorPopupNode::MakeNodeId(EditorPopupNode* parent,
		const std::string& name,
		_Out_ size_t& nodeId,
		_Out_ size_t& tooltipId,
		_Out_ size_t& shortCutId)noexcept
	{
		std::string path = name;
		EditorPopupNode* nowParent = parent;
		while (nowParent != nullptr)
		{
			path += "\\" + nowParent->name;
			nowParent = nowParent->parent;
		}
		nodeId = JCommonUtility::CalculateGuid(path);
		tooltipId = JCommonUtility::CalculateGuid(path + "Node_Tooltip");
		shortCutId = JCommonUtility::CalculateGuid(path + "Node_ShortCut");
	}
}
