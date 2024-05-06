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
#include"JEditorPageEnum.h"
#include"../../Core/JDataType.h"
#include"../../Core/Func/Functor/JFunctor.h"
#include"../../Utility/JVector.h" 

namespace JinEngine
{
	namespace Editor
	{  
		class JEditorWindowDockUpdateHelper
		{
		public:
			struct UpdateData
			{
			public:
				J_EDITOR_PAGE_TYPE page;
				std::unique_ptr<Core::JBindHandleBase> rollbackBind = nullptr; 
			};
		private:
			const uint pageWndID;
			const uint dockSpaceID;
		private: 
			uint lastDockNodeID; 
			uint lastParentNodeID;
			bool hasLastParentNode;
			int lastDockNodeSplitDir;
			float lastDockNodeArea;
			uint lastDockTabItemCount;
			uint lastSameTabWindowID;
			JVector2<float> lastDockNodePos;
			JVector2<float> lastDockNodeSize;
		private:
			bool isLastWindow = false;
			bool isLastDock = false; 
			bool isLockSplit = false;
			bool isLockOver = false;
			bool isLockMove = false;
			bool requestRollBack = false;
		public:
			JEditorWindowDockUpdateHelper(const J_EDITOR_PAGE_TYPE page);
		public:
			bool IsLastWindow()const noexcept;
			bool IsLastDock()const noexcept; 
			bool IsLockSplitAcitvated()const noexcept;
			bool IsLockOverAcitvated()const noexcept;
			bool IsLockMove()const noexcept;
		public:
			void Update(UpdateData& updateData); 
		private:
			void UpdateDockNodeInfo(UpdateData& updateData);
			void UpdateExistingWindow(UpdateData& updateData)noexcept;
			void UpdateExistingDockNode(UpdateData& updateData)noexcept;
			void UpdateDraggingDockNode(UpdateData& updateData);
			void RestrictDraggingTitlebarSpace(UpdateData& updateData);
			void RollBackLastDockNode(UpdateData& updateData);
		};
	}
}