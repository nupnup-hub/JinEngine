#include"JOctree.h"  
#include"../../../Object/GameObject/JGameObject.h" 
#include"../../../Object/Component/RenderItem/JRenderItem.h"
#include<DirectXMath.h>
#include<algorithm>

namespace JinEngine
{
	/*
	*	Front	Back
		2 3		6 7
		0 1		4 5
	*   Left Hand Coord
	*/
	using namespace DirectX;
	namespace Core
	{
		JOctree::JOctree(){}
		JOctree::~JOctree(){}
		void JOctree::Build()noexcept
		{ 
			BuildOctree();
		}
		void JOctree::UnBuild()noexcept
		{
			if (rootNodeCash != nullptr)
				rootNodeCash->Clear();
			containNodeMap.clear();
			allNode.clear();
			rootNodeCash = nullptr;
		}
		void JOctree::Clear()noexcept
		{
			UnBuild();
			JSpaceSpatial::Clear();
		}
		void JOctree::OnDebugGameObject()noexcept
		{
			const uint allNodeCount = (uint)allNode.size();
			if (allNodeCount > 0)
			{
				for (uint i = 0; i < allNodeCount; ++i)
					allNode[i]->CreateDebugGameObject(GetDebugRoot(), IsDebugLeafOnly());
			}
		}
		void JOctree::OffDebugGameObject()noexcept
		{
			const uint allNodeCount = (uint)allNode.size();
			if (allNodeCount > 0)
			{
				for (uint i = 0; i < allNodeCount; ++i)
					allNode[i]->DestroyDebugGameObject();
			}
		}
		void JOctree::OffCulling()noexcept
		{
			if (rootNodeCash != nullptr)
				rootNodeCash->OffCulling();
		}
		void JOctree::Culling(const JCullingFrustum& camFrustum)noexcept
		{
			if(rootNodeCash != nullptr)
				rootNodeCash->Culling(camFrustum, J_CULLING_FLAG::NONE);
		}
		void JOctree::Culling(const BoundingFrustum& camFrustum)noexcept
		{
			if (rootNodeCash != nullptr)
			{
				DirectX::BoundingFrustum nearFrustum{ camFrustum };
				nearFrustum.Far = 1;
				nearFrustum.Near = 0;
				rootNodeCash->Culling(camFrustum, nearFrustum);
			}
		}
		void JOctree::UpdateGameObject(JGameObject* gameObject)noexcept
		{
			auto octNode = containNodeMap.find(gameObject->GetGuid());
			if (octNode != containNodeMap.end())
			{
				RemoveGameObject(gameObject);
				AddGameObject(gameObject);
			}
		}
		void JOctree::AddGameObject(JGameObject* newGameObject)noexcept
		{
			if (!GetInnerRoot()->IsParentLine(newGameObject))
				return;

			JRenderItem* rItem = newGameObject->GetRenderItem();
			JOctreeNode* octNode = FindOptimalNode(rootNodeCash, rItem->GetBoundingBox());
			if (octNode == nullptr)
				return;

			if (octNode->AddGameObject(newGameObject, isLooseOctree))
			{
				containNodeMap.emplace(newGameObject->GetGuid(), octNode);
				if (IsDebugActivated())
					octNode->CreateDebugGameObject(GetDebugRoot(), IsDebugLeafOnly());
			}
		}
		void JOctree::RemoveGameObject(JGameObject* gameObj)noexcept
		{
			auto octNode = containNodeMap.find(gameObj->GetGuid());
			if (octNode != containNodeMap.end())
			{
				octNode->second->RemoveGameObject(gameObj); 
				containNodeMap.erase(gameObj->GetGuid());
			}
			if (IsDebugRoot(gameObj))
			{
				JSpaceSpatialOption option = GetCommonOption();
				option.debugRoot = nullptr;
				SetCommonOption(option);
			}
			if (IsInnerRoot(gameObj))
			{
				JSpaceSpatialOption option = GetCommonOption();
				option.innerRoot = nullptr;
				SetCommonOption(option);
			}
		}
		J_SPACE_SPATIAL_TYPE JOctree::GetType()const noexcept
		{
			return J_SPACE_SPATIAL_TYPE::OCTREE;
		}
		JOctreeOption JOctree::GetOctreeOption()const noexcept
		{
			return JOctreeOption(minSize, octreeSizeSquare, looseFactor, GetCommonOption());
		}
		void JOctree::SetOctreeOption(const JOctreeOption& newOption)noexcept
		{
			JOctreeOption preOption = GetOctreeOption();
			minSize = newOption.minSize;
			octreeSizeSquare = newOption.octreeSizeSquare;
			octreeSize = ((uint)pow(2, octreeSizeSquare));
			looseFactor = std::clamp(newOption.looseFactor, looseFactorMin, looseFactorMax);
			 
			if (!preOption.EqualCommonOption(newOption))
				SetCommonOption(newOption.commonOption);
			else
			{
				if (!preOption.EqualOctreeOption(newOption))
				{ 
					if (IsSpaceSpatialActivated())
					{
						UnBuild();
						Build(); 
						if (IsDebugActivated())
							OnDebugGameObject();
					}
				}
			}
		}
		void JOctree::BuildOctree()noexcept
		{
			std::vector<JGameObject*> gameObject = GetInnerObject();
			if (rootNodeCash != nullptr)
			{
				rootNodeCash->Clear();
				rootNodeCash = nullptr;
			}
			allNode.clear();

			float octreeHalfSize = octreeSize * 0.5f;
			XMFLOAT3 maxVector(octreeHalfSize, octreeHalfSize, octreeHalfSize);
			XMFLOAT3 minVector(-octreeHalfSize, -octreeHalfSize, -octreeHalfSize);

			BoundingBox rootBoundingBox;
			XMStoreFloat3(&rootBoundingBox.Center, 0.5f * (XMLoadFloat3(&maxVector) + XMLoadFloat3(&minVector)));
			XMStoreFloat3(&rootBoundingBox.Extents, 0.5f * (XMLoadFloat3(&maxVector) - XMLoadFloat3(&minVector)));

			std::unique_ptr<JOctreeNode> root = std::make_unique<JOctreeNode>(rootBoundingBox, isLooseOctree);
			rootNodeCash = root.get();
			allNode.push_back(std::move(root));

			BuildOctreeNode(rootNodeCash, 1);
			FindNeighborOctreeNode(rootNodeCash);

			const uint gameObjCount = (uint)gameObject.size();
			for (uint i = 0; i < gameObjCount; ++i)
				AddGameObject(gameObject[i]);

			if (IsDebugActivated())
				OnDebugGameObject();
		}
		void JOctree::BuildOctreeNode(JOctreeNode* parent, const uint depth)noexcept
		{
			if (parent == nullptr)
				return;

			//const uint parentInnerGameObjCount = parent->GetInnerGameObjectCount();
			//if (parentInnerGameObjCount == 0)
			//	return;

			float extentLengthHalf = (float)(GetExtentLength(depth) * 0.5f);
			if (extentLengthHalf < minSize)
				return;

			float extentLooseLengthHalf = 0;
			if (isLooseOctree)
				extentLooseLengthHalf = (float)(GetLooseExtentLength(depth) * 0.5f);

			BoundingBox pBoundingBox = parent->GetBoundingBox();
			BoundingBox cBoundingBox[8];
			cBoundingBox[0].Center = XMFLOAT3(pBoundingBox.Center.x - extentLengthHalf, pBoundingBox.Center.y - extentLengthHalf, pBoundingBox.Center.z - extentLengthHalf);
			cBoundingBox[1].Center = XMFLOAT3(pBoundingBox.Center.x + extentLengthHalf, pBoundingBox.Center.y - extentLengthHalf, pBoundingBox.Center.z - extentLengthHalf);
			cBoundingBox[2].Center = XMFLOAT3(pBoundingBox.Center.x - extentLengthHalf, pBoundingBox.Center.y + extentLengthHalf, pBoundingBox.Center.z - extentLengthHalf);
			cBoundingBox[3].Center = XMFLOAT3(pBoundingBox.Center.x + extentLengthHalf, pBoundingBox.Center.y + extentLengthHalf, pBoundingBox.Center.z - extentLengthHalf);
			cBoundingBox[4].Center = XMFLOAT3(pBoundingBox.Center.x - extentLengthHalf, pBoundingBox.Center.y - extentLengthHalf, pBoundingBox.Center.z + extentLengthHalf);
			cBoundingBox[5].Center = XMFLOAT3(pBoundingBox.Center.x + extentLengthHalf, pBoundingBox.Center.y - extentLengthHalf, pBoundingBox.Center.z + extentLengthHalf);
			cBoundingBox[6].Center = XMFLOAT3(pBoundingBox.Center.x - extentLengthHalf, pBoundingBox.Center.y + extentLengthHalf, pBoundingBox.Center.z + extentLengthHalf);
			cBoundingBox[7].Center = XMFLOAT3(pBoundingBox.Center.x + extentLengthHalf, pBoundingBox.Center.y + extentLengthHalf, pBoundingBox.Center.z + extentLengthHalf);

			if (isLooseOctree)
				extentLengthHalf = extentLooseLengthHalf;

			XMFLOAT3 extents = XMFLOAT3(extentLengthHalf, extentLengthHalf, extentLengthHalf);
			for (uint i = 0; i < 8; ++i)
				cBoundingBox[i].Extents = extents;

			std::unique_ptr<JOctreeNode> cNode[8];
			for (uint i = 0; i < 8; ++i)
				cNode[i] = std::make_unique<JOctreeNode>(cBoundingBox[i], isLooseOctree, parent);
			for (uint i = 0; i < 8; ++i)
			{
				BuildOctreeNode(cNode[i].get(), depth + 1);
				allNode.push_back(std::move(cNode[i]));
			}
		}
		void JOctree::FindNeighborOctreeNode(JOctreeNode* node)noexcept
		{
			if (node == nullptr)
				return;

			XMFLOAT3 neighborCenter[6];
			XMFLOAT3 center = node->GetBoundingBox().Center;
			float extent = node->GetBoundingBox().Extents.x;
			if (isLooseOctree)
				extent *= 0.5f;
			extent *= 2;
			/*
			index order -x. x, -y, y, -z, z
			*/
			neighborCenter[0] = XMFLOAT3(center.x - extent, center.y, center.z);
			neighborCenter[1] = XMFLOAT3(center.x + extent, center.y, center.z);
			neighborCenter[2] = XMFLOAT3(center.x, center.y - extent, center.z);
			neighborCenter[3] = XMFLOAT3(center.x, center.y + extent, center.z);
			neighborCenter[4] = XMFLOAT3(center.x, center.y, center.z - extent);
			neighborCenter[5] = XMFLOAT3(center.x, center.y, center.z + extent);

			for (uint i = 0; i < 6; ++i)
			{
				JOctreeNode* neighborNode = FindNodeNearCenter(rootNodeCash, XMLoadFloat3(&neighborCenter[i]));
				if (neighborNode != nullptr)
					node->AddNeighborNode(neighborNode);
			}

			if (!node->IsLeafNode())
			{
				for (uint i = 0; i < 8; ++i)
					FindNeighborOctreeNode(node->GetChildNode(i));
			}
		}
		JOctreeNode* JOctree::FindNodeNearCenter(JOctreeNode* node, const DirectX::XMVECTOR point)noexcept
		{
			JOctreeNode* nearestNode = nullptr;
			ContainmentType containType = node->GetBoundingBox().Contains(point);
			if (containType == ContainmentType::CONTAINS)
				nearestNode = node;

			if (!node->IsLeafNode() && containType == ContainmentType::CONTAINS)
			{
				for (uint i = 0; i < 8; ++i)
				{
					JOctreeNode* loopRes = FindNodeNearCenter(node->GetChildNode(i), point);
					if (loopRes != nullptr)
					{
						if (nearestNode == nullptr)
							nearestNode = loopRes;
						else
						{
							XMFLOAT3 nearestCenter = nearestNode->GetBoundingBox().Center;
							XMFLOAT3 loopResCenter = loopRes->GetBoundingBox().Center;

							XMFLOAT3 nearestDisF;
							XMFLOAT3 loopResDisF;

							XMStoreFloat3(&nearestDisF, XMVector3Length(XMVectorSubtract(point, XMLoadFloat3(&nearestCenter))));
							XMStoreFloat3(&loopResDisF, XMVector3Length(XMVectorSubtract(point, XMLoadFloat3(&loopResCenter))));

							if (nearestDisF.x > loopResDisF.x)
								nearestNode = loopRes;
						}
					}
				}
			}
			return nearestNode;
		}
		JOctreeNode* JOctree::FindOptimalNode(JOctreeNode* node, const DirectX::BoundingBox& tarBBox)noexcept
		{
			JOctreeNode* nearestNode = nullptr;
			ContainmentType containType = node->GetBoundingBox().Contains(tarBBox);
			if (containType == ContainmentType::CONTAINS)
				nearestNode = node;

			if (!node->IsLeafNode())
			{
				for (uint i = 0; i < 8; ++i)
				{
					JOctreeNode* loopRes = FindOptimalNode(node->GetChildNode(i), tarBBox);
					if (loopRes != nullptr)
					{
						if (nearestNode == nullptr)
							nearestNode = loopRes;
						else
						{
							const XMVECTOR point = XMLoadFloat3(&tarBBox.Center);
							XMFLOAT3 nearestExtents = nearestNode->GetBoundingBox().Extents;
							XMFLOAT3 loopResExtents = loopRes->GetBoundingBox().Extents;

							XMFLOAT3 nearestDisF;
							XMFLOAT3 loopResDisF;

							XMStoreFloat3(&nearestDisF, XMVector3Length(XMVectorSubtract(point, XMLoadFloat3(&nearestExtents))));
							XMStoreFloat3(&loopResDisF, XMVector3Length(XMVectorSubtract(point, XMLoadFloat3(&loopResExtents))));

							if (nearestDisF.x > loopResDisF.x)
								nearestNode = loopRes;
						}
					}
				}
			}
			return nearestNode;
		}
		uint JOctree::GetExtentLength(const uint depth)const noexcept
		{
			return (uint)(octreeSize / (1 << depth));
		}
		uint JOctree::GetLooseExtentLength(const uint depth)const noexcept
		{
			return (uint)((looseFactor * octreeSize) / (1 << depth));
		}
	}
}