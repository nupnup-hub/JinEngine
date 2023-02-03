#include"JEditorMenuBar.h"   
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../String/JEditorStringMap.h"

namespace JinEngine
{
	namespace Editor
	{
		namespace Constants
		{
			static constexpr float tooltipInnerPaddingRate = 0.001f;
		}
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

		JEditorMenuBar::ExtraWidget::ExtraWidget(const size_t guid)
			:guid(guid)
		{}
		std::string JEditorMenuBar::ExtraWidget::GetUniqueLabel()const noexcept
		{
			return "##" + std::to_string(guid);
		}

		JEditorMenuBar::SwitchIcon::SwitchIcon(const size_t guid,
			bool* newIsActivatedPtr,
			std::unique_ptr<GetGResourceF::Functor>&& newGetGResourceFunctor,
			std::unique_ptr<Core::JBindHandleBase>&& newPressBind)
			: ExtraWidget(guid)
		{
			isActivatedPtr = newIsActivatedPtr;
			getGResourceFunctor = std::move(newGetGResourceFunctor);
			pressBind = std::move(newPressBind);
		}
		void JEditorMenuBar::SwitchIcon::Update(const JEditorStringMap* tooltipMap)
		{
			const JVector2<float> switchPos = ImGui::GetCursorScreenPos();
			const float barHeight = ImGui::GetCurrentWindow()->MenuBarHeight();
			const JVector2<float> iconSize = JVector2<float>(barHeight, barHeight);
			if (JImGuiImpl::ImageSwitch(GetUniqueLabel().c_str(),
				*((*getGResourceFunctor)()),
				*isActivatedPtr,
				iconSize,
				IM_COL32(180, 180, 180, 225),
				IM_COL32(90, 90, 90, 0)))
				pressBind->InvokeCompletelyBind();

			if (tooltipMap != nullptr)
			{
				std::string tooltip = tooltipMap->GetString(GetGuid());
				if (tooltip.empty())
					return;
				 
				if (JImGuiImpl::IsMouseInRect(switchPos, iconSize))
				{
					const JVector2<float> padding = ImGui::GetWindowSize() * Constants::tooltipInnerPaddingRate;
					const JVector2<float> windowPos = switchPos + padding + iconSize;			 
					JImGuiImpl::DrawToolTipBox(GetUniqueLabel() + "_ToolTip", tooltip.c_str(), windowPos, padding, true);
				}
			}  
		}

		JEditorMenuBar::JEditorMenuBar(std::unique_ptr<JEditorMenuNode> newRoot, const bool isMainMenu)
			:rootNode(newRoot.get()), isMainMenu(isMainMenu)
		{
			editStrMap = std::make_unique<JEditorStringMap>();
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
		void JEditorMenuBar::RegisterExtraWidget(std::unique_ptr<ExtraWidget>&& newExtraWidget)noexcept
		{
			extraWidgetVec.push_back(std::move(newExtraWidget));
		}
		bool JEditorMenuBar::RegisterEditorString(const size_t guid, const std::vector<std::string>& strVec)noexcept
		{
			return editStrMap->AddString(guid, strVec);
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
					if (leafNode[i]->IsOpendNode())
						leafNode[i]->ExecuteUpdateBind();
				}
			}
			else
				LoopNode(rootNode, [](JEditorMenuNode* node) {if (node->IsOpendNode())node->ExecuteUpdateBind(); });
		}
		bool JEditorMenuBar::UpdateMenuBar()
		{
			selectedNode = nullptr;
			if (isMainMenu)
			{
				if (JImGuiImpl::BeginMainMenuBar())
				{
					LoopNode(rootNode);
					UpdateExtraWidget();
					JImGuiImpl::EndMainMenuBar();
				}
			}
			else
			{
				if (JImGuiImpl::BeginMenuBar())
				{
					LoopNode(rootNode);
					UpdateExtraWidget();
					JImGuiImpl::EndMenuBar();
				}
			}
			return selectedNode != nullptr;
		}
		void JEditorMenuBar::UpdateExtraWidget()
		{
			const uint extraWidgetCount = extraWidgetVec.size();
			for (uint i = 0; i < extraWidgetCount; ++i)
				extraWidgetVec[i]->Update(editStrMap.get());
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
					JImGuiImpl::SetColorToDeep(ImGuiCol_Header, 0.15f);
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