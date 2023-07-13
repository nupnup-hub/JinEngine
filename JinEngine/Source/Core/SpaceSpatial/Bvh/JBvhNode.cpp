#include"JBvhNode.h" 
#include"../../../Object/Resource/Mesh/JMeshGeometry.h" 
#include"../../../Object/GameObject/JGameObject.h" 
#include"../../../Object/GameObject/JGameObjectCreator.h"
#include"../../../Object/Component/RenderItem/JRenderItem.h"
#include"../../../Object/Component/Transform/JTransform.h" 
#include"../../../Editor/EditTool/JEditorViewStructure.h"
#include"../../../Utility/JCommonUtility.h"
#include"../../../Utility/JMathHelper.h"

using namespace DirectX;
namespace JinEngine
{
	namespace Core
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
		void JBvhNode::Culling(Graphic::JCullingUserInterface& cullUser, const JCullingFrustum& camFrustum, J_CULLING_FLAG flag)noexcept
		{
			J_CULLING_RESULT res = camFrustum.IsBoundingBoxIn(bbox, flag);
			if (res == J_CULLING_RESULT::CONTAIN)
				SetVisible(cullUser);
			else if (res == J_CULLING_RESULT::DISJOINT)
				SetInVisible(cullUser);
			else
			{
				if (type == J_BVH_NODE_TYPE::LEAF)
					SetVisible(cullUser);
				else
				{
					left->Culling(cullUser, camFrustum, flag);
					right->Culling(cullUser, camFrustum, flag);
				}
			}
		}
		void JBvhNode::Culling(Graphic::JCullingUserInterface& cullUser, const DirectX::BoundingFrustum& camFrustum, const DirectX::BoundingFrustum& cullingFrustum)noexcept
		{
			//frustum.Contain이 box.Contain보다 빠르다
			if (type == J_BVH_NODE_TYPE::LEAF)
			{
				DirectX::BoundingOrientedBox oriBox = innerGameObject->GetRenderItem()->GetOrientedBoundingBox();
				if (camFrustum.Contains(oriBox) == ContainmentType::DISJOINT)
					SetInVisible(cullUser);
				else  
					SetVisible(cullUser, cullingFrustum, cullingFrustum.Contains(oriBox) != DirectX::DISJOINT);
			}
			else
			{
				ContainmentType res = camFrustum.Contains(bbox);
				if (res == ContainmentType::CONTAINS)
					SetVisible(cullUser, cullingFrustum, cullingFrustum.Contains(bbox) != DirectX::DISJOINT);
				else if (res == ContainmentType::DISJOINT)
					SetInVisible(cullUser);
				else
				{
					left->Culling(cullUser, camFrustum, cullingFrustum);
					right->Culling(cullUser, camFrustum, cullingFrustum);
				}
			}
		} 
		JUserPtr<JGameObject> JBvhNode::IntersectFirst(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir, const bool allowContainRayPos)const noexcept
		{
			if (type == J_BVH_NODE_TYPE::LEAF)
			{
				if (!allowContainRayPos && innerGameObject->GetRenderItem()->GetOrientedBoundingBox().Contains(ori) == DirectX::CONTAINS)
					return nullptr;
				else
					return innerGameObject;
			}
			else
			{
				float leftDist = FLT_MAX;
				float rightDist = FLT_MAX;

				bool leftRes = false;
				bool rightRes = false;

				if (left->type == J_BVH_NODE_TYPE::LEAF)
					leftRes = left->innerGameObject->GetRenderItem()->GetOrientedBoundingBox().Intersects(ori, dir, leftDist);
				else
					leftRes = left->bbox.Intersects(ori, dir, leftDist);

				if(right->type == J_BVH_NODE_TYPE::LEAF)
					rightRes = right->innerGameObject->GetRenderItem()->GetOrientedBoundingBox().Intersects(ori, dir, rightDist);
				else
					rightRes = right->bbox.Intersects(ori, dir, rightDist);

				JUserPtr<JGameObject> res = nullptr;
				if (leftDist < rightDist)
				{
					if (leftRes)
						res = left->IntersectFirst(ori, dir, allowContainRayPos);
					if (rightRes && res == nullptr)
						res = right->IntersectFirst(ori, dir, allowContainRayPos);
				}
				else
				{
					if (rightRes)
						res = right->IntersectFirst(ori, dir, allowContainRayPos);
					if (leftRes && res == nullptr)
						res = left->IntersectFirst(ori, dir, allowContainRayPos);
				}
				return res;
			}
		}
		void JBvhNode::IntersectAscendingSort(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir, _Out_ std::vector<JUserPtr<JGameObject>>& res)const noexcept
		{
			if (type == J_BVH_NODE_TYPE::LEAF)
				res.push_back(innerGameObject);
			else
			{
				float leftDist = FLT_MAX;
				float rightDist = FLT_MAX;

				bool leftRes = false;
				bool rightRes = false;
				if (left->type == J_BVH_NODE_TYPE::LEAF)
					leftRes = left->innerGameObject->GetRenderItem()->GetOrientedBoundingBox().Intersects(ori, dir, leftDist);
				else
					leftRes = left->bbox.Intersects(ori, dir, leftDist);

				if (right->type == J_BVH_NODE_TYPE::LEAF)
					rightRes = right->innerGameObject->GetRenderItem()->GetOrientedBoundingBox().Intersects(ori, dir, rightDist);
				else
					rightRes = right->bbox.Intersects(ori, dir, rightDist);

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
		void JBvhNode::IntersectDescendingSort(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir, _Out_ std::vector<JUserPtr<JGameObject>>& res)const noexcept
		{
			IntersectAscendingSort(ori, dir, res);
			std::reverse(res.begin(), res.end());
		}
		void JBvhNode::Intersect(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir, _Out_ std::vector<JUserPtr<JGameObject>>& res)const noexcept
		{
			IntersectAscendingSort(ori, dir, res);
		}
		void JBvhNode::UpdateInnerGameObject()noexcept
		{
			if (type == J_BVH_NODE_TYPE::LEAF)
			{
				bbox = innerGameObject->GetRenderItem()->GetBoundingBox();
				SetDebugObjectTransform();
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
		void JBvhNode::SetVisible(Graphic::JCullingUserInterface& cullUser)noexcept
		{
			if (type == J_BVH_NODE_TYPE::LEAF)
			{
				JUserPtr<JRenderItem> rItem = innerGameObject->GetRenderItem();
				if ((rItem->GetSpaceSpatialMask() & SPACE_SPATIAL_ALLOW_CULLING) > 0)
					OffCulling(cullUser, rItem);
			}
			else
			{
				left->SetVisible(cullUser);
				right->SetVisible(cullUser);
			}
		}
		void JBvhNode::SetVisible(Graphic::JCullingUserInterface& cullUser, const DirectX::BoundingFrustum& cullingFrustum, const bool camInParentBBox)noexcept
		{
			if (type == J_BVH_NODE_TYPE::LEAF)
			{
				JUserPtr<JRenderItem> rItem = innerGameObject->GetRenderItem();
				if ((rItem->GetSpaceSpatialMask() & SPACE_SPATIAL_ALLOW_CULLING) > 0)
				{
					if (camInParentBBox && cullingFrustum.Contains(rItem->GetOrientedBoundingBox()) != DirectX::DISJOINT)
						SetCulling(cullUser, rItem);
					else
						OffCulling(cullUser, rItem);
				}
			}
			else
			{
				if (camInParentBBox)
				{
					left->SetVisible(cullUser, cullingFrustum, cullingFrustum.Contains(bbox) != DirectX::DISJOINT);
					right->SetVisible(cullUser, cullingFrustum, cullingFrustum.Contains(bbox) != DirectX::DISJOINT);
				}
				else
				{
					left->SetVisible(cullUser, cullingFrustum, camInParentBBox);
					right->SetVisible(cullUser, cullingFrustum, camInParentBBox);
				}
			}
		}
		void JBvhNode::SetInVisible(Graphic::JCullingUserInterface& cullUser)noexcept
		{
			if (type == J_BVH_NODE_TYPE::LEAF)
			{
				JUserPtr<JRenderItem> rItem = innerGameObject->GetRenderItem();
				if ((rItem->GetSpaceSpatialMask() & SPACE_SPATIAL_ALLOW_CULLING) > 0)
					SetCulling(cullUser, rItem);
			}
			else
			{
				left->SetInVisible(cullUser);
				right->SetInVisible(cullUser);
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
			JUserPtr<JTransform> transform = debugGameObject->GetTransform();
			const BoundingBox debugBox = debugGameObject->GetRenderItem()->GetMesh()->GetBoundingBox();

			if (type == J_BVH_NODE_TYPE::LEAF)
			{
				const BoundingOrientedBox debugOriented = innerGameObject->GetRenderItem()->GetOrientedBoundingBox();
			//	XMMATRIX matWorld = XMMatrixRotationQuaternion(XMLoadFloat4(&debugOriented.Orientation));
				//XMMATRIX matScale = XMMatrixScaling(debugOriented.Extents.x, debugOriented.Extents.y, debugOriented.Extents.z);
				//matWorld = XMMatrixMultiply(matScale, matWorld);
				//XMVECTOR position = XMLoadFloat3(&debugOriented.Center);
				//matWorld.r[3] = XMVectorSelect(matWorld.r[3], position, g_XMSelect1110);

				transform->SetScale(XMFLOAT3(debugOriented.Extents.x / debugBox.Extents.x + outlineFactor,
					debugOriented.Extents.y / debugBox.Extents.y + outlineFactor,
					debugOriented.Extents.z / debugBox.Extents.z + outlineFactor));
				transform->SetRotation(debugOriented.Orientation);
				transform->SetPosition(debugOriented.Center);
			}
			else
			{
				//Bounding Box는 회전에 상관없이 일정한 모양을 유지하므로
				//Object에 회전에 맞는 DebugBox는 생성할 수 없다.
				//BoundingOriented를 사용하면 회전에 맞는 bbox를 생성할수있다
				transform->SetScale(XMFLOAT3(bbox.Extents.x / debugBox.Extents.x + outlineFactor,
					bbox.Extents.y / debugBox.Extents.y + outlineFactor,
					bbox.Extents.z / debugBox.Extents.z + outlineFactor));
				transform->SetPosition(JMathHelper::Vector3Minus(bbox.Center, debugBox.Center));
			}
		}
		void JBvhNode::BuildDebugNode(Editor::JEditorBinaryTreeView& treeView)
		{
			if (type == J_BVH_NODE_TYPE::ROOT)
			{
				treeView.BuildNode(std::to_string(nodeNumber), nodeNumber, treeView.GetDefaultGroupGuid(), "Root");
				if (left != nullptr)
					left->BuildDebugNode(treeView);
				if (right != nullptr)
					right->BuildDebugNode(treeView);
			}
			else if (type == J_BVH_NODE_TYPE::NODE)
			{
				treeView.BuildNode(std::to_string(nodeNumber), nodeNumber);
				left->BuildDebugNode(treeView);
				right->BuildDebugNode(treeView);
				if (IsLeftNode())
					treeView.BuildEndSplit();
				else
					treeView.BuildEndSplit();
			}
			else
			{
				treeView.BuildNode(std::to_string(nodeNumber),
					nodeNumber,
					treeView.GetDefaultGroupGuid(),
					"InnerGameObject: " + JCUtil::WstrToU8Str(innerGameObject->GetName()));
				if (IsLeftNode())
					treeView.BuildEndSplit();
				else
					treeView.BuildEndSplit();
			}
		}
	}
}