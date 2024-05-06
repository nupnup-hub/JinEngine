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
#include"../../Core/Func/Functor/JFunctor.h"
#include"../../Core/Math/JVector.h" 

namespace JinEngine
{
	namespace Editor
	{
		class JEditorStringMap;
		class JEditorWindow;
		class JEditorPopupNode
		{ 
		private:
			const std::string name;
			const J_EDITOR_POPUP_NODE_TYPE nodeType;
			const size_t nodeId;
		private:
			JEditorPopupNode* parent;
			std::vector<JEditorPopupNode*> children;
		private:
			const bool hasTooltip;
			const bool hasShortCut;
			const size_t tooltipId;
			const size_t shortCutId;
		private:
			JVector2<float> popupPos;	//Root Internal
			JVector2<float> popupSize;	//Root Internal
			bool isOpen; 		//Internal
			bool isActivated;	//Toggle
		private:
			std::unique_ptr<Core::JBindHandleBase> selectBind;	//valid in leaf node 
			std::unique_ptr<Core::JBindHandleBase> enableBind;	//valid in leaf node 
		public:
			JEditorPopupNode(const std::string name,
				const J_EDITOR_POPUP_NODE_TYPE nodeType,
				JEditorPopupNode* parent,
				const bool hasTooltip = false,
				const bool hasShortCut = false);
			~JEditorPopupNode();
		public:
			void RegisterSelectBind(std::unique_ptr<Core::JBindHandleBase>&& newSelectBind)noexcept;
			void RegisterEnableBind(std::unique_ptr<Core::JBindHandleBase>&& newEnableBind)noexcept;
			void InvokeSelectBind()noexcept;
		public:
			JEditorPopupNode* PopupOnScreen(_In_ JEditorStringMap* editorString);
			void PrintTooltip(_In_ JEditorStringMap* editorString)noexcept;
		public:
			size_t GetNodeId()const noexcept;
			size_t GetTooltipId()const noexcept;
			size_t GetShortCutId()const noexcept;
			J_EDITOR_POPUP_NODE_TYPE GetNodeType()const noexcept;
		public:
			bool IsOpen()const noexcept;
			bool IsMouseInPopup()const noexcept;
		};
	}
}