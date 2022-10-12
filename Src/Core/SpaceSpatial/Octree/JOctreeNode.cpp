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
			{
				parentNode->childrenNode.push_back(this);
				const uint pInnerGameObjCount = parentNode->GetInnerGameObjectCount();
				for (uint i = 0; i < pInnerGameObjCount; ++i)
					AddGameObject(parentNode->innerGameObject[i], isLooseOctree);
			}
		}
		JOctreeNode::~JOctreeNode() {}
		bool JOctreeNode::AddGameObject(JGameObject* gameObj, bool isLooseOctree)noexcept
		{
			JRenderItem* rItem = gameObj->GetRenderItem();
			BoundingBox rItemBoundingBox = rItem->GetBoundingBox();
			ContainmentType res = boundingBox.Contains(rItemBoundingBox);

			if (isLooseOctree)
			{
				if (res == ContainmentType::CONTAINS)
				{
					if (parentNode != nullptr)
						parentNode->RemoveInnerGameObject(gameObj);
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
					if (parentNode != nullptr)
						parentNode->RemoveInnerGameObject(gameObj);
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
		void JOctreeNode::CreateDebugGameObject(JGameObject* parent, bool onlyLeafNod)noexcept
		{
			if (debugGameObject == nullptr)
			{
				if (innerGameObject.size() == 0 && onlyLeafNod)
					return;

				debugGameObject = JGFU::CreateDebugGameObject(*parent, OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX, J_DEFAULT_MATERIAL::DEBUG_LINE_GREEN);
				float scaleFactor = boundingBox.Extents.x * 2;
				debugGameObject->GetTransform()->SetScale(XMFLOAT3(scaleFactor, scaleFactor, scaleFactor));
				debugGameObject->GetTransform()->SetPosition(boundingBox.Center);
			}
		}
		void JOctreeNode::DestroyDebugGameObject()noexcept
		{
			if (debugGameObject != nullptr)
			{
				debugGameObject->BeginDestroy();
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
				DestroyDebugGameObject();
			}

			if (childrenNode.size() != 0)
			{
				for (uint i = 0; i < 8; ++i)
					childrenNode[i]->Clear();
			}
		}
		void JOctreeNode::Culling(const DirectX::BoundingFrustum& camFrustum)noexcept
		{
			const uint innerGameObjCount = (uint)innerGameObject.size();
			if (innerGameObjCount == 0)
				return;

			ContainmentType res = camFrustum.Contains(boundingBox);
			if (res == ContainmentType::DISJOINT)
			{
				for (uint i = 0; i < innerGameObjCount; ++i)
					innerGameObject[i]->GetRenderItem()->SetRenderVisibility(J_RENDER_VISIBILITY::INVISIBLE);
			}
			else if (res == ContainmentType::CONTAINS)
			{
				for (uint i = 0; i < innerGameObjCount; ++i)
					innerGameObject[i]->GetRenderItem()->SetRenderVisibility(J_RENDER_VISIBILITY::VISIBLE);
			}
			else
			{
				if (childrenNode.size() > 0)
				{
					for (uint i = 0; i < 8; ++i)
						childrenNode[i]->Culling(camFrustum);
				}
				else
				{
					for (uint i = 0; i < innerGameObjCount; ++i)
						innerGameObject[i]->GetRenderItem()->SetRenderVisibility(J_RENDER_VISIBILITY::VISIBLE);
				}
			}
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
		bool JOctreeNode::IsLeafNode()const noexcept
		{
			return childrenNode.size() == 0;
		}
		bool JOctreeNode::IsRootNode()const noexcept
		{
			return parentNode == nullptr;
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
	}
}