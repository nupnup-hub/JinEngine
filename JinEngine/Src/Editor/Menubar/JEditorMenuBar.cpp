#include"JEditorMenuBar.h"   
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../String/JEditorStringMap.h"
#include"../../Utility/JVectorExtend.h"
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Editor
	{
		namespace 
		{
			static constexpr float tooltipInnerPaddingRate = 0.001f;
			static std::unordered_map <size_t, std::vector<JEditorMenuBar::SwitchIcon*>> switchGroupMap;
		}
	 
		JEditorMenuNode::JEditorMenuNode(const std::string& nodeName, 
			bool isRoot,
			bool isLeaf,
			bool isControlOpendPtr,
			bool* isOpend,
			JEditorMenuNode* parent)
			:nodeName(nodeName + "##_MenuBarNode"),
			isRoot(isRoot),
			isLeaf(isLeaf),
			isControlOpendPtr(isControlOpendPtr),
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
			std::unique_ptr<Core::JBindHandleBase>&& newCloseBindHandle,
			std::unique_ptr<Core::JBindHandleBase>&& newActivateBindHandle,
			std::unique_ptr<Core::JBindHandleBase>&& newDeActivateBindHandle,
			std::unique_ptr<Core::JBindHandleBase>&& newUpdateBindHandle)
		{
			if (newOpenBindHandle != nullptr)
				openBindHandle = std::move(newOpenBindHandle);
			if (newCloseBindHandle != nullptr)
				closeBindHandle = std::move(newCloseBindHandle);
			if (newActivateBindHandle != nullptr)
				activateBindHandle = std::move(newActivateBindHandle);
			if (newDeActivateBindHandle != nullptr)
				deActivateBindHandle = std::move(newDeActivateBindHandle);
			if (newUpdateBindHandle != nullptr)
				updateBindHandle = std::move(newUpdateBindHandle);
		}
		void JEditorMenuNode::ExecuteOpenBind()
		{
			if (isControlOpendPtr)
				*isOpend = true;

			if (openBindHandle != nullptr)
				openBindHandle->InvokeCompletelyBind();
		}
		void JEditorMenuNode::ExecuteCloseBind()
		{
			if (isControlOpendPtr)
				*isOpend = false;

			if (closeBindHandle != nullptr)
				closeBindHandle->InvokeCompletelyBind();
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

		JEditorMenuBar::Icon::Icon(const size_t guid, std::unique_ptr<GetGResourceF::Functor>&& getGResourceFunctor)
			:ExtraWidget(guid), getGResourceFunctor(std::move(getGResourceFunctor))
		{}
		JEditorMenuBar::Icon::GetGResourceF::Functor* JEditorMenuBar::Icon::GetGResourceFunctor()const noexcept
		{
			return getGResourceFunctor.get();
		}
		void JEditorMenuBar::Icon::DisplayTooltip(const JEditorStringMap* tooltipMap,
			const JVector2<float> pos,
			const JVector2<float> size)
		{
			if (tooltipMap != nullptr)
			{
				std::string tooltip = tooltipMap->GetString(GetGuid());
				if (tooltip.empty())
					return;

				if (JImGuiImpl::IsMouseInRect(pos, size))
				{
					const JVector2<float> padding = ImGui::GetWindowSize() * tooltipInnerPaddingRate;
					const JVector2<float> windowPos = pos + padding + size;
					JImGuiImpl::DrawToolTipBox(GetUniqueLabel() + "_ToolTip", tooltip, windowPos, padding, true);
				}
			}
		}

		JEditorMenuBar::ButtonIcon::ButtonIcon(const size_t guid,
			std::unique_ptr<GetGResourceF::Functor>&& getGResourceFunctor,
			std::unique_ptr<Core::JBindHandleBase>&& pressBind)
			:Icon(guid, std::move(getGResourceFunctor)), pressBind(std::move(pressBind))
		{}
		void JEditorMenuBar::ButtonIcon::Update(const JEditorStringMap* tooltipMap)
		{
			const float barHeight = ImGui::GetCurrentWindow()->MenuBarHeight();
			const JVector2<float> pos = ImGui::GetCursorScreenPos();
			const JVector2<float> size = JVector2<float>(barHeight, barHeight);
			   
			if (JImGuiImpl::ImageButton(GetUniqueLabel(),
				((*GetGResourceFunctor())()),
				size,
				IM_COL32(180, 180, 180, 225),
				IM_COL32(90, 90, 90, 0)))
			{
				pressBind->InvokeCompletelyBind();
			}
			DisplayTooltip(tooltipMap, pos, size);
		}


		JEditorMenuBar::SwitchIcon::SwitchIcon(const size_t guid,
			std::unique_ptr<GetGResourceF::Functor>&& getGResourceFunctor,
			std::unique_ptr<Core::JBindHandleBase>&& onBind,
			std::unique_ptr<Core::JBindHandleBase>&& offBind,
			bool* isActivatedPtr)
			:Icon(guid, std::move(getGResourceFunctor)),
			onBind(std::move(onBind)),
			offBind(std::move(offBind)),
			isActivatedPtr(isActivatedPtr)
		{}
		void JEditorMenuBar::SwitchIcon::Update(const JEditorStringMap* tooltipMap)
		{
			const float barHeight = ImGui::GetCurrentWindow()->MenuBarHeight();
			const JVector2<float> pos = ImGui::GetCursorScreenPos();			 
			const JVector2<float> size = JVector2<float>(barHeight, barHeight);
			 
			if (JImGuiImpl::ImageSwitch(GetUniqueLabel(),
				((*GetGResourceFunctor())()),
				*isActivatedPtr,
				true, 
				size,
				IM_COL32(180, 180, 180, 225),
				IM_COL32(90, 90, 90, 0)))
			{
				//if to turn on
				if (IsActivated())
					onBind->InvokeCompletelyBind();
				else
					offBind->InvokeCompletelyBind();				 
			}
			DisplayTooltip(tooltipMap, pos, size);
		}
		bool JEditorMenuBar::SwitchIcon::IsActivated()const noexcept
		{
			return *isActivatedPtr;
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
			{
				auto selectedNode = GetSelectedNode();
				if (selectedNode->IsOpendNode())
					selectedNode->ExecuteCloseBind();
				else
					selectedNode->ExecuteOpenBind();
			}

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
				bool canToSoftCol = node->GetParent()->IsRootNode() && node->IsOpendNode();
				if (canToSoftCol)
					JImGuiImpl::SetColorToSoft(ImGuiCol_Header, CreateVec4(-0.15f));
				if (JImGuiImpl::MenuItem(nodeName.c_str(), node->IsOpendNode(), true))
					selectedNode = node;
				if (canToSoftCol)
					JImGuiImpl::SetColorToSoft(ImGuiCol_Header, CreateVec4(0.15f));
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