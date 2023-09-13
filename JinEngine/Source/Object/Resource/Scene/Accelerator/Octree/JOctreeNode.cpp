#include"JOctreeNode.h"  
#include"../JAcceleratorOption.h"
#include"../../../../Resource/Mesh/JMeshGeometry.h" 
#include"../../../../GameObject/JGameObject.h"
#include"../../../../GameObject/JGameObjectCreator.h"
#include"../../../../Component/RenderItem/JRenderItem.h"
#include"../../../../Component/RenderItem/JRenderItemPrivate.h"
#include"../../../../Component/Transform/JTransform.h" 
#include"../../../../../Core/Math/JMathHelper.h"  
#include"../../../../../Core/Math/JVectorExtend.h"
#include"../../../../../Graphic/Culling/JCullingInterface.h"

namespace JinEngine
{
	using namespace DirectX;
	JOctreeNode::JOctreeNode(const DirectX::BoundingBox& boundingBox, bool isLooseOctree, JOctreeNode* parentNode)
		:boundingBox(boundingBox), parentNode(parentNode)
	{
		if (parentNode != nullptr)
		{
			for (uint i = 0; i < childCount; ++i)
			{
				if (parentNode->childrenNode[i] == nullptr)
				{
					parentNode->childrenNode[i] = this;
					break;
				}
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
				debugGameObject = JGCI::CreateDebugLineShape(parent, OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::BOUNDING_BOX_LINE, J_DEFAULT_MATERIAL::DEBUG_LINE_RED, false);
			else
				debugGameObject = JGCI::CreateDebugLineShape(parent, OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::BOUNDING_BOX_LINE, J_DEFAULT_MATERIAL::DEBUG_LINE_GREEN, false);

			const float outlineFactor = 0.01f;
			const BoundingBox rBBox = debugGameObject->GetRenderItem()->GetMesh()->GetBoundingBox();

			debugGameObject->GetTransform()->SetScale(JVector3<float>(boundingBox.Extents) / rBBox.Extents + outlineFactor);
			debugGameObject->GetTransform()->SetPosition(JVector3<float>(boundingBox.Center) + rBBox.Center);
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
			if (childrenNode[i] != nullptr)
				childrenNode[i]->Clear();
			childrenNode[i] = nullptr;
		}
	}
	void JOctreeNode::Culling(JAcceleratorCullingInfo& info, Core::J_CULLING_FLAG flag)noexcept
	{
		Core::J_CULLING_RESULT res = Contain(info, boundingBox, flag);
		if (res == Core::J_CULLING_RESULT::DISJOINT)
			SetInVisible(info);
		else if (res == Core::J_CULLING_RESULT::CONTAIN)
			SetVisible(info, flag);
		else
		{
			if (IsLeafNode())
				CullingInnerObject(info, flag);
			else
			{
				for (uint i = 0; i < childCount; ++i)
					childrenNode[i]->Culling(info, flag);
			}
		}
	}
	void JOctreeNode::Culling(JAcceleratorCullingInfo& info)noexcept
	{
		ContainmentType res = Contain(info, boundingBox);
		if (res == ContainmentType::DISJOINT)
			SetInVisible(info);
		else if (res == ContainmentType::CONTAINS)
			SetVisible(info);
		else
		{
			if (IsLeafNode())
				CullingInnerObject(info);
			else
			{
				for (uint i = 0; i < childCount; ++i)
					childrenNode[i]->Culling(info);
			}
		}
	}
	void JOctreeNode::FindIntersect(JAcceleratorIntersectInfo& info)const noexcept
	{
		if (info.untilFirst)
			info.firstResult = FindIntersectFirst(info);
		else
		{
			if (info.sortType == J_ACCELERATOR_SORT_TYPE::ASCENDING)
				FindIntersectAscendingSort(info);
			else if (info.sortType == J_ACCELERATOR_SORT_TYPE::DESCENDING)
			{
				FindIntersectAscendingSort(info);
				std::reverse(info.resultObjVec.begin(), info.resultObjVec.end());
			}
			else
				FindIntersectNotSort(info);
		}
	}
	void JOctreeNode::FindContain(JAcceleratorContainInfo& info)const noexcept
	{
		FindContainNotSort(info);
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
	void JOctreeNode::CullingInnerObject(JAcceleratorCullingInfo& info, Core::J_CULLING_FLAG oriFlag)
	{
		const uint innerGameObjCount = (uint)innerGameObject.size();
		for (uint i = 0; i < innerGameObjCount; ++i)
		{
			JUserPtr<JRenderItem> rItem = innerGameObject[i]->GetRenderItem();
			if ((rItem->GetAcceleratorMask() & ACCELERATOR_ALLOW_CULLING) == 0)
				continue;

			Core::J_CULLING_FLAG flag = oriFlag;
			if (Contain(info, rItem->GetBoundingBox(), flag) != Core::J_CULLING_RESULT::DISJOINT)
				OffCulling(info, rItem);
			else
				SetCulling(info, rItem);
		}
	}
	void JOctreeNode::CullingInnerObject(JAcceleratorCullingInfo& info)
	{
		const uint innerGameObjCount = (uint)innerGameObject.size();
		for (uint i = 0; i < innerGameObjCount; ++i)
		{
			JUserPtr<JRenderItem> rItem = innerGameObject[i]->GetRenderItem();
			if ((rItem->GetAcceleratorMask() & ACCELERATOR_ALLOW_CULLING) == 0)
				continue;

			const BoundingOrientedBox oriBBox = rItem->GetOrientedBoundingBox();
			const ContainmentType res = Contain(info, oriBBox);
			if (res == ContainmentType::CONTAINS)
				OffCulling(info, rItem);
			else if (res == ContainmentType::INTERSECTS)
			{
				if (IsIntersectCullingFrustum(info, oriBBox))
					SetCulling(info, rItem);
				else
					OffCulling(info, rItem);
			}
			else
				SetCulling(info, rItem);
		}
	}
	void JOctreeNode::SetVisible(JAcceleratorCullingInfo& info, Core::J_CULLING_FLAG oriFlag)noexcept
	{
		CullingInnerObject(info, oriFlag);
		if (!IsLeafNode())
		{
			for (uint i = 0; i < childCount; ++i)
				childrenNode[i]->SetVisible(info, oriFlag);
		}
	}
	void JOctreeNode::SetVisible(JAcceleratorCullingInfo& info)noexcept
	{
		CullingInnerObject(info);
		if (!IsLeafNode())
		{
			for (uint i = 0; i < childCount; ++i)
				childrenNode[i]->SetVisible(info);
		}
	}
	void JOctreeNode::SetInVisible(JAcceleratorCullingInfo& info)noexcept
	{
		const uint innerGameObjCount = (uint)innerGameObject.size();
		for (uint i = 0; i < innerGameObjCount; ++i)
		{
			JUserPtr<JRenderItem> rItem = innerGameObject[i]->GetRenderItem();
			if ((rItem->GetAcceleratorMask() & ACCELERATOR_ALLOW_CULLING) > 0)
				SetCulling(info, rItem);
		}

		if (!IsLeafNode())
		{
			for (uint i = 0; i < childCount; ++i)
				childrenNode[i]->SetInVisible(info);
		}
	}
	JUserPtr<JGameObject> JOctreeNode::FindIntersectFirst(JAcceleratorIntersectInfo& info)const noexcept
	{
		if (IsLeafNode())
		{
			float minDist = FLT_MAX;
			int index = -1;
			const uint innerCount = (uint)innerGameObject.size();
			for (uint i = 0; i < innerCount; ++i)
			{
				float dist = 0;
				auto oriBBox = innerGameObject[i]->GetRenderItem()->GetOrientedBoundingBox();
				if (oriBBox.Intersects(info.ray.PosV(), info.ray.DirV(), dist) && info.CanAdd(oriBBox))
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
			JAcceleratorIntersectInfo::Intermediate intermediate[childCount];
			StuffChildIntermediateData(info, intermediate);

			JUserPtr<JGameObject> res = nullptr;
			for (uint i = 0; i < childCount; ++i)
			{
				if (!intermediate[i].isIntersect)
					continue;

				res = childrenNode[intermediate[i].index]->FindIntersectFirst(info);
				if (res != nullptr)
					break;
			}
			return res;
		}
	}
	void JOctreeNode::FindIntersectAscendingSort(JAcceleratorIntersectInfo& info)const noexcept
	{
		if (IsLeafNode())
		{
			const uint innerCount = (uint)innerGameObject.size();
			if (info.intermediate.size() < innerCount)
				info.intermediate.resize(innerCount);

			for (uint i = 0; i < innerCount; ++i)
			{
				info.intermediate[i].index = i;
				info.intermediate[i].isIntersect = innerGameObject[i]->GetRenderItem()->GetOrientedBoundingBox().Intersects(info.ray.PosV(), info.ray.DirV(), info.intermediate[i].dist);
			}
			info.SortIntermediate(innerCount, true);
			for (uint i = 0; i < innerCount; ++i)
			{
				auto& intermidiate = info.intermediate[i];
				if (intermidiate.isIntersect && info.CanAdd(innerGameObject[intermidiate.index]))
					info.resultObjVec.push_back(innerGameObject[intermidiate.index]);
			}
		}
		else
		{
			JAcceleratorIntersectInfo::Intermediate intermediate[childCount];
			StuffChildIntermediateData(info, intermediate);
			for (uint i = 0; i < childCount; ++i)
			{
				if (!intermediate[i].isIntersect)
					continue;

				childrenNode[intermediate[i].index]->FindIntersectAscendingSort(info);
			}
		}
	}
	void JOctreeNode::FindIntersectNotSort(JAcceleratorIntersectInfo& info)const noexcept
	{
		if (IsLeafNode())
		{
			for (const auto& data : innerGameObject)
			{
				float dist = 0;
				auto oriBBox = data->GetRenderItem()->GetOrientedBoundingBox();
				if (oriBBox.Intersects(info.ray.PosV(), info.ray.DirV(), dist) && info.CanAdd(oriBBox))
					info.resultObjVec.push_back(data);
			}
		}
		else
		{
			for (const auto& data : childrenNode)
			{
				float dist = 0;
				if (data->boundingBox.Intersects(info.ray.PosV(), info.ray.DirV(), dist))
					data->FindIntersectNotSort(info);
			}
		}
	}
	void JOctreeNode::FindContainNotSort(JAcceleratorContainInfo& info)const noexcept
	{
		if (IsLeafNode())
		{
			for (const auto& data : innerGameObject)
			{
				if (IsContain(info, data->GetRenderItem()->GetOrientedBoundingBox()))
					info.resultObjVec.push_back(data);
			}
		}
		else
		{
			for (const auto& data : childrenNode)
			{
				if (IsContain(info, data->boundingBox))
					data->FindContainNotSort(info);
			}
		}
	}
	void JOctreeNode::StuffChildIntermediateData(JAcceleratorIntersectInfo& info, JAcceleratorIntersectInfo::Intermediate* ptr)const noexcept
	{
		for (uint i = 0; i < childCount; ++i)
		{
			ptr[i].index = i;
			ptr[i].isIntersect = childrenNode[i]->boundingBox.Intersects(info.ray.PosV(), info.ray.DirV(), ptr[i].dist);
		}
		std::sort(ptr, ptr + 8, &JAcceleratorIntersectInfo::Intermediate::CompareAsc);
	}
}