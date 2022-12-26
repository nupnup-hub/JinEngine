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
					debugGameObject = JGFU::CreateDebugGameObject(*parent, OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX_LINE, J_DEFAULT_MATERIAL::DEBUG_LINE_RED);
				else
					debugGameObject = JGFU::CreateDebugGameObject(*parent, OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX_LINE, J_DEFAULT_MATERIAL::DEBUG_LINE_GREEN);

				const float outlineFactor = 0.01f;
				const BoundingBox rBBox = debugGameObject->GetRenderItem()->GetBoundingBox();
				debugGameObject->GetTransform()->SetScale(XMFLOAT3(bbox.Extents.x / rBBox.Extents.x + outlineFactor,
					bbox.Extents.y / rBBox.Extents.y + outlineFactor,
					bbox.Extents.z / rBBox.Extents.z + outlineFactor)); 
				debugGameObject->GetTransform()->SetPosition(JMathHelper::Vector3Plus(bbox.Center, rBBox.Center));
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
		void JBvhNode::UpdateInnerGameObject()noexcept
		{
			if (type == J_BVH_NODE_TYPE::LEAF)
			{
				bbox = innerGameObject->GetRenderItem()->GetBoundingBox();
				if (debugGameObject != nullptr)
				{			 
					debugGameObject->GetTransform()->SetScale(XMFLOAT3(bbox.Extents.x * 2, bbox.Extents.y * 2, bbox.Extents.z * 2));
					debugGameObject->GetTransform()->SetPosition(bbox.Center);
				}
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