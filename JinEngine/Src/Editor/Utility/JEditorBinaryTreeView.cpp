#include"JEditorBinaryTreeView.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"

namespace JinEngine
{
	namespace Editor
	{
		JEditorBinaryTreeView::JNodeSizeInfo::JNodeSizeInfo(const uint width,
			const uint height,
			const uint paddingX,
			const uint paddingY,
			const uint spacingX,
			const uint spacingY,
			const JVector2<float> mouseOffset,
			const float zoomRate,
			const uint maxDepth)
			:width(width* zoomRate),
			height(height* zoomRate),
			paddingX(paddingX* zoomRate),
			paddingY(paddingY* zoomRate),
			spacingX(spacingX* zoomRate),
			spacingY(spacingY* zoomRate),
			halfWidth((width * zoomRate) / 2),
			halfHeight((height* zoomRate) / 2), 
			mouseOffset(mouseOffset),
			lineThickness(1.25f * zoomRate),
			maxDepth(maxDepth)
		{}

		JEditorBinaryTreeView::JNode::JNode(JNode* parent, const std::string& name, const std::string& info)
			: parent(parent), name(name), info(info)
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
		void JEditorBinaryTreeView::JNode::NodeOnScreen(const JNodeSizeInfo& sizeInfo, const float centerX, const float centerY, const uint depth)
		{ 
			JVector2<float> minP = JVector2<float>(centerX - sizeInfo.halfWidth + sizeInfo.mouseOffset.x, centerY - sizeInfo.halfHeight + sizeInfo.mouseOffset.y);
			JVector2<float> maxP = JVector2<float>(centerX + sizeInfo.halfWidth + sizeInfo.mouseOffset.x, centerY + sizeInfo.halfHeight + sizeInfo.mouseOffset.y);

			JVector2<float> textOffset = JVector2<float>(sizeInfo.width * 0.1f, (sizeInfo.height * 0.5f) - (JImGuiImpl::GetAlphabetSize().y * 0.5f));
			ImDrawList* drawList = ImGui::GetWindowDrawList();

			if (IsLeaf())
				drawList->AddRectFilled(minP, maxP, IM_COL32(90, 90, 125, 255));
			else if (IsRoot())
				drawList->AddRectFilled(minP, maxP, IM_COL32(125, 90, 90, 255));
			else
				drawList->AddRectFilled(minP, maxP, IM_COL32(90, 125, 90, 255));

			drawList->AddText(textOffset + minP, IM_COL32(225, 225, 225, 200), name.c_str());
			if (info.size() > 0)
			{
				JVector2<float> nextOffset = JVector2<float>(0, JImGuiImpl::GetAlphabetSize().y);
				drawList->AddText(textOffset + minP + nextOffset, IM_COL32(225, 225, 225, 200), info.c_str());
			}

			if (!IsLeaf())
			{ 
				const float mulFactor = (1 << (sizeInfo.maxDepth - depth)) * 0.5f;
				const float centerXOffset = (mulFactor * sizeInfo.width + (mulFactor - 1) * sizeInfo.paddingX);
				 
				const float leftCenterX = centerX - (centerXOffset + sizeInfo.paddingX);
				const float leftCenterY = centerY + (sizeInfo.height + sizeInfo.paddingY);

				const float rightCenterX = centerX + (centerXOffset + sizeInfo.paddingX);
				const float rightCenterY = centerY + (sizeInfo.height + sizeInfo.paddingY);

				JVector2<float> lineCP = JVector2<float>(centerX + sizeInfo.mouseOffset.x, centerY + sizeInfo.halfHeight + sizeInfo.mouseOffset.y);
				JVector2<float> lineLP = JVector2<float>(leftCenterX + sizeInfo.mouseOffset.x, leftCenterY + sizeInfo.mouseOffset.y);
				JVector2<float> lineRP = JVector2<float>(rightCenterX + sizeInfo.mouseOffset.x, rightCenterY + sizeInfo.mouseOffset.y);
				 
				drawList->AddLine(lineCP, lineLP, IM_COL32(155, 155, 155, 255), sizeInfo.lineThickness);
				drawList->AddLine(lineCP, lineRP, IM_COL32(155, 155, 155, 255), sizeInfo.lineThickness);
				 
				if(left != nullptr)
					left->NodeOnScreen(sizeInfo, leftCenterX, leftCenterY, depth + 1);
				if(right != nullptr)
					right->NodeOnScreen(sizeInfo, rightCenterX, rightCenterY, depth + 1);
			}
		}
		bool JEditorBinaryTreeView::JNode::IsRoot()const noexcept
		{
			return parent == nullptr;
		}
		bool JEditorBinaryTreeView::JNode::IsLeaf()const noexcept
		{
			return left == nullptr && right == nullptr;
		}

		void JEditorBinaryTreeView::Initialize(const uint nodeCount)noexcept
		{
			coordGrid.SetGridSize(20000);
			JEditorBinaryTreeView::nodeCount = nodeCount;
			allNodes.resize(nodeCount);
			nowNodeIndex = 0;
			depth = 0;
			maxDepth = 0; 
		}
		void JEditorBinaryTreeView::Clear()noexcept
		{
			allNodes.clear();
			nowNode = nullptr;
			nowNodeIndex = 0;
		}
		void JEditorBinaryTreeView::BuildNode(const std::string& name, const std::string& info)noexcept
		{
			allNodes[nowNodeIndex] = std::make_unique<JNode>(nowNode, name, info);
			nowNode = allNodes[nowNodeIndex].get();
			++nowNodeIndex;
			++depth;
		}
		void JEditorBinaryTreeView::EndLeft()noexcept
		{
			--depth;
			nowNode = nowNode->parent;
			if (depth > maxDepth)
				maxDepth = depth;
		}
		void JEditorBinaryTreeView::EndRight()noexcept
		{
			--depth;
			nowNode = nowNode->parent;
			if (depth > maxDepth)
				maxDepth = depth;
		}
		void JEditorBinaryTreeView::TreeOnScreen(const std::string& treeWindowName, bool& isOpen)
		{
			JVector2<int> windowSize = JImGuiImpl::GetClientWindowSize();
			const uint nodePaddingX = windowSize.x * nodePaddingXRate;
			const uint nodePaddingY = windowSize.y * nodePaddingYRate;
			const uint nodeSpacingX = windowSize.x * nodeSpacingXRate;
			const uint nodeSpacingY = windowSize.y * nodeSpacingYRate;
 
			const uint nodeWidth = (uint)(windowSize.x * nodeWidthRate);
			const uint nodeHeight = (uint)(windowSize.y * nodeHeightRate);

			ImGuiWindowFlags flag = ImGuiWindowFlags_NoDocking;

			if (JImGuiImpl::BeginWindow(treeWindowName + "##BinaryTreeView", &isOpen, flag))
			{  
				if (JImGuiImpl::Button("Look Mid"))
					coordGrid.Clear();
				ImGui::SameLine();
				JImGuiImpl::Text("Count: " + std::to_string(allNodes.size()));
				if (allNodes.size() > 0)
				{
					coordGrid.Update();
					coordGrid.Draw();
					const float zoomRate = coordGrid.GetZoomRate();
					const JNodeSizeInfo sizeInfo = JNodeSizeInfo(nodeWidth, nodeHeight,
						nodePaddingX, nodePaddingY + nodeHeight,
						nodeSpacingX, nodeSpacingY,
						coordGrid.GetMouseOffset(),
						zoomRate,
						maxDepth);

					allNodes[0]->NodeOnScreen(sizeInfo, (uint)(windowSize.x / 2), nodeSpacingY + (uint)(nodeHeight / 2), 0);
				}
				JImGuiImpl::EndWindow();
			}
		}
	}
}