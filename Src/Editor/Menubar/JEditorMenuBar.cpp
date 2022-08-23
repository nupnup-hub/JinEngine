#include"JEditorMenuBar.h"  
#include"../../Utility/JCommonUtility.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"

namespace JinEngine
{
	namespace Editor
	{
		JMenuNode::JMenuNode(const std::string& windowName, bool isRoot, bool isLeaf, bool* isOpend, JMenuNode* parent)
			:nodeName(windowName + "##_MenuBar"),
			windowName(windowName),
			isRoot(isRoot),
			isLeaf(isLeaf),
			isOpend(isOpend),
			parent(parent)
		{
			if (!isRoot)
				parent->children.push_back(this);
		}
		JMenuNode::~JMenuNode() {}
		std::string JMenuNode::GetMenuNodeName()const noexcept
		{
			return nodeName;
		}
		std::string JMenuNode::GetWindowName()const noexcept
		{
			return windowName;
		}
		const uint JMenuNode::GetChildrenCount()const noexcept
		{
			return (uint)children.size();
		}
		JMenuNode* JMenuNode::GetChild(const uint index)const noexcept
		{
			if (children.size() <= index)
				return nullptr;
			else
				return children[index];
		}
		bool JMenuNode::IsRootNode()const noexcept
		{
			return isRoot;
		}
		bool JMenuNode::IsLeafNode()const noexcept
		{
			return isLeaf;
		}
		bool JMenuNode::IsOpendNode()const noexcept
		{
			return *isOpend;
		}

		JEditorMenuBar::JEditorMenuBar() {}
		JEditorMenuBar::~JEditorMenuBar() {}
		JMenuNode* JEditorMenuBar::GetSelectedNode()noexcept
		{
			return selectedNode;
		}
		bool JEditorMenuBar::UpdateMenuBar()
		{
			selectedNode = nullptr;
			JImGuiImpl::BeginMainMenuBar();
			{
				LoopNode(rootNode);
				JImGuiImpl::EndMenuBar();
			}
			return selectedNode != nullptr;
		}
		void JEditorMenuBar::LoopNode(JMenuNode* node)
		{
			std::string nodeName = node->GetMenuNodeName();
			if (node->IsLeafNode())
			{
				if (JImGuiImpl::MenuItem(nodeName.c_str(), node->IsOpendNode(), true))
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
				else if (JImGuiImpl::BeginMenu(nodeName))
				{
					for (uint i = 0; i < childrenCount; ++i)
						LoopNode(node->GetChild(i));
					JImGuiImpl::EndMenu();
				}
			}
		}
	}
}