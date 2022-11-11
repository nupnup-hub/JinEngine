#pragma once
#include"JEditorPopupType.h" 
#include"../../Core/JDataType.h"
#include"../../Utility/JVector.h" 
#include<vector>
#include<string> 

namespace JinEngine
{
	namespace Editor
	{
		class JEditorString;
		class JEditorPopupNode
		{
		protected:
			JEditorPopupNode* parent;
			const std::string name;
			const J_EDITOR_POPUP_NODE_TYPE nodeType;
			size_t nodeId;

			const bool hasTooltip;
			const bool hasShortCut;
			size_t tooltipId;
			size_t shortCutId;

			std::vector<JEditorPopupNode*> children;
			JVector2<float> popupPos;	//Root Internal
			JVector2<float> popupSize;	//Root Internal
			bool isOpen; 		//Internal
			bool isActivated;	//Toggle
		public:
			JEditorPopupNode(const std::string name,
				const J_EDITOR_POPUP_NODE_TYPE nodeType,
				JEditorPopupNode* parent,
				const bool hasTooltip = false,
				const bool hasShortCut = false);
			~JEditorPopupNode();
			size_t GetNodeId()const noexcept;
			size_t GetTooltipId()const noexcept;
			size_t GetShortCutId()const noexcept;
			J_EDITOR_POPUP_NODE_TYPE GetNodeType()const noexcept;
			void PopupOnScreen(_In_ JEditorString* editorString, _Out_ J_EDITOR_POPUP_NODE_RES& res, _Out_ std::size_t& clickMenuGuid);
			void PrintTooltip(_In_ JEditorString* editorString)noexcept;
			bool IsOpen()const noexcept;
			bool IsMouseInPopup()const noexcept;
		};
	}
}