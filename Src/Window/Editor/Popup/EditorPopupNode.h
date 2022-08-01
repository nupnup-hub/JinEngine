#pragma once
#include"EditorPopupType.h"
#include"../ImGuiEx/ImGuiEx.h"
#include"../../../Core/JDataType.h" 
#include<vector>
#include<string> 

namespace JinEngine
{
	class EditorString;
	class EditorPopupNode
	{
	protected:
		EditorPopupNode* parent;
		const std::string name;
		const EDITOR_POPUP_NODE_TYPE nodeType;
		size_t nodeId;

		const bool hasTooltip;
		const bool hasShortCut;
		size_t tooltipId;
		size_t shortCutId;

		std::vector<EditorPopupNode*> children;
		ImVec2 popupPos;	//Root Internal
		ImVec2 popupSize;	//Root Internal
		bool isOpen; 		//Internal
		bool isActivated;	//Toggle
	public:
		EditorPopupNode(const std::string name,
			const EDITOR_POPUP_NODE_TYPE nodeType,
			EditorPopupNode* parent,
			const bool hasTooltip = false,
			const bool hasShortCut = false);
		~EditorPopupNode(); 
		size_t GetNodeId()const noexcept;
		size_t GetTooltipId()const noexcept;
		size_t GetShortCutId()const noexcept;
		EDITOR_POPUP_NODE_TYPE GetNodeType()const noexcept;
		void PopupOnScreen(_In_ EditorString* editorString, _Out_ EDITOR_POPUP_NODE_RES& res, _Out_ std::size_t& clickMenuGuid);
		void PrintTooltip(_In_ EditorString* editorString)noexcept;
		bool IsOpen()const noexcept;
		bool IsMouseInPopup()const noexcept;
		static void MakeNodeId(EditorPopupNode* parent,
			const std::string& name,
			_Out_ size_t& nodeId,
			_Out_ size_t& tooltipId,
			_Out_ size_t& shortCutId)noexcept;
	};
}