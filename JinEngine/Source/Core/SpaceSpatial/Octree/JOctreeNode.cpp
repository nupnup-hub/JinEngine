#include"JOctreeNode.h"  
#include"../../../Graphic/Culling/JCullingInterface.h"
#include"../../../Object/Resource/Mesh/JMeshGeometry.h" 
#include"../../../Object/GameObject/JGameObject.h"
#include"../../../Object/GameObject/JGameObjectCreator.h"
#include"../../../Object/Component/RenderItem/JRenderItem.h"
#include"../../../Object/Component/RenderItem/JRenderItemPrivate.h"
#include"../../../Object/Component/Transform/JTransform.h"
#include"../../../Utility/JMathHelper.h"

namespace JinEngine
{
	using namespace DirectX;
	namespace Core
	{ 
		JOctreeNode::JOctreeNode(const DirectX::BoundingBox& boundingBox, bool isLooseOctree, JOctreeNode* parentNode)
			:boundingBox(boundingBox), parentNode(parentNode)
		{
			if (parentNode != nullptr)
			{
				for (uint i = 0; i < childCount; ++i)
				{
					if (parentNode->childrenNode[i] == nullptr)
						parentNode->childrenNode[i] = this;
				}
			}
		}
		JOctreeNode::~JOctreeNode() {}
		void JOctreeNode::CreateDebugGameObject(JUserPtr<JGameObject> parent, bool onlyLeafNod)noexcept
		{
			if (debugGameObject == nullptr)
			{
				if (innerGameObject.size() == 0 && onlyLeafNod)
					return;

				if (innerGameObject.size() == 0)
					debugGameObject =JGCI::CreateDebugLineShape(parent, OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::BOUNDING_BOX_LINE, J_DEFAULT_MATERIAL::DEBUG_LINE_RED, false);
				else
					debugGameObject =JGCI::CreateDebugLineShape(parent, OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::BOUNDING_BOX_LINE, J_DEFAULT_MATERIAL::DEBUG_LINE_GREEN, false);
				
				const float outlineFactor = 0.01f;
				const BoundingBox rBBox = debugGameObject->GetRenderItem()->GetMesh()->GetBoundingBox();
				debugGameObject->GetTransform()->SetScale(XMFLOAT3(boundingBox.Extents.x / rBBox.Extents.x + outlineFactor,
					boundingBox.Extents.y / rBBox.Extents.y + outlineFactor,
					boundingBox.Extents.z / rBBox.Extents.z + outlineFactor));
				debugGameObject->GetTransform()->SetPosition(JMathHelper::Vector3Plus(boundingBox.Center, rBBox.Center));
			}
		}
		void JOctreeNode::DestroyDebugGameObject()noexcept
		{
			if (debugGameObject != nullptr)
			{
				JGameObject::BeginDestroy(debugGameObject.Get()); 
				debugGameObject = nullptr;
			}
		}
		void JOctreeNode::Clear()
		{
			const uint innerCount = (uint)innerGameObject.size();
			for (uint i = 0; i < innerCount; ++i)
				innerGameObject[i] = nullptr;

			innerGameObject.clear();
			DestroyDebugGameObject();

			for (uint i = 0; i < childCount; ++i)
			{
				if(childrenNode[i] != nullptr)
					childrenNode[i]->Clear();
				childrenNode[i] = nullptr;
			}
		}
		void JOctreeNode::Culling(Graphic::JCullingUserInterface& cullUser, const JCullingFrustum& camFrustum, J_CULLING_FLAG flag)noexcept
		{
			J_CULLING_RESULT res = camFrustum.IsBoundingBoxIn(boundingBox, flag);
			if (res == J_CULLING_RESULT::DISJOINT)
				SetInVisible(cullUser);
			else if (res == J_CULLING_RESULT::CONTAIN)
				SetVisible(cullUser, camFrustum, flag);
			else
			{			
				if (IsLeafNode())
					CullingInnerObject(cullUser, camFrustum, flag);
				else
				{
					for (uint i = 0; i < childCount; ++i)
						childrenNode[i]->Culling(cullUser, camFrustum, flag);
				}
			}
		}
		void JOctreeNode::Culling(Graphic::JCullingUserInterface& cullUser, const DirectX::BoundingFrustum& camFrustum, const DirectX::BoundingFrustum& cullingFrustum)noexcept
		{
			ContainmentType res = camFrustum.Contains(boundingBox);
			if (res == ContainmentType::DISJOINT)
				SetInVisible(cullUser);
			else if (res == ContainmentType::CONTAINS)
				SetVisible(cullUser, camFrustum, cullingFrustum);
			else
			{
				if (IsLeafNode())
					CullingInnerObject(cullUser, camFrustum, cullingFrustum);
				else
				{
					for (uint i = 0; i < childCount; ++i)
						childrenNode[i]->Culling(cullUser, camFrustum, cullingFrustum);
				}
			}	
		}
		JUserPtr<JGameObject> JOctreeNode::IntersectFirst(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir, const bool allowContainRayPos)const noexcept
		{
			if (IsLeafNode())
			{
				float minDist = FLT_MAX;
				int index = -1;
				const uint innerCount = (uint)innerGameObject.size();
				for (uint i = 0; i < innerCount; ++i)
				{
					auto oriBBox = innerGameObject[i]->GetRenderItem()->GetOrientedBoundingBox();
					if (!allowContainRayPos && oriBBox.Contains(ori) == DirectX::CONTAINS)
						continue;

					float dist = 0;
					if (oriBBox.Intersects(ori, dir, dist))
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
				JIntersectInfo info[childCount];
				for (uint i = 0; i < childCount; ++i)
				{
					info[i].index = i;
					info[i].isIntersect = childrenNode[i]->boundingBox.Intersects(ori, dir, info[i].dist);
				}

				std::sort(info, info + 8, &JIntersectInfo::CompareAsc);

				JUserPtr<JGameObject> res = nullptr;
				for (uint i = 0; i < childCount; ++i)
				{
					if (!info[i].isIntersect)
						continue;

					res = childrenNode[info[i].index]->IntersectFirst(ori, dir, allowContainRayPos);
					if (res != nullptr)
						break;
				}
				return res;
			}
		}
		void JOctreeNode::IntersectAscendingSort(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir, _Out_ std::vector<JUserPtr<JGameObject>>& res, std::vector<JIntersectInfo>& info)const noexcept
		{
			if (IsLeafNode())
			{
				const uint innerCount = (uint)innerGameObject.size();
				if (info.size() < innerCount)
					info.resize(innerCount);

				for (uint i = 0; i < innerCount; ++i)
				{
					info[i].index = i;
					info[i].isIntersect = innerGameObject[i]->GetRenderItem()->GetOrientedBoundingBox().Intersects(ori, dir, info[i].dist);
				}
				std::sort(info.begin(), info.begin() + innerCount, &JIntersectInfo::CompareAsc);
				for (uint i = 0; i < innerCount; ++i)
				{
					if (info[i].isIntersect)
						res.push_back(innerGameObject[info[i].index]);
				}
			}
			else
			{
				JIntersectInfo cInfo[childCount];
				for (uint i = 0; i < childCount; ++i)
				{
					cInfo[i].index = i;
					cInfo[i].isIntersect = childrenNode[i]->boundingBox.Intersects(ori, dir, cInfo[i].dist);
				}
				std::sort(cInfo, cInfo + 8, &JIntersectInfo::CompareAsc);
				for (uint i = 0; i < childCount; ++i)
				{
					if (!cInfo[i].isIntersect)
						continue;

					childrenNode[cInfo[i].index]->IntersectAscendingSort(ori, dir, res, info);
				}
			}
		}
		void JOctreeNode::IntersectDescendingSort(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir, _Out_ std::vector<JUserPtr<JGameObject>>& res, std::vector<JIntersectInfo>& info)const noexcept
		{
			IntersectAscendingSort(ori, dir, res, info);
			std::reverse(res.begin(), res.end());
		}
		void JOctreeNode::Intersect(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir, _Out_ std::vector<JUserPtr<JGameObject>>& res)const noexcept
		{
			if (IsLeafNode())
			{
				for (const auto& data : innerGameObject)
				{
					float dist = 0;
					if (data->GetRenderItem()->GetOrientedBoundingBox().Intersects(ori, dir, dist))
						res.push_back(data);
				}
			}
			else
			{
				for (const auto& data : childrenNode)
				{
					float dist = 0;
					if(data->boundingBox.Intersects(ori, dir, dist))
						data->Intersect(ori, dir, res);
				}
			}
		}
		bool JOctreeNode::AddGameObject(JUserPtr<JGameObject> gameObj, bool isLooseOctree)noexcept
		{
			JUserPtr<JRenderItem> rItem = gameObj->GetRenderItem();
			BoundingBox rItemBoundingBox = rItem->GetBoundingBox();
			ContainmentType res = boundingBox.Contains(rItemBoundingBox);

			if (isLooseOctree)
			{
				if (res == ContainmentType::CONTAINS)
				{
					innerGameObject.push_back(gameObj);
					return true;
				}
				else
					return false;
			}
			else
			{
				if (res == ContainmentType::CONTAINS)
				{
					innerGameObject.push_back(gameObj);
					return true;
				}
				else if (res == ContainmentType::INTERSECTS)
				{
					innerGameObject.push_back(gameObj);
					return true;
				}
				else
					return false;
			}
		}
		bool JOctreeNode::AddNeighborNode(JOctreeNode* octreeNode)noexcept
		{
			if (octreeNode == nullptr)
				return false;
			neighborNode.push_back(octreeNode);
			return true;
		}
		bool JOctreeNode::RemoveGameObject(JUserPtr<JGameObject> gameObj)noexcept
		{
			if (gameObj != nullptr)
				return RemoveInnerGameObject(gameObj);
			else
				return false;
		}
		bool JOctreeNode::IsLeafNode()const noexcept
		{
			return childrenNode[0] == nullptr;
		}
		bool JOctreeNode::IsRootNode()const noexcept
		{
			return parentNode == nullptr;
		}
		DirectX::BoundingBox JOctreeNode::GetBoundingBox()const noexcept
		{
			return boundingBox;
		}
		DirectX::BoundingBox JOctreeNode::GetInnerGameObjectBoundingBox(const uint index)const noexcept
		{
			return innerGameObject[index]->GetRenderItem()->GetBoundingBox();
		}
		JOctreeNode* JOctreeNode::GetParentNode()noexcept
		{
			return parentNode;
		}
		JOctreeNode* JOctreeNode::GetChildNode(const uint index)noexcept
		{
			if (index >= childCount)
				return nullptr;
			else
				return childrenNode[index];
		}
		uint JOctreeNode::GetInnerGameObjectCount()const noexcept
		{
			return (uint)innerGameObject.size();
		}
		bool JOctreeNode::RemoveInnerGameObject(JUserPtr<JGameObject> gameObject)noexcept
		{
			const size_t tarGuid = gameObject->GetGuid();
			const uint innerGameObjectCount = (uint)innerGameObject.size();
			for (uint i = 0; i < innerGameObjectCount; ++i)
			{
				if (innerGameObject[i]->GetGuid() == tarGuid)
				{
					innerGameObject.erase(innerGameObject.begin() + i);
					return true;
				}
			}
			return false;
		}
		void JOctreeNode::CullingInnerObject(Graphic::JCullingUserInterface& cullUser, const JCullingFrustum& camFrustum, J_CULLING_FLAG oriFlag)
		{
			const uint innerGameObjCount = (uint)innerGameObject.size();
			for (uint i = 0; i < innerGameObjCount; ++i)
			{
				JUserPtr<JRenderItem> rItem = innerGameObject[i]->GetRenderItem();
				if ((rItem->GetSpaceSpatialMask() & SPACE_SPATIAL_ALLOW_CULLING) == 0)
					continue;

				J_CULLING_FLAG flag = oriFlag;
				if (camFrustum.IsBoundingBoxIn(rItem->GetBoundingBox(), flag) != J_CULLING_RESULT::DISJOINT)
					OffCulling(cullUser, rItem); 
				else
					SetCulling(cullUser, rItem);
			}
		}
		void JOctreeNode::CullingInnerObject(Graphic::JCullingUserInterface& cullUser, const DirectX::BoundingFrustum& camFrustum, const DirectX::BoundingFrustum& cullingFrustum)
		{
			const uint innerGameObjCount = (uint)innerGameObject.size();
			for (uint i = 0; i < innerGameObjCount; ++i)
			{
				JUserPtr<JRenderItem> rItem = innerGameObject[i]->GetRenderItem();
				if ((rItem->GetSpaceSpatialMask() & SPACE_SPATIAL_ALLOW_CULLING) == 0)
					continue;

				const BoundingOrientedBox oriBBox = rItem->GetOrientedBoundingBox();
				const ContainmentType res = camFrustum.Contains(oriBBox);
				if (res == ContainmentType::CONTAINS)
					OffCulling(cullUser, rItem);
				else if (res == ContainmentType::INTERSECTS)
				{
					if (cullingFrustum.Contains(oriBBox) == ContainmentType::DISJOINT)
						SetCulling(cullUser, rItem);
					else
						OffCulling(cullUser, rItem);
				}
				else
					SetCulling(cullUser, rItem);
			}
		}
		void JOctreeNode::SetVisible(Graphic::JCullingUserInterface& cullUser, const JCullingFrustum& camFrustum, J_CULLING_FLAG flag)noexcept
		{
			CullingInnerObject(cullUser, camFrustum, flag);
			if (!IsLeafNode())
			{
				for (uint i = 0; i < childCount; ++i)
					childrenNode[i]->SetVisible(cullUser, camFrustum, flag);
			}
		}
		void JOctreeNode::SetVisible(Graphic::JCullingUserInterface& cullUser, const DirectX::BoundingFrustum& camFrustum, const DirectX::BoundingFrustum& cullingFrustum)noexcept
		{
			CullingInnerObject(cullUser, camFrustum, cullingFrustum);
			if (!IsLeafNode())
			{
				for (uint i = 0; i < childCount; ++i)
					childrenNode[i]->SetVisible(cullUser, camFrustum, cullingFrustum);
			}
		}
		void JOctreeNode::SetInVisible(Graphic::JCullingUserInterface& cullUser)noexcept
		{
			const uint innerGameObjCount = (uint)innerGameObject.size();
			for (uint i = 0; i < innerGameObjCount; ++i)
			{
				JUserPtr<JRenderItem> rItem = innerGameObject[i]->GetRenderItem();
				if ((rItem->GetSpaceSpatialMask() & SPACE_SPATIAL_ALLOW_CULLING) > 0)
					SetCulling(cullUser, rItem);
			}

			if (!IsLeafNode())
			{
				for (uint i = 0; i < childCount; ++i)
					childrenNode[i]->SetInVisible(cullUser);
			}
		}
	}
}