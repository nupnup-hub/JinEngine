#include"JBvhNode.h"
#include"../../../Object/Resource/Scene/JScene.h"
#include"../../../Object/GameObject/JGameObject.h" 
#include"../../../Object/GameObject/JGameObjectFactoryUtility.h"
#include"../../../Object/Component/RenderItem/JRenderItem.h"
#include"../../../Object/Component/Transform/JTransform.h" 

using namespace DirectX;
namespace JinEngine
{
	namespace Core
	{
		JBvhNode::JBvhNode(const uint nodeNumber, const J_BVH_NODE_TYPE type, const DirectX::BoundingBox& bv, JBvhNode* parent, JGameObject* innerGameObject, bool isLeftNode)
			:nodeNumber(nodeNumber), type(type), bv(bv), parent(parent), innerGameObject(innerGameObject)
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
		void JBvhNode::CreateDebugGameObject(JGameObject* parent, bool onlyLeafNode)noexcept
		{
			if (debugGameObject == nullptr)
			{
				if (innerGameObject == nullptr && onlyLeafNode)
					return;

				if (innerGameObject == nullptr)
					debugGameObject = JGFU::CreateDebugGameObject(*parent, OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX, J_DEFAULT_MATERIAL::DEBUG_LINE_RED);
				else
					debugGameObject = JGFU::CreateDebugGameObject(*parent, OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX, J_DEFAULT_MATERIAL::DEBUG_LINE_GREEN);

				debugGameObject->GetTransform()->SetScale(XMFLOAT3(bv.Extents.x * 2, bv.Extents.y * 2, bv.Extents.z * 2));
				debugGameObject->GetTransform()->SetPosition(bv.Center);
			}
		}
		void JBvhNode::EraseDebugGameObject()noexcept
		{
			if (debugGameObject != nullptr)
			{
				JGameObject::EraseGameObject(debugGameObject);
				debugGameObject = nullptr;
			}
		}
		void JBvhNode::Clear()noexcept
		{
			EraseDebugGameObject();
			parent = nullptr;
			left = nullptr;
			right = nullptr;
			innerGameObject = nullptr;
		}
		void JBvhNode::Culling(const JCullingFrustum& camFrustum, J_CULLING_FLAG flag)noexcept
		{
			J_CULLING_RESULT res = camFrustum.IsBoundingBoxIn(bv, flag);
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
		void JBvhNode::Culling(const DirectX::BoundingFrustum& camFrustum)noexcept
		{
			ContainmentType res = camFrustum.Contains(bv);
			if (res == ContainmentType::CONTAINS)
				SetVisible();
			else if (res == ContainmentType::DISJOINT)
				SetInVisible();
			else
			{
				if (type == J_BVH_NODE_TYPE::LEAF)
					SetVisible();
				else
				{
					left->Culling(camFrustum);
					right->Culling(camFrustum);
				}
			}
		}
		void JBvhNode::UpdateInnerGameObject()noexcept
		{
			if (type == J_BVH_NODE_TYPE::LEAF)
			{
				bv = innerGameObject->GetRenderItem()->GetBoundingBox();
				if (debugGameObject != nullptr)
				{
					debugGameObject->GetTransform()->SetScale(XMFLOAT3(bv.Extents.x * 2, bv.Extents.y * 2, bv.Extents.z * 2));
					debugGameObject->GetTransform()->SetPosition(bv.Center);
				}
			}
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
			return bv.Contains(boundBox) == ContainmentType::CONTAINS;
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
			return bv;
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
			ContainmentType res = bv.Contains(boundBox);
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
			ContainmentType res = bv.Contains(boundBox);
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
		JGameObject* JBvhNode::GetInnerGameObject()noexcept
		{
			return innerGameObject;
		}
		JGameObject* JBvhNode::GetDebugGameObject()noexcept
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
		}
		void JBvhNode::SetVisible()noexcept
		{
			if (type == J_BVH_NODE_TYPE::LEAF)
				innerGameObject->GetRenderItem()->SetRenderVisibility(J_RENDER_VISIBILITY::VISIBLE);
			else
			{
				left->SetVisible();
				right->SetVisible();
			}
		}
		void JBvhNode::SetInVisible()noexcept
		{
			if (type == J_BVH_NODE_TYPE::LEAF)
				innerGameObject->GetRenderItem()->SetRenderVisibility(J_RENDER_VISIBILITY::INVISIBLE);
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
	}
}