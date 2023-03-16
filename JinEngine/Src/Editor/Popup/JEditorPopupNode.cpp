#include"JEditorPopupNode.h" 
#include"../String/JEditorStringMap.h" 
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
			nodeId (Core::MakeGuid()),
			parent(parent),
			hasTooltip(hasTooltip),
			hasShortCut(hasShortCut),
			tooltipId(Core::MakeGuid()),
			shortCutId(Core::MakeGuid())
		{
			if (parent != nullptr)
				parent->children.push_back(this); 
		}
		JEditorPopupNode::~JEditorPopupNode() {}
		JEditorPopupNode* JEditorPopupNode::PopupOnScreen(_In_ JEditorStringMap* editorString)
		{
			JEditorPopupNode* selected = nullptr;
			const uint childrenCount = (uint)children.size();
			switch (nodeType)
			{
			case J_EDITOR_POPUP_NODE_TYPE::ROOT:
			{
				popupPos = JImGuiImpl::GetGuiWindowPos();
				popupSize = JImGuiImpl::GetGuiWindowSize();
				for (uint i = 0; i < childrenCount; ++i)
				{
					JEditorPopupNode* res = children[i]->PopupOnScreen(editorString);
					if (res != nullptr)
						selected = res;
				}
				break;
			}
			case J_EDITOR_POPUP_NODE_TYPE::INTERNAL:
			{
				if (ImGui::BeginMenu(editorString->GetString(nodeId).c_str()))
				{
					isOpen = true;
					popupPos = JImGuiImpl::GetGuiWindowPos();
					popupSize = JImGuiImpl::GetGuiWindowSize();
					for (uint i = 0; i < childrenCount; ++i)
					{
						JEditorPopupNode* res = children[i]->PopupOnScreen(editorString);
						if (res != nullptr)
							selected = res;
					}
					ImGui::EndMenu();
				}
				else
					isOpen = false;
				PrintTooltip(editorString);
				break;
			}
			case J_EDITOR_POPUP_NODE_TYPE::LEAF:
			{
				bool isEnable = enableBind != nullptr ? enableBind->Invoke() : true;
				if (hasShortCut)
				{
					if (ImGui::MenuItem(editorString->GetString(nodeId).c_str(), editorString->GetString(shortCutId).c_str(), false, isEnable))
						selected = this;
					PrintTooltip(editorString);
				}
				else
				{
					if (ImGui::MenuItem(editorString->GetString(nodeId).c_str(), 0, false, isEnable))
						selected = this;
					PrintTooltip(editorString);
				}
				break;
			}
			case J_EDITOR_POPUP_NODE_TYPE::LEAF_TOGGLE:
			{
				bool isEnable = enableBind != nullptr ? enableBind->Invoke() : true;
				if (hasShortCut)
				{
					if (ImGui::MenuItem(editorString->GetString(nodeId).c_str(), editorString->GetString(shortCutId).c_str(), &isActivated, isEnable))
						selected = this;
					PrintTooltip(editorString);
				}
				else
				{
					if (ImGui::MenuItem(editorString->GetString(nodeId).c_str(), "", &isActivated, isEnable))
						selected = this;
					PrintTooltip(editorString);
				}
				break;
			}
			case J_EDITOR_POPUP_NODE_TYPE::LEAF_CHECK_BOX:
				break;
			default:
				break;
			}
			return selected;
		}
		void JEditorPopupNode::PrintTooltip(_In_ JEditorStringMap* editorString)noexcept
		{
			if (hasTooltip)
			{
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip(editorString->GetString(tooltipId).c_str());
			}
		}
		void JEditorPopupNode::RegisterSelectBind(std::unique_ptr<Core::JBindHandleBase>&& newSelectBind)noexcept
		{
			selectBind = std::move(newSelectBind);
		}
		void JEditorPopupNode::RegisterEnableBind(std::unique_ptr<EnableF::CompletelyBind>&& newEnableBind)noexcept
		{
			enableBind = std::move(newEnableBind);
		}
		void JEditorPopupNode::InvokeSelectBind()noexcept
		{
			if (selectBind != nullptr)
				selectBind->InvokeCompletelyBind();
		}
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