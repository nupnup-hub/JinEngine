#include"JEditorViewStructure.h"
#include"../Gui/JGui.h"
#include"../Align/JEditorAlignCalculator.h"
#include"../../Core/Geometry/JBBox.h"
#include"../../Core/Geometry/JRay.h"
#include"../../Core/File/JFileIOHelper.h"
#include"../../Core/Math/JMathHelper.h"
#include"../../Core/Storage/JStorage.h"
#include<unordered_map>
#include<algorithm>

namespace JinEngine
{
	namespace Editor
	{
		namespace Private
		{
			static constexpr float nodeSizeRate = 0.05f;
			static constexpr float nodeOutPaddingRate = 0.01f;
			static constexpr float nodeSpacingRate = 0.05f;
			static constexpr float nodeInnerPaddingRate = 0.025f;
			static constexpr uint descBlankCount = 8;
			static constexpr uint initGridSize = 1000;
			static constexpr float thicknessFactor = 7.5f;
			static constexpr float triangleLengthFactor = 12.5f;
		}

		enum class LAST_HINT_TYPE
		{
			HOVERED,
			SELECTED,
			COUNT
		};
		struct LastNodeHint
		{
		public:
			size_t guid = 0;
			bool condition = false;
		public:
			void Clear()
			{
				guid = 0;
				condition = false;
			}
		};
		struct LastEdgeHint
		{
		public:
			size_t fromGuid = 0;
			size_t toGuid = 0;
			bool condition = false;
		public:
			void Clear()
			{
				fromGuid = 0;
				toGuid = 0;
				condition = false;
			}
		};

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
			JVector2<float> mouseOffset;
		public:
			float thickness = 0;
			uint maxDepth = 0;
		public:
			float triangleLength = 0;
		public:
			JVector2<float> preMousePos;
		public:
			mutable	LastNodeHint lastNodeHint[(int)LAST_HINT_TYPE::COUNT];
			mutable LastEdgeHint lastEdgeHint[(int)LAST_HINT_TYPE::COUNT];
		public:
			mutable bool isDragging = false;
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
				windowSize = JGui::GetClientWindowSize();
				nodeSize = windowSize * Private::nodeSizeRate * zoomRate;
				nodeHalfSize = nodeSize / 2.0f;
				nodeOutPaddingSize = windowSize * Private::nodeOutPaddingRate * zoomRate;
				nodeInnerPaddingSize = windowSize * Private::nodeInnerPaddingRate * zoomRate;
				nodeSpacingSize = windowSize * Private::nodeSpacingRate * zoomRate; 

				mouseOffset = newMouseOffset;
				thickness = Private::thicknessFactor * zoomRate;
				maxDepth = newMaxDepth;
				triangleLength = Private::triangleLengthFactor * zoomRate;
				updateBit = newUpdateBit;
				JEditorViewUpdateHelper::hasNewNode = hasNewNode;

				ClearLastHint();
			}
		public:
			void UpdateLastNodeHint(const LAST_HINT_TYPE type, const size_t guid)const
			{
				lastNodeHint[(uint)type].guid = guid;
				lastNodeHint[(uint)type].condition = true;
				//isDragging = false; 
			}
			void UpdateLastEdgeHint(const LAST_HINT_TYPE type, const size_t fromGuid, const size_t toGuid)const
			{
				lastEdgeHint[(uint)type].fromGuid = fromGuid;
				lastEdgeHint[(uint)type].toGuid = toGuid;
				lastEdgeHint[(uint)type].condition = true;
			}
			void ClearLastNodeHint(const LAST_HINT_TYPE type)const
			{
				lastNodeHint[(uint)type].Clear();
			}
			void ClaerLastEdgeHint(const LAST_HINT_TYPE type)const
			{
				lastEdgeHint[(uint)type].Clear();
			}
			void ClearLastHint()
			{
				for (uint i = 0; i < (uint)LAST_HINT_TYPE::COUNT; ++i)
				{
					lastNodeHint[i].Clear();
					lastEdgeHint[i].Clear();
				}
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

		JEditorNodeBase::JEditorNodeBase(const std::string& name, const size_t guid, const std::string& desc, const bool isSelectedNode)
			:name(name), guid(guid), desc(desc), isSelected(isSelectedNode)
		{}
		void JEditorNodeBase::DrawRect(const JEditorViewUpdateHelper* updateHelper)
		{
			const Core::JBBox2D bbox = Private::GetBBox2D(this, updateHelper);
			const JVector2<float> bboxDistance = bbox.DistanceVector();
 
			const bool isMouseInRect = bbox.Contain(JGui::GetMousePos());
			if (isMouseInRect && JGui::IsCurrentWindowFocused(J_GUI_FOCUS_FLAG_CHILD_WINDOW | J_GUI_FOCUS_FLAG_DOCK_HIERARCHY | J_GUI_FOCUS_FLAG_NO_POPUP_HIERARCHY))
			{
				updateHelper->UpdateLastNodeHint(LAST_HINT_TYPE::HOVERED, GetGuid()); 
				if (JGui::AnyMouseClicked(false))
					updateHelper->UpdateLastNodeHint(LAST_HINT_TYPE::SELECTED, GetGuid());
			}

			JVector4<float> rectColor = GetRectInnerColor();
			if(isSelected)
				rectColor += GetRectSelectedDeltaColor();
			else if (isMouseInRect)
				rectColor += GetRectHoveredDeltaColor();

			if (updateHelper->isDragging && isSelected)
				rectColor -= GetRectDraggingDeltaColor();

			if (!updateHelper->isDragging)
			{
				if (isSelected)
					JGui::DrawRectFrame(bbox.minP, bboxDistance, GetRectSelectedFrameColor(), updateHelper->thickness, false, true);
				else
					JGui::DrawRectFrame(bbox.minP, bboxDistance, GetRectFrameColor(), updateHelper->thickness, false, true);
			}		 

			JGui::DrawRectFilledMultiColor(bbox.minP, bboxDistance, rectColor);
			const JVector2<float> innerPadding = bboxDistance * Private::nodeInnerPaddingRate * 2;
			JEditorTextAlignCalculator textAlignCal;
			JEditorCursorPosCalculator cursorCal;

			textAlignCal.Update(GetName(), bboxDistance - innerPadding, true);
			const std::string alignedCompressName = textAlignCal.MiddleAligned();

			cursorCal.SetMiddline(bbox.minP,
				bboxDistance,
				JGui::CalTextSize(alignedCompressName),
				JVector2<float>(innerPadding.x * 0.5f, 0),
				true);
			JGui::Text(alignedCompressName);

			if (isMouseInRect && !updateHelper->isDragging)
			{
				const JVector2<float> alphaSize = JGui::GetAlphabetSize();
				const JVector2<float> nameTextSize = JGui::CalTextSize(GetName());
				const float blankWidth = JGui::CalTextSize(" ").x * Private::descBlankCount;
				const JVector2<float> nameBBoxMin = bbox.Center();
				const JVector2<float> nameBBoxSize = JVector2<float>(nameTextSize.x + blankWidth, nameTextSize.y + alphaSize.y * 2);

				textAlignCal.Update(GetName(), JVector2<float>(nameBBoxSize.x, 0), false);
				const std::string alignedName = textAlignCal.MiddleAligned();

				std::string desc = GetDesc();
				if (desc.size() > 0)
				{
					textAlignCal.Update(desc, JVector2<float>(nameBBoxSize.x, 0), false);
					const std::string alignedInfo = textAlignCal.LeftAligned();
				 
					const JVector2<float> descBBoxSize = nameBBoxSize + JVector2<float>(0, JGui::CalTextSize(alignedInfo).y + alphaSize.y * 2);
					JGui::DrawRectFilledMultiColor(nameBBoxMin, descBBoxSize, GetDescBoxColor());
					JGui::SetCursorScreenPos(nameBBoxMin + JVector2<float>(0, alphaSize.y));
					JGui::Text(alignedName);
					JGui::SetCursorScreenPos(nameBBoxMin + JVector2<float>(0, nameBBoxSize.y));
					 
					JGui::Text(alignedInfo, 0.8f); 
				}
				else
				{ 
					JGui::DrawRectFilledMultiColor(nameBBoxMin, nameBBoxSize, GetDescBoxColor());
					JGui::SetCursorScreenPos(nameBBoxMin + JVector2<float>(0, alphaSize.y));
					JGui::Text(alignedName);
				}
			}
			 
			if (isSelected)
			{
				if (JGui::IsMouseDown(Core::J_MOUSE_BUTTON::LEFT) && isMouseInRect)
					updateHelper->isDragging = true;	 
			}

			if (updateHelper->isDragging)
			{
				if (!JGui::IsMouseDragging(Core::J_MOUSE_BUTTON::LEFT))
					updateHelper->isDragging = false;
				else if(isSelected)
					SetCenter(GetCenter() + (JGui::GetMousePos() - updateHelper->preMousePos));
			}
		}
		void JEditorNodeBase::DrawLine(const JEditorViewUpdateHelper* updateHelper, const JEditorNodeBase* to, const bool isSelecetdEdge, const bool isBilateralEdge)
		{   
			JVector2<float> fromCenter = GetValidCenter(updateHelper);
			JVector2<float> toCenter = to->GetValidCenter(updateHelper);
			JVector4<float> lineColor = isSelecetdEdge ? GetSelectedLineColor() : GetLineColor();
			JVector4<float> triangleColor = isSelecetdEdge ? GetSelectedTriangleColor (): GetTriangleColor();

			const JVector2<float> dir = (toCenter - fromCenter).Normalize();
			JVector2<float> bottomDir;
			bottomDir.x = dir.x * std::cos(90 * JMathHelper::DegToRad) + dir.y * std::sin(90 * JMathHelper::DegToRad);
			bottomDir.y = dir.x * std::sin(-90 * JMathHelper::DegToRad) + dir.y * std::cos(90 * JMathHelper::DegToRad);
			if (isBilateralEdge && AllowBilateralEdge())
			{
				JVector2<float> additionalPos = bottomDir * updateHelper->nodeHalfSize.x * GetBilateralEdgeOffsetPosRate();
				fromCenter += additionalPos;
				toCenter += additionalPos;
			}

			const JVector2<float> xFactor = bottomDir * updateHelper->triangleLength;
			const JVector2<float> yFactor = dir * updateHelper->triangleLength;
			const JVector2<float> lineCenter = fromCenter + ((toCenter - fromCenter) * 0.5f);
			const JVector2<float> p1 = lineCenter + yFactor;
			const JVector2<float> p2 = lineCenter - yFactor + xFactor;
			const JVector2<float> p3 = lineCenter - yFactor - xFactor;

			if (JGui::IsMouseInLine(fromCenter, toCenter, updateHelper->thickness * 2) || 
				JMathHelper::Contained(p1, p2, p3, JGui::GetMousePos()))
			{
				lineColor += GetRectHoveredDeltaColor();
				triangleColor += GetRectHoveredDeltaColor();

				if (JGui::IsCurrentWindowFocused(J_GUI_FOCUS_FLAG_CHILD_WINDOW | J_GUI_FOCUS_FLAG_DOCK_HIERARCHY | J_GUI_FOCUS_FLAG_NO_POPUP_HIERARCHY))
				{
					updateHelper->UpdateLastEdgeHint(LAST_HINT_TYPE::HOVERED, GetGuid(), to->GetGuid());
					if (JGui::AnyMouseClicked(false))
						updateHelper->UpdateLastEdgeHint(LAST_HINT_TYPE::SELECTED, GetGuid(), to->GetGuid());
				}
			}
			JGui::AddLine(fromCenter, toCenter, lineColor, updateHelper->thickness);
			JGui::AddTriangleFilled(p1, p2, p3, triangleColor);
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
		JVector4<float> JEditorNodeBase::GetRectFrameColor()const noexcept
		{
			return JVector4<float>(0.5f, 0.5f, 0.5f, 0.8f);
		}
		JVector4<float> JEditorNodeBase::GetRectSelectedFrameColor()const noexcept
		{
			return JVector4<float>(0.75f, 0.75f, 0.75f, 1.0f);
		}
		JVector4<float> JEditorNodeBase::GetRectSelectedDeltaColor()const noexcept
		{
			return JVector4<float>(0.35f, 0.35f, 0.35f, 0.4f);
		}
		JVector4<float> JEditorNodeBase::GetRectHoveredDeltaColor()const noexcept
		{ 
			return JVector4<float>(0.125f, 0.125f, 0.125f, 0.125f);
		}
		JVector4<float> JEditorNodeBase::GetRectDraggingDeltaColor()const noexcept
		{
			return JVector4<float>(0.0f, 0.0f, 0.0f, 0.5f);
		} 
		JVector4<float> JEditorNodeBase::GetDescBoxColor()const noexcept
		{
			return JVector4<float>(0.25f, 0.25f, 0.25f, 0.7f);
		}
		JVector4<float> JEditorNodeBase::GetLineColor()const noexcept
		{
			return JVector4<float>(0.5f, 0.5f, 0.5f, 0.8f);
		}
		JVector4<float> JEditorNodeBase::GetTriangleColor()const noexcept
		{
			return JVector4<float>(0.625f, 0.625f, 0.625f, 0.9f);
		}
		JVector4<float> JEditorNodeBase::GetSelectedTriangleColor()const noexcept
		{
			return JVector4<float>(0.875f, 0.875f, 0.875f, 1.0f);
		}
		JVector4<float> JEditorNodeBase::GetSelectedLineColor()const noexcept
		{
			return JVector4<float>(0.75f, 0.75f, 0.75f, 1.0f);
		}
		bool JEditorNodeBase::IsSame(JEditorNodeBase* ptr)const noexcept
		{
			return ptr != nullptr && ptr->GetGuid() == GetGuid() ? true : false;
		}
		bool JEditorNodeBase::IsNewNode()const noexcept
		{
			return isNewNode;
		}
		std::string JEditorNodeBase::GetUniqueName()const noexcept
		{
			return JGui::CreateGuiLabel(name, guid);
		}

		class JEditorTreeNodeBase : public JEditorNodeBase
		{
		private:
			JEditorTreeNodeBase* parent = nullptr;
		private:
			bool isSelectedParentEdge = false;
		public:
			JEditorTreeNodeBase(const std::string& name,
				const size_t guid, 
				const std::string& desc,
				const bool isSelectedNode,
				const bool isSelectedParentEdge,
				JEditorTreeNodeBase* parent)
				:JEditorNodeBase(name, guid, desc, isSelectedNode), isSelectedParentEdge(isSelectedParentEdge), parent(parent)
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
			float GetBilateralEdgeOffsetPosRate()const noexcept final
			{
				return 0.0f;
			}
		public: 
			bool IsRoot()const noexcept
			{
				return parent == nullptr;
			}
			virtual bool IsLeaf()const noexcept = 0;
			bool AllowBilateralEdge()const noexcept final
			{
				return false;
			}
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
					DrawLine(updateHelper, parent, isSelectedParentEdge, false);
			}
		protected:
			JVector4<float> GetRectInnerColor()const noexcept final
			{
				if (IsRoot())
					return JVector4<float>(0.5f, 0.35f, 0.35f, 1.0f);
				else if (IsLeaf())
					return JVector4<float>(0.35f, 0.35f, 0.5f, 1.0f);
				else
					return JVector4<float>(0.35f, 0.5f, 0.35f, 1.0f);
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
			JEditorBinaryNode(const std::string& name, 
				const size_t guid, 
				const std::string& desc,
				const bool isSelectedNode,
				const bool isSelectedParentEdge,
				JEditorBinaryNode* parent)
				:JEditorTreeNodeBase(name, guid, desc, isSelectedNode, isSelectedParentEdge, parent)
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
			JEditorTreeNode(const std::string& name, 
				const size_t guid, 
				const std::string& desc,
				const bool isSelectedNode,
				const bool isSelectedParentEdge,
				JEditorTreeNode* parent)
				:JEditorTreeNodeBase(name, guid, desc, isSelectedNode, isSelectedParentEdge, parent)
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
				JEditorGraphNode* node;
				bool isSelected;
				bool isBilateral;
			public:
				JEditprGraphEdge(JEditorGraphNode* node, bool isSelected, bool isBilateral)
					:node(node), isSelected(isSelected), isBilateral(isBilateral)
				{}
			};
		private:
			Core::JVectorStorage<JEditprGraphEdge> edge;
			//std::vector<JEditprGraphEdge> edge;
			const bool isStartNode = false;
			bool isOnScreenThisFrame = false;
		public:
			JEditorGraphNode(const std::string& name, 
				const size_t guid,
				const std::string& desc,
				const bool isSelectedNode,
				const bool isStartNode)
				:JEditorNodeBase(name, guid, desc, isSelectedNode), isStartNode(isStartNode)
			{}
		protected:
			void Initialize(const JEditorViewUpdateHelper* updateHelper)noexcept final
			{
				SetCenter(JGui::GetMousePos() - JGui::GetCursorScreenPos());
			}
			void SettingDrawLine(const JEditorViewUpdateHelper* updateHelper)final
			{
				const JVector2<float> fromCenter = GetValidCenter(updateHelper);
				const uint edgeCount = edge.Count();
				for (uint i = 0; i < edgeCount; ++i)
				{
					auto edgeData = edge.Get(i);
					DrawLine(updateHelper, edgeData->node, edgeData->isSelected, edgeData->isBilateral);
				}
			}
			bool DoInitializeWhenAddedNewNode()const noexcept final
			{
				return false;
			}
		public:
			void ConnectNode(JEditorGraphNode* otherNode, const bool isSelected)
			{
				if (otherNode == nullptr || IsSame(otherNode))
					return;
				 
				int otherToIndex = GetNodeEdgeIndex(otherNode, this);
				bool isBilateral = otherToIndex != invalidIndex;
				if (isBilateral)
					otherNode->edge.Get(otherToIndex)->isBilateral = true;

				edge.Add(JEditprGraphEdge(otherNode, isSelected, isBilateral));
			}
			void DisConnectNode(JEditorGraphNode* otherNode)
			{
				if (otherNode == nullptr || IsSame(otherNode))
					return;

				int edgeIndex = GetNodeEdgeIndex(this, otherNode);
				if (edgeIndex == invalidIndex)
					return;

				int otherToIndex = GetNodeEdgeIndex(otherNode, this);
				bool isBilateral = otherToIndex != invalidIndex;
				if (isBilateral)
					otherNode->edge.Get(otherToIndex)->isBilateral = false;

				edge.Remove(edgeIndex);
			}
		private:
			static int GetNodeEdgeIndex(JEditorGraphNode* from, JEditorGraphNode* to)
			{
				bool(*equalPtr)(JEditprGraphEdge*, size_t) = [](JEditprGraphEdge* a, size_t guid) {return a->node->GetGuid() == guid; };
				return from->edge.GetIndex(equalPtr, to->GetGuid());
			}
			JVector4<float> GetRectInnerColor()const noexcept final
			{
				return isStartNode ? JVector4<float>(0.375f, 0.375f, 0.775f, 1.0f) : JVector4<float>(0.775f, 0.375f, 0.375f, 1.0f);
			}
		public:
			float GetBilateralEdgeOffsetPosRate()const noexcept final
			{
				return 0.2f;
			}
		public:
			bool AllowBilateralEdge()const noexcept final
			{
				return true;
			}
		};
#pragma endregion
#pragma region ViewStrcture

		JEditorViewBase::JEditorViewBase()
			:guid(Core::MakeGuid())
		{
			Private::AddViewData(GetGuid());
			coordGrid = std::make_unique<JEditorGuiCoordGrid>();
			coordGrid->SetGridSize(Private::initGridSize);
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
		size_t JEditorViewBase::GetLastUpdateHoveredNodeGuid()const noexcept
		{
			return updateHelper->lastNodeHint[(uint)LAST_HINT_TYPE::HOVERED].guid;
		}
		size_t JEditorViewBase::GetLastUpdateSeletedNodeGuid()const noexcept
		{
			return updateHelper->lastNodeHint[(uint)LAST_HINT_TYPE::SELECTED].guid;
		}
		void JEditorViewBase::GetLastUpdateHoveredEdgeGuid(_Out_ size_t& fromGuid, _Out_ size_t& toGuid)const noexcept
		{
			fromGuid = updateHelper->lastEdgeHint[(uint)LAST_HINT_TYPE::HOVERED].fromGuid;
			toGuid = updateHelper->lastEdgeHint[(uint)LAST_HINT_TYPE::HOVERED].toGuid;
		}
		void JEditorViewBase::GetLastUpdateSelectedEdgeGuid(_Out_ size_t& fromGuid, _Out_ size_t& toGuid)const noexcept
		{
			fromGuid = updateHelper->lastEdgeHint[(uint)LAST_HINT_TYPE::SELECTED].fromGuid;
			toGuid = updateHelper->lastEdgeHint[(uint)LAST_HINT_TYPE::SELECTED].toGuid;
		}
		bool JEditorViewBase::IsLastUpdateHoveredNode()const noexcept
		{
			return updateHelper->lastNodeHint[(uint)LAST_HINT_TYPE::HOVERED].condition;
		}
		bool JEditorViewBase::IsLastUpdateHoveredEdge()const noexcept
		{
			return updateHelper->lastEdgeHint[(uint)LAST_HINT_TYPE::HOVERED].condition;
		}
		bool JEditorViewBase::IsLastUpdateSeletedNode()const noexcept
		{
			return updateHelper->lastNodeHint[(uint)LAST_HINT_TYPE::SELECTED].condition;
		}
		bool JEditorViewBase::IsLastUpdateSeletedEdge()const noexcept
		{
			return updateHelper->lastEdgeHint[(uint)LAST_HINT_TYPE::SELECTED].condition;
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
		JEditorNodeBase* JEditorViewBase::GetNode(const size_t guid)const noexcept
		{
			return nodeMap.find(guid)->second;
		}
		JEditorNodeBase* JEditorViewBase::GetNodeByIndex(const uint index)const noexcept
		{
			return allNodes[index].get();
		}
		JEditorNodeBase* JEditorViewBase::GetLastHoveredNode()const noexcept
		{
			return IsLastUpdateHoveredNode() ? nodeMap.find(GetLastUpdateHoveredNodeGuid())->second : nullptr;
		}
		JEditorNodeBase* JEditorViewBase::GetLastSelectedNode()const noexcept
		{
			return IsLastUpdateSeletedNode() ? nodeMap.find(GetLastUpdateSeletedNodeGuid())->second : nullptr;
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
			isLastViewOpen = JGui::BeginWindow(uniqueName, isOpen, guiWindowFlag);
			return isLastViewOpen;
		}
		void JEditorViewBase::OnScreen(const size_t groupGuid)
		{
			if (isLastViewOpen || !useViewWindow)
			{
				JEditorGuiCoordGrid* grid = GetGrid();
				if (JGui::Button("Look Mid"))
					grid->Clear();
				JGui::SameLine();
				JGui::Text("Count: " + std::to_string(GetNodeCount()));
				grid->Update();
				grid->Draw();
				if (GetNodeCount() > 0)
				{
					updateBit = (updateBit + 1) % 2;
					const float zoomRate = grid->GetZoomRate();
					updateHelper->UpdateData(groupGuid,
						grid->GetMouseOffset() + JGui::GetCursorScreenPos(),
						zoomRate,
						GetMaxDepth(),
						updateBit,
						hasNewNode);

					const uint allNodeCount = GetNodeCount();
					NodeOnScreen(updateHelper.get());

					Private::EraseUnusedData(GetGuid(), groupGuid, updateBit);
					updateHelper->preMousePos = JGui::GetMousePos();
					hasNewNode = false;
				}
			}
		}
		void JEditorViewBase::EndView()
		{
			JGui::EndWindow();
		}
		void JEditorViewBase::UseBeginViewWindow(const bool value)noexcept
		{
			useViewWindow = value;
		}
		void JEditorViewBase::StoreData(const std::wstring& path)
		{
			JFileIOTool tool;
			if (!tool.Begin(path, JFileIOTool::TYPE::JSON))
				return;
			 
			auto viewData = Private::GetViewData(GetGuid());
			uint canStoreGroup = 0;
			for (auto& groupData : viewData->groupDataMap)
				canStoreGroup += groupData.second.CanStoreGroupData(groupData.first) ? 1 : 0;

			JFileIOHelper::StoreAtomicData(tool, canStoreGroup, "GroupCount");
			tool.PushArrayOwner("GroupData");

			for (auto& groupData : viewData->groupDataMap)
			{
				if (groupData.second.CanStoreGroupData(groupData.first))
				{
					tool.PushArrayMember();
					JFileIOHelper::StoreAtomicData(tool, groupData.first, "GroupGuid");
					JFileIOHelper::StoreAtomicData(tool, groupData.second.nodeDataMap.size(), "NodeCount");

					tool.PushArrayOwner("NodeData");
					for (auto& nodeData : groupData.second.nodeDataMap)
					{
						tool.PushArrayMember();
						JFileIOHelper::StoreAtomicData(tool, nodeData.first, "NodeGuid");
						JFileIOHelper::StoreVector2(tool, nodeData.second.center, "Center");
						tool.PopStack();
					}
					tool.PopStack();
					tool.PopStack();
				}
			} 

			tool.PopStack(); 
			tool.Close(JFileIOTool::CLOSE_OPTION_JSON_STORE_DATA);
		}
		void JEditorViewBase::LoadData(const std::wstring& path)
		{
			JFileIOTool tool;
			if (!tool.Begin(path, JFileIOTool::TYPE::JSON, JFileIOTool::BEGIN_OPTION_JSON_TRY_LOAD_DATA))
				return;

			std::wstring guide;
			uint groupCount = 0;
			 
			JFileIOHelper::LoadAtomicData(tool, groupCount, "GroupCount");
			tool.PushExistStack("GroupInfo");

			for (uint i = 0; i < groupCount; ++i)
			{
				tool.PushExistStack();
				size_t groupGuid = 0;
				uint nodeCount = 0;
				JFileIOHelper::LoadAtomicData(tool, groupGuid);
				JFileIOHelper::LoadAtomicData(tool, nodeCount);
				Private::AddGroupData(GetGuid(), groupGuid);

				tool.PushExistStack("NodeInfo");
				for (uint j = 0; j < nodeCount; ++j)
				{
					tool.PushExistStack();
					size_t nodeGuid = 0;
					JVector2<float> center;
					JFileIOHelper::LoadAtomicData(tool, nodeGuid);
					JFileIOHelper::LoadVector2(tool, center);
					Private::AddNodeData(GetGuid(), groupGuid, nodeGuid);
					Private::GetNodeData(GetGuid(), groupGuid, nodeGuid)->center = center;
					tool.PopStack();
				}
				tool.PopStack();
				tool.PopStack();
			}
			tool.PopStack();
			tool.Close();
		}

		void JEditorTreeViewBase::NodeOnScreen(const JEditorViewUpdateHelper* updateHelper)noexcept
		{
			static_cast<JEditorTreeNodeBase*>(GetRootNode())->NodeOnScreen(updateHelper);
		}
		void JEditorBinaryTreeView::BuildNode(const std::string& name,
			const size_t nodeGuid,
			const size_t groupGuid,
			const std::string& desc,
			const bool isSelectedNode,
			const bool isSelectedParentEdge)noexcept
		{
			AddNode(groupGuid, std::make_unique<JEditorBinaryNode>(name, nodeGuid, desc, isSelectedNode, isSelectedParentEdge, static_cast<JEditorBinaryNode*>(GetLastAddedNode())));
		}
		void JEditorBinaryTreeView::BuildEndSplit()noexcept
		{
			JEditorTreeNodeBase* added = static_cast<JEditorTreeNodeBase*>(GetLastAddedNode());
			SetLastAddedNode(added->GetParent());
			uint depth = added->GetDepth();
			if (depth > GetMaxDepth())
				SetMaxDepth(depth);
		}
		void JEdtiorTreeView::BuildNode(const std::string& name,
			const size_t nodeGuid,
			const size_t groupGuid,
			const std::string& desc,
			const bool isSelectedNode,
			const bool isSelectedParentEdge)noexcept
		{
			JEditorNodeBase* newNode = AddNode(groupGuid, std::make_unique<JEditorTreeNode>(name, nodeGuid, desc, isSelectedNode, isSelectedParentEdge, static_cast<JEditorTreeNode*>(GetLastAddedNode())));
			const uint gridSize = GetGrid()->GetGridSize();
			newNode->SetCenter(JVector2<float>(gridSize / 2.0f, gridSize / 2.0f));
		}

		void JEditorGraphView::BuildNode(const std::string& name,
			const size_t nodeGuid, 
			const size_t groupGuid,
			const std::string& desc,
			const bool isSelectedNode)noexcept
		{
			JEditorNodeBase* newNode = AddNode(groupGuid, std::make_unique<JEditorGraphNode>(name, nodeGuid, desc, isSelectedNode, GetNodeCount() == 0));
			const uint gridSize = GetGrid()->GetGridSize();
			newNode->SetCenter(JVector2<float>(gridSize / 2.0f, gridSize / 2.0f));
		}
		void JEditorGraphView::BuildNode(const std::string& name,
			const size_t nodeGuid,
			const size_t groupGuid,
			const bool isSelectedNode)noexcept
		{
			BuildNode(name, nodeGuid, groupGuid, "JinWoo is best enginner of graphics\n shiro is cute", isSelectedNode);
		}
		void JEditorGraphView::ConnectNode(const size_t from, const size_t to, const bool isSelected)noexcept
		{
			JEditorGraphNode* fromNode = static_cast<JEditorGraphNode*>(GetNode(from));
			JEditorGraphNode* toNode = static_cast<JEditorGraphNode*>(GetNode(to));
			if (fromNode != nullptr && toNode != nullptr)
				fromNode->ConnectNode(toNode, isSelected);
		}
		void JEditorGraphView::SetConnectNodeMode(const size_t from, std::unique_ptr<Core::JBindHandleBase>&& newSuccessBind)noexcept
		{
			JEditorNodeBase* selecetd = GetNode(from);
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
				{
					OffConnectNodeMode();
					return;
				}

				JVector2<float> lineStart = fromNode->GetValidCenter(updateHelper);
				JVector2<float> lineEnd = JGui::GetMousePos();
				JGui::AddLine(lineStart, lineEnd, JVector4<float>(0.333f, 0.333f, 0.686f, 1.0f), updateHelper->thickness);

				const bool isLeftClick = JGui::IsMouseClicked(Core::J_MOUSE_BUTTON::LEFT);
				const bool isRightClick = JGui::IsMouseClicked(Core::J_MOUSE_BUTTON::RIGHT);
				//JGui::IsCurrentWindowFocused(J_GUI_FOCUS_FLAG_CHILD_WINDOW | J_GUI_FOCUS_FLAG_DOCK_HIERARCHY)
				if ((isLeftClick || isRightClick))
				{ 
					//selectedNode에 transition을 추가하는 시나리오이기 때문에
					//새로운 selectedNode가아닌 hoveredNode에 transition을 연결해야한다. 
					//새로운 selectedNode에 추가하고싶다면 기존에 selectedNode와 trnasition상태를 저장할 구조가 팔요하다.
					if (IsLastUpdateHoveredNode() && isLeftClick)
					{
						JEditorGraphNode* toNode = static_cast<JEditorGraphNode*>(GetLastHoveredNode());
						if (toNode != nullptr)
						{
							connectToGuid = toNode->GetGuid();
							fromNode->ConnectNode(toNode, false);
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