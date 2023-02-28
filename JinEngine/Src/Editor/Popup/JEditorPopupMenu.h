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
		class JEditorStringMap;
		class JEditorPopupNode;
		class JEditorPopupMenu
		{
		private:
			const std::string name;
			JEditorPopupNode* popupRoot;
			std::vector<std::unique_ptr<JEditorPopupNode>>allPopupNode;
			bool isOpen;
			bool isLastSelected = false;
		public:
			JEditorPopupMenu(const std::string& name, std::unique_ptr<JEditorPopupNode> popupRoot);
			~JEditorPopupMenu(); 
		public: 
			//Update popup life
			void Update();
		public:
			void AddPopupNode(std::unique_ptr<JEditorPopupNode> child)noexcept;
			//should call api in owner window func
			void ExecutePopup(_In_ JEditorStringMap* editorString)noexcept;
		public:
			void SetOpen(bool value)noexcept;
		public:
			bool IsOpen()const noexcept;
			bool IsMouseInPopup()const noexcept;
			//It is valid until call update
			bool IsLastSelected()const noexcept;
		};
	}
}