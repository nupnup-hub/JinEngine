#include"JEditorMenuBar.h"   
#include"../Gui/JGui.h"
#include"../Gui/JGuiImageInfo.h"
#include"../String/JEditorStringMap.h"
#include"../../Core/Utility/JCommonUtility.h"
#include"../../Core/Math/JVectorExtend.h"
 
namespace JinEngine
{
	namespace Editor
	{
		namespace 
		{
			static constexpr float tooltipInnerPaddingRate = 0.001f;
			static std::unordered_map <size_t, std::vector<JEditorMenuBar::SwitchIcon*>> switchGroupMap;
		}
		namespace
		{
			static bool IsClickedInLastItemArea(const JVector2<float>& preCursorPos)noexcept
			{
				if (!JGui::IsMouseClicked(Core::J_MOUSE_BUTTON::LEFT))
					return false;
				 
				return JGui::IsMouseInRect(preCursorPos, JGui::GetLastItemRectSize());
			}
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

		void JEditorMenuBar::UpdateData::SetUpdateStartState()noexcept
		{
			isContentsClick = isNextContentsClick = false;
		}
		void JEditorMenuBar::UpdateData::ClickContents()noexcept
		{
			isContentsClick = true;
		} 
		void JEditorMenuBar::UpdateData::ClickContentsNextFrame()noexcept
		{
			isNextContentsClick = true;
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

				if (JGui::IsMouseInRect(pos, size))
				{
					const JVector2<float> padding = JGui::GetWindowSize() * tooltipInnerPaddingRate;
					const JVector2<float> windowPos = pos + padding + size;
					JGui::DrawToolTipBox(GetUniqueLabel() + "_ToolTip", tooltip, windowPos, padding, true);
				}
			}
		}

		JEditorMenuBar::ButtonIcon::ButtonIcon(const size_t guid,
			std::unique_ptr<GetGResourceF::Functor>&& getGResourceFunctor,
			std::unique_ptr<Core::JBindHandleBase>&& pressBind)
			:Icon(guid, std::move(getGResourceFunctor)), pressBind(std::move(pressBind))
		{}
		void JEditorMenuBar::ButtonIcon::Update(const JEditorStringMap* tooltipMap, UpdateData& uData)
		{
			const float barHeight = JGui::GetWindowMenuBarSize().y;
			const JVector2<float> pos = JGui::GetCursorScreenPos();
			const JVector2<float> size = JVector2<float>(barHeight, barHeight);
			   
			/*
			const std::string name,
				JGuiImageInfo info,
				const JVector2<float>& size,
				const JVector2<float>& uv0 = JVector2<float>(0, 0),
				const JVector2<float>& uv1 = JVector2<float>(1, 1),
				float framePadding = -1,
				const JVector4<float>& bgCol = JVector4<float>(0, 0, 0, 0),
				const JVector4<float>& tintCol = JVector4<float>(1, 1, 1, 1)
			*/
			JGuiImageInfo imageInfo(((*GetGResourceFunctor())()));
			if (JGui::ImageButton(GetUniqueLabel(),
				imageInfo,
				size,
				JVector4F(0.7f, 0.7f, 0.7f, 0.88f),
				JVector4F(0.35f, 0.35f, 0.35f, 0)))
			{
				pressBind->InvokeCompletelyBind(); 
				uData.ClickContents();
			}
			if (IsClickedInLastItemArea(pos))
				uData.ClickContentsNextFrame();
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
		void JEditorMenuBar::SwitchIcon::Update(const JEditorStringMap* tooltipMap, UpdateData& uData)
		{
			const float barHeight = JGui::GetWindowMenuBarSize().y;
			const JVector2<float> pos = JGui::GetCursorScreenPos();			 
			const JVector2<float> size = JVector2<float>(barHeight, barHeight);
	 
			JGuiImageInfo imageInfo(((*GetGResourceFunctor())()));
			if (JGui::ImageSwitch(GetUniqueLabel(),
				imageInfo,
				*isActivatedPtr,
				true, 
				size,
				JVector4F(0.7f, 0.7f, 0.7f, 0.88f),
				JVector4F(0.35f, 0.35f, 0.35f, 0)))
			{
				//if to turn on
				if (IsActivated())
					onBind->InvokeCompletelyBind();
				else
					offBind->InvokeCompletelyBind();
				uData.ClickContents();
			}
			if (IsClickedInLastItemArea(pos))
				uData.ClickContentsNextFrame();
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
		JEditorMenuNode* JEditorMenuBar::GetRootNode()const noexcept
		{
			return rootNode;
		}
		JEditorMenuNode* JEditorMenuBar::GetSelectedNode()const noexcept
		{
			return selectedNode;
		}
		bool JEditorMenuBar::IsLastUpdateClickedContents()const noexcept
		{
			return  uData.isContentsClick;
		} 
		bool JEditorMenuBar::IsNextUpdateClickedContents()const noexcept
		{
			return  uData.isNextContentsClick;
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
			uData.SetUpdateStartState();
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
				if (JGui::BeginMainMenuBar())
				{
					LoopNode(rootNode);
					UpdateExtraWidget();
					JGui::EndMainMenuBar();
				}
			}
			else
			{
				if (JGui::BeginMenuBar())
				{
					LoopNode(rootNode);
					UpdateExtraWidget();
					JGui::EndMenuBar();
				}
			} 
			return selectedNode != nullptr;
		}
		void JEditorMenuBar::UpdateExtraWidget()
		{
			const uint extraWidgetCount = extraWidgetVec.size();
			for (uint i = 0; i < extraWidgetCount; ++i)
				extraWidgetVec[i]->Update(editStrMap.get(), uData);
		}
		void JEditorMenuBar::LoopNode(JEditorMenuNode* node)
		{
			std::string nodeName = node->GetNodeName();
			if (node->IsLeafNode())
			{
				const JVector2<float>preCursorPos = JGui::GetCursorScreenPos();
				const bool canToSoftCol = node->GetParent()->IsRootNode() && node->IsOpendNode();
				if (canToSoftCol)
					JGui::PushColorToSoft(J_GUI_COLOR::HEADER, CreateVec4(-0.15f));
				if (JGui::MenuItem(nodeName.c_str(), node->IsOpendNode(), true))
				{
					selectedNode = node;
					uData.ClickContents();
				}
				if (IsClickedInLastItemArea(preCursorPos))
					uData.ClickContentsNextFrame();
				if (canToSoftCol)
					JGui::PopColor();
			}
			else
			{
				const uint childrenCount = node->GetChildrenCount();
				if (node->IsRootNode())
				{
					for (uint i = 0; i < childrenCount; ++i)
						LoopNode(node->GetChild(i));
				}
				else if (JGui::BeginMenu(nodeName))
				{
					for (uint i = 0; i < childrenCount; ++i)
						LoopNode(node->GetChild(i));
					JGui::EndMenu();
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