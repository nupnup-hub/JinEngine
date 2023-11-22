#include"JImGuiDockUpdateHelper.h"    
#include"../../../Gui/JGui.h" 
#include"../../JEditorPageShareData.h"
#include"../../../../Core/Utility/JCommonUtility.h"
#include"../../../../Core/Geometry/JBBox.h"
#include"../../../../../ThirdParty/imgui/imgui.h"
#include"../../../../../ThirdParty/imgui/imgui_internal.h"
 
namespace JinEngine
{
	namespace Editor
	{
		namespace Private
		{
			static ImGuiDockNode* GetDockNodeRoot(ImGuiDockNode* dock)noexcept
			{
				if (dock->IsRootNode())
					return dock;

				return GetDockNodeRoot(dock->ParentNode);
			}
			static ImGuiDockNode* GetDockNode(ImGuiDockNode* dock, const ImGuiID id)
			{
				if (dock == nullptr)
					return nullptr;

				ImGuiDockNode* result = nullptr;
				if (!dock->IsLeafNode())
				{
					ImGuiDockNode* left = GetDockNode(dock->ChildNodes[0], id);
					ImGuiDockNode* right = GetDockNode(dock->ChildNodes[1], id);
					result = left ? left : right;
				}
				if (dock->ID == id)
					result = dock;

				return result;
			}
			static int ActivatedWindowCount(ImGuiDockNode* dock)
			{
				if (dock == nullptr)
					return 0;

				int actCount = 0;
				if (!dock->IsLeafNode())
				{
					actCount += ActivatedWindowCount(dock->ChildNodes[0]);
					actCount += ActivatedWindowCount(dock->ChildNodes[1]);
				}

				const int wndSize = (int)dock->Windows.size();
				for (int i = 0; i < wndSize; ++i)
				{
					if (dock->Windows[i]->WasActive)
						++actCount;
				}
				return actCount;
			}
			static ImGuiWindow* IntersectPageWindow(const JVector2<float>& pos)
			{
				struct IntersectData
				{
				public:
					ImGuiWindow* window = nullptr;
					int order = 0;
				};

				IntersectData data[(int)J_EDITOR_PAGE_TYPE::COUNT];
				for (int i = 0; i < (int)J_EDITOR_PAGE_TYPE::COUNT; ++i)
				{
					J_EDITOR_PAGE_TYPE type = (J_EDITOR_PAGE_TYPE)i;
					if (JEditorPageShareData::IsRegisteredPage(type))
					{ 
						 data[i].window = ImGui::FindWindowByID(JEditorPageShareData::GetPageGuiWindowID(type));
						 if (data[i].window == nullptr)
							 continue;
						 data[i].order = data[i].window->BeginOrderWithinContext; 
					}
				}

				int hitIndex = -1;
				int hitOrder = INT_MAX;

				for (int i = 0; i < (int)J_EDITOR_PAGE_TYPE::COUNT; ++i)
				{
					if (data[i].window == nullptr || !data[i].window->WasActive)
						continue;

					if (Core::JBBox2D{ data[i].window->Pos, data[i].window->Pos + data[i].window->Size }.Contain(pos))
					{
						if (hitOrder > data[i].order)
							hitIndex = i;
					}
				}
				return hitIndex != -1 ? data[hitIndex].window : nullptr;
			}
		}
		 
		JImGuiDockUpdateHelper::JImGuiDockUpdateHelper(const J_EDITOR_PAGE_TYPE pageType)
			:pageType(pageType)
		{
			pageID = ImHashStr(JEditorPageShareData::GetPageName(pageType).c_str());
			dockSpaceID = ImHashStr(JEditorPageShareData::GetPageDockSpaceName(pageType).c_str());
		}
		bool JImGuiDockUpdateHelper::IsLastWindow()const noexcept
		{
			return isLastWindow;
		}
		bool JImGuiDockUpdateHelper::IsLastDock()const noexcept
		{
			return isLastDock;
		}
		bool JImGuiDockUpdateHelper::IsLockSplitAcitvated()const noexcept
		{
			return isLockSplit;
		}
		bool JImGuiDockUpdateHelper::IsLockOverAcitvated()const noexcept
		{
			return isLockOver;
		}
		bool JImGuiDockUpdateHelper::IsLockMove()const noexcept
		{
			return isLockMove;
		}
		void JImGuiDockUpdateHelper::Update(UpdateData& updataData)
		{
			isLastWindow = isLastDock = isLockSplit = isLockOver = isLockMove = false; 

			UpdateDockNodeInfo(updataData);
			UpdateExistingWindow(updataData);
			UpdateExistingDockNode(updataData);
			UpdateDraggingDockNode(updataData);
			RestrictDraggingTabBarSpace(updataData); 
			RollBackLastDockNode(updataData);
		}
		void JImGuiDockUpdateHelper::UpdateDockNodeInfo(UpdateData& updataData)
		{
			ImGuiWindow* curWnd = ImGui::GetCurrentWindow();
			ImGuiDockNode* dockNode = ImGui::GetWindowDockNode();
			if (dockNode != nullptr)
			{
				lastDockNodeID = dockNode->ID;
				lastParentNodeID = dockNode->ParentNode ? dockNode->ParentNode->ID : -1;
				lastDockTabItemCount = dockNode->TabBar ? dockNode->TabBar->Tabs.size() : 0;
				lastDockNodePos = dockNode->Pos;
				lastDockNodeSize = dockNode->Size;

				if (lastParentNodeID != -1)
				{
					ImGuiDockNode* lastParentDock = dockNode->ParentNode;
					if (lastDockNodePos.x != lastParentDock->Pos.x)
					{
						if (lastDockNodePos.x > lastParentDock->Pos.x)
							lastDockNodeSplitDir = ImGuiDir_Right;
						else
							lastDockNodeSplitDir = ImGuiDir_Left;
					}
					else
					{
						if (lastDockNodePos.y > lastParentDock->Pos.y)
							lastDockNodeSplitDir = ImGuiDir_Down;
						else
							lastDockNodeSplitDir = ImGuiDir_Up;
					}
					lastDockNodeArea = (lastDockNodeSize.x * lastDockNodeSize.y) / (lastParentDock->Size.x * lastParentDock->Size.y);
					hasLastParentNode = true;
				}
				else
				{
					lastDockNodeSplitDir = ImGuiDir_Up;
					lastDockNodeArea = 1;
					hasLastParentNode = false;
				}
				if (lastDockTabItemCount > 0)
				{
					for (const auto& data : dockNode->TabBar->Tabs)
					{
						if (data.ID != curWnd->ID)
						{
							lastSameTabWindowID = data.ID;
							break;
						}
					}
				}
				requestRollBack = false;
			}
		}
		void JImGuiDockUpdateHelper::UpdateExistingWindow(UpdateData& updataData)noexcept
		{
			ImGuiDockNode* dockNode = ImGui::GetWindowDockNode();
			if (dockNode == nullptr)
				return;

			int actCount = Private::ActivatedWindowCount(Private::GetDockNodeRoot(dockNode));
			if (actCount == 1)
			{ 
				isLastWindow = true;
				isLockMove = true; 
			}
		}
		void JImGuiDockUpdateHelper::UpdateExistingDockNode(UpdateData& updataData)noexcept
		{
			ImGuiDockNode* dockNode = ImGui::GetWindowDockNode();
			if (dockNode == nullptr)
				return;

			ImGuiDockNode* parentDockNode = dockNode->ParentNode;
			if (parentDockNode != nullptr && parentDockNode->IsRootNode())
			{
				int childCount = 0;
				int leafCount = 0;
				int tabItemCount = 0;
				bool isSameId = false;
				ImGuiDockNode* child00 = parentDockNode->ChildNodes[0];
				ImGuiDockNode* child01 = parentDockNode->ChildNodes[1];

				if (child00 != nullptr)
				{
					++childCount;
					tabItemCount += child00->Windows.size();
					if (child00->IsLeafNode())
						++leafCount;
					isSameId = child00->ID == dockNode->ID;
				}
				if (child01 != nullptr)
				{
					++childCount;
					tabItemCount += child01->Windows.size();
					if (child01->IsLeafNode())
						++leafCount;
					isSameId |= (child01->ID == dockNode->ID);
				}

				if (leafCount == 2 && tabItemCount == 1 && isSameId)
				{ 
					isLastDock = true;
					isLockMove = true; 
					return;
				}
			}
		}
		void JImGuiDockUpdateHelper::UpdateDraggingDockNode(UpdateData& updataData)
		{
			const ImGuiPayload* p = ImGui::GetDragDropPayload();
			const bool isDraggingWindow = p != nullptr && (JCUtil::EraseSideChar(p->DataType, '\0') == IMGUI_PAYLOAD_TYPE_WINDOW);
			const bool isDraggingCurrentWindow = isDraggingWindow ? (ImGui::GetCurrentWindow()->ID == p->SourceParentId) : false;
			if (isDraggingCurrentWindow)
			{
				ImGuiWindow* result = Private::IntersectPageWindow(ImGui::GetMousePos());
				if (result != nullptr && result->ID != pageID)
				{
					isLockOver = isLockSplit = true; 
				}
			}
		}
		//주의! ImGui API Error
		//현재 Engine에서 DockTabItem을 소유한 Window에 TitleBar를 Dragging시 (TabItem이 아닌 TitleBar Empty Space에 Mouse cursor가 위치)
		//버그발생... 따라서 Mouse가 TitleBar에 Empty Space에 Hover중일시에 isLockMove = true로 문제를 해결한다.
		void JImGuiDockUpdateHelper::RestrictDraggingTabBarSpace(UpdateData& updataData)
		{
			ImGuiDockNode* dockNode = ImGui::GetWindowDockNode();
			if (dockNode == nullptr)
				return;

			ImGuiWindow* wnd = ImGui::GetCurrentWindow();
			const ImRect titleBarRect = wnd->TitleBarRect();
			if (dockNode->TabBar == nullptr || !JGui::IsMouseInRect(titleBarRect.Min, titleBarRect.GetSize()))
				return;
			 
			const uint tabItemSize = dockNode->TabBar->Tabs.Size;
			if (tabItemSize == 0)
				return;
			 
			const ImRect lastTabItemRect = dockNode->TabBar->Tabs[tabItemSize - 1].Window->DockTabItemRect;
			const bool canRestrict = lastTabItemRect.Max.x <= ImGui::GetMousePos().x;
			if (!canRestrict)
				return;
			 
			isLockMove = true; 
			//isLockOver = isLockSplit = true; 
		}
		void JImGuiDockUpdateHelper::RollBackLastDockNode(UpdateData& updataData)
		{
			ImGuiWindow* curWnd = ImGui::GetCurrentWindow();
			ImGuiDockNode* dockNode = ImGui::GetWindowDockNode();
			if (dockNode != nullptr || requestRollBack)
				return;

			//Wait end dragging
			if (ImGui::GetDragDropPayload() != nullptr)
				return;

			ImGuiWindow* pageWnd = ImGui::FindWindowByID(pageID);
			ImGuiContext* ctx = ImGui::GetCurrentContext();

			if (lastDockTabItemCount == 1)
			{
				auto splitLam = [](std::string wndName,
					ImGuiID dockSpaceID,
					uint lastDockNodeID,
					ImGuiID lastParentDockID,
					ImGuiDir_ dir,
					float areaSize)
				{
					ImGui::DockBuilderRemoveNode(lastDockNodeID);
					lastDockNodeID = ImGui::DockBuilderSplitNode(lastParentDockID, dir, areaSize, nullptr, &lastParentDockID);
					ImGui::DockBuilderDockWindow(wndName.c_str(), lastDockNodeID);
					ImGui::DockBuilderFinish(dockSpaceID); 
				};
				using splitLamF = Core::JSFunctorType<void, std::string, ImGuiID, uint, ImGuiID, ImGuiDir_, float>;
				static splitLamF::Functor functor(splitLam);

				ImGuiID spaceID = dockSpaceID;
				updataData.rollbackBind = std::make_unique< splitLamF::CompletelyBind>(functor,
					std::string(curWnd->Name),
					std::move(spaceID),
					std::move(lastDockNodeID),
					std::move(hasLastParentNode ? (ImGuiID)lastParentNodeID : spaceID),
					std::move((ImGuiDir_)lastDockNodeSplitDir),
					std::move(lastDockNodeArea));
			}
			else
			{
				auto overNodeLam = [](std::string wndName, uint dockSpaceID, uint lastDockNodeID)
				{
					ImGui::DockBuilderDockWindow(wndName.c_str(), lastDockNodeID);
					ImGui::DockBuilderFinish(dockSpaceID); 
				};
				using overNodeF = Core::JSFunctorType<void, std::string, uint, ImGuiID>;
				static overNodeF::Functor functor(overNodeLam);

				uint spaceID = dockSpaceID;
				updataData.rollbackBind = std::make_unique<overNodeF::CompletelyBind>(functor,
					std::string(curWnd->Name),
					std::move(spaceID),
					std::move(ImGui::FindWindowByID(lastSameTabWindowID)->DockId));
			}
			requestRollBack = true;
		}
	}
}