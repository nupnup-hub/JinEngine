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


#include"JKdTreeNode.h" 
#include"../JAcceleratorOption.h"
#include"../JAcceleratorVisualizeInterface.h"
#include"../../../../Resource/Mesh/JMeshGeometry.h" 
#include"../../../../GameObject/JGameObject.h" 
#include"../../../../GameObject/JGameObjectCreator.h"
#include"../../../../Component/RenderItem/JRenderItem.h"
#include"../../../../Component/RenderItem/JRenderItemPrivate.h"
#include"../../../../Component/Transform/JTransform.h" 
#include"../../../../../Core/Math/JMathHelper.h"  
#include"../../../../../Core/Math/JVectorExtend.h"
#include"../../../../../Core/Utility/JCommonUtility.h"  

//Test
//#include"../../../Object/Resource/JResourceManager.h"

using namespace DirectX;
namespace JinEngine
{

	JKdTreeNode::JKdTreeNode(const uint nodeNumber, const J_KDTREE_NODE_TYPE nodeType, const DirectX::BoundingBox& bbox, JKdTreeNode* parent)
		:nodeNumber(nodeNumber), nodeType(nodeType), bbox(bbox), parent(parent)
	{
		if (parent != nullptr)
		{
			if (parent->left == nullptr)
				parent->left = this;
			else
				parent->right = this; 
		} 
		//else
		//	JKdTreeNode::nodeType = J_KDTREE_NODE_TYPE::ROOT;
	}
	JKdTreeNode::~JKdTreeNode() {}
	void JKdTreeNode::CreateDebugGameObject(const JUserPtr<JGameObject>& parent, bool onlyLeafNode)noexcept
	{
		if (debugGameObject == nullptr)
		{
			if (nodeType != J_KDTREE_NODE_TYPE::LEAF && onlyLeafNode)
				return;

			if (nodeType != J_KDTREE_NODE_TYPE::LEAF)
				debugGameObject = JGCI::CreateDebugLineShape(parent, OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::BOUNDING_BOX_LINE, J_DEFAULT_MATERIAL::DEBUG_LINE_RED, false);
			else
				debugGameObject = JGCI::CreateDebugLineShape(parent, OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::BOUNDING_BOX_LINE, J_DEFAULT_MATERIAL::DEBUG_LINE_GREEN, false);

			const float outlineFactor = 0.01f;
			const BoundingBox debugBBox = debugGameObject->GetRenderItem()->GetMesh()->GetBoundingBox();

			debugGameObject->GetTransform()->SetScale(JVector3<float>(bbox.Extents) / debugBBox.Extents + outlineFactor);
			debugGameObject->GetTransform()->SetPosition(JVector3<float>(bbox.Center) - debugBBox.Center);
		}
	}
	void JKdTreeNode::DestroyDebugGameObject()noexcept
	{
		if (debugGameObject != nullptr)
		{
			JGameObject::BeginDestroy(debugGameObject.Get());
			debugGameObject = nullptr;
		}
	}
	void JKdTreeNode::Clear()noexcept
	{
		innerGameObject.clear();
		parent = nullptr;
		left = nullptr;
		right = nullptr;
		DestroyDebugGameObject();
	}
	/*
	void JKdTreeNode::Culling(JAcceleratorCullingInfo& info, Core::J_CULLING_FLAG flag)noexcept
	{
		Core::J_CULLING_RESULT res = Contain(info, bbox, flag);
		if (res == Core::J_CULLING_RESULT::CONTAIN)
			SetVisible(info);
		else if (res == Core::J_CULLING_RESULT::DISJOINT)
			SetInVisible(info);
		else
		{
			if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
				SetVisible(info);
			else
			{
				left->Culling(info, flag);
				right->Culling(info, flag);
			}
		}
	}
	*/
	void JKdTreeNode::Culling(JAcceleratorCullingInfo& info)noexcept
	{
		ContainmentType res = Contain(info, bbox);
		if (res == ContainmentType::CONTAINS)
			SetVisible(info);
		else if (res == ContainmentType::DISJOINT)
			SetInVisible(info);
		else
		{
			if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
				SetVisible(info);
			else
			{
				if (info.allowCullingOrderedByDistance)
				{
					if (IsNearRight(info))
					{
						right->Culling(info);
						left->Culling(info);
					}
					else
					{
						left->Culling(info);
						right->Culling(info);
					}
				}
				else
				{
					left->Culling(info);
					right->Culling(info);
				}
			}
		}
	}
	void JKdTreeNode::FindIntersect(JAcceleratorIntersectInfo& info)const noexcept
	{
		if (info.untilFirst)
			info.result.push_back(FindFirstntersect(info));
		else
		{
			if (info.sortType == J_ACCELERATOR_SORT_TYPE::ASCENDING)
			{
				FindIntersectNotSort(info);
				info.SortResult();
			}
			else if (info.sortType == J_ACCELERATOR_SORT_TYPE::DESCENDING)
			{
				FindIntersectNotSort(info); 
				info.SortResult();
			}
			else
				FindIntersectNotSort(info);
		}
	}
	void JKdTreeNode::FindContain(JAcceleratorContainInfo& info)const noexcept
	{
		FindContainNotSort(info);
	}
	void JKdTreeNode::AlignLeafNode(const JAcceleratorAlignInfo& info, std::vector<JKdTreeNode*>& alignNode, uint& index, const uint depth)noexcept
	{
		if (index >= alignNode.size())
			return;

		if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
		{
			bool canAdd = true;
			if (info.tool == JAcceleratorAlignInfo::ALIGN_TOOL::FRUSTUM && info.hasCullingArea)
				canAdd = info.cullingFrustum.Intersects(bbox) == ContainmentType::DISJOINT;

			if (canAdd)
				alignNode[index++] = this;
		}
		else if (info.alignMaxDepth < depth)
		{
			left->AlignLeafNode(info, alignNode, index, depth + 1);
			right->AlignLeafNode(info, alignNode, index, depth + 1);
		}
		else
		{
			if (IsNearRight(info))
			{
				right->AlignLeafNode(info, alignNode, index, depth + 1);
				left->AlignLeafNode(info, alignNode, index, depth + 1);
			}
			else
			{
				left->AlignLeafNode(info, alignNode, index, depth + 1);
				right->AlignLeafNode(info, alignNode, index, depth + 1);
			}
		}
	}
	uint JKdTreeNode::GetNodeNumber()const noexcept
	{
		return nodeNumber;
	}
	uint JKdTreeNode::GetLeftNumberEnd()const noexcept
	{
		if (left == nullptr)
			return nodeNumber;
		else
			return left->FindRightLeafNode()->nodeNumber;
	}
	uint JKdTreeNode::GetRightNumberEnd()const noexcept
	{
		if (right == nullptr)
			return nodeNumber;
		else
			return right->FindRightLeafNode()->nodeNumber;
	} 
	J_KDTREE_NODE_TYPE JKdTreeNode::GetNodeType()const noexcept
	{
		return nodeType;
	}
	DirectX::BoundingBox JKdTreeNode::GetBoundingBox()const noexcept
	{
		return bbox;
	}
	JKdTreeNode* JKdTreeNode::GetParentNode()noexcept
	{
		return parent;
	}
	JKdTreeNode* JKdTreeNode::GetLeftNode()const noexcept
	{
		return left;
	}
	JKdTreeNode* JKdTreeNode::GetRightNode()const noexcept
	{
		return right;
	}
	std::vector<JUserPtr<JGameObject>> JKdTreeNode::GetInnerGameObject()const noexcept
	{
		return innerGameObject;
	}
	std::vector<JUserPtr<JGameObject>> JKdTreeNode::GetAreaInnerGameObject()const noexcept
	{
		if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
			return innerGameObject;
		else
		{
			std::vector<JUserPtr<JGameObject>> leftRes = left->GetAreaInnerGameObject();
			std::vector<JUserPtr<JGameObject>> rightRes = right->GetAreaInnerGameObject();
			leftRes.insert(leftRes.end(), rightRes.begin(), rightRes.end());
			return leftRes;
		}
	}
	uint JKdTreeNode::GetInnerGameObjectCount()const noexcept
	{
		return (uint)innerGameObject.size();
	}
	uint JKdTreeNode::GetDepth()const noexcept
	{
		uint depth = 0;
		JKdTreeNode* nowParent = parent;
		while (nowParent != nullptr)
		{
			nowParent = nowParent->parent;
			++depth;
		}
		return depth;
	}
	void JKdTreeNode::SetNodeNumber(const uint newNodeNumber)noexcept
	{
		nodeNumber = newNodeNumber;
	}
	void JKdTreeNode::SetSplitType(const uint newSplitType)noexcept
	{
		if (newSplitType == 0)
			splitType = J_KDTREE_NODE_SPLIT_AXIS::X;
		else if (newSplitType == 1)
			splitType = J_KDTREE_NODE_SPLIT_AXIS::Y;
		else
			splitType = J_KDTREE_NODE_SPLIT_AXIS::Z;
	}
	void JKdTreeNode::SetSplitType(const J_KDTREE_NODE_SPLIT_AXIS newSplitType)noexcept
	{
		splitType = newSplitType;
	}
	void JKdTreeNode::SetLeftNode(JKdTreeNode* newLeftNode)noexcept
	{
		if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
			return;

		left = newLeftNode;
		if (left != nullptr)
		{
			left->parent = this;
		}
	}
	void JKdTreeNode::SetRightNode(JKdTreeNode* newRightNode)noexcept
	{
		if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
			return;

		right = newRightNode;
		if (right != nullptr)
			right->parent = this;
	}
	void JKdTreeNode::SetVisible(JAcceleratorCullingInfo& info)noexcept
	{
		if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
		{
			const uint innerCount = (uint)innerGameObject.size();
			for (uint i = 0; i < innerCount; ++i)
			{
				JUserPtr<JRenderItem> rItem = innerGameObject[i]->GetRenderItem();
				if ((rItem->GetAcceleratorMask() & ACCELERATOR_ALLOW_CULLING) > 0)
				{
					if (IsIntersectCullingFrustum(info, bbox))
						SetCulling(info, rItem);
					else
					{
						if (info.allowPushVisibleObjVec)
						{
							for(const auto& data: innerGameObject)
							{
								(*info.appAlignedObjVec)[info.pushedCount] = data;
								++info.pushedCount;
							}
						}
						OffCulling(info, rItem);
					}
				}
			}
		}
		else
		{
			left->SetVisible(info);
			right->SetVisible(info);
		}
	}
	void JKdTreeNode::SetInVisible(JAcceleratorCullingInfo& info)noexcept
	{
		if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
		{
			const uint innerCount = (uint)innerGameObject.size();
			for (uint i = 0; i < innerCount; ++i)
			{
				JUserPtr<JRenderItem> rItem = innerGameObject[i]->GetRenderItem();
				if ((rItem->GetAcceleratorMask() & ACCELERATOR_ALLOW_CULLING) > 0)
					SetCulling(info, rItem);
			}
		}
		else
		{
			left->SetInVisible(info);
			right->SetInVisible(info);
		}
	}
	bool JKdTreeNode::IsLeftNode()const noexcept
	{
		return parent != nullptr ? parent->left->nodeNumber == nodeNumber : false;
	}
	bool JKdTreeNode::IsChildNode(const uint pNodeNumber)const noexcept
	{
		JKdTreeNode* nowParent = parent;
		while (nowParent != nullptr)
		{
			if (nowParent->nodeNumber == pNodeNumber)
				return true;
		}
		return false;
	}
	bool JKdTreeNode::HasGameObject(const size_t guid)noexcept
	{
		return JCUtil::GetTypeIndex(innerGameObject, guid) != JCUtil::searchFail;
	}
	bool JKdTreeNode::IsNearRight(const JAcceleratorCullingInfo& info)
	{
		float centerFactor = 0;
		if (splitType == J_KDTREE_NODE_SPLIT_AXIS::X)
			centerFactor = bbox.Center.x;
		else if (splitType == J_KDTREE_NODE_SPLIT_AXIS::Y)
			centerFactor = bbox.Center.y;
		else
			centerFactor = bbox.Center.z;
		return info.pos[(int)splitType] > centerFactor;
	}
	bool JKdTreeNode::IsNearRight(const JAcceleratorAlignInfo& info)
	{
		float centerFactor = 0;
		if (splitType == J_KDTREE_NODE_SPLIT_AXIS::X)
			centerFactor = bbox.Center.x;
		else if (splitType == J_KDTREE_NODE_SPLIT_AXIS::Y)
			centerFactor = bbox.Center.y;
		else
			centerFactor = bbox.Center.z;
		return info.pos[(int)splitType] > centerFactor;
	}
	void JKdTreeNode::AddInnerGameObject(const JUserPtr<JGameObject>& newInnerGameObject)noexcept
	{
		innerGameObject.push_back(newInnerGameObject);
	}
	void JKdTreeNode::AddInnerGameObject(const std::vector<JUserPtr<JGameObject>>& newInnerGameObject)noexcept
	{
		innerGameObject = newInnerGameObject;
	}
	void JKdTreeNode::RemoveInnerGameObject(const size_t guid)noexcept
	{
		int index = JCUtil::GetTypeIndex(innerGameObject, guid);
		if (index != JCUtil::searchFail)
			innerGameObject.erase(innerGameObject.begin() + index);
	}
	void JKdTreeNode::StuffInnerGameObject(std::vector<JUserPtr<JGameObject>>& objList,
		uint& listIndex,
		JAcceleratorAlignInfo::AlignPassCondPtr cond)
	{
		const uint innerCount = (uint)innerGameObject.size();
		for (uint i = 0; i < innerCount; ++i)
		{
			if (cond != nullptr && !cond(innerGameObject[i].Get()))
				continue;

			bool isOverlap = false;
			const size_t guid = innerGameObject[i]->GetGuid();
			for (uint j = 0; j < listIndex; ++j)
			{
				if (objList[j]->GetGuid() == guid)
				{ 
					isOverlap = true;
					break;
				}
			}
			if (!isOverlap)
			{
				objList[listIndex] = innerGameObject[i];
				++listIndex;
			}
		}
		//offset += innerCount;
	}
	JKdTreeNode* JKdTreeNode::FindRightLeafNode() noexcept
	{
		if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
			return this;
		else
			return right->FindRightLeafNode();
	}
	JAcceleratorIntersectInfo::Result JKdTreeNode::FindFirstntersect(JAcceleratorIntersectInfo& info)const noexcept
	{
		using Result = JAcceleratorIntersectInfo::Result;
		if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
		{
			float minDist = FLT_MAX;
			int index = -1;
			const uint innerCount = (uint)innerGameObject.size();
			for (uint i = 0; i < innerCount; ++i)
			{
				auto oriBBox = innerGameObject[i]->GetRenderItem()->GetOrientedBoundingBox();
				float dist = 0;
				if (oriBBox.Intersects(info.ray.PosV(), info.ray.DirV(), dist))
				{
					if (minDist > dist && info.CanAdd(oriBBox))
					{
						minDist = dist;
						index = i;
					}
				}
			}
			if (index != -1)
				return Result{ innerGameObject[index] , minDist };
			else
				return Result();
		}
		else
		{
			float leftDist = FLT_MAX;
			float rightDist = FLT_MAX;

			const bool leftIntersected = left->bbox.Intersects(info.ray.PosV(), info.ray.DirV(), leftDist);
			const bool rightIntersected = right->bbox.Intersects(info.ray.PosV(), info.ray.DirV(), rightDist);

			Result res;
			if (leftDist < 0 || rightDist < 0)
			{
				Result leftRes = leftIntersected ? left->FindFirstntersect(info) : Result();
				Result rightRes = rightIntersected ? right->FindFirstntersect(info) : Result();

				if (leftRes.dist < rightRes.dist)
				{
					if (leftRes.obj != nullptr)
						res = leftRes;
					if (res.obj == nullptr && rightRes.obj != nullptr)
						res = rightRes;
				}
				else
				{
					if (rightRes.obj != nullptr)
						res = rightRes;
					if (res.obj == nullptr && leftRes.obj != nullptr)
						res = leftRes;
				}
			}
			else if (leftDist < rightDist)
			{
				if (leftIntersected)
					res = left->FindFirstntersect(info);
				if (rightIntersected && res.obj == nullptr)
					res = right->FindFirstntersect(info);
			}
			else
			{
				if (rightIntersected)
					res = right->FindFirstntersect(info);
				if (leftIntersected && res.obj == nullptr)
					res = left->FindFirstntersect(info);
			}
			return res;
		}
	} 
	void JKdTreeNode::FindIntersectNotSort(JAcceleratorIntersectInfo& info)const noexcept
	{
		if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
		{
			const uint innerCount = (uint)innerGameObject.size();
			for (uint i = 0; i < innerCount; ++i)
			{
				float dist = 0;
				if (innerGameObject[i]->GetRenderItem()->GetOrientedBoundingBox().Intersects(info.ray.PosV(), info.ray.DirV(), dist) &&
					info.CanAdd(innerGameObject[i]))
					info.result.push_back({ innerGameObject[i], dist });
			}
		}
		else
		{
			float leftDist = FLT_MAX;
			float rightDist = FLT_MAX;

			if (left->bbox.Intersects(info.ray.PosV(), info.ray.DirV(), leftDist))
				left->FindIntersectNotSort(info);
			if (right->bbox.Intersects(info.ray.PosV(), info.ray.DirV(), rightDist))
				right->FindIntersectNotSort(info);
		}
	}
	void JKdTreeNode::FindContainNotSort(JAcceleratorContainInfo& info)const noexcept
	{
		if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
		{
			const uint innerCount = (uint)innerGameObject.size();
			for (uint i = 0; i < innerCount; ++i)
			{
				if (IsContain(info, innerGameObject[i]->GetRenderItem()->GetOrientedBoundingBox()))
					info.resultObjVec.push_back(innerGameObject[i]);
			}
		}
		else
		{
			if (IsContain(info, left->bbox))
				left->FindContainNotSort(info);
			if (IsContain(info, right->bbox))
				right->FindContainNotSort(info);
		}
	}
	void JKdTreeNode::BuildDebugNode(JAcceleratorVisualizeInterface* treeView)
	{
		if (nodeType == J_KDTREE_NODE_TYPE::ROOT)
		{
			treeView->BuildNode(std::to_string(nodeNumber), nodeNumber, "Root");
			if (left != nullptr)
				left->BuildDebugNode(treeView);
			if (right != nullptr)
				right->BuildDebugNode(treeView);
		}
		else if (nodeType == J_KDTREE_NODE_TYPE::NODE)
		{
			treeView->BuildNode(std::to_string(nodeNumber), nodeNumber);
			left->BuildDebugNode(treeView);
			right->BuildDebugNode(treeView);
			if (IsLeftNode())
				treeView->BuildEndSplit();
			else
				treeView->BuildEndSplit();
		}
		else
		{
			std::wstring info = L"InnerGameObject: \n";
			for (const auto& data : innerGameObject)
				info += data->GetName() + L"\n";
			treeView->BuildNode(std::to_string(nodeNumber), nodeNumber, JCUtil::WstrToU8Str(info));
			if (IsLeftNode())
				treeView->BuildEndSplit();
			else
				treeView->BuildEndSplit();
		}
	} 
}