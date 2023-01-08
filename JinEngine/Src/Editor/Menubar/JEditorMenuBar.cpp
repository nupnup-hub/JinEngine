#include"JEditorMenuBar.h"   
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"

namespace JinEngine
{
	namespace Editor
	{
		JEditorMenuNode::JEditorMenuNode(const std::string& nodeName, bool isRoot, bool isLeaf, bool* isOpend, JEditorMenuNode* parent)
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
				JEditorMenuNode::isOpend = new bool();
				isCreateOpendPtr = true;
			}
		}
		JEditorMenuNode::~JEditorMenuNode()
		{
			if (isCreateOpendPtr)
				delete isOpend;
		}
		std::string JEditorMenuNode::GetNodeName()const noexcept
		{
			return nodeName;
		}
		const uint JEditorMenuNode::GetChildrenCount()const noexcept
		{
			return (uint)children.size();
		}
		JEditorMenuNode* JEditorMenuNode::GetParent()const noexcept
		{
			return parent;
		}
		JEditorMenuNode* JEditorMenuNode::GetChild(const uint index)const noexcept
		{
			if (children.size() <= index)
				return nullptr;
			else
				return children[index];
		}
		bool JEditorMenuNode::IsRootNode()const noexcept
		{
			return isRoot;
		}
		bool JEditorMenuNode::IsLeafNode()const noexcept
		{
			return isLeaf;
		}
		bool JEditorMenuNode::IsOpendNode()const noexcept
		{
			return *isOpend;
		}
		void JEditorMenuNode::RegisterBindHandle(std::unique_ptr<Core::JBindHandleBase>&& newOpenBindHandle,
			std::unique_ptr<Core::JBindHandleBase>&& newActivateBindHandle,
			std::unique_ptr<Core::JBindHandleBase>&& newDeActivateBindHandle,
			std::unique_ptr<Core::JBindHandleBase>&& newUpdateBindHandle)
		{
			if (newOpenBindHandle != nullptr)
				openBindHandle = std::move(newOpenBindHandle);
			if (newActivateBindHandle != nullptr)
				activateBindHandle = std::move(newActivateBindHandle);
			if (newDeActivateBindHandle != nullptr)
				deActivateBindHandle = std::move(newDeActivateBindHandle);
			if (newUpdateBindHandle != nullptr)
				updateBindHandle = std::move(newUpdateBindHandle);
		}
		void JEditorMenuNode::ExecuteOpenBind()
		{
			if (openBindHandle != nullptr)
				openBindHandle->InvokeCompletelyBind();
		}
		void JEditorMenuNode::ExecuteActivateBind()
		{
			if (activateBindHandle != nullptr)
				activateBindHandle->InvokeCompletelyBind();
		}
		void JEditorMenuNode::ExecuteDeActivateBind()
		{
			if (deActivateBindHandle != nullptr)
				deActivateBindHandle->InvokeCompletelyBind();
		}
		void JEditorMenuNode::ExecuteUpdateBind()
		{
			if (updateBindHandle != nullptr)
				updateBindHandle->InvokeCompletelyBind();
		}

		JEditorMenuBar::JEditorMenuBar(std::unique_ptr<JEditorMenuNode> newRoot, const bool isMainMenu)
			:rootNode(newRoot.get()), isMainMenu(isMainMenu)
		{
			allNode.push_back(std::move(newRoot));
		}
		JEditorMenuBar::~JEditorMenuBar() {}
		JEditorMenuNode* JEditorMenuBar::GetRootNode()noexcept
		{
			return rootNode;
		}
		JEditorMenuNode* JEditorMenuBar::GetSelectedNode()noexcept
		{
			return selectedNode;
		}
		void JEditorMenuBar::AddNode(std::unique_ptr<JEditorMenuNode> newNode)noexcept
		{
			if (newNode->IsLeafNode())
				leafNode.push_back(newNode.get());
			allNode.push_back(std::move(newNode));
		}
		void JEditorMenuBar::RegisterExtraWidgetBind(std::unique_ptr<Core::JBindHandleBase> newExtraWidgetBind)noexcept
		{ 
			extraWidgetBind = std::move(newExtraWidgetBind);
		}
		void JEditorMenuBar::Update(const bool leafNodeOnly)
		{
			bool isSelected = UpdateMenuBar();
			if (isSelected)
				GetSelectedNode()->ExecuteOpenBind();
			
			if (leafNodeOnly)
			{
				const uint leafNodeCount = (uint)leafNode.size();
				for (uint i = 0; i < leafNodeCount; ++i)
				{
					if(leafNode[i]->IsOpendNode())
						leafNode[i]->ExecuteUpdateBind();
				}
			}
			else
				LoopNode(rootNode, [](JEditorMenuNode* node) {if(node->IsOpendNode())node->ExecuteUpdateBind(); });
		}
		bool JEditorMenuBar::UpdateMenuBar()
		{
			selectedNode = nullptr;
			if (isMainMenu)
			{
				if (JImGuiImpl::BeginMainMenuBar())
				{
					LoopNode(rootNode);
					if (extraWidgetBind != nullptr)
						extraWidgetBind->InvokeCompletelyBind();
					JImGuiImpl::EndMainMenuBar();
				}
			}
			else
			{
				if (JImGuiImpl::BeginMenuBar())
				{
					LoopNode(rootNode);
					if (extraWidgetBind != nullptr)
						extraWidgetBind->InvokeCompletelyBind();
					JImGuiImpl::EndMenuBar();
				}
			}
			return selectedNode != nullptr;
		}
		void JEditorMenuBar::LoopNode(JEditorMenuNode* node)
		{
			std::string nodeName = node->GetNodeName();
			if (node->IsLeafNode())
			{
				if (node->GetParent()->IsRootNode() && node->IsOpendNode())
					JImGuiImpl::SetColorToDeep(ImGuiCol_Header, -0.15f);
				if (JImGuiImpl::MenuItem(nodeName.c_str(), node->IsOpendNode(), true))
					selectedNode = node;
				if (node->GetParent()->IsRootNode())
					JImGuiImpl::SetColorToDefault(ImGuiCol_Header);
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
		void JEditorMenuBar::LoopNode(JEditorMenuNode* node, LoopNodePtr ptr)
		{
			(*ptr)(node);
			const uint childrenCount = node->GetChildrenCount();
			for (uint i = 0; i < childrenCount; ++i)
				LoopNode(node->GetChild(i), ptr);
		}
		void JEditorMenuBar::ActivateOpenNode(const bool leafNodeOnly)
		{
			if (leafNodeOnly)
			{
				const uint leafNodeCount = (uint)leafNode.size();
				for (uint i = 0; i < leafNodeCount; ++i)
				{
					if (leafNode[i]->IsOpendNode())
						leafNode[i]->ExecuteActivateBind();
				}
			}
			else
				LoopNode(rootNode, [](JEditorMenuNode* node) {if (node->IsOpendNode())node->ExecuteActivateBind(); });
		}
		void JEditorMenuBar::DeActivateOpenNode(const bool leafNodeOnly)
		{
			if (leafNodeOnly)
			{
				const uint leafNodeCount = (uint)leafNode.size();
				for (uint i = 0; i < leafNodeCount; ++i)
				{
					if (leafNode[i]->IsOpendNode())
						leafNode[i]->ExecuteDeActivateBind();
				}
			}
			else
				LoopNode(rootNode, [](JEditorMenuNode* node) {if (node->IsOpendNode())node->ExecuteDeActivateBind(); });
		}
	}
}