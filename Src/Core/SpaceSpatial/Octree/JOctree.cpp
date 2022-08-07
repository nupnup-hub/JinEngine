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
		JOctree::JOctree(std::vector<JGameObject*>& gameObject, const uint octreeSizeSquare, const float looseFactor, const bool isLooseOctree)
			:octreeSize((uint)pow(2, octreeSizeSquare)), looseFactor(looseFactor), isLooseOctree(isLooseOctree)
		{
			if (JOctree::looseFactor > looseFactorMax)
				JOctree::looseFactor = looseFactorMax;
			BuildOctree(gameObject);
		}
		JOctree::~JOctree() {}
		void JOctree::OnDebugGameObject(JGameObject* newDebugRoot, bool onlyLeafNode)noexcept
		{
			if (debugRoot != nullptr && debugRoot->GetGuid() != newDebugRoot->GetGuid())
				OffDebugGameObject();

			if (!isDebugModeActivated)
			{
				isDebugModeActivated = true;
				debugRoot = newDebugRoot;
				const uint allNodeCount = (uint)allNode.size();
				if (allNodeCount > 0)
				{
					for (uint i = 0; i < allNodeCount; ++i)
						allNode[i]->CreateDebugGameObject(debugRoot, onlyLeafNode);
				}
			}
		}
		void JOctree::OffDebugGameObject()noexcept
		{
			if (isDebugModeActivated)
			{
				isDebugModeActivated = false;
				const uint allNodeCount = (uint)allNode.size();
				if (allNodeCount > 0)
				{
					for (uint i = 0; i < allNodeCount; ++i)
						allNode[i]->EraseDebugGameObject();
				}
			}
		}
		void JOctree::Culling(const BoundingFrustum& camFrustum)noexcept
		{
			rootNodeCash->Culling(camFrustum);
		}
		void JOctree::Clear()noexcept
		{
			if (rootNodeCash != nullptr)
				rootNodeCash->Clear();
			allNode.clear();
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

			if (!node->IsLeafNode())
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
		void JOctree::BuildOctree(std::vector<JGameObject*>& gameObject)noexcept
		{
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

			const uint gameObjCount = (uint)gameObject.size();
			for (uint i = 0; i < gameObjCount; ++i)
				rootNodeCash->AddGameObject(gameObject[i], isLooseOctree);
			BuildOctreeNode(rootNodeCash, 1);
			FindNeighborOctreeNode(rootNodeCash);
		}
		void JOctree::BuildOctreeNode(JOctreeNode* parent, const uint depth)noexcept
		{
			if (parent == nullptr)
				return;

			const uint parentInnerGameObjCount = parent->GetInnerGameObjectCount();
			if (parentInnerGameObjCount == 0)
				return;

			uint extentLengthHalf = (uint)(GetExtentLength(depth) * 0.5f);
			uint extentLooseLengthHalf = 0;
			if (isLooseOctree)
			{
				extentLooseLengthHalf = (uint)(GetLooseExtentLength(depth) * 0.5f);
				if (extentLooseLengthHalf <= minSize)
					return;
			}
			else
			{
				if (extentLengthHalf <= minSize)
					return;
			}

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

			XMFLOAT3 extents = XMFLOAT3(static_cast<float>(extentLengthHalf), static_cast<float>(extentLengthHalf), (static_cast<float>(extentLengthHalf)));
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