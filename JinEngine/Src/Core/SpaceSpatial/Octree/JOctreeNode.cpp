#include"JOctreeNode.h"  
#include"../../../Object/GameObject/JGameObject.h"
#include"../../../Object/GameObject/JGameObjectFactoryUtility.h"
#include"../../../Object/Component/RenderItem/JRenderItem.h"
#include"../../../Object/Component/Transform/JTransform.h"

namespace JinEngine
{
	using namespace DirectX;
	namespace Core
	{
		JOctreeNode::JOctreeNode(const DirectX::BoundingBox& boundingBox, bool isLooseOctree, JOctreeNode* parentNode)
			:boundingBox(boundingBox), parentNode(parentNode)
		{
			if (parentNode != nullptr)
				parentNode->childrenNode.push_back(this);
		}
		JOctreeNode::~JOctreeNode() {}
		void JOctreeNode::CreateDebugGameObject(JGameObject* parent, bool onlyLeafNod)noexcept
		{
			if (debugGameObject == nullptr)
			{
				if (innerGameObject.size() == 0 && onlyLeafNod)
					return;

				if (innerGameObject.size() == 0)
					debugGameObject =JGFU::CreateDebugLineShape(*parent, OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX_LINE, J_DEFAULT_MATERIAL::DEBUG_LINE_RED);
				else
					debugGameObject =JGFU::CreateDebugLineShape(*parent, OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX_LINE, J_DEFAULT_MATERIAL::DEBUG_LINE_GREEN);
				
				const float outlineFactor = 0.01f;
				const BoundingBox rBBox = debugGameObject->GetRenderItem()->GetBoundingBox();
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
				JGameObject::BeginDestroy(debugGameObject); 
				debugGameObject = nullptr;
			}
		}
		void JOctreeNode::Clear()
		{
			const uint innerCount = (uint)innerGameObject.size();
			for (uint i = 0; i < innerCount; ++i)
			{
				innerGameObject[i]->GetRenderItem()->SetRenderVisibility(J_RENDER_VISIBILITY::VISIBLE);
				innerGameObject[i] = nullptr;
			}
			innerGameObject.clear();
			DestroyDebugGameObject();
			if (childrenNode.size() != 0)
			{
				for (uint i = 0; i < 8; ++i)
					childrenNode[i]->Clear();
			}
		}
		void JOctreeNode::Culling(const JCullingFrustum& camFrustum, J_CULLING_FLAG flag)noexcept
		{
			J_CULLING_RESULT res = camFrustum.IsBoundingBoxIn(boundingBox, flag);
			if (res == J_CULLING_RESULT::DISJOINT)
				SetInVisible();
			else if (res == J_CULLING_RESULT::CONTAIN)
				SetVisible(camFrustum, flag);
			else
			{			
				if (childrenNode.size() > 0)
				{ 
					for (uint i = 0; i < 8; ++i)
						childrenNode[i]->Culling(camFrustum, flag);
				}
				else
					CullingInnerObject(camFrustum, flag);
			}
		}
		void JOctreeNode::Culling(const DirectX::BoundingFrustum& camFrustum, const DirectX::BoundingFrustum& nearFrustum)noexcept
		{
			ContainmentType res = camFrustum.Contains(boundingBox);
			if (res == ContainmentType::DISJOINT)
				SetInVisible();
			else if (res == ContainmentType::CONTAINS)
				SetVisible(camFrustum, nearFrustum);
			else
			{
				if (childrenNode.size() > 0)
				{
					for (uint i = 0; i < 8; ++i)
						childrenNode[i]->Culling(camFrustum, nearFrustum);
				}
				else
					CullingInnerObject(camFrustum, nearFrustum);
			}	
		}
		void JOctreeNode::OffCulling()
		{
			const uint innerCount = (uint)innerGameObject.size();
			for (uint i = 0; i < innerCount; ++i)
				innerGameObject[i]->GetRenderItem()->SetRenderVisibility(J_RENDER_VISIBILITY::VISIBLE);
			if (childrenNode.size() > 0)
			{
				for (uint i = 0; i < 8; ++i)
					childrenNode[i]->OffCulling();
			}
		}
		bool JOctreeNode::AddGameObject(JGameObject* gameObj, bool isLooseOctree)noexcept
		{
			JRenderItem* rItem = gameObj->GetRenderItem();
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
		bool JOctreeNode::RemoveGameObject(JGameObject* gameObj)noexcept
		{
			if (gameObj != nullptr)
				return RemoveInnerGameObject(gameObj);
			else
				return false;
		}
		bool JOctreeNode::IsLeafNode()const noexcept
		{
			return childrenNode.size() == 0;
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
			if (index >= childrenNode.size())
				return nullptr;
			else
				return childrenNode[index];
		}
		uint JOctreeNode::GetInnerGameObjectCount()const noexcept
		{
			return (uint)innerGameObject.size();
		}
		bool JOctreeNode::RemoveInnerGameObject(JGameObject* gameObject)noexcept
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
		void JOctreeNode::CullingInnerObject(const JCullingFrustum& camFrustum, J_CULLING_FLAG oriFlag)
		{
			const uint innerGameObjCount = (uint)innerGameObject.size();
			for (uint i = 0; i < innerGameObjCount; ++i)
			{
				JRenderItem* rItem = innerGameObject[i]->GetRenderItem();
				if ((rItem->GetSpaceSpatialMask() & SPACE_SPATIAL_ALLOW_CULLING) == 0)
					continue;

				J_CULLING_FLAG flag = oriFlag;
				if (camFrustum.IsBoundingBoxIn(rItem->GetBoundingBox(), flag) != J_CULLING_RESULT::DISJOINT)
					rItem->SetRenderVisibility(J_RENDER_VISIBILITY::VISIBLE);
				else
					rItem->SetRenderVisibility(J_RENDER_VISIBILITY::INVISIBLE);
			}
		}
		void JOctreeNode::CullingInnerObject(const DirectX::BoundingFrustum& camFrustum, const DirectX::BoundingFrustum& nearFrustum)
		{
			const uint innerGameObjCount = (uint)innerGameObject.size();
			for (uint i = 0; i < innerGameObjCount; ++i)
			{
				JRenderItem* rItem = innerGameObject[i]->GetRenderItem();				 
				if ((rItem->GetSpaceSpatialMask() & SPACE_SPATIAL_ALLOW_CULLING) == 0)
					continue;

				const BoundingBox bbox = rItem->GetBoundingBox();
				const ContainmentType res = camFrustum.Contains(bbox);
				if (res == ContainmentType::CONTAINS)
					rItem->SetRenderVisibility(J_RENDER_VISIBILITY::VISIBLE);
				else if (res == ContainmentType::INTERSECTS)
				{
					if (nearFrustum.Contains(bbox) == ContainmentType::DISJOINT)
						rItem->SetRenderVisibility(J_RENDER_VISIBILITY::VISIBLE);
					else
						rItem->SetRenderVisibility(J_RENDER_VISIBILITY::INVISIBLE);
				}
				else
					rItem->SetRenderVisibility(J_RENDER_VISIBILITY::INVISIBLE);
			}
		}
		void JOctreeNode::SetVisible(const JCullingFrustum& camFrustum, J_CULLING_FLAG flag)noexcept
		{
			CullingInnerObject(camFrustum, flag);
			if (childrenNode.size() > 0)
			{
				for (uint i = 0; i < 8; ++i)
					childrenNode[i]->SetVisible(camFrustum, flag);
			}
		}
		void JOctreeNode::SetVisible(const DirectX::BoundingFrustum& camFrustum, const DirectX::BoundingFrustum& nearFrustum)noexcept
		{
			CullingInnerObject(camFrustum, nearFrustum);
			if (childrenNode.size() > 0)
			{
				for (uint i = 0; i < 8; ++i)
					childrenNode[i]->SetVisible(camFrustum, nearFrustum);
			}
		}
		void JOctreeNode::SetInVisible()noexcept
		{
			const uint innerGameObjCount = (uint)innerGameObject.size();
			for (uint i = 0; i < innerGameObjCount; ++i)
			{
				JRenderItem* rItem = innerGameObject[i]->GetRenderItem();
				if ((rItem->GetSpaceSpatialMask() & SPACE_SPATIAL_ALLOW_CULLING) > 0)
					rItem->SetRenderVisibility(J_RENDER_VISIBILITY::INVISIBLE);
			}

			if (childrenNode.size() > 0)
			{
				for (uint i = 0; i < 8; ++i)
					childrenNode[i]->SetInVisible();
			}
		}
	}
}