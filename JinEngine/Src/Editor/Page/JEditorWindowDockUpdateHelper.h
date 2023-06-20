#pragma once
#include"JEditorPageEnum.h"
#include"../../Core/JDataType.h"
#include"../../Core/Func/Functor/JFunctor.h"
#include"../../Utility/JVector.h"
#include<vector>

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