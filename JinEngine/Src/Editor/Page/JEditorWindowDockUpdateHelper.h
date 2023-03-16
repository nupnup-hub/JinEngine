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
			bool requestRollBack = false;
		public:
			JEditorWindowDockUpdateHelper(const J_EDITOR_PAGE_TYPE page);
		public:
			bool IsLastWindow()const noexcept;
			bool IsLastDock()const noexcept; 
			bool IsLockSplitAcitvated()const noexcept;
			bool IsLockOverAcitvated()const noexcept;
		public:
			void Update(UpdateData& updataData);
		private:
			void UpdateDockNodeInfo(UpdateData& updataData);
			void UpdateExistingWindow(UpdateData& updataData)noexcept;
			void UpdateExistingDockNode(UpdateData& updataData)noexcept;
			void UpdateDraggingDockNode(UpdateData& updataData);
			void RollBackLastDockNode(UpdateData& updataData);
		};
	}
}