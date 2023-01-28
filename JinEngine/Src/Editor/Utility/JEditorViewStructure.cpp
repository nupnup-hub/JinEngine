#include"JEditorViewStructure.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../Utility/JEditorLineCalculator.h"
#include"../../Core/Geometry/JBBox.h"
#include<unordered_map>

namespace JinEngine
{
	namespace Editor
	{
		namespace Constants
		{
			static constexpr float nodeSizeRate = 0.05f; 
			static constexpr float nodeOutPaddingRate = 0.01f;
			static constexpr float nodeSpacingRate = 0.05f;
			static constexpr float nodeInnerPaddingRate = 0.025f; 
			static constexpr uint initGridSize = 1000;
			static constexpr uint nodeFrameThickness = 1;
			static constexpr uint infoLinePerAlphabet = 18;
		}

		struct JEditorViewUpdateHelper
		{
		public:
			const size_t viewGuid;
		public:
			JVector2<uint> windowSize;
			JVector2<uint> nodeSize;
			JVector2<uint> nodeHalfSize;
			JVector2<uint> nodeOutPaddingSize;
			JVector2<uint> nodeInnerPaddingSize;
			JVector2<uint> nodeSpacingSize; 
			JVector2<uint> nodeFrameThickness; 
			JVector2<float> mouseOffset;
		public:
			float lineThickness;
			uint maxDepth;
		public:
			JVector2<float> preMousePos;
		public:
			mutable size_t selectedGuid = 0;
			mutable bool hasSelected = false; 
			mutable bool isDragging = false;
		public:
			uint8 updateBit;
			bool hasNewNode = false;
		public:
			JEditorViewUpdateHelper(const size_t viewGuid)
				:viewGuid(viewGuid)
			{}
			~JEditorViewUpdateHelper() = default;
		public:
			void UpdateData(const JVector2<float> newMouseOffset,
				const float zoomRate,
				const uint newMaxDepth,
				const uint8 newUpdateBit,
				const bool hasNewNode)
			{ 
				windowSize = JImGuiImpl::GetClientWindowSize();
				nodeSize = windowSize * Constants::nodeSizeRate * zoomRate;
				nodeHalfSize = nodeSize / 2.0f;
				nodeOutPaddingSize = windowSize * Constants::nodeOutPaddingRate * zoomRate;
				nodeInnerPaddingSize = windowSize * Constants::nodeInnerPaddingRate * zoomRate;
				nodeSpacingSize = windowSize * Constants::nodeSpacingRate * zoomRate;
				nodeFrameThickness = JVector2<uint>(Constants::nodeFrameThickness, Constants::nodeFrameThickness);

				mouseOffset = newMouseOffset;
				lineThickness = 1.25f * zoomRate;
				maxDepth = newMaxDepth;
				updateBit = newUpdateBit;
				JEditorViewUpdateHelper::hasNewNode = hasNewNode;
			}
		};

		namespace Private
		{
			struct JEditorNodeStoredData
			{
			public:
				JVector2<float> center;
				uint8 updateBit = 0;
			};
			using NodeDataMap = std::unordered_map<size_t, JEditorNodeStoredData>;
			static std::unordered_map<size_t, NodeDataMap> viewData;

			static void AddView(const size_t guid)
			{
				viewData.emplace(guid, NodeDataMap());
			}
			static void RemoveView(const size_t guid)
			{
				viewData.erase(guid);
			}
			static void EraseUnusedData(const size_t guid, const uint8 validUpdateBit)
			{
				std::vector<size_t> unusedGuidVec;
				NodeDataMap& nodeMap = viewData.find(guid)->second;
				for (auto& data : nodeMap)
				{
					if (data.second.updateBit != validUpdateBit)
						unusedGuidVec.push_back(data.first);
				}

				const uint vCount = (uint)unusedGuidVec.size();
				for (uint i = 0; i < vCount; ++i)
					nodeMap.erase(unusedGuidVec[i]);
			}
			static bool HasNodeData(const size_t viewGuid, const size_t nodeGuid)
			{
				NodeDataMap& nodeMap = viewData.find(viewGuid)->second;
				return nodeMap.find(nodeGuid) != nodeMap.end();
			}
			static Core::JBBox2D GetBBox2D(JEditorNodeBase* node, const JEditorViewUpdateHelper* updateHelper)
			{
				const JVector2<float> center = node->GetValidCenter(updateHelper);
				const float xFactor = updateHelper->nodeHalfSize.x;
				const float yFactor = updateHelper->nodeHalfSize.y;
 
				return Core::JBBox2D(center - JVector2<float>(xFactor, yFactor), center + JVector2<float>(xFactor, yFactor));
			}
		}

#pragma region Node

		JEditorNodeBase::JEditorNodeBase(const std::string& name, const size_t guid, const std::string& info)
			:name(name), guid(guid), info(info)
		{}
		void JEditorNodeBase::NodeOnScreen(const JEditorViewUpdateHelper* updateHelper)
		{
			if (IsNewNode() || (updateHelper->hasNewNode && DoInitializeWhenAddedNewNode()))
			{
				Initialize(updateHelper);
				SetNewNodeTrigger(false);
			}
			else
				LoadUpdateNodeData(updateHelper);

			DrawRect(updateHelper);
			DrawLine(updateHelper);
			CallChildNode(updateHelper);
			StoreUpdatedNodeData(updateHelper);
		}
		void JEditorNodeBase::StoreUpdatedNodeData(const JEditorViewUpdateHelper* updateHelper)noexcept
		{
			Private::NodeDataMap& nodeMap = Private::viewData.find(updateHelper->viewGuid)->second;
			Private::JEditorNodeStoredData& nodeData = nodeMap.find(GetGuid())->second;
			nodeData.center = GetCenter();
			nodeData.updateBit = updateHelper->updateBit;
		}
		void JEditorNodeBase::LoadUpdateNodeData(const JEditorViewUpdateHelper* updateHelper)noexcept
		{
			Private::NodeDataMap& nodeMap = Private::viewData.find(updateHelper->viewGuid)->second;
			Private::JEditorNodeStoredData& nodeData = nodeMap.find(GetGuid())->second;
			SetCenter(nodeData.center);
		}
		bool JEditorNodeBase::IsSame(JEditorNodeBase* ptr)const noexcept
		{
			return ptr != nullptr && ptr->GetGuid() == GetGuid() ? true : false;
		}
		bool JEditorNodeBase::IsNewNode()const noexcept
		{
			return isNewNode;
		}
		JVector2<float> JEditorNodeBase::GetValidCenter(const JEditorViewUpdateHelper* updateHelper)noexcept
		{
			return center + updateHelper->mouseOffset;
		}
		std::string JEditorNodeBase::GetComporessName(const JEditorViewUpdateHelper* updateHelper)noexcept
		{ 
			return JCUtil::CompressString(GetName(), updateHelper->nodeSize.x - updateHelper->nodeInnerPaddingSize.x);
		}
		void JEditorNodeBase::SetNewNodeTrigger(const bool value)noexcept
		{
			isNewNode = value;
		}
		class JEditorTreeNodeBase : public JEditorNodeBase
		{
		private:
			JEditorTreeNodeBase* parent = nullptr;
		public:
			JEditorTreeNodeBase(const std::string& name, const size_t guid, const std::string& info, JEditorTreeNodeBase* parent)
				:JEditorNodeBase(name, guid, info), parent(parent)
			{}
		public:
			JEditorTreeNodeBase* GetParent()const noexcept
			{
				return parent;
			}
			uint GetDepth()const noexcept
			{
				uint depth = 0;
				JEditorTreeNodeBase* nowParent = parent;
				while (nowParent != nullptr)
				{
					nowParent = nowParent->parent;
					++depth;
				}
				return depth;
			}
			bool IsRoot()const noexcept
			{
				return parent == nullptr;
			}
			virtual bool IsLeaf()const noexcept = 0;
		protected:
			void DrawRect(const JEditorViewUpdateHelper* updateHelper)final
			{
				Core::JBBox2D bbox = Private::GetBBox2D(this, updateHelper);
				ImDrawList* drawList = ImGui::GetWindowDrawList();

				bool isMouseInRect = false;
				if (bbox.Contain(ImGui::GetMousePos()))
				{				 
					if(ImGui::IsMouseClicked(0) && ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_DockHierarchy))
					{
						updateHelper->hasSelected = true;
						updateHelper->selectedGuid = GetGuid();
					}
					isMouseInRect = true;
				}

				if (updateHelper->hasSelected && updateHelper->selectedGuid == GetGuid())
				{
					drawList->AddRect(bbox.min - JVector2<float>(2, 2), bbox.max + JVector2<float>(2, 2), IM_COL32(45, 45, 175, 255));
					if (isMouseInRect && ImGui::IsMouseDragging(0))
						SetCenter(GetCenter() + ImGui::GetMouseDragDelta(0));
				}

				if (IsLeaf())
					drawList->AddRectFilled(bbox.min, bbox.max, IM_COL32(90, 90, 125, 255));
				else if (IsRoot())
					drawList->AddRectFilled(bbox.min, bbox.max, IM_COL32(125, 90, 90, 255));
				else
					drawList->AddRectFilled(bbox.min, bbox.max, IM_COL32(90, 125, 90, 255));

				JVector2<float> textOffset = JVector2<float>(updateHelper->nodeSize.x * Constants::nodeInnerPaddingRate, updateHelper->nodeHalfSize.y - (JImGuiImpl::GetAlphabetSize().y * 0.5f));
				drawList->AddText(textOffset + bbox.min, IM_COL32(225, 225, 225, 200), GetComporessName(updateHelper).c_str());
				
				if (isMouseInRect)
				{			
					const std::string info = GetInfo();
					if (info.size() > 0)
					{
						JVector2<float> nextOffset = JVector2<float>(0, JImGuiImpl::GetAlphabetSize().y);
						drawList->AddText(textOffset + bbox.min + nextOffset, IM_COL32(225, 225, 225, 200), info.c_str());
					}
				}
			}
			void DrawLine(const JEditorViewUpdateHelper* updateHelper)final
			{
				if (!IsRoot())
				{
					const JVector2<float> center = GetValidCenter(updateHelper);
					const JVector2<float> pCenter = parent->GetCenter() + updateHelper->mouseOffset;
					JVector2<float> lineStart = JVector2<float>(center.x, center.y + updateHelper->nodeHalfSize.y);
					JVector2<float> lineEnd = JVector2<float>(pCenter.x, pCenter.y + updateHelper->nodeHalfSize.y);
					ImGui::GetWindowDrawList()->AddLine(lineStart, lineEnd, IM_COL32(155, 155, 155, 255), updateHelper->lineThickness);
				}
			}
		};
		class JEditorBinaryNode : public JEditorTreeNodeBase
		{
		private:
			JEditorTreeNodeBase* left = nullptr;
			JEditorTreeNodeBase* right = nullptr;
		private:
			bool isLeft = false;
		public:
			JEditorBinaryNode(const std::string& name, const size_t guid, const std::string& info, JEditorBinaryNode* parent)
				:JEditorTreeNodeBase(name, guid, info, parent)
			{
				if (parent != nullptr)
				{
					if (parent->left == nullptr)
					{
						parent->left = this;
						isLeft = true;
					}
					else
					{
						parent->right = this;
						isLeft = false;
					}
				}
			}
			~JEditorBinaryNode() = default;
		public:
			bool IsLeaf()const noexcept final
			{
				return left == nullptr && right == nullptr;
			}
		protected:
			void Initialize(const JEditorViewUpdateHelper* updateHelper)noexcept final
			{
				JEditorTreeNodeBase* parent = GetParent();
				if (parent == nullptr)
				{
					SetCenter(JVector2<float>(updateHelper->windowSize.x / 2, updateHelper->nodeSpacingSize.y + updateHelper->nodeHalfSize.y));
					return;
				}
				const float mulFactor = (1 << (updateHelper->maxDepth - GetDepth())) * 0.5f;
				const float centerXOffset = (mulFactor * updateHelper->nodeSize.x + (mulFactor - 1) * updateHelper->nodeOutPaddingSize.x);

				JVector2<float> pCenter = parent->GetCenter();
				JVector2<float> nCenter(0, 0);
				if (isLeft)
				{
					nCenter.x = pCenter.x - (centerXOffset + updateHelper->nodeOutPaddingSize.x);
					nCenter.y = pCenter.y + (updateHelper->nodeSize.y + updateHelper->nodeOutPaddingSize.y);
				}
				else
				{
					nCenter.x = pCenter.x + (centerXOffset + updateHelper->nodeOutPaddingSize.x);
					nCenter.y = pCenter.y + (updateHelper->nodeSize.y + updateHelper->nodeOutPaddingSize.y);
				}
				SetCenter(nCenter); 
			}
			void CallChildNode(const JEditorViewUpdateHelper* updateHelper)noexcept final
			{
				if (left != nullptr)
					left->NodeOnScreen(updateHelper);
				if (right != nullptr)
					right->NodeOnScreen(updateHelper);
			}
			bool DoInitializeWhenAddedNewNode()const noexcept final
			{
				return true;
			}
		};
		class JEditorTreeNode : public JEditorTreeNodeBase
		{
		private:
			std::vector<JEditorTreeNode*> children;
			uint childNumber = 0;
		public:
			JEditorTreeNode(const std::string& name, const size_t guid, const std::string& info, JEditorTreeNode* parent)
				:JEditorTreeNodeBase(name, guid, info, parent)
			{
				if (parent != nullptr)
				{
					childNumber = (uint)parent->children.size();
					parent->children.push_back(this);
				}
			}
			~JEditorTreeNode() = default;
		public:
			bool IsLeaf()const noexcept
			{
				return children.size() == 0;
			}
		protected:
			void Initialize(const JEditorViewUpdateHelper* updateHelper)noexcept final
			{
				JEditorTreeNodeBase* parent = GetParent();
				if (parent == nullptr)
					SetCenter(JVector2<float>(updateHelper->windowSize.x / 2, updateHelper->nodeSpacingSize.y + updateHelper->nodeHalfSize.y));

				const float centerXOffset = updateHelper->nodeSize.x + updateHelper->nodeOutPaddingSize.x;

				JVector2<float> pCenter = parent->GetCenter();
				JVector2<float> nCenter(pCenter.x, pCenter.y);
				nCenter.x += centerXOffset * childNumber;
				nCenter.y += updateHelper->nodeSize.y + updateHelper->nodeOutPaddingSize.y;
				SetCenter(nCenter);
			}
			void CallChildNode(const JEditorViewUpdateHelper* updateHelper)noexcept final
			{
				const uint childrenCount = (uint)children.size();
				for (uint i = 0; i < childrenCount; ++i)
					children[i]->NodeOnScreen(updateHelper);
			}
			bool DoInitializeWhenAddedNewNode()const noexcept final
			{
				return false;
			}
		};

		class JEditorGraphNode : public JEditorNodeBase
		{
		private:
			struct JEditprGraphEdge
			{
			public:
				std::vector<JEditorGraphNode*> nodes;
			};
		private:
			JEditprGraphEdge edge;
			const bool isStartNode = false;
			bool isOnScreenThisFrame = false;
		public:
			JEditorGraphNode(const std::string& name, const size_t guid, const std::string& info, const bool isStartNode)
				:JEditorNodeBase(name, guid, info), isStartNode(isStartNode)
			{}
		protected:
			void Initialize(const JEditorViewUpdateHelper* updateHelper)noexcept final
			{
				SetCenter(JVector2<float>(updateHelper->windowSize / 2));
			}
			void DrawRect(const JEditorViewUpdateHelper* updateHelper)final
			{ 
				Core::JBBox2D bbox = Private::GetBBox2D(this, updateHelper); 
				ImDrawList* drawList = ImGui::GetWindowDrawList();

				bool isMouseInRect = false;
				if (bbox.Contain(ImGui::GetMousePos()))
				{
					if (ImGui::IsMouseClicked(0) && ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_DockHierarchy))
					{
						updateHelper->isDragging = false;
						updateHelper->hasSelected = true;
						updateHelper->selectedGuid = GetGuid();
					}
					isMouseInRect = true;
				}

				ImU32 rectColor = isStartNode ? IM_COL32(90, 90, 200, 255) : IM_COL32(200, 90, 90, 255);
				if (updateHelper->isDragging)
					rectColor += IM_COL32(25, 25, 25, 0);
				 
				drawList->AddRectFilledMultiColor(bbox.min, bbox.max, 
					rectColor + IM_COL32(15, 15, 15, 0),
					rectColor - IM_COL32(15, 15, 15, 0),
					rectColor + IM_COL32(15, 15, 15, 0),
					rectColor - IM_COL32(15, 15, 15, 0));
				 
				JVector2<float> textOffset = JVector2<float>(updateHelper->nodeSize.x * Constants::nodeInnerPaddingRate, updateHelper->nodeHalfSize.y - (JImGuiImpl::GetAlphabetSize().y * 0.5f));
				drawList->AddText(textOffset + bbox.min, IM_COL32(225, 225, 225, 200), GetName().c_str());
				if (isMouseInRect)
				{
					std::string info = GetInfo();
					const uint infoSize = (uint)info.size();
					if (infoSize > 0)
					{
						const JVector2<float> alphaSize = JImGuiImpl::GetAlphabetSize();
						const uint rowCount = infoSize / Constants::infoLinePerAlphabet + 1;
						const JVector2<float> infoBBoxExtent = JVector2<float>(Constants::infoLinePerAlphabet* alphaSize.x, rowCount * alphaSize.y) / 2.0f;

						JVector2<float> center = GetCenter();
						JVector2<float> infoBBoxMin(center.x - infoBBoxExtent.x, center.y - infoBBoxExtent.y);
						JVector2<float> infoBBoxMax(center.x + infoBBoxExtent.x, center.y + infoBBoxExtent.y);
						 
						JVector2<float> prePos = infoBBoxMin;
						drawList->AddRectFilled(infoBBoxMin, infoBBoxMax, IM_COL32(75, 75, 75, 75));

						JEditorTextLineCalculator textCal; 
						textCal.Update(info, infoBBoxExtent * 2, prePos, false);
						ImGui::GetCurrentWindow()->DC.CursorPos = prePos;
						textCal.MiddleAligned();

						//JVector2<float> nextOffset = JVector2<float>(0, JImGuiImpl::GetAlphabetSize().y);
						//drawList->AddText(textOffset + bbox.min + nextOffset, IM_COL32(225, 225, 225, 200), info.c_str());
					}
				}

				JVector2<float> frameSize(Constants::nodeFrameThickness, Constants::nodeFrameThickness);
				if (updateHelper->hasSelected && updateHelper->selectedGuid == GetGuid())
				{
					drawList->AddRect(bbox.min - frameSize, bbox.max + frameSize, IM_COL32(45, 45, 175, 255));
					if (ImGui::IsMouseDown(0) && isMouseInRect)
						updateHelper->isDragging = true;

					if (updateHelper->isDragging)
					{
						if (!ImGui::IsMouseDragging(0))
							updateHelper->isDragging = false;
						else
							SetCenter(GetCenter() + (ImGui::GetMousePos() - updateHelper->preMousePos));
					}
				}
				else
					drawList->AddRect(bbox.min - frameSize, bbox.max + frameSize, IM_COL32(45, 45, 45, 255));
			}
			void DrawLine(const JEditorViewUpdateHelper* updateHelper)final
			{
				const JVector2<float> center = GetValidCenter(updateHelper);
				const uint edgeCount = (uint)edge.nodes.size();
				for (uint i = 0; i < edgeCount; ++i)
				{
					const JVector2<float> oCenter = edge.nodes[i]->GetValidCenter(updateHelper);
					JVector2<float> lineStart = JVector2<float>(center.x, center.y + updateHelper->nodeHalfSize.y);
					JVector2<float> lineEnd = JVector2<float>(oCenter.x, oCenter.y + updateHelper->nodeHalfSize.y);
					ImGui::GetWindowDrawList()->AddLine(lineStart, lineEnd, IM_COL32(155, 155, 155, 255), updateHelper->lineThickness);
				}
			}
			void CallChildNode(const JEditorViewUpdateHelper* updateHelper)noexcept final
			{
				isOnScreenThisFrame = true;
				const uint edgeCount = (uint)edge.nodes.size();
				for (uint i = 0; i < edgeCount; ++i)
				{
					if (edge.nodes[i]->isOnScreenThisFrame)
						continue;
					edge.nodes[i]->NodeOnScreen(updateHelper);
				}
				isOnScreenThisFrame = false;
			}
			bool DoInitializeWhenAddedNewNode()const noexcept final
			{
				return false;
			}
		public:
			void ConnectNode(JEditorGraphNode* otherNode)
			{
				if (otherNode == nullptr || IsSame(otherNode))
					return;
				edge.nodes.push_back(otherNode);
			}
		};
#pragma endregion
#pragma region ViewStrcture

		JEditorViewBase::JEditorViewBase()
			:guid(Core::MakeGuid())
		{
			Private::AddView(GetGuid());
			coordGrid = std::make_unique<JEditorGuiCoordGrid>();
			coordGrid->SetGridSize(Constants::initGridSize);
			updateHelper = std::make_unique<JEditorViewUpdateHelper>(GetGuid());
		}
		JEditorViewBase::~JEditorViewBase()
		{
			Private::RemoveView(GetGuid());
		}
		void JEditorViewBase::Clear()
		{
			selectedNode = nullptr;
			allNodes.clear();
			maxDepth = 0;
		}
		size_t JEditorViewBase::GetGuid()const noexcept
		{
			return guid;
		}
		uint JEditorViewBase::GetNodeCount()const noexcept
		{
			return (uint)allNodes.size();
		}
		JEditorNodeBase* JEditorViewBase::GetRootNode()const noexcept
		{
			return allNodes[0].get();
		}
		JEditorGuiCoordGrid* JEditorViewBase::GetGrid()const noexcept
		{
			return coordGrid.get();
		}
		void JEditorViewBase::SetGridSize(const uint gridSize)noexcept
		{
			if (coordGrid != nullptr)
				coordGrid->SetGridSize(gridSize);
		}
		JEditorNodeBase* JEditorViewBase::AddNode(std::unique_ptr<JEditorNodeBase>&& newNode)
		{
			allNodes.emplace_back(std::move(newNode));
			JEditorNodeBase* addedNode = allNodes[allNodes.size() - 1].get();
			SetSelectedNode(addedNode);

			Private::NodeDataMap& nodeMap = Private::viewData.find(GetGuid())->second;
			if (nodeMap.find(addedNode->GetGuid()) == nodeMap.end())
			{
				nodeMap.emplace(addedNode->GetGuid(), Private::JEditorNodeStoredData());
				addedNode->SetNewNodeTrigger(true);
				hasNewNode = true;
			}

			return addedNode;
		}
		bool JEditorViewBase::BeginView(const std::string& uniqueName, bool* isOpen, int windowFlag)
		{
			isLastViewOpen = JImGuiImpl::BeginWindow(uniqueName, isOpen, windowFlag);
			return isLastViewOpen;
		}
		void JEditorViewBase::OnScreen()
		{ 
			if (isLastViewOpen)
			{
				JEditorGuiCoordGrid* grid = GetGrid();
				if (JImGuiImpl::Button("Look Mid"))
					grid->Clear();
				ImGui::SameLine();
				JImGuiImpl::Text("Count: " + std::to_string(GetNodeCount()));
				grid->Update();
				grid->Draw();
				if (GetNodeCount() > 0)
				{
					updateBit = (updateBit + 1) % 2;
					const float zoomRate = grid->GetZoomRate();
					updateHelper->UpdateData(grid->GetMouseOffset(),
						zoomRate,
						GetMaxDepth(),
						updateBit,
						hasNewNode);
					 
					GetRootNode()->NodeOnScreen(updateHelper.get());
					Private::EraseUnusedData(GetGuid(), updateBit);
					updateHelper->preMousePos = ImGui::GetMousePos();
					hasNewNode = false;
				}
			}
		}
		void JEditorViewBase::EndView()
		{
			JImGuiImpl::EndWindow();
		}
		void JEditorBinaryTreeView::BuildNode(const std::string& name, const size_t guid, const std::string& info)noexcept
		{
			AddNode(std::make_unique<JEditorBinaryNode>(name, guid, info, static_cast<JEditorBinaryNode*>(GetSelectedNode())));
		}
		void JEditorBinaryTreeView::BuildEndSplit()noexcept
		{
			JEditorTreeNodeBase* selected = static_cast<JEditorTreeNodeBase*>(GetSelectedNode());
			SetSelectedNode(selected->GetParent());
			uint depth = selected->GetDepth();
			if (depth > GetMaxDepth())
				SetMaxDepth(depth);
		}
		void JEdtiorTreeView::BuildNode(const std::string& name, const size_t guid, const std::string& info)noexcept
		{
			JEditorNodeBase* newNode = AddNode(std::make_unique<JEditorTreeNode>(name, guid, info, static_cast<JEditorTreeNode*>(GetSelectedNode())));
			const uint gridSize = GetGrid()->GetGridSize();
			newNode->SetCenter(JVector2<float>(gridSize / 2.0f, gridSize / 2.0f));
		}

		void JEditorGraphView::BuildNode(const std::string& name, const size_t guid, const std::string& info)noexcept
		{
			JEditorNodeBase* newNode = AddNode(std::make_unique<JEditorGraphNode>(name, guid, info, GetNodeCount() == 0));
			const uint gridSize = GetGrid()->GetGridSize();
			newNode->SetCenter(JVector2<float>(gridSize / 2.0f, gridSize / 2.0f));
		}
		void JEditorGraphView::ConncetNode(const size_t from, const size_t to)
		{

		}
#pragma endregion
	}
}