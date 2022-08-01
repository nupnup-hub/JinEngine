#pragma once
#include"EditorPopupType.h"
#include"../../../Core/JDataType.h" 
#include"../../../../Lib/imgui/imgui.h"
#include<vector>
#include<string>
#include<memory> 

namespace JinEngine
{ 
	class EditorString;
	class EditorPopupNode;
	class EditorPopup
	{	
	private:
		const std::string name;
		EditorPopupNode* popupRoot;
		std::vector<std::unique_ptr<EditorPopupNode>>allPopupNode;
		bool isOpen; 
	public:
		EditorPopup(const std::string& name, std::unique_ptr<EditorPopupNode> popupRoot);
		~EditorPopup();
		void AddPopupNode(std::unique_ptr<EditorPopupNode> child)noexcept;
		void ExecutePopup(_In_ EditorString* editorString, _Out_ EDITOR_POPUP_NODE_RES& res, _Out_ size_t& clickMenuGuid)noexcept;
		bool IsOpen()const noexcept;
		bool IsMouseInPopup()noexcept; 
		void SetOpen(bool value)noexcept;
	};
}