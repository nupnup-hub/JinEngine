#include"JEditorViewStructure.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../Align/JEditorAlignCalculator.h"
#include"../../Core/Geometry/JBBox.h"
#include"../../Core/Geometry/JRay.h"
#include"../../Core/File/JFileIOHelper.h"
#include"../../Utility/JMathHelper.h"
#include<unordered_map>
#include<algorithm>

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
			static constexpr uint infoBlankCount = 8;
			static constexpr uint initGridSize = 1000;
			static constexpr uint nodeFrameThickness = 1;
		}

		struct JEditorViewUpdateHelper
		{
		public:
			const size_t viewGuid;
			size_t groupGuid = 0;
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
			float lineThickness = 0;
			uint maxDepth = 0;
		public:
			JVector2<float> preMousePos;
		public:
			mutable size_t selectedNodeGuid = 0;
			mutable bool isSelectedNode = false;
			mutable bool isDragging = false;
		public:
			mutable size_t selectedFromNodeGuid = 0;
			mutable size_t selectedToNodeGuid = 0;
			mutable bool isSelectedEdge = false;
		public:
			uint8 updateBit = 0;
			bool hasNewNode = false;
		public:
			JEditorViewUpdateHelper(const size_t viewGuid)
				:viewGuid(viewGuid)
			{}
			~JEditorViewUpdateHelper() = default;
		public:
			void UpdateData(const size_t groupGuid,
				const JVector2<float> newMouseOffset,
				const float zoomRate,
				const uint newMaxDepth,
				const uint8 newUpdateBit,
				const bool hasNewNode)
			{
				JEditorViewUpdateHelper::groupGuid = groupGuid;
				windowSize = JImGuiImpl::GetClientWindowSize();
				nodeSize = windowSize * Constants::nodeSizeRate * zoomRate;
				nodeHalfSize = nodeSize / 2.0f;
				nodeOutPaddingSize = windowSize * Constants::nodeOutPaddingRate * zoomRate;
				nodeInnerPaddingSize = windowSize * Constants::nodeInnerPaddingRate * zoomRate;
				nodeSpacingSize = windowSize * Constants::nodeSpacingRate * zoomRate;
				nodeFrameThickness = JVector2<uint>(Constants::nodeFrameThickness, Constants::nodeFrameThickness);

				mouseOffset = newMouseOffset;
				lineThickness = 5 * zoomRate;
				maxDepth = newMaxDepth;
				updateBit = newUpdateBit;
				JEditorViewUpdateHelper::hasNewNode = hasNewNode;
			}
		public:
			void SelectNode(const size_t guid)const 
			{
				selectedNodeGuid = guid;
				isSelectedNode = true;
				isDragging = false;
				DeSelectEdge();
			}
			void SelectEdge(const size_t fromGuid, const size_t toGuid)const
			{
				selectedFromNodeGuid = fromGuid;
				selectedToNodeGuid = toGuid;
				isSelectedEdge = true;
				DeSelectNode();
			}
			void DeSelectNode()const
			{
				selectedNodeGuid = 0;
				isSelectedNode = false;
			}
			void DeSelectEdge()const
			{
				selectedFromNodeGuid = 0;
				selectedToNodeGuid = 0;
				isSelectedEdge = false;
			}
		};

		namespace Private
		{
			using IsValidGroupF = JEditorViewBase::IsValidGroupF;
			struct JEditorNodeStoredData
			{
			public:
				JVector2<float> center;
				uint8 updateBit = 0;
			};
			using NodeDataMap = std::unordered_map<size_t, JEditorNodeStoredData>;
			struct JEditorNodeGroupStoredData
			{
			public:
				NodeDataMap nodeDataMap;
				std::unique_ptr<IsValidGroupF::Functor> isValidGroupF;
			public:
				void RegisterIsValidFunctor(IsValidGroupF::Ptr ptr)noexcept
				{
					isValidGroupF = std::make_unique<IsValidGroupF::Functor>(ptr);
				}
				bool IsValidGroup(const size_t guid)const noexcept
				{
					return isValidGroupF != nullptr ? (*isValidGroupF)(guid) : false;
				}
				bool CanStoreGroupData(const size_t guid)const noexcept
				{
					return IsValidGroup(guid);
				}
			};
			using GroupNodeMap = std::unordered_map<size_t, JEditorNodeGroupStoredData>;

			struct JEditorViewStoredData
			{
			public:
				GroupNodeMap groupDataMap;
			};

			static std::unordered_map<size_t, JEditorViewStoredData> viewDataMap;

			static void AddViewData(const size_t guid)
			{
				viewDataMap.emplace(guid, JEditorViewStoredData());
			}
			static void RemoveViewData(const size_t guid)
			{
				viewDataMap.erase(guid);
			}
			static JEditorViewStoredData* GetViewData(const size_t guid)
			{
				auto data = viewDataMap.find(guid);
				return data != viewDataMap.end() ? &data->second : nullptr;
			}

			static void AddGroupData(const size_t viewGuid, const size_t groupGuid)
			{
				auto data = GetViewData(viewGuid);
				if (data == nullptr)
					return;

				data->groupDataMap.emplace(groupGuid, JEditorNodeGroupStoredData());
			}
			static void EraseGroupData(const size_t viewGuid, const size_t groupGuid)
			{
				auto data = GetViewData(viewGuid);
				if (data == nullptr)
					return;

				data->groupDataMap.erase(groupGuid);
			}
			static JEditorNodeGroupStoredData* GetGroupData(const size_t viewGuid, const size_t groupGuid)
			{
				auto data = GetViewData(viewGuid);
				if (data == nullptr)
					return nullptr;

				auto groupData = data->groupDataMap.find(groupGuid);
				return groupData != data->groupDataMap.end() ? &groupData->second : nullptr;
			}

			static void AddNodeData(const size_t viewGuid, const size_t groupGuid, const size_t nodeGuid)
			{
				auto data = GetGroupData(viewGuid, groupGuid);
				if (data == nullptr)
					return;

				data->nodeDataMap.emplace(nodeGuid, JEditorNodeStoredData());
			}
			static void RemoveNodeData(const size_t viewGuid, const size_t groupGuid, const size_t nodeGuid)
			{
				auto data = GetGroupData(viewGuid, groupGuid);
				if (data == nullptr)
					return;

				data->nodeDataMap.erase(nodeGuid);
			}
			Private::JEditorNodeStoredData* GetNodeData(const size_t viewGuid, const size_t groupGuid, const size_t nodeGuid)
			{
				auto data = GetGroupData(viewGuid, groupGuid);
				if (data == nullptr)
					return nullptr;

				auto nodeData = data->nodeDataMap.find(nodeGuid);
				return nodeData != data->nodeDataMap.end() ? &nodeData->second : nullptr;
			}

			static void EraseUnusedData(const size_t viewGuid, const size_t groupGuid, const uint8 validUpdateBit)
			{
				std::vector<size_t> unusedGuidVec;
				auto groupData = GetGroupData(viewGuid, groupGuid);
				for (auto& data : groupData->nodeDataMap)
				{
					if (data.second.updateBit != validUpdateBit)
						unusedGuidVec.push_back(data.first);
				}
				const uint vCount = (uint)unusedGuidVec.size();
				for (uint i = 0; i < vCount; ++i)
					groupData->nodeDataMap.erase(unusedGuidVec[i]);
			}
			static Core::JBBox2D GetBBox2D(JEditorNodeBase* node, const JEditorViewUpdateHelper* updateHelper)
			{
				const JVector2<float> center = node->GetValidCenter(updateHelper);
				const float xFactor = (float)updateHelper->nodeHalfSize.x;
				const float yFactor = (float)updateHelper->nodeHalfSize.y;

				return Core::JBBox2D(center - JVector2<float>(xFactor, yFactor), center + JVector2<float>(xFactor, yFactor));
			}
		}

#pragma region Node

		JEditorNodeBase::JEditorNodeBase(const std::string& name, const size_t guid, const std::string& info)
			:name(name), guid(guid), info(info + "TEST")
		{}
		void JEditorNodeBase::DrawRect(const JEditorViewUpdateHelper* updateHelper)
		{
			const Core::JBBox2D bbox = Private::GetBBox2D(this, updateHelper);
			const JVector2<float> bboxDistance = bbox.DistanceVector();
			ImDrawList* drawList = ImGui::GetWindowDrawList();

			const bool isSelected = updateHelper->isSelectedNode && updateHelper->selectedNodeGuid == GetGuid();
			const bool isMouseInRect = bbox.Contain(ImGui::GetMousePos());
			if (isMouseInRect)
			{
				if (ImGui::IsMouseClicked(0) && ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_DockHierarchy))
					updateHelper->SelectNode(GetGuid());
			}

			ImU32 rectColor = GetRectInnerColor();
			if (isMouseInRect)
				rectColor += GetRectHoveredDeltaColor(); 
			if (updateHelper->isDragging && isSelected)
				rectColor -= GetRectDraggingDeltaColor();

			const ImU32 vertexDeltaColor = GetRectVertexDeltaColor();
			JImGuiImpl::DrawRectFilledMultiColor(bbox.minP, bboxDistance, rectColor, vertexDeltaColor, false);
			if (isSelected)
				JImGuiImpl::DrawRectFrame(bbox.minP, bboxDistance, 2.5f, GetRectSelectedFrameColor(), false);
			else
				JImGuiImpl::DrawRectFrame(bbox.minP, bboxDistance, 2.5f, GetRectFrameColor(), false);

			const JVector2<float> innerPadding = bboxDistance * Constants::nodeInnerPaddingRate * 2;
			JEditorTextAlignCalculator textAlignCal;
			JEditorCursorPosCalculator cursorCal;

			textAlignCal.Update(GetName(), bboxDistance - innerPadding, true);
			const std::string alignedCompressName = textAlignCal.MiddleAligned();

			cursorCal.SetMiddline(bbox.minP,
				bboxDistance,
				ImGui::CalcTextSize(alignedCompressName.c_str()),
				JVector2<float>(innerPadding.x * 0.5f, 0),
				true);
			JImGuiImpl::Text(alignedCompressName);

			if (isMouseInRect && !updateHelper->isDragging)
			{
				const JVector2<float> alphaSize = JImGuiImpl::GetAlphabetSize();
				const JVector2<float> nameTextSize = ImGui::CalcTextSize(GetName().c_str());
				const float blankWidth = ImGui::CalcTextSize(" ").x * Constants::infoBlankCount;
				const JVector2<float> nameBBoxMin = bbox.Center();
				const JVector2<float> nameBBoxSize = JVector2<float>(nameTextSize.x + blankWidth, nameTextSize.y + alphaSize.y * 2);
				 
				textAlignCal.Update(GetName(), JVector2<float>(nameBBoxSize.x, 0), false);
				const std::string alignedName = textAlignCal.MiddleAligned(); 

				std::string info = GetInfo();
				if (info.size() > 0)
				{		 
					textAlignCal.Update(info, JVector2<float>(nameBBoxSize.x, 0), false);
					const std::string alignedInfo = textAlignCal.LeftAligned();
					 
					const JVector2<float> infoBBoxSize = nameBBoxSize + JVector2<float>(0, ImGui::CalcTextSize(alignedInfo.c_str()).y + alphaSize.y * 2);
					JImGuiImpl::DrawRectFilledMultiColor(nameBBoxMin, infoBBoxSize, GetInfoBoxColor(), vertexDeltaColor, true);
					ImGui::SetCursorScreenPos(nameBBoxMin + JVector2<float>(0, alphaSize.y));
					JImGuiImpl::Text(alignedName);
					ImGui::SetCursorScreenPos(nameBBoxMin + JVector2<float>(0, nameBBoxSize.y));

					const float fontScale = ImGui::GetIO().FontGlobalScale;
					ImGui::SetWindowFontScale(fontScale * 0.8f);
					JImGuiImpl::Text(alignedInfo);
					ImGui::SetWindowFontScale(fontScale);
				}
				else
				{
					JImGuiImpl::DrawRectFilledMultiColor(nameBBoxMin, nameBBoxSize, GetInfoBoxColor(), vertexDeltaColor, false);
					ImGui::SetCursorScreenPos(nameBBoxMin + JVector2<float>(0, alphaSize.y));
					JImGuiImpl::Text(alignedName);
				} 
			}

			JVector2<float> frameSize(Constants::nodeFrameThickness, Constants::nodeFrameThickness);
			if (isSelected)
			{
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
		}
		void JEditorNodeBase::DrawLine(const JEditorViewUpdateHelper* updateHelper, const JEditorNodeBase* to)
		{
			const JVector2<float> fromCenter = GetValidCenter(updateHelper);
			const JVector2<float> toCenter = to->GetValidCenter(updateHelper);

			const bool isSelected = updateHelper->isSelectedEdge && updateHelper->selectedFromNodeGuid == GetGuid() &&
				updateHelper->selectedToNodeGuid == to->GetGuid();
			 
			ImU32 color = isSelected ? GetSelectedLineColor() : GetLineColor(); 
			if (JImGuiImpl::IsMouseInLine(fromCenter, toCenter, updateHelper->lineThickness))
			{
				color += GetRectHoveredDeltaColor(); 
				if (ImGui::IsMouseClicked(0) && ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_DockHierarchy))
					updateHelper->SelectEdge(GetGuid(), to->GetGuid());
			}
			ImGui::GetWindowDrawList()->AddLine(fromCenter, toCenter, color, updateHelper->lineThickness);			 
		}
		void JEditorNodeBase::StoreUpdatedNodeData(const JEditorViewUpdateHelper* updateHelper)noexcept
		{
			Private::JEditorNodeStoredData* nodeData = Private::GetNodeData(updateHelper->viewGuid, updateHelper->groupGuid, GetGuid());
			nodeData->center = GetCenter();
			nodeData->updateBit = updateHelper->updateBit;
		}
		void JEditorNodeBase::LoadUpdateNodeData(const JEditorViewUpdateHelper* updateHelper)noexcept
		{
			if (IsNewNode() || (updateHelper->hasNewNode && DoInitializeWhenAddedNewNode()))
			{
				Initialize(updateHelper);
				SetNewNodeTrigger(false);
			}
			else
			{
				Private::JEditorNodeStoredData* nodeData = Private::GetNodeData(updateHelper->viewGuid, updateHelper->groupGuid, GetGuid());
				SetCenter(nodeData->center);
			}
		}
		bool JEditorNodeBase::IsSame(JEditorNodeBase* ptr)const noexcept
		{
			return ptr != nullptr && ptr->GetGuid() == GetGuid() ? true : false;
		}
		bool JEditorNodeBase::IsNewNode()const noexcept
		{
			return isNewNode;
		}
		JVector2<float> JEditorNodeBase::GetValidCenter(const JEditorViewUpdateHelper* updateHelper)const noexcept
		{
			return center + updateHelper->mouseOffset;
		}
		std::string JEditorNodeBase::GetCompressName(const JEditorViewUpdateHelper* updateHelper)const noexcept
		{
			return JCUtil::CompressString(GetName(), updateHelper->nodeSize.x - updateHelper->nodeInnerPaddingSize.x);
		}
		void JEditorNodeBase::SetNewNodeTrigger(const bool value)noexcept
		{
			isNewNode = value;
		}
		uint JEditorNodeBase::GetRectFrameColor()const noexcept
		{
			return IM_COL32(35, 35, 35, 255);
		}
		uint JEditorNodeBase::GetRectSelectedFrameColor()const noexcept
		{
			return IM_COL32(125, 125, 35, 255);
		}
		uint JEditorNodeBase::GetRectHoveredDeltaColor()const noexcept
		{
			return IM_COL32(25, 25, 25, 0);
		}
		uint JEditorNodeBase::GetRectDraggingDeltaColor()const noexcept
		{
			return IM_COL32(0, 0, 0, 75);
		}
		uint JEditorNodeBase::GetRectVertexDeltaColor()const noexcept
		{
			return IM_COL32(25, 25, 25, 0);
		}
		uint JEditorNodeBase::GetTextColor()const noexcept
		{
			return IM_COL32(225, 225, 225, 255);
		}
		uint JEditorNodeBase::GetInfoBoxColor()const noexcept
		{
			return IM_COL32(55, 55, 55, 175);
		}
		uint JEditorNodeBase::GetLineColor()const noexcept
		{
			return IM_COL32(155, 155, 155, 255);
		}
		uint JEditorNodeBase::GetSelectedLineColor()const noexcept
		{
			return IM_COL32(155, 155, 75, 255);
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
		public:
			void NodeOnScreen(const JEditorViewUpdateHelper* updateHelper)
			{
				LoadUpdateNodeData(updateHelper);
				DrawRect(updateHelper);
				SettingDrawLine(updateHelper);
				CallChildNode(updateHelper);
				StoreUpdatedNodeData(updateHelper);
			}
		protected:
			virtual void CallChildNode(const JEditorViewUpdateHelper* updateHelper)noexcept = 0;
			void SettingDrawLine(const JEditorViewUpdateHelper* updateHelper)final
			{
				if (!IsRoot())
					DrawLine(updateHelper, parent);
			}
		protected:
			uint GetRectInnerColor()const noexcept final
			{
				if (IsRoot())
					return IM_COL32(125, 90, 90, 255);
				else if (IsLeaf())
					return IM_COL32(90, 90, 125, 255);
				else
					return IM_COL32(90, 125, 90, 255);
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
					SetCenter(JVector2<float>((float)updateHelper->windowSize.x / 2.0f, (float)updateHelper->nodeSpacingSize.y + updateHelper->nodeHalfSize.y));
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
					SetCenter(JVector2<float>((float)updateHelper->windowSize.x / 2.0f, (float)updateHelper->nodeSpacingSize.y + updateHelper->nodeHalfSize.y));

				const uint centerXOffset = updateHelper->nodeSize.x + updateHelper->nodeOutPaddingSize.x;

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
				SetCenter(ImGui::GetMousePos() - ImGui::GetCursorScreenPos());
			}
			void SettingDrawLine(const JEditorViewUpdateHelper* updateHelper)final
			{ 
				const JVector2<float> fromCenter = GetValidCenter(updateHelper);
				const uint edgeCount = (uint)edge.nodes.size();
				for (uint i = 0; i < edgeCount; ++i)
					DrawLine(updateHelper, edge.nodes[i]);
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
		protected:
			uint GetRectInnerColor()const noexcept final
			{
				return isStartNode ? IM_COL32(100, 100, 200, 255) : IM_COL32(200, 100, 100, 255);
			}
		};
#pragma endregion
#pragma region ViewStrcture

		JEditorViewBase::JEditorViewBase()
			:guid(Core::MakeGuid())
		{
			Private::AddViewData(GetGuid());
			coordGrid = std::make_unique<JEditorGuiCoordGrid>();
			coordGrid->SetGridSize(Constants::initGridSize);
			updateHelper = std::make_unique<JEditorViewUpdateHelper>(GetGuid());
			RegisterGroup(GetDefaultGroupGuid());
		}
		JEditorViewBase::~JEditorViewBase()
		{
			Private::RemoveViewData(GetGuid());
		}
		void JEditorViewBase::ClearNode()
		{
			lastAddedNode = nullptr;
			nodeMap.clear();
			allNodes.clear();
			maxDepth = 0;
		}
		void JEditorViewBase::ClearSeletedCash()
		{
			updateHelper->DeSelectNode();
			updateHelper->DeSelectEdge();
		}
		size_t JEditorViewBase::GetDefaultGroupGuid()noexcept
		{
			static size_t guid = Core::MakeGuid();
			return guid;
		}
		uint JEditorViewBase::GetNodeCount()const noexcept
		{
			return (uint)allNodes.size();
		}
		JEditorGuiCoordGrid* JEditorViewBase::GetGrid()const noexcept
		{
			return coordGrid.get();
		}
		size_t JEditorViewBase::GetLastUpdateSeletedNodeGuid()const noexcept
		{
			return lastSelectedNodeGuid;
		}
		void JEditorViewBase::GetLastUpdateSelectedEdgeGuid(_Out_ size_t& fromGuid, _Out_ size_t& toGuid)const noexcept
		{
			fromGuid = lastSelectedFromNodeGuid;
			toGuid = lastSelectedToNodeGuid;
		}
		bool JEditorViewBase::IsLastUpdateSeletedNode()const noexcept
		{
			return isLastSelectedNode;
		}
		bool JEditorViewBase::IsLastUpdateSeletedEdge()const noexcept
		{
			return isLastSelectedEdge;
		}
		void JEditorViewBase::SetGridSize(const uint gridSize)noexcept
		{
			if (coordGrid != nullptr)
				coordGrid->SetGridSize(gridSize);
		}
		JEditorNodeBase* JEditorViewBase::GetRootNode()const noexcept
		{
			return allNodes[0].get();
		}
		JEditorNodeBase* JEditorViewBase::GetNode(const size_t guid)noexcept
		{
			return nodeMap.find(guid)->second;
		}
		JEditorNodeBase* JEditorViewBase::GetNodeByIndex(const uint index)const noexcept
		{
			return allNodes[index].get();
		}
		JEditorNodeBase* JEditorViewBase::GetLastSelectedNode()noexcept
		{ 
			return nodeMap.find(lastSelectedNodeGuid)->second;
		}
		JEditorNodeBase* JEditorViewBase::AddNode(const size_t groupGuid, std::unique_ptr<JEditorNodeBase>&& newNode)
		{
			nodeMap.emplace(newNode->GetGuid(), newNode.get());
			allNodes.emplace_back(std::move(newNode));
			JEditorNodeBase* addedNode = allNodes[allNodes.size() - 1].get();
			SetLastAddedNode(addedNode);

			auto nodeData = Private::GetNodeData(GetGuid(), groupGuid, addedNode->GetGuid());
			if (nodeData == nullptr)
			{
				Private::AddNodeData(GetGuid(), groupGuid, addedNode->GetGuid());
				addedNode->SetNewNodeTrigger(true);
				hasNewNode = true;
			}
			return addedNode;
		}
		void JEditorViewBase::RegisterGroup(const size_t groupGuid, IsValidGroupF::Ptr ptr)noexcept
		{
			if (!HasGroupData(groupGuid))
				Private::AddGroupData(GetGuid(), groupGuid);

			if (ptr != nullptr)
				Private::GetGroupData(GetGuid(), groupGuid)->RegisterIsValidFunctor(ptr);
		}
		bool JEditorViewBase::HasGroupData(const size_t groupGuid)const noexcept
		{
			return Private::GetGroupData(GetGuid(), groupGuid) != nullptr;
		}
		bool JEditorViewBase::BeginView(const std::string& uniqueName, bool* isOpen, int guiWindowFlag)
		{
			isLastViewOpen = JImGuiImpl::BeginWindow(uniqueName, isOpen, guiWindowFlag);
			return isLastViewOpen;
		}
		void JEditorViewBase::OnScreen(const size_t groupGuid)
		{
			if (isLastViewOpen || !useViewWindow)
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
					updateHelper->UpdateData(groupGuid,
						grid->GetMouseOffset() + ImGui::GetCursorScreenPos(),
						zoomRate,
						GetMaxDepth(),
						updateBit,
						hasNewNode);

					const uint allNodeCount = GetNodeCount();
					NodeOnScreen(updateHelper.get()); 

					Private::EraseUnusedData(GetGuid(), groupGuid, updateBit);
					updateHelper->preMousePos = ImGui::GetMousePos();

					const bool canChangeSelectedNode = updateHelper->isSelectedNode && (lastSelectedNodeGuid != updateHelper->selectedNodeGuid);
					const bool canChangeSelectedEdge = updateHelper->isSelectedEdge && (lastSelectedFromNodeGuid != updateHelper->selectedFromNodeGuid)
						&& (lastSelectedToNodeGuid != updateHelper->selectedToNodeGuid);
					
					if (canChangeSelectedNode)
					{
						JEditorNodeBase* newSelected = GetNode(updateHelper->selectedNodeGuid);
						if (newSelected == nullptr)
							updateHelper->DeSelectNode();
						else
						{
							isLastSelectedNode = true;
							isLastSelectedEdge = false;
							lastSelectedNodeGuid = updateHelper->selectedNodeGuid;
							lastSelectedFromNodeGuid = lastSelectedToNodeGuid = 0;
						}
					}
					else if (canChangeSelectedEdge)
					{
						JEditorNodeBase* fromSelected = GetNode(updateHelper->selectedFromNodeGuid);
						JEditorNodeBase* toSelected = GetNode(updateHelper->selectedToNodeGuid);
						if (fromSelected == nullptr || toSelected == nullptr)
							updateHelper->DeSelectEdge();
						else
						{
							isLastSelectedNode = false;
							isLastSelectedEdge = true;
							lastSelectedNodeGuid = 0;
							lastSelectedFromNodeGuid = updateHelper->selectedFromNodeGuid;
							lastSelectedToNodeGuid = updateHelper->selectedToNodeGuid;
						}
					}
					hasNewNode = false;
				}
			}
		}
		void JEditorViewBase::EndView()
		{
			JImGuiImpl::EndWindow();
		}
		void JEditorViewBase::UseBeginViewWindow(const bool value)noexcept
		{
			useViewWindow = value;
		}
		void JEditorViewBase::StoreData(const std::wstring& path)
		{
			std::wofstream stream;
			stream.open(path, std::ios::binary | std::ios::out);
			if (stream.is_open())
			{
				JFileIOHelper::StoreJString(stream, L"EditorViewData", L"");
				auto viewData = Private::GetViewData(GetGuid());

				uint canStoreGroup = 0;
				for (auto& groupData : viewData->groupDataMap)
					canStoreGroup += groupData.second.CanStoreGroupData(groupData.first) ? 1 : 0;

				JFileIOHelper::StoreAtomicData(stream, L"GroupCount", canStoreGroup);
				for (auto& groupData : viewData->groupDataMap)
				{
					if (groupData.second.CanStoreGroupData(groupData.first))
					{
						JFileIOHelper::StoreAtomicData(stream, L"GroupGuid", groupData.first);
						JFileIOHelper::StoreAtomicData(stream, L"NodeCount", groupData.second.nodeDataMap.size());
						for (auto& nodeData : groupData.second.nodeDataMap)
						{
							JFileIOHelper::StoreAtomicData(stream, L"NodeGuid", nodeData.first);
							JFileIOHelper::StoreXMFloat2(stream, L"Center", nodeData.second.center.ConvertXMF());
						}
					}
				}
				stream.close();
			}
		}
		void JEditorViewBase::LoadData(const std::wstring& path)
		{
			std::wifstream stream;
			stream.open(path, std::ios::binary | std::ios::in);
			if (stream.is_open())
			{
				std::wstring guide;
				uint groupCount = 0;
				JFileIOHelper::LoadJString(stream, guide);
				JFileIOHelper::LoadAtomicData(stream, groupCount);
				for (uint i = 0; i < groupCount; ++i)
				{
					size_t groupGuid = 0;
					uint nodeCount = 0;
					JFileIOHelper::LoadAtomicData(stream, groupGuid);
					JFileIOHelper::LoadAtomicData(stream, nodeCount);

					Private::AddGroupData(GetGuid(), groupGuid);
					for (uint j = 0; j < nodeCount; ++j)
					{
						size_t nodeGuid = 0;
						DirectX::XMFLOAT2 center;
						JFileIOHelper::LoadAtomicData(stream, nodeGuid);
						JFileIOHelper::LoadXMFloat2(stream, center);
						Private::AddNodeData(GetGuid(), groupGuid, nodeGuid);
						Private::GetNodeData(GetGuid(), groupGuid, nodeGuid)->center = center;
					}
				}
				stream.close();
			}
		}

		void JEditorTreeViewBase::NodeOnScreen(const JEditorViewUpdateHelper* updateHelper)noexcept
		{
			static_cast<JEditorTreeNodeBase*>(GetRootNode())->NodeOnScreen(updateHelper);
		}
		void JEditorBinaryTreeView::BuildNode(const std::string& name, const size_t nodeGuid, const size_t groupGuid, const std::string& info)noexcept
		{
			AddNode(groupGuid, std::make_unique<JEditorBinaryNode>(name, nodeGuid, info, static_cast<JEditorBinaryNode*>(GetLastAddedNode())));
		}
		void JEditorBinaryTreeView::BuildEndSplit()noexcept
		{
			JEditorTreeNodeBase* added = static_cast<JEditorTreeNodeBase*>(GetLastAddedNode());
			SetLastAddedNode(added->GetParent());
			uint depth = added->GetDepth();
			if (depth > GetMaxDepth())
				SetMaxDepth(depth);
		}
		void JEdtiorTreeView::BuildNode(const std::string& name, const size_t nodeGuid, const size_t groupGuid, const std::string& info)noexcept
		{
			JEditorNodeBase* newNode = AddNode(groupGuid, std::make_unique<JEditorTreeNode>(name, nodeGuid, info, static_cast<JEditorTreeNode*>(GetLastAddedNode())));
			const uint gridSize = GetGrid()->GetGridSize();
			newNode->SetCenter(JVector2<float>(gridSize / 2.0f, gridSize / 2.0f));
		}

		void JEditorGraphView::BuildNode(const std::string& name, const size_t nodeGuid, const size_t groupGuid, const std::string& info)noexcept
		{
			JEditorNodeBase* newNode = AddNode(groupGuid, std::make_unique<JEditorGraphNode>(name, nodeGuid, info, GetNodeCount() == 0));
			const uint gridSize = GetGrid()->GetGridSize();
			newNode->SetCenter(JVector2<float>(gridSize / 2.0f, gridSize / 2.0f));
		}
		void JEditorGraphView::ConnectNode(const size_t from, const size_t to)noexcept
		{
			JEditorGraphNode* fromNode = static_cast<JEditorGraphNode*>(GetNode(from));
			JEditorGraphNode* toNode = static_cast<JEditorGraphNode*>(GetNode(to));
			if (fromNode != nullptr && toNode != nullptr)
				fromNode->ConnectNode(toNode);
		}
		void JEditorGraphView::SetConnectNodeMode(std::unique_ptr<Core::JBindHandleBase>&& newSuccessBind)noexcept
		{
			JEditorNodeBase* selecetd = GetLastSelectedNode();
			if (selecetd == nullptr)
				return;

			isConnectModeActivated = true;
			connectFromGuid = selecetd->GetGuid();
			if (newSuccessBind != nullptr)
				successBind = std::move(newSuccessBind);
		}
		void JEditorGraphView::OffConnectNodeMode()noexcept
		{
			connectFromGuid = 0;
			connectToGuid = 0;
			isConnectModeActivated = false;
			successBind.reset();
		}
		void JEditorGraphView::NodeOnScreen(const JEditorViewUpdateHelper* updateHelper)noexcept
		{
			const uint allNodeCount = GetNodeCount();
			for (uint i = 0; i < allNodeCount; ++i)
				GetNodeByIndex(i)->LoadUpdateNodeData(updateHelper);
			for (uint i = 0; i < allNodeCount; ++i)
				GetNodeByIndex(i)->SettingDrawLine(updateHelper);
			for (uint i = 0; i < allNodeCount; ++i)
			{  
				GetNodeByIndex(i)->DrawRect(updateHelper);
				GetNodeByIndex(i)->StoreUpdatedNodeData(updateHelper);
			}

			if (isConnectModeActivated)
			{
				JEditorGraphNode* fromNode = static_cast<JEditorGraphNode*>(GetNode(connectFromGuid));
				if (fromNode == nullptr)
					SetConnectNodeMode();
				 
				JVector2<float> lineStart = fromNode->GetValidCenter(updateHelper);
				JVector2<float> lineEnd = ImGui::GetMousePos();
				ImGui::GetWindowDrawList()->AddLine(lineStart, lineEnd, IM_COL32(85, 85, 175, 255), updateHelper->lineThickness);

				if (ImGui::IsMouseDown(0) || ImGui::IsMouseDown(1))
				{
					if (updateHelper->isSelectedNode)
					{
						JEditorGraphNode* toNode = static_cast<JEditorGraphNode*>(GetNode(updateHelper->selectedNodeGuid));
						if (toNode != nullptr)
						{
							connectToGuid = toNode->GetGuid();
							fromNode->ConnectNode(toNode);
							if (successBind != nullptr)
								successBind->InvokeCompletelyBind();
						}
					}
					OffConnectNodeMode();
				}
			}
		}
#pragma endregion
	}
}