#include"EditorMenuBar.h" 
#include"../Utility/EditorUtility.h"
#include"../../../Utility/JCommonUtility.h"
#include"../../../../Lib/imgui/imgui.h"

namespace JinEngine
{
	MenuNode::MenuNode(const std::string& windowName, bool isRoot, bool isLeaf, bool* isOpend, MenuNode* parent)
		:nodeName(windowName + "##_MenuBar"),
		windowName(windowName), 
		isRoot(isRoot), 
		isLeaf(isLeaf), 
		isOpend(isOpend),
		parent(parent)
	{
		if(!isRoot)
			parent->children.push_back(this);
	}
	MenuNode::~MenuNode(){}
	std::string MenuNode::GetMenuNodeName()const noexcept
	{
		return nodeName;
	}
	std::string MenuNode::GetWindowName()const noexcept
	{
		return windowName;
	}
	const uint MenuNode::GetChildrenCount()const noexcept
	{
		return (uint)children.size();
	}
	MenuNode* MenuNode::GetChild(const uint index)const noexcept
	{
		if (children.size() <= index)
			return nullptr;
		else
			return children[index];
	}
	bool MenuNode::IsRootNode()const noexcept
	{
		return isRoot;
	}
	bool MenuNode::IsLeafNode()const noexcept
	{
		return isLeaf;
	}
	bool MenuNode::IsOpendNode()const noexcept
	{
		return *isOpend;
	}
	EditorMenuBar::EditorMenuBar(){}
	EditorMenuBar::~EditorMenuBar(){}
	MenuNode* EditorMenuBar::GetSelectedNode()noexcept
	{
		return selectedNode;
	}
	bool EditorMenuBar::UpdateMenuBar(EditorUtility* editorUtility)
	{
		selectedNode = nullptr;
		ImGui::BeginMainMenuBar();
		{ 
			LoopNode(rootNode);
			ImGui::EndMenuBar();
		}
		return selectedNode != nullptr;
	}
	void EditorMenuBar::LoopNode(MenuNode* node)
	{
		std::string nodeName = node->GetMenuNodeName();
		if (node->IsLeafNode())
		{
			if (ImGui::MenuItem(nodeName.c_str(), NULL, node->IsOpendNode()))
				selectedNode = node;
		}
		else
		{
			const uint childrenCount = node->GetChildrenCount();
			if (node->IsRootNode())
			{
				for (uint i = 0; i < childrenCount; ++i)
					LoopNode(node->GetChild(i));
			}
			else if (ImGui::BeginMenu(nodeName.c_str()))
			{		 
				for (uint i = 0; i < childrenCount; ++i)
					LoopNode(node->GetChild(i));
				ImGui::EndMenu();
			}
		}
	}
}