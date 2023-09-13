#pragma once
#include"../JDockUpdateHelper.h"
#include"../../JEditorPageEnum.h"
namespace JinEngine
{
	namespace Editor
	{
		class JImGuiDockUpdateHelper final: public JDockUpdateHelper
		{
		private:
			const J_EDITOR_PAGE_TYPE pageType;
		private:
			GuiID pageID;
			GuiID dockSpaceID;
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
			JImGuiDockUpdateHelper(const J_EDITOR_PAGE_TYPE pageType);
		public:
			bool IsLastWindow()const noexcept final;
			bool IsLastDock()const noexcept final;
			bool IsLockSplitAcitvated()const noexcept final;
			bool IsLockOverAcitvated()const noexcept final;
			bool IsLockMove()const noexcept final;
		public:
			void Update(UpdateData& updateData) final;
		private:
			void UpdateDockNodeInfo(UpdateData& updateData);
			void UpdateExistingWindow(UpdateData& updateData)noexcept;
			void UpdateExistingDockNode(UpdateData& updateData)noexcept;
			void UpdateDraggingDockNode(UpdateData& updateData); 
			void RestrictDraggingTabBarSpace(UpdateData& updateData);
			void RollBackLastDockNode(UpdateData& updateData);
		};
	}
}