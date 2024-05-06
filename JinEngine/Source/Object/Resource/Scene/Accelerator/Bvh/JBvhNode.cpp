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


#include"JBvhNode.h" 
#include"../JAcceleratorOption.h"
#include"../JAcceleratorVisualizeInterface.h"
#include"../../../../Resource/Mesh/JMeshGeometry.h" 
#include"../../../../GameObject/JGameObject.h" 
#include"../../../../GameObject/JGameObjectCreator.h"
#include"../../../../Component/RenderItem/JRenderItem.h"
#include"../../../../Component/Transform/JTransform.h" 
#include"../../../../../Core/Utility/JCommonUtility.h"
#include"../../../../../Core/Math/JMathHelper.h"  
#include"../../../../../Core/Math/JVectorExtend.h" 
  
using namespace DirectX;
namespace JinEngine
{

	JBvhNode::JBvhNode(const uint nodeNumber, const J_BVH_NODE_TYPE type, const DirectX::BoundingBox& bbox, JBvhNode* parent, const JUserPtr<JGameObject>& innerGameObject, bool isLeftNode)
		:nodeNumber(nodeNumber), type(type), bbox(bbox), parent(parent), innerGameObject(innerGameObject)
	{
		if (parent != nullptr)
		{
			if (isLeftNode)
				parent->left = this;
			else
				parent->right = this;
		} 
	}
	JBvhNode::~JBvhNode() {}
	void JBvhNode::CreateDebugGameObject(const JUserPtr<JGameObject>& parent, bool onlyLeafNode)noexcept
	{
		if (type != J_BVH_NODE_TYPE::LEAF && onlyLeafNode)
			return;

		if (debugGameObject == nullptr)
		{
			if (type == J_BVH_NODE_TYPE::LEAF)
				debugGameObject = JGCI::CreateDebugLineShape(parent, OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::BOUNDING_BOX_LINE, J_DEFAULT_MATERIAL::DEBUG_LINE_GREEN, false);
			else
				debugGameObject = JGCI::CreateDebugLineShape(parent, OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::BOUNDING_BOX_LINE, J_DEFAULT_MATERIAL::DEBUG_LINE_RED, false);
			SetDebugObjectTransform();
		}
	}
	void JBvhNode::DestroyDebugGameObject()noexcept
	{
		if (debugGameObject != nullptr)
		{
			JGameObject::BeginDestroy(debugGameObject.Get());
			debugGameObject = nullptr;
		}
	}
	void JBvhNode::Clear()noexcept
	{
		innerGameObject = nullptr;
		parent = nullptr;
		left = nullptr;
		right = nullptr;
		DestroyDebugGameObject();
	}
/*
* 	void JBvhNode::Culling(JAcceleratorCullingInfo& info, Core::J_CULLING_FLAG flag)noexcept
	{
		Core::J_CULLING_RESULT res = Contain(info, bbox, flag);
		if (res == Core::J_CULLING_RESULT::CONTAIN)
			SetVisible(info);
		else if (res == Core::J_CULLING_RESULT::DISJOINT)
			SetInVisible(info);
		else
		{
			if (type == J_BVH_NODE_TYPE::LEAF)
				SetVisible(info);
			else
			{
				left->Culling(info, flag);
				right->Culling(info, flag);
			}
		}
	}
*/
	void JBvhNode::Culling(JAcceleratorCullingInfo& info)noexcept
	{
		//frustum.Contain이 box.Contain보다 빠르다
		if (type == J_BVH_NODE_TYPE::LEAF)
		{
			DirectX::BoundingOrientedBox oriBox = innerGameObject->GetRenderItem()->GetOrientedBoundingBox();
			if (Contain(info, oriBox) == ContainmentType::DISJOINT)
				SetInVisible(info);
			else
				SetVisible(info, IsIntersectCullingFrustum(info, oriBox));
		}
		else
		{
			ContainmentType res = Contain(info, bbox);
			if (res == ContainmentType::CONTAINS)
				SetVisible(info, IsIntersectCullingFrustum(info, bbox));
			else if (res == ContainmentType::DISJOINT)
				SetInVisible(info);
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
	void JBvhNode::FindIntersect(JAcceleratorIntersectInfo& info)const noexcept
	{ 
		if (info.untilFirst)
		{
			info.sortType = J_ACCELERATOR_SORT_TYPE::ASCENDING;
			FindIntersectNotSort(info, FLT_MAX);
			info.SortResult();
			/*
			* FindFirstIntersect는 정확성에 문제가 있으므로 위에 방식을 사용한다
			* 성능은 object 1000개 기준 3 ~ 4배정도 나지만 정확하다.
			* FindFirstIntersect가 비정확한 이유
			* 1. 두개의 자식 노드중 인터섹팅 결과가 가까운노드에서 항상 FirstIntersected를 구할수있는건 아니다.
			* 2. ray ori가 노드에 내부에서 시작된경우 더더욱 Intersect out dist가 음수이므로 FirstIntersected가 있는 자식노드에
			*    접근하는게 어렵다.
			* 위에 이유때문에 정확히 구하기 위해선 FindIntersectNotSort와 같이 여러 노드들을 탐색해야할 필요가있으며
			* 현재는 FindIntersectNotSort를 사용하며 추후에 개선하도록한다.
			*/
			//FindFirstIntersect(info, FLT_MAX)
			//info.result.push_back(FindFirstIntersect(info, FLT_MAX));
		}
		else
		{
			if (info.sortType == J_ACCELERATOR_SORT_TYPE::ASCENDING)
			{
				FindIntersectNotSort(info, FLT_MAX);
				info.SortResult();
			}
			else if (info.sortType == J_ACCELERATOR_SORT_TYPE::DESCENDING)
			{
				FindIntersectNotSort(info, FLT_MAX);
				info.SortResult();
			}
			else
				FindIntersectNotSort(info, FLT_MAX);
		}
	}
	void JBvhNode::FindContain(JAcceleratorContainInfo& info)const noexcept
	{
		FindContainNotSort(info);
	}
	void JBvhNode::AlignLeafNode(const JAcceleratorAlignInfo& info, std::vector<JUserPtr<JGameObject>>& alignGameObject, uint& index, const uint depth)noexcept
	{ 
		if (index >= alignGameObject.size())
			return;

		if (type == J_BVH_NODE_TYPE::LEAF)
		{
			bool canAdd = true;
			if (info.tool == JAcceleratorAlignInfo::ALIGN_TOOL::FRUSTUM && info.hasCullingArea)
				canAdd = info.cullingFrustum.Intersects(bbox) == ContainmentType::DISJOINT;

			if (info.alignPassCondPtr != nullptr)
				canAdd = info.alignPassCondPtr(innerGameObject.Get());

			if (canAdd)
			{
				alignGameObject[index] = innerGameObject;
				++index;
			}
		}
		else if (info.alignMaxDepth < depth)
		{
			left->AlignLeafNode(info, alignGameObject, index, depth + 1);
			right->AlignLeafNode(info, alignGameObject, index, depth + 1);
		}
		else
		{
			if (IsNearRight(info))
			{
				right->AlignLeafNode(info, alignGameObject, index, depth + 1);
				left->AlignLeafNode(info, alignGameObject, index, depth + 1);
			}
			else
			{
				left->AlignLeafNode(info, alignGameObject, index, depth + 1);
				right->AlignLeafNode(info, alignGameObject, index, depth + 1);
			}
		}
	}
	void JBvhNode::UpdateInnerGameObject()noexcept
	{
		if (type == J_BVH_NODE_TYPE::LEAF)
		{
			bbox = innerGameObject->GetRenderItem()->GetBoundingBox();
			SetDebugObjectTransform();
		}
	}
	uint JBvhNode::GetNodeNumber()const noexcept
	{
		return nodeNumber;
	}
	uint JBvhNode::GetLeftNumberEnd()const noexcept
	{
		if (left == nullptr)
			return nodeNumber;
		else
			return left->FindRightLeafNode()->nodeNumber;
	}
	uint JBvhNode::GetRightNumberEnd()const noexcept
	{
		if (right == nullptr)
			return nodeNumber;
		else
			return right->FindRightLeafNode()->nodeNumber;
	}
	J_BVH_NODE_TYPE JBvhNode::GetNodeType()const noexcept
	{
		return type;
	}
	DirectX::BoundingBox JBvhNode::GetBoundingBox()const noexcept
	{
		return bbox;
	}
	JBvhNode* JBvhNode::GetParentNode()noexcept
	{
		return parent;
	}
	JBvhNode* JBvhNode::GetLeftNode()noexcept
	{
		return left;
	}
	JBvhNode* JBvhNode::GetRightNode()noexcept
	{
		return right;
	}
	JBvhNode* JBvhNode::GetContainNodeToRoot(const DirectX::BoundingBox& boundBox)noexcept
	{
		ContainmentType res = bbox.Contains(boundBox);
		if (res == ContainmentType::CONTAINS)
			return this;
		else
		{
			if (parent == nullptr)
				return nullptr;
			else
				return parent->GetContainNodeToRoot(boundBox);
		}
	}
	JBvhNode* JBvhNode::GetContainNodeToLeaf(const DirectX::BoundingBox& boundBox)noexcept
	{
		ContainmentType res = bbox.Contains(boundBox);
		if (res == ContainmentType::CONTAINS)
		{
			if (type == J_BVH_NODE_TYPE::LEAF || (left == nullptr || right == nullptr))
				return this;
			else
			{
				JBvhNode* leftRes = left->GetContainNodeToLeaf(boundBox);
				JBvhNode* rightRes = right->GetContainNodeToLeaf(boundBox);

				if (leftRes != nullptr && rightRes == nullptr)
					return leftRes;
				else if (leftRes == nullptr && rightRes != nullptr)
					return rightRes;
				else
					return this;
			}
		}
		else
			return nullptr;
	}
	JUserPtr<JGameObject> JBvhNode::GetInnerGameObject()const noexcept
	{
		return innerGameObject;
	}
	JUserPtr<JGameObject> JBvhNode::GetDebugGameObject()const noexcept
	{
		return debugGameObject;
	}
	void JBvhNode::SetNodeNumber(const uint newNumber)noexcept
	{
		nodeNumber = newNumber;
	}
	void JBvhNode::SetNodeType(const J_BVH_NODE_TYPE newNodeType)noexcept
	{
		type = newNodeType;
	}
	void JBvhNode::SetLeftNode(JBvhNode* newLeftNode)noexcept
	{
		if (type == J_BVH_NODE_TYPE::LEAF)
			return;

		left = newLeftNode;
		if (left != nullptr)
			left->parent = this;
	}
	void JBvhNode::SetRightNode(JBvhNode* newRightNode)noexcept
	{
		if (type == J_BVH_NODE_TYPE::LEAF)
			return;

		right = newRightNode;
		if (right != nullptr)
			right->parent = this;
	}
	void JBvhNode::SetInnerGameObject(const JUserPtr<JGameObject>& newInnerGameObject)noexcept
	{
		innerGameObject = newInnerGameObject;
	}
	void JBvhNode::SetVisible(JAcceleratorCullingInfo& info)noexcept
	{
		if (type == J_BVH_NODE_TYPE::LEAF)
		{
			JUserPtr<JRenderItem> rItem = innerGameObject->GetRenderItem();
			if ((rItem->GetAcceleratorMask() & ACCELERATOR_ALLOW_CULLING) > 0)
			{
				if (info.allowPushVisibleObjVec)
				{
					(*info.appAlignedObjVec)[info.pushedCount] = innerGameObject;
					++info.pushedCount;
				}
				OffCulling(info, rItem);
			}
		}
		else
		{
			left->SetVisible(info);
			right->SetVisible(info);
		}
	}
	void JBvhNode::SetVisible(JAcceleratorCullingInfo& info, const bool camInParentBBox)noexcept
	{
		if (type == J_BVH_NODE_TYPE::LEAF)
		{
			JUserPtr<JRenderItem> rItem = innerGameObject->GetRenderItem();
			if ((rItem->GetAcceleratorMask() & ACCELERATOR_ALLOW_CULLING) > 0)
			{
				if (camInParentBBox && IsIntersectCullingFrustum(info, rItem->GetOrientedBoundingBox()))
					SetCulling(info, rItem);
				else
				{
					if (info.allowPushVisibleObjVec)
					{ 
						(*info.appAlignedObjVec)[info.pushedCount] = innerGameObject;
						++info.pushedCount;
					}
					OffCulling(info, rItem);
				}
			}
		}
		else
		{
			if (camInParentBBox)
			{
				left->SetVisible(info, IsIntersectCullingFrustum(info, bbox));
				right->SetVisible(info, IsIntersectCullingFrustum(info, bbox));
			}
			else
			{
				left->SetVisible(info, camInParentBBox);
				right->SetVisible(info, camInParentBBox);
			}
		}
	}
	void JBvhNode::SetInVisible(JAcceleratorCullingInfo& info)noexcept
	{
		if (type == J_BVH_NODE_TYPE::LEAF)
		{
			JUserPtr<JRenderItem> rItem = innerGameObject->GetRenderItem();
			if ((rItem->GetAcceleratorMask() & ACCELERATOR_ALLOW_CULLING) > 0)
				SetCulling(info, rItem);
		}
		else
		{
			left->SetInVisible(info);
			right->SetInVisible(info);
		}
	}
	void JBvhNode::SetDebugObjectTransform()noexcept
	{
		if (debugGameObject == nullptr)
			return;

		static constexpr float outlineFactor = 0.01f;
		JUserPtr<JTransform> transform = debugGameObject->GetTransform();
		const BoundingBox debugBox = debugGameObject->GetRenderItem()->GetMesh()->GetBoundingBox();

		if (type == J_BVH_NODE_TYPE::LEAF)
		{
			const BoundingOrientedBox innterOriented = innerGameObject->GetRenderItem()->GetOrientedBoundingBox();
			//	XMMATRIX matWorld = XMMatrixRotationQuaternion(XMLoadFloat4(&debugOriented.Orientation));
				//XMMATRIX matScale = XMMatrixScaling(debugOriented.Extents.x, debugOriented.Extents.y, debugOriented.Extents.z);
				//matWorld = XMMatrixMultiply(matScale, matWorld);
				//XMVECTOR position = XMLoadFloat3(&debugOriented.Center);
				//matWorld.r[3] = XMVectorSelect(matWorld.r[3], position, g_XMSelect1110);

			transform->SetScale((JVector3F(innterOriented.Extents) / debugBox.Extents) + outlineFactor);
			transform->SetRotation(innterOriented.Orientation);
			transform->SetPosition((JVector3F(innterOriented.Center) - debugBox.Center));
		}
		else
		{
			//Bounding Box는 회전에 상관없이 일정한 모양을 유지하므로
			//Object에 회전에 맞는 DebugBox는 생성할 수 없다.
			//BoundingOriented를 사용하면 회전에 맞는 bbox를 생성할수있다
			debugGameObject->GetTransform()->SetScale(JVector3<float>(bbox.Extents) / debugBox.Extents + outlineFactor);
			debugGameObject->GetTransform()->SetPosition(JVector3<float>(bbox.Center) - debugBox.Center);
		}
	}
	bool JBvhNode::IsLeftNode()const noexcept
	{
		if (parent != nullptr)
			return parent->left->nodeNumber == nodeNumber;
		else
			return false;
	}
	bool JBvhNode::IsContainNode(const DirectX::BoundingBox& boundBox)const noexcept
	{
		return bbox.Contains(boundBox) == ContainmentType::CONTAINS;
	}
	bool JBvhNode::IsNearRight(const JAcceleratorCullingInfo& info)
	{
		return (left->bbox.Center - info.pos).Length() > (right->bbox.Center - info.pos).Length();
	}
	bool JBvhNode::IsNearRight(const JAcceleratorAlignInfo& info)
	{
		return (left->bbox.Center - info.pos).Length() > (right->bbox.Center - info.pos).Length();
	}
	JBvhNode* JBvhNode::FindRightLeafNode()noexcept
	{
		if (type == J_BVH_NODE_TYPE::LEAF)
			return this;
		else
			return right->FindRightLeafNode();
	}
	JAcceleratorIntersectInfo::Result JBvhNode::FindFirstIntersect(JAcceleratorIntersectInfo& info, const float dist)const noexcept
	{
		using Result = JAcceleratorIntersectInfo::Result;
		if (type == J_BVH_NODE_TYPE::LEAF)
		{
			if (info.CanAdd(innerGameObject))
				return Result{ innerGameObject, dist };
			else
				return Result();
		}
		else
		{
			float leftDist = 0;
			float rightDist = 0;

			bool leftIntersected = false;
			bool rightIntersected = false;

			if (left->type == J_BVH_NODE_TYPE::LEAF)
				leftIntersected = left->innerGameObject->GetRenderItem()->GetOrientedBoundingBox().Intersects(info.ray.PosV(), info.ray.DirV(), leftDist);
			else
				leftIntersected = left->bbox.Intersects(info.ray.PosV(), info.ray.DirV(), leftDist);

			if (right->type == J_BVH_NODE_TYPE::LEAF)
				rightIntersected = right->innerGameObject->GetRenderItem()->GetOrientedBoundingBox().Intersects(info.ray.PosV(), info.ray.DirV(), rightDist);
			else
				rightIntersected = right->bbox.Intersects(info.ray.PosV(), info.ray.DirV(), rightDist);
			
			Result res;
			//레이에 시작점이 bbox내부에 있을경우
			if (leftDist < 0 || rightDist < 0)
			{ 
				Result leftRes = leftIntersected ? left->FindFirstIntersect(info, leftDist) : Result();
				Result rightRes = rightIntersected ? right->FindFirstIntersect(info, rightDist) : Result();
 
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
					res = left->FindFirstIntersect(info, leftDist);
				if (rightIntersected && res.obj == nullptr)
					res = right->FindFirstIntersect(info, rightDist);
			}
			else
			{ 	
				if (rightIntersected)
					res = right->FindFirstIntersect(info, rightDist);
				if (leftIntersected && res.obj == nullptr)
					res = left->FindFirstIntersect(info, leftDist);
			}
			return res;
		}
	}
	void JBvhNode::FindIntersectNotSort(JAcceleratorIntersectInfo& info, const float dist)const noexcept
	{
		if (type == J_BVH_NODE_TYPE::LEAF)
		{
			if (info.CanAdd(innerGameObject))
				info.result.push_back(JAcceleratorIntersectInfo::Result{ innerGameObject , dist });
		}
		else
		{
			float leftDist = FLT_MAX;
			float rightDist = FLT_MAX;

			bool leftIntersected = false;
			bool rightIntersected = false;
			if (left->type == J_BVH_NODE_TYPE::LEAF)
				leftIntersected = left->innerGameObject->GetRenderItem()->GetOrientedBoundingBox().Intersects(info.ray.PosV(), info.ray.DirV(), leftDist);
			else
				leftIntersected = left->bbox.Intersects(info.ray.PosV(), info.ray.DirV(), leftDist);

			if (right->type == J_BVH_NODE_TYPE::LEAF)
				rightIntersected = right->innerGameObject->GetRenderItem()->GetOrientedBoundingBox().Intersects(info.ray.PosV(), info.ray.DirV(), rightDist);
			else
				rightIntersected = right->bbox.Intersects(info.ray.PosV(), info.ray.DirV(), rightDist);

			//레이에 시작점이 bbox내부에 있을경우
			if (leftIntersected)
				left->FindIntersectNotSort(info, leftDist);
			if (rightIntersected)
				right->FindIntersectNotSort(info, rightDist);
		}
	}
	void JBvhNode::FindContainNotSort(JAcceleratorContainInfo& info)const noexcept
	{
		if (type == J_BVH_NODE_TYPE::LEAF)
			info.resultObjVec.push_back(innerGameObject);
		else
		{
			bool leftRes = false;
			bool rightRes = false;
			if (left->type == J_BVH_NODE_TYPE::LEAF)
				leftRes = IsContain(info, left->innerGameObject->GetRenderItem()->GetOrientedBoundingBox());
			else
				leftRes = IsContain(info, left->bbox);

			if (right->type == J_BVH_NODE_TYPE::LEAF)
				rightRes = IsContain(info, right->innerGameObject->GetRenderItem()->GetOrientedBoundingBox());
			else
				rightRes = IsContain(info, right->bbox);

			if (leftRes)
				left->FindContainNotSort(info);
			if (rightRes)
				right->FindContainNotSort(info);
		}
	}
	void JBvhNode::BuildDebugNode(JAcceleratorVisualizeInterface* treeView)
	{
		if (type == J_BVH_NODE_TYPE::ROOT)
		{
			treeView->BuildNode(std::to_string(nodeNumber), nodeNumber, "Root");
			if (left != nullptr)
				left->BuildDebugNode(treeView);
			if (right != nullptr)
				right->BuildDebugNode(treeView);
		}
		else if (type == J_BVH_NODE_TYPE::NODE)
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
			treeView->BuildNode(std::to_string(nodeNumber), nodeNumber, "InnerGameObject: " + JCUtil::WstrToU8Str(innerGameObject->GetName()));
			if (IsLeftNode())
				treeView->BuildEndSplit();
			else
				treeView->BuildEndSplit();
		}
	}
}
