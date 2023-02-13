#include"JEditorWindowDockUpdateHelper.h"   
#include"JEditorPageShareData.h"
#include"../../Utility/JCommonUtility.h"
#include"../../Core/Geometry/JBBox.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../../Lib/imgui/imgui.h"
#include"../../../../Lib/imgui/imgui_internal.h"

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
						data[i].window = JImGuiImpl::GetGuiWindow(JEditorPageShareData::GetPageGuiWindowID(type), data[i].order);
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

		JEditorWindowDockUpdateHelper::JEditorWindowDockUpdateHelper(const J_EDITOR_PAGE_TYPE page)
			:pageWndID(JEditorPageShareData::GetPageGuiWindowID(page)),
			dockSpaceID(JEditorPageShareData::GetPageDockSpaceID(page))
		{
		}
		bool JEditorWindowDockUpdateHelper::IsLastWindow()const noexcept
		{
			return isLastWindow;
		}
		bool JEditorWindowDockUpdateHelper::IsLastDock()const noexcept
		{
			return isLastDock;
		}
		bool JEditorWindowDockUpdateHelper::IsLockSplitAcitvated()const noexcept
		{
			return isLockSplit;
		}
		bool JEditorWindowDockUpdateHelper::IsLockOverAcitvated()const noexcept
		{
			return isLockOver;
		}
		void JEditorWindowDockUpdateHelper::Update(UpdateData& updataData)
		{
			isLastWindow = isLastDock = isLockSplit = isLockOver = false;
			UpdateDockNodeInfo(updataData);
			UpdateExistingWindow(updataData);
			UpdateExistingDockNode(updataData);
			UpdateDraggingDockNode(updataData);
			RollBackLastDockNode(updataData);
		}
		void JEditorWindowDockUpdateHelper::UpdateDockNodeInfo(UpdateData& updataData)
		{
			ImGuiWindow* curWnd = ImGui::GetCurrentWindow();
			ImGuiDockNode* dockNode = ImGui::GetWindowDockNode();
			if (dockNode != nullptr)
			{ 
				lastDockNodeID = dockNode->ID;
				lastDockTabItemCount = dockNode->TabBar ? dockNode->TabBar->Tabs.size() : 0;
				lastDockNodePos = dockNode->Pos;
				lastDockNodeSize = dockNode->Size;
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
		void JEditorWindowDockUpdateHelper::UpdateExistingWindow(UpdateData& updataData)noexcept
		{
			ImGuiDockNode* dockNode = ImGui::GetWindowDockNode();
			if (dockNode == nullptr)
				return;

			int actCount = Private::ActivatedWindowCount(Private::GetDockNodeRoot(dockNode));
			if (actCount == 1)
				isLastWindow = true;
		}
		void JEditorWindowDockUpdateHelper::UpdateExistingDockNode(UpdateData& updataData)noexcept
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
					return;
				}
			}
		}
		void JEditorWindowDockUpdateHelper::UpdateDraggingDockNode(UpdateData& updataData)
		{
			const ImGuiPayload* p = ImGui::GetDragDropPayload();
			const bool isDraggingWindow = p != nullptr && (JCUtil::EraseSideChar(p->DataType, '\0') == IMGUI_PAYLOAD_TYPE_WINDOW);
			const bool isDraggingCurrentWindow = isDraggingWindow ? (ImGui::GetCurrentWindow()->ID == p->SourceParentId) : false;
			if (isDraggingCurrentWindow)
			{
				ImGuiWindow* result = Private::IntersectPageWindow(ImGui::GetMousePos());
				if (result != nullptr && result->ID != pageWndID)
					isLockOver = isLockSplit = true;
			}
		}
		void JEditorWindowDockUpdateHelper::RollBackLastDockNode(UpdateData& updataData)
		{
			ImGuiWindow* curWnd = ImGui::GetCurrentWindow();
			ImGuiDockNode* dockNode = ImGui::GetWindowDockNode();
			if (dockNode != nullptr || requestRollBack)
				return;
			 
			//Wait end dragging
			if (ImGui::GetDragDropPayload() != nullptr)
				return;
			 
			ImGuiWindow* pageWnd = JImGuiImpl::GetGuiWindow(pageWndID);
			ImGuiContext* ctx = ImGui::GetCurrentContext();
		 
			if (lastDockTabItemCount == 1)
			{
				ImGuiDockNode* lastParentDock = ((ImGuiDockNode*)(ctx->DockContext.Nodes.GetVoidPtr(lastDockNodeID)))->ParentNode;
				ImGuiDir_ dir = ImGuiDir_None;
				float areaSize = 1;
				if (lastParentDock != nullptr)
				{
					if (lastDockNodePos.x != lastParentDock->Pos.x)
					{
						if (lastDockNodePos.x > lastParentDock->Pos.x)
							dir = ImGuiDir_Right;
						else
							dir = ImGuiDir_Left;
					}
					else
					{
						if (lastDockNodePos.y > lastParentDock->Pos.y)
							dir = ImGuiDir_Down;
						else
							dir = ImGuiDir_Up;
					}
					areaSize = (lastDockNodeSize.x * lastDockNodeSize.y) / (lastParentDock->Size.x * lastParentDock->Size.y);
				}
			 
				auto splitLam = [](std::string wndName,
					uint dockSpaceID,
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
				using splitLamF = Core::JSFunctorType<void, std::string, uint, uint, ImGuiID, ImGuiDir_, float>;
				static splitLamF::Functor functor(splitLam);

				uint spaceID = dockSpaceID;
				updataData.rollbackBind = std::make_unique< splitLamF::CompletelyBind>(functor,
					std::string(curWnd->Name),
					std::move(spaceID),
					std::move(lastDockNodeID),
					std::move(lastParentDock->ID),
					std::move(dir),
					std::move(areaSize));
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
					std::move(JImGuiImpl::GetGuiWindow(lastSameTabWindowID)->DockId));
			}
			requestRollBack = true;
		}
	}
}