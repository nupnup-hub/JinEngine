/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once
#include"JEditorPopupType.h"
#include"../../Core/JCoreEssential.h"   

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
			bool isPopupContentsClicked = false;
			bool isLeafPopupContentsClicked = false; 
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
			bool IsPopupContentsClicked()const noexcept;
			bool IsLeafPopupContentsClicked()const noexcept;
		};
	}
}