#pragma once
#include"JEditorPopupType.h"
#include"../../Core/JDataType.h" 
#include"../../../Lib/imgui/imgui.h"
#include<vector>
#include<string>
#include<memory> 

namespace JinEngine
{
	namespace Editor
	{
		class JEditorString;
		class JEditorPopupNode;
		class JEditorPopup
		{
		private:
			const std::string name;
			JEditorPopupNode* popupRoot;
			std::vector<std::unique_ptr<JEditorPopupNode>>allPopupNode;
			bool isOpen;
		public:
			JEditorPopup(const std::string& name, std::unique_ptr<JEditorPopupNode> popupRoot);
			~JEditorPopup();
		public:
			//Update popup
			//should call api in owner window func
			void Update();
		public:
			void AddPopupNode(std::unique_ptr<JEditorPopupNode> child)noexcept;
			void ExecutePopup(_In_ JEditorString* editorString, _Out_ J_EDITOR_POPUP_NODE_RES& res, _Out_ size_t& clickedPopupGuid)noexcept;
		public:
			void SetOpen(bool value)noexcept;
		public:
			bool IsOpen()const noexcept;
			bool IsMouseInPopup()noexcept;
		};
	}
}