#include"JKdTreeNode.h" 
#include"../../../Object/Resource/Mesh/JMeshGeometry.h" 
#include"../../../Object/GameObject/JGameObject.h" 
#include"../../../Object/GameObject/JGameObjectCreator.h"
#include"../../../Object/Component/RenderItem/JRenderItem.h"
#include"../../../Object/Component/Transform/JTransform.h" 
#include"../../../Editor/EditTool/JEditorViewStructure.h"
#include"../../../Utility/JCommonUtility.h" 
#include"../../../Utility/JVectorExtend.h" 
#include"../../../Utility/JMathHelper.h"

//Test
//#include"../../../Object/Resource/JResourceManager.h"

using namespace DirectX;
namespace JinEngine
{
	namespace Core
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
					debugGameObject = JGCI::CreateDebugLineShape(parent, OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX_LINE, J_DEFAULT_MATERIAL::DEBUG_LINE_RED, false);
				else
					debugGameObject = JGCI::CreateDebugLineShape(parent, OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX_LINE, J_DEFAULT_MATERIAL::DEBUG_LINE_GREEN, false);

				const float outlineFactor = 0.01f;
				const BoundingBox rBBox = debugGameObject->GetRenderItem()->GetMesh()->GetBoundingBox();
				debugGameObject->GetTransform()->SetScale(XMFLOAT3(bbox.Extents.x / rBBox.Extents.x + outlineFactor,
					bbox.Extents.y / rBBox.Extents.y + outlineFactor,
					bbox.Extents.z / rBBox.Extents.z + outlineFactor));
				debugGameObject->GetTransform()->SetPosition(JMathHelper::Vector3Plus(bbox.Center, rBBox.Center));
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
			if (innerGameObject.size() > 0)
				SetVisible();
			innerGameObject.clear();
			parent = nullptr;
			left = nullptr;
			right = nullptr;
			DestroyDebugGameObject();
		}
		void JKdTreeNode::Culling(const JCullingFrustum& camFrustum, J_CULLING_FLAG flag)noexcept
		{
			J_CULLING_RESULT res = camFrustum.IsBoundingBoxIn(bbox, flag);
			if (res == J_CULLING_RESULT::CONTAIN)
				SetVisible();
			else if (res == J_CULLING_RESULT::DISJOINT)
				SetInVisible();
			else
			{
				if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
					SetVisible();
				else
				{
					left->Culling(camFrustum, flag);
					right->Culling(camFrustum, flag);
				}
			}
		}
		void JKdTreeNode::Culling(const DirectX::BoundingFrustum& camFrustum, const DirectX::FXMVECTOR camPos)noexcept
		{
			ContainmentType res = camFrustum.Contains(bbox);
			if (res == ContainmentType::CONTAINS)
				SetVisible();
			else if (res == ContainmentType::DISJOINT)
				SetInVisible();
			else
			{
				if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
				{
					if (bbox.Contains(camPos) == ContainmentType::DISJOINT)
						SetVisible();
					else
						SetInVisible();
				}
				else
				{
					left->Culling(camFrustum, camPos);
					right->Culling(camFrustum, camPos);
				}
			}
		}
		JUserPtr<JGameObject> JKdTreeNode::IntersectFirst(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir)const noexcept
		{
			if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
			{
				float minDist = FLT_MAX;
				int index = -1;
				const uint innerCount = (uint)innerGameObject.size();
				for (uint i = 0; i < innerCount; ++i)
				{
					float dist = 0;
					if (innerGameObject[i]->GetRenderItem()->GetBoundingBox().Intersects(ori, dir, dist))
					{
						if (minDist > dist)
						{
							minDist = dist;
							index = i;
						}
					}
				}
				if (index != -1)
					return innerGameObject[index];
				else
					return nullptr;
			}
			else
			{
				float leftDist = FLT_MAX;
				float rightDist = FLT_MAX;

				const bool leftRes = left->bbox.Intersects(ori, dir, leftDist);
				const bool rightRes = right->bbox.Intersects(ori, dir, rightDist);

				JUserPtr<JGameObject> res = nullptr;
				if (leftDist < rightDist)
				{
					if (leftRes)
						res = left->IntersectFirst(ori, dir);
					if (rightRes && res == nullptr)
						res = right->IntersectFirst(ori, dir);
				}
				else
				{
					if (rightRes)
						res = right->IntersectFirst(ori, dir);
					if (leftRes && res == nullptr)
						res = left->IntersectFirst(ori, dir);
				}
				return res;
			}
		}
		void JKdTreeNode::IntersectAscendingSort(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir, _Out_ std::vector<JUserPtr<JGameObject>>& res, std::vector<float>& distVec)const noexcept
		{
			if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
			{
				const uint innerCount = (uint)innerGameObject.size();
				for (uint i = 0; i < innerCount; ++i)
				{
					float dist = 0;
					if (innerGameObject[i]->GetRenderItem()->GetBoundingBox().Intersects(ori, dir, dist))
					{
						bool isHit = false;
						const uint distCount = (uint)distVec.size();
						for (uint j = 0; j < distCount; ++j)
						{
							if (dist < distVec[j])
							{
								isHit = true;
								JUserPtr<JGameObject> tempObj = res[j];
								res[j] = innerGameObject[i];
								res.push_back(tempObj);

								float tempFloat = distVec[j];
								distVec[j] = dist;
								distVec.push_back(tempFloat);
								break;
							}
						}
						if (!isHit)
						{
							res.push_back(innerGameObject[i]);
							distVec.push_back(dist);
						}
					}
				}
			}
			else
			{
				float leftDist = FLT_MAX;
				float rightDist = FLT_MAX;

				const bool leftRes = left->bbox.Intersects(ori, dir, leftDist);
				const bool rightRes = right->bbox.Intersects(ori, dir, rightDist);

				if (leftDist < rightDist)
				{
					if (leftRes)
						left->IntersectAscendingSort(ori, dir, res, distVec);
					if (rightRes)
						right->IntersectAscendingSort(ori, dir, res, distVec);
				}
				else
				{
					if (rightRes)
						right->IntersectAscendingSort(ori, dir, res, distVec);
					if (leftRes)
						left->IntersectAscendingSort(ori, dir, res, distVec);
				}
			}
		}
		void JKdTreeNode::IntersectDescendingSort(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir, _Out_ std::vector<JUserPtr<JGameObject>>& res, std::vector<float>& distVec)const noexcept
		{
			if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
			{
				const uint innerCount = (uint)innerGameObject.size();
				for (uint i = 0; i < innerCount; ++i)
				{
					float dist = 0;
					if (innerGameObject[i]->GetRenderItem()->GetBoundingBox().Intersects(ori, dir, dist))
					{
						bool isHit = false;
						const uint distCount = (uint)distVec.size();
						for (uint j = 0; j < distCount; ++j)
						{
							if (dist > distVec[j])
							{
								isHit = true;
								JUserPtr<JGameObject> tempObj = res[j];
								res[j] = innerGameObject[i];
								res.push_back(tempObj);

								float tempFloat = distVec[j];
								distVec[j] = dist;
								distVec.push_back(tempFloat);
								break;
							}
						}
						if (!isHit)
						{
							res.push_back(innerGameObject[i]);
							distVec.push_back(dist);
						}
					}
				}
			}
			else
			{
				float leftDist = FLT_MAX;
				float rightDist = FLT_MAX;

				const bool leftRes = left->bbox.Intersects(ori, dir, leftDist);
				const bool rightRes = right->bbox.Intersects(ori, dir, rightDist);

				if (leftDist > rightDist)
				{
					if (leftRes)
						left->IntersectDescendingSort(ori, dir, res, distVec);
					if (rightRes)
						right->IntersectDescendingSort(ori, dir, res, distVec);
				}
				else
				{
					if (rightRes)
						right->IntersectDescendingSort(ori, dir, res, distVec);
					if (leftRes)
						left->IntersectDescendingSort(ori, dir, res, distVec);
				}
			}
		}
		void JKdTreeNode::Intersect(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir, _Out_ std::vector<JUserPtr<JGameObject>>& res)const noexcept
		{
			if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
			{
				const uint innerCount = (uint)innerGameObject.size();
				for (uint i = 0; i < innerCount; ++i)
				{
					float dist = 0;
					if (innerGameObject[i]->GetRenderItem()->GetBoundingBox().Intersects(ori, dir, dist))
						res.push_back(innerGameObject[i]);
				}
			}
			else
			{
				float leftDist = FLT_MAX;
				float rightDist = FLT_MAX;

				const bool leftRes = left->bbox.Intersects(ori, dir, leftDist);
				const bool rightRes = right->bbox.Intersects(ori, dir, rightDist);

				if (leftRes)
					left->Intersect(ori, dir, res);
				if (rightRes)
					right->Intersect(ori, dir, res);
			}
		}
		void JKdTreeNode::OffCulling()noexcept
		{
			SetVisible();
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
		void JKdTreeNode::GetAlignLeafNode(const JVector3<float>& pos, std::vector<JKdTreeNode*>& alignNode, uint& index) noexcept
		{
			if (index >= alignNode.size())
				return;

			if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
				alignNode[index++] = this;
			else
			{
				float centerFactor = 0;
				if (splitType == J_KDTREE_NODE_SPLIT_AXIS::X)
					centerFactor = bbox.Center.x;
				else if (splitType == J_KDTREE_NODE_SPLIT_AXIS::Y)
					centerFactor = bbox.Center.y;
				else
					centerFactor = bbox.Center.z;

				if (pos[(int)splitType] > centerFactor)
				{
					right->GetAlignLeafNode(pos, alignNode, index);
					left->GetAlignLeafNode(pos, alignNode, index);
				}
				else
				{
					left->GetAlignLeafNode(pos, alignNode, index);
					right->GetAlignLeafNode(pos, alignNode, index);
				}
			}
		}
		void JKdTreeNode::GetAlignLeafNode(const DirectX::BoundingFrustum& camFrustum, std::vector<JKdTreeNode*>& alignNode, uint& index)noexcept
		{
			if (index >= alignNode.size())
				return;

			if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
				alignNode[index++] = this;
			else
			{
				if (camFrustum.Contains(bbox) == ContainmentType::DISJOINT)
					return;

				bool isRight = false;
				if (splitType == J_KDTREE_NODE_SPLIT_AXIS::X)
					isRight = camFrustum.Origin.x > bbox.Center.x;
				else if (splitType == J_KDTREE_NODE_SPLIT_AXIS::Y)
					isRight = camFrustum.Origin.y > bbox.Center.y;
				else
					isRight = camFrustum.Origin.z > bbox.Center.z;

				if (isRight)
				{
					right->GetAlignLeafNode(camFrustum, alignNode, index);
					left->GetAlignLeafNode(camFrustum, alignNode, index);
				}
				else
				{
					left->GetAlignLeafNode(camFrustum, alignNode, index);
					right->GetAlignLeafNode(camFrustum, alignNode, index);
				}
			}
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
				left->parent = this;
		}
		void JKdTreeNode::SetRightNode(JKdTreeNode* newRightNode)noexcept
		{
			if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
				return;

			right = newRightNode;
			if (right != nullptr)
				right->parent = this;
		}
		bool JKdTreeNode::HasGameObject(const size_t guid)noexcept
		{
			return JCUtil::GetTypeIndex(innerGameObject, guid) != JCUtil::searchFail;
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
		void JKdTreeNode::StuffInnerGameObject(std::vector<JUserPtr<JGameObject>>& objList, uint& listIndex)
		{
			const uint innerCount = (uint)innerGameObject.size();
			for (uint i = 0; i < innerCount; ++i)
			{
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
		void JKdTreeNode::SetVisible()noexcept
		{
			if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
			{
				const uint innerCount = (uint)innerGameObject.size();
				for (uint i = 0; i < innerCount; ++i)
				{
					JUserPtr<JRenderItem> rItem = innerGameObject[i]->GetRenderItem();
					if ((rItem->GetSpaceSpatialMask() & SPACE_SPATIAL_ALLOW_CULLING) > 0)
						rItem->SetRenderVisibility(J_RENDER_VISIBILITY::VISIBLE);
				}
			}
			else
			{
				left->SetVisible();
				right->SetVisible();
			}
		}
		void JKdTreeNode::SetInVisible()noexcept
		{
			if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
			{
				const uint innerCount = (uint)innerGameObject.size();
				for (uint i = 0; i < innerCount; ++i)
				{
					JUserPtr<JRenderItem> rItem = innerGameObject[i]->GetRenderItem();
					if ((rItem->GetSpaceSpatialMask() & SPACE_SPATIAL_ALLOW_CULLING) > 0)
						rItem->SetRenderVisibility(J_RENDER_VISIBILITY::INVISIBLE);
				}
			}
			else
			{
				left->SetInVisible();
				right->SetInVisible();
			}
		}
		JKdTreeNode* JKdTreeNode::FindRightLeafNode() noexcept
		{
			if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
				return this;
			else
				return right->FindRightLeafNode();
		}
		void JKdTreeNode::BuildDebugNode(Editor::JEditorBinaryTreeView& treeView)
		{
			if (nodeType == J_KDTREE_NODE_TYPE::ROOT)
			{
				treeView.BuildNode(std::to_string(nodeNumber), nodeNumber, treeView.GetDefaultGroupGuid(), "Root");
				if (left != nullptr)
					left->BuildDebugNode(treeView);
				if (right != nullptr)
					right->BuildDebugNode(treeView);
			}
			else if (nodeType == J_KDTREE_NODE_TYPE::NODE)
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
				std::wstring info = L"InnerGameObject: \n";
				for (const auto& data : innerGameObject)
					info += data->GetName() + L"\n";
				treeView.BuildNode(std::to_string(nodeNumber), nodeNumber, treeView.GetDefaultGroupGuid(), JCUtil::WstrToU8Str(info));
				if (IsLeftNode())
					treeView.BuildEndSplit();
				else
					treeView.BuildEndSplit();
			}
		}
		//void JKdTreeNode::AddLog(const std::string& newNodeLog)noexcept
		//{
		//	nodeLog += newNodeLog;
		//}
	}
}