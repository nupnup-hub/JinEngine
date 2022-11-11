#include"JEditorMenuBar.h"   
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"

namespace JinEngine
{
	namespace Editor
	{
		JMenuNode::JMenuNode(const std::string& nodeName, bool isRoot, bool isLeaf, bool* isOpend, JMenuNode* parent)
			:nodeName(nodeName + "##_MenuBarNode"),
			isRoot(isRoot),
			isLeaf(isLeaf),
			isOpend(isOpend),
			parent(parent)
		{
			if (!isRoot)
				parent->children.push_back(this);

			if (isLeaf && isOpend == nullptr)
			{
				JMenuNode::isOpend = new bool();
				isCreateOpendPtr = true;
			}
		}
		JMenuNode::~JMenuNode()
		{
			if (isCreateOpendPtr)
				delete isOpend;
		}
		std::string JMenuNode::GetNodeName()const noexcept
		{
			return nodeName;
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
		void JMenuNode::RegisterBind(std::unique_ptr<Core::JBindHandleBase>&& newBindHandle)
		{
			bindHandle = std::move(newBindHandle);
		}
		void JMenuNode::ExecuteBind()
		{
			if (bindHandle != nullptr)
				bindHandle->InvokeCompletelyBind();
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
			if(JImGuiImpl::BeginMainMenuBar())
			{
				LoopNode(rootNode);
				JImGuiImpl::EndMenuBar();
			}
			return selectedNode != nullptr;
		}
		void JEditorMenuBar::LoopNode(JMenuNode* node)
		{
			std::string nodeName = node->GetNodeName();
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