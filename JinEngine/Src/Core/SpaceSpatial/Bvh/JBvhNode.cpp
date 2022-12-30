#include"JBvhNode.h" 
#include"../../../Object/GameObject/JGameObject.h" 
#include"../../../Object/GameObject/JGameObjectFactoryUtility.h"
#include"../../../Object/Component/RenderItem/JRenderItem.h"
#include"../../../Object/Component/Transform/JTransform.h" 
#include"../../../Editor/Utility/JEditorBinaryTreeView.h"
#include"../../../Utility/JCommonUtility.h"

using namespace DirectX;
namespace JinEngine
{
	namespace Core
	{
		JBvhNode::JBvhNode(const uint nodeNumber, const J_BVH_NODE_TYPE type, const DirectX::BoundingBox& bbox, JBvhNode* parent, JGameObject* innerGameObject, bool isLeftNode)
			:nodeNumber(nodeNumber), type(type), bbox(bbox), parent(parent), innerGameObject(innerGameObject)
		{
			if (parent != nullptr)
			{
				if (isLeftNode)
					parent->left = this;
				else
					parent->right = this;
			}
			if (JBvhNode::innerGameObject != nullptr)
				JBvhNode::innerGameObject->GetRenderItem()->SetRenderVisibility(J_RENDER_VISIBILITY::VISIBLE);
		}
		JBvhNode::~JBvhNode() {}
		void JBvhNode::CreateDebugGameObject(JGameObject* parent, bool onlyLeafNode)noexcept
		{
			if (debugGameObject == nullptr)
			{
				if (type != J_BVH_NODE_TYPE::LEAF && onlyLeafNode)
					return;

				if (type != J_BVH_NODE_TYPE::LEAF)
					debugGameObject = JGFU::CreateDebugLineShape(*parent, OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX_LINE, J_DEFAULT_MATERIAL::DEBUG_LINE_RED);
				else
					debugGameObject = JGFU::CreateDebugLineShape(*parent, OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX_LINE, J_DEFAULT_MATERIAL::DEBUG_LINE_GREEN);

				SetDebugObjectTransform();
			}
		}
		void JBvhNode::DestroyDebugGameObject()noexcept
		{
			if (debugGameObject != nullptr)
			{
				JGameObject::BeginDestroy(debugGameObject);
				debugGameObject = nullptr;
			}
		}
		void JBvhNode::Clear()noexcept
		{
			if (innerGameObject != nullptr)
				SetVisible();
			innerGameObject = nullptr;
			parent = nullptr;
			left = nullptr;
			right = nullptr;
			DestroyDebugGameObject();
		}
		void JBvhNode::Culling(const JCullingFrustum& camFrustum, J_CULLING_FLAG flag)noexcept
		{
			J_CULLING_RESULT res = camFrustum.IsBoundingBoxIn(bbox, flag);
			if (res == J_CULLING_RESULT::CONTAIN)
				SetVisible();
			else if (res == J_CULLING_RESULT::DISJOINT)
				SetInVisible();
			else
			{
				if (type == J_BVH_NODE_TYPE::LEAF)
					SetVisible();
				else
				{
					left->Culling(camFrustum, flag);
					right->Culling(camFrustum, flag);
				}
			}
		}
		void JBvhNode::Culling(const DirectX::BoundingFrustum& camFrustum, const DirectX::BoundingFrustum& nearFrustum)noexcept
		{
			ContainmentType res = camFrustum.Contains(bbox);
			if (res == ContainmentType::CONTAINS)
				SetVisible();
			else if (res == ContainmentType::DISJOINT)
				SetInVisible();
			else
			{ 
				if (type == J_BVH_NODE_TYPE::LEAF)
				{
					if (nearFrustum.Contains(bbox) == ContainmentType::DISJOINT)
						SetVisible();
					else
						SetInVisible();
				}
				else
				{
					left->Culling(camFrustum, nearFrustum);
					right->Culling(camFrustum, nearFrustum);
				}
			}
		}
		JGameObject* JBvhNode::IntersectFirst(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir)const noexcept
		{
			if (type == J_BVH_NODE_TYPE::LEAF)
				return innerGameObject;
			else
			{
				float leftDist = FLT_MAX;
				float rightDist = FLT_MAX;

				const bool leftRes = left->bbox.Intersects(ori, dir, leftDist);
				const bool rightRes = right->bbox.Intersects(ori, dir, rightDist);

				JGameObject* res = nullptr;
				if (leftDist < rightDist)
				{
					if (leftRes)
						res = left->IntersectFirst(ori, dir);
					if (rightRes && !res)
						res = right->IntersectFirst(ori, dir);
				}
				else
				{
					if (rightRes)
						res = right->IntersectFirst(ori, dir);
					if (leftRes && !res)
						res = left->IntersectFirst(ori, dir);
				}
				return res;
			}
		}
		void JBvhNode::IntersectAscendingSort(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir, _Out_ std::vector<JGameObject*>& res)const noexcept
		{
			if (type == J_BVH_NODE_TYPE::LEAF)
				res.push_back(innerGameObject);
			else
			{
				float leftDist = FLT_MAX;
				float rightDist = FLT_MAX;

				const bool leftRes = left->bbox.Intersects(ori, dir, leftDist);
				const bool rightRes = right->bbox.Intersects(ori, dir, rightDist);

				if (leftDist < rightDist)
				{
					if (leftRes)
						left->IntersectAscendingSort(ori, dir, res);
					if (rightRes)
						right->IntersectAscendingSort(ori, dir, res);
				}
				else
				{
					if (rightRes)
						right->IntersectAscendingSort(ori, dir, res);
					if (leftRes)
						left->IntersectAscendingSort(ori, dir, res);
				}
			}
		}
		void JBvhNode::IntersectDescendingSort(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir, _Out_ std::vector<JGameObject*>& res)const noexcept
		{
			if (type == J_BVH_NODE_TYPE::LEAF)
				res.push_back(innerGameObject);
			else
			{
				float leftDist = FLT_MAX;
				float rightDist = FLT_MAX;

				const bool leftRes = left->bbox.Intersects(ori, dir, leftDist);
				const bool rightRes = right->bbox.Intersects(ori, dir, rightDist);

				if (leftDist > rightDist)
				{
					if (leftRes)
						left->IntersectAscendingSort(ori, dir, res);
					if (rightRes)
						right->IntersectAscendingSort(ori, dir, res);
				}
				else
				{
					if (rightRes)
						right->IntersectAscendingSort(ori, dir, res);
					if (leftRes)
						left->IntersectAscendingSort(ori, dir, res);
				}
			}
		}
		void JBvhNode::Intersect(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir, _Out_ std::vector<JGameObject*>& res)const noexcept
		{
			if (type == J_BVH_NODE_TYPE::LEAF)
				res.push_back(innerGameObject);
			else
			{
				float leftDist = FLT_MAX;
				float rightDist = FLT_MAX;

				const bool leftRes = left->bbox.Intersects(ori, dir, leftDist);
				const bool rightRes = right->bbox.Intersects(ori, dir, rightDist);

				if (leftRes)
					left->IntersectAscendingSort(ori, dir, res);
				if (rightRes)
					right->IntersectAscendingSort(ori, dir, res);
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
		void JBvhNode::OffCulling()noexcept
		{
			SetVisible();
		}
		bool JBvhNode::IsLeftNode()const noexcept
		{
			if (parent != nullptr)
				return parent->left->nodeNumber == nodeNumber;
			else
				return false;
		}
		bool JBvhNode::IsContain(const DirectX::BoundingBox& boundBox)const noexcept
		{
			return bbox.Contains(boundBox) == ContainmentType::CONTAINS;
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
		JGameObject* JBvhNode::GetInnerGameObject()const noexcept
		{
			return innerGameObject;
		}
		JGameObject* JBvhNode::GetDebugGameObject()const noexcept
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
		void JBvhNode::SetInnerGameObject(JGameObject* newInnerGameObject)noexcept
		{
			innerGameObject = newInnerGameObject;
			if (innerGameObject != nullptr)
				innerGameObject->GetRenderItem()->SetRenderVisibility(J_RENDER_VISIBILITY::VISIBLE);
		}
		void JBvhNode::SetVisible()noexcept
		{
			if (type == J_BVH_NODE_TYPE::LEAF)
			{
				JRenderItem* rItem = innerGameObject->GetRenderItem();
				if ((rItem->GetSpaceSpatialMask() & SPACE_SPATIAL_ALLOW_CULLING) > 0)
					rItem->SetRenderVisibility(J_RENDER_VISIBILITY::VISIBLE);
			}
			else
			{
				left->SetVisible();
				right->SetVisible();
			}
		}
		void JBvhNode::SetInVisible()noexcept
		{
			if (type == J_BVH_NODE_TYPE::LEAF)
			{
				JRenderItem* rItem = innerGameObject->GetRenderItem();
				if ((rItem->GetSpaceSpatialMask() & SPACE_SPATIAL_ALLOW_CULLING) > 0)
					rItem->SetRenderVisibility(J_RENDER_VISIBILITY::INVISIBLE);
			}
			else
			{
				left->SetInVisible();
				right->SetInVisible();
			}
		}
		JBvhNode* JBvhNode::FindRightLeafNode()noexcept
		{
			if (type == J_BVH_NODE_TYPE::LEAF)
				return this;
			else
				return right->FindRightLeafNode();
		}
		void JBvhNode::SetDebugObjectTransform()noexcept
		{
			if (debugGameObject == nullptr)
				return;

			static constexpr float outlineFactor = 0.01f;
			JTransform* transform = debugGameObject->GetTransform(); 

			const BoundingBox debugBox = debugGameObject->GetRenderItem()->GetMesh()->GetBoundingBox(); 
			transform->SetScale(XMFLOAT3(bbox.Extents.x / debugBox.Extents.x + outlineFactor,
				bbox.Extents.y / debugBox.Extents.y + outlineFactor,
				bbox.Extents.z / debugBox.Extents.z + outlineFactor));
			transform->SetPosition(JMathHelper::Vector3Minus(bbox.Center, debugBox.Center));
		}
		void JBvhNode::BuildDebugNode(Editor::JEditorBinaryTreeView& treeView)
		{
			if (type == J_BVH_NODE_TYPE::ROOT)
			{
				treeView.BuildNode(std::to_string(nodeNumber));
				if (left != nullptr)
					left->BuildDebugNode(treeView);
				if (right != nullptr)
					right->BuildDebugNode(treeView);
			}
			else if (type == J_BVH_NODE_TYPE::NODE)
			{
				treeView.BuildNode(std::to_string(nodeNumber));
				left->BuildDebugNode(treeView);
				right->BuildDebugNode(treeView);
				if (IsLeftNode())
					treeView.EndLeft();
				else
					treeView.EndRight();
			}
			else
			{
				treeView.BuildNode(std::to_string(nodeNumber), JCUtil::WstrToU8Str(innerGameObject->GetName()));
				if (IsLeftNode())
					treeView.EndLeft();
				else
					treeView.EndRight();
			}
		}
	}
}