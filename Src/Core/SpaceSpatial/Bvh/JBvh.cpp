#include"JBvh.h"
#include"../../DirectXEx/JBBox.h"
#include"../../DirectXEx/JDirectXCollisionEx.h"
#include"../../../Object/GameObject/JGameObject.h"
#include"../../../Object/Component/RenderItem/JRenderItem.h"
#include <algorithm>

namespace JinEngine
{
	using namespace DirectX;
	static constexpr float Inf = FLT_MAX;
	namespace Core
	{
		struct BucketInfo
		{
			int count = 0;
			JBBox bounds = JBBox(XMFLOAT3(Inf, Inf, Inf), XMFLOAT3(-Inf, -Inf, -Inf));
		};

		JBvh::JBvh(std::vector<JGameObject*> objectList, const J_BVH_BUILD_TYPE buildType, const J_BVH_SPLIT_TYPE splitType)
			:buildType(buildType), splitType(splitType)
		{
			constexpr int rootBvSize = 1 << 16;
			allNodes.emplace_back(std::make_unique<JBvhNode>(0,
				J_BVH_NODE_TYPE::ROOT, GetRootBoundingBox(),
				nullptr,
				nullptr,
				false));
			root = allNodes[0].get();
			if (objectList.size() > 0)
			{
				if (objectList.size() == 1)
					innerGameObjectCandidate = objectList[0];
				else
				{
					switch (buildType)
					{
					case J_BVH_BUILD_TYPE::TOP_DOWN:
					{
						switch (splitType)
						{
						case J_BVH_SPLIT_TYPE::SAH:
							BuildTopdownBvhBySAH(root, objectList, allNodes, 0, (int)objectList.size(), 0);
							break;
						default:
							break;
						}
					}
					break;
					case J_BVH_BUILD_TYPE::BOTTOP_UP:
						break;
					default:
						break;
					}
				}
			}
			//MessageBox(0, std::to_wstring(allNodes.size()).c_str(), 0, 0);
		}
		JBvh::~JBvh() {}
		void JBvh::OnDebugGameObject(JGameObject* newDebugRoot, bool onlyLeafNode)noexcept
		{
			if (debugRoot != nullptr && debugRoot->GetGuid() != newDebugRoot->GetGuid())
				OffDebugGameObject();

			debugRoot = newDebugRoot;
			onDebugObject = true;
			makeOnlyLeafNodeDebugObject = onlyLeafNode;
			const int allNodeCount = (int)allNodes.size();
			for (int i = 0; i < allNodeCount; ++i)
				allNodes[i]->CreateDebugGameObject(debugRoot, onlyLeafNode);
		}
		void JBvh::OffDebugGameObject()noexcept
		{
			onDebugObject = false;
			makeOnlyLeafNodeDebugObject = false;
			const int allNodeCount = (int)allNodes.size();
			for (int i = 0; i < allNodeCount; ++i)
				allNodes[i]->EraseDebugGameObject();
		}
		void JBvh::Culling(const JCullingFrustum& camFrustum)noexcept
		{
			if (allNodes.size() > 1)
				root->Culling(camFrustum, J_CULLING_FLAG::NONE);
		}
		void JBvh::Culling(const DirectX::BoundingFrustum& camFrustum)noexcept
		{
			if (allNodes.size() > 1)
				root->Culling(camFrustum);
		}
		void JBvh::Clear()noexcept
		{
			OffDebugGameObject();
			allNodes.clear();
			root = nullptr;
			innerGameObjectCandidate = nullptr;
		}
		void JBvh::UpdateGameObject(const size_t guid)noexcept
		{
			auto leafNode = leafNodeMap.find(guid);
			if (leafNode != leafNodeMap.end())
			{
				leafNode->second->UpdateInnerGameObject();
				BoundingBox boundBox = leafNode->second->GetInnerGameObject()->GetRenderItem()->GetBoundingBox();

				if (!leafNode->second->GetParentNode()->IsContain(boundBox))
				{
					JBvhNode* containNode = leafNode->second->GetParentNode()->GetContainNodeToRoot(boundBox);
					if (containNode == nullptr)
						ReBuildBvh(0);
					else
						ReBuildBvh(containNode->GetNodeNumber());
				}
			}
		}
		void JBvh::AddGameObject(JGameObject* newGameObject)noexcept
		{
			JBvhNode* containNode = root->GetContainNodeToLeaf(newGameObject->GetRenderItem()->GetBoundingBox());
			if (containNode != nullptr)
				ReBuildBvh(containNode->GetNodeNumber(), newGameObject);
		}
		void JBvh::EraeGameObject(const size_t guid)noexcept
		{
			auto leafNode = leafNodeMap.find(guid);
			if (leafNode != leafNodeMap.end())
				EraseBvhNode(leafNode->second->GetNodeNumber());
			else if (innerGameObjectCandidate != nullptr && innerGameObjectCandidate->GetGuid() == guid)
				innerGameObjectCandidate = nullptr;
		}
		DirectX::BoundingBox JBvh::GetRootBoundingBox()const noexcept
		{
			constexpr int rootBvSize = 1 << 16;
			return  BoundingBox(XMFLOAT3(0, 0, 0), XMFLOAT3(rootBvSize, rootBvSize, rootBvSize));
		}
		int JBvh::GetMaximumDimension(const DirectX::BoundingBox& box)const noexcept
		{
			if (box.Extents.x >= box.Extents.y && box.Extents.x >= box.Extents.z)
				return 0;
			else if (box.Extents.y >= box.Extents.x && box.Extents.y >= box.Extents.z)
				return 1;
			else
				return 2;
		}
		float JBvh::GetDimensionValue(const DirectX::XMFLOAT3& point, const int dim)const noexcept
		{
			if (dim == 0)
				return point.x;
			else if (dim == 1)
				return point.y;
			else
				return point.z;
		}
		void JBvh::BuildTopdownBvhBySAH(JBvhNode* parent,
			std::vector<JGameObject*>& objectList,
			std::vector<std::unique_ptr<JBvhNode>>& nodeVec,
			const int start,
			const int end,
			const int numberOffset)noexcept
		{
			constexpr int minObjectPerLeaf = 1;
			const int objectCount = end - start;

			if (objectCount <= minObjectPerLeaf)
			{
				leafNodeMap.emplace(objectList[start]->GetGuid(), parent);
				return;
			}
			else
			{
				JBBox bound(XMFLOAT3(Inf, Inf, Inf), XMFLOAT3(-Inf, -Inf, -Inf));
				for (int i = start; i < end; ++i)
					bound = JBBox::Union(bound, objectList[i]->GetRenderItem()->GetBoundingBox());

				JBBox centroidBound(XMFLOAT3(Inf, Inf, Inf), XMFLOAT3(-Inf, -Inf, -Inf));
				for (int i = start; i < end; ++i)
					centroidBound = JBBox::Union(centroidBound, objectList[i]->GetRenderItem()->GetBoundingBox().Center);

				int dim = centroidBound.MaxDimension();
				constexpr int bucketCount = 12;
				BucketInfo buckets[bucketCount];

				float centroidMinDim = GetDimensionValue(centroidBound.min, dim);
				float centroidMaxDim = GetDimensionValue(centroidBound.max, dim);
				float centroidSub = centroidMaxDim - centroidMinDim;

				for (int i = start; i < end; ++i)
				{
					float centroid = GetDimensionValue(objectList[i]->GetRenderItem()->GetBoundingBox().Center, dim);
					int b = 0;
					if (centroidSub != 0)
						b = bucketCount * ((centroid - centroidMinDim) / centroidSub);
					if (b >= bucketCount)
						b = bucketCount - 1;
					buckets[b].count++;
					buckets[b].bounds = JBBox::Union(buckets[b].bounds, objectList[i]->GetRenderItem()->GetBoundingBox());
				}

				constexpr float traverseRate = 0.125f;

				float boundSurface = bound.Surface();
				float cost[bucketCount - 1];
				for (int i = 0; i < bucketCount - 1; ++i)
				{
					JBBox b0(XMFLOAT3(Inf, Inf, Inf), XMFLOAT3(-Inf, -Inf, -Inf));
					JBBox b1(XMFLOAT3(Inf, Inf, Inf), XMFLOAT3(-Inf, -Inf, -Inf));

					int count0 = 0;
					int count1 = 0;

					for (int j = 0; j <= i; ++j)
					{
						b0 = JBBox::Union(b0, buckets[j].bounds);
						count0 += buckets[j].count;
					}
					for (int j = i + 1; j < bucketCount; ++j)
					{
						b1 = JBBox::Union(b1, buckets[j].bounds);
						count1 += buckets[j].count;
					}
					//교차값 = 1, 횡단값 = 1 / 8
					cost[i] = traverseRate + (count0 * b0.Surface() + count1 * b1.Surface()) / boundSurface;
				}

				float minCost = cost[0];
				int minCostSplitBucket = 0;
				for (int i = 1; i < bucketCount - 1; ++i)
				{
					if (cost[i] < minCost)
					{
						minCost = cost[i];
						minCostSplitBucket = i;
					}
				}
				JGameObject** pmid = std::partition(&objectList[start], &objectList[end - 1] + 1,
					[=](JGameObject* gameObj)
					{
						float centroid = GetDimensionValue(gameObj->GetRenderItem()->GetBoundingBox().Center, dim);
						int b = (int)(bucketCount * ((centroid - centroidMinDim) / (centroidMaxDim - centroidMinDim)));
						if (b == bucketCount)
							b = bucketCount - 1;
						return b <= minCostSplitBucket;
					});
				int mid = (int)(pmid - &objectList[0]);
				if (mid == end)
					mid = (start + end) / 2;

				JBBox leftBv(XMFLOAT3(Inf, Inf, Inf), XMFLOAT3(-Inf, -Inf, -Inf));
				JBBox rightBv(XMFLOAT3(Inf, Inf, Inf), XMFLOAT3(-Inf, -Inf, -Inf));
				for (int i = start; i < mid; ++i)
					leftBv = JBBox::Union(leftBv, objectList[i]->GetRenderItem()->GetBoundingBox());
				for (int i = mid; i < end; ++i)
					rightBv = JBBox::Union(rightBv, objectList[i]->GetRenderItem()->GetBoundingBox());

				if (mid - start <= minObjectPerLeaf)
					nodeVec.emplace_back(std::make_unique<JBvhNode>((uint)nodeVec.size() + numberOffset, J_BVH_NODE_TYPE::LEAF, leftBv.Convert(), parent, objectList[start], true));
				else
					nodeVec.emplace_back(std::make_unique<JBvhNode>((uint)nodeVec.size() + numberOffset, J_BVH_NODE_TYPE::NODE, leftBv.Convert(), parent, nullptr, true));
				BuildTopdownBvhBySAH(parent->GetLeftNode(), objectList, nodeVec, start, mid, numberOffset);

				if (end - mid <= minObjectPerLeaf)
					nodeVec.emplace_back(std::make_unique<JBvhNode>((uint)nodeVec.size() + numberOffset, J_BVH_NODE_TYPE::LEAF, rightBv.Convert(), parent, objectList[mid], false));
				else
					nodeVec.emplace_back(std::make_unique<JBvhNode>((uint)nodeVec.size() + numberOffset, J_BVH_NODE_TYPE::NODE, rightBv.Convert(), parent, nullptr, false));
				BuildTopdownBvhBySAH(parent->GetRightNode(), objectList, nodeVec, mid, end, numberOffset);

				//int mid = Partition(objectList, bucketCount, minCostSplitBucket, dim, start, end, centroidMinDim, centroidMaxDim);
			}
		}
		void JBvh::ReBuildBvh(const uint nodeNumber, JGameObject* additionalGameObj)noexcept
		{
			std::vector<JGameObject*> objectList;
			if (additionalGameObj != nullptr)
				objectList.push_back(additionalGameObj);

			JBvhNode* tarNode = allNodes[nodeNumber].get();
			JBvhNode* tarNodeLeft = tarNode->GetLeftNode();
			JBvhNode* tarNodeRight = tarNode->GetRightNode();
			uint tarNodeStNumber;
			uint tarNodeEndNumber;
			if (tarNode->GetNodeType() == J_BVH_NODE_TYPE::LEAF)
			{
				if (additionalGameObj == nullptr)
					return;

				tarNodeStNumber = tarNode->GetNodeNumber() + 1;
				tarNode->SetNodeType(J_BVH_NODE_TYPE::NODE);
				leafNodeMap.erase(tarNode->GetInnerGameObject()->GetGuid());
				objectList.push_back(tarNode->GetInnerGameObject());
				tarNode->SetInnerGameObject(nullptr);
			}
			else
			{
				if (allNodes.size() == 1)
				{
					if (additionalGameObj == nullptr)
						return;

					if (innerGameObjectCandidate == nullptr)
					{
						innerGameObjectCandidate = additionalGameObj;
						return;
					}
					else
					{
						tarNodeStNumber = 1;
						objectList.push_back(innerGameObjectCandidate);
						innerGameObjectCandidate = nullptr;
					}
				}
				else
				{
					tarNodeStNumber = tarNodeLeft->GetNodeNumber();
					tarNodeEndNumber = tarNode->GetRightNumberEnd() + 1;

					for (uint i = tarNodeStNumber; i < tarNodeEndNumber; ++i)
					{
						if (allNodes[i]->GetNodeType() == J_BVH_NODE_TYPE::LEAF)
						{
							JGameObject* innerGameObject = allNodes[i]->GetInnerGameObject();
							if (innerGameObject != nullptr)
								objectList.push_back(innerGameObject);
						}
					}

					if (tarNodeLeft->GetParentNode() != nullptr)
						ClearBvhNode(tarNodeLeft->GetNodeNumber());
					if (tarNodeRight->GetParentNode() != nullptr)
						ClearBvhNode(tarNodeRight->GetNodeNumber());

					tarNode->SetLeftNode(nullptr);
					tarNode->SetRightNode(nullptr);

					allNodes.erase(allNodes.begin() + tarNodeStNumber, allNodes.begin() + tarNodeEndNumber);
				}
			}

			std::vector<std::unique_ptr<JBvhNode>> newNodeVec;
			switch (buildType)
			{
			case J_BVH_BUILD_TYPE::TOP_DOWN:
			{
				switch (splitType)
				{
				case J_BVH_SPLIT_TYPE::SAH:
					BuildTopdownBvhBySAH(tarNode, objectList, newNodeVec, 0, (int)objectList.size(), tarNode->GetNodeNumber() + 1);
					break;
				default:
					break;
				}
			}
			break;
			case J_BVH_BUILD_TYPE::BOTTOP_UP:
				break;
			default:
				break;
			}
			allNodes.insert(allNodes.begin() + tarNodeStNumber,
				std::make_move_iterator(newNodeVec.begin()),
				std::make_move_iterator(newNodeVec.end()));

			if (onDebugObject)
				OnDebugGameObject(debugRoot, makeOnlyLeafNodeDebugObject);
		}
		void JBvh::ClearBvhNode(const uint nodeNumber)noexcept
		{
			const uint nodeNumberEnd = allNodes[nodeNumber]->GetRightNumberEnd();
			for (uint i = nodeNumber; i <= nodeNumberEnd; ++i)
			{
				if (allNodes[i]->GetNodeType() == J_BVH_NODE_TYPE::LEAF)
				{
					JGameObject* innerGameObject = allNodes[i]->GetInnerGameObject();
					leafNodeMap.erase(innerGameObject->GetGuid());
				}
				allNodes[i]->Clear();
			}
		}
		void JBvh::EraseBvhNode(const uint nodeNumber)noexcept
		{
			if (nodeNumber == 0)
				return;

			JBvhNode* parentNode = allNodes[nodeNumber]->GetParentNode();
			if (parentNode->GetNodeType() == J_BVH_NODE_TYPE::ROOT)
			{
				const uint allNodeCount = (uint)allNodes.size();
				if (allNodeCount == 3)
				{
					if (allNodes[nodeNumber]->IsLeftNode())
						innerGameObjectCandidate = parentNode->GetRightNode()->GetInnerGameObject();
					else
						innerGameObjectCandidate = parentNode->GetLeftNode()->GetInnerGameObject();

					ClearBvhNode(root->GetLeftNode()->GetNodeNumber());
					ClearBvhNode(root->GetRightNode()->GetNodeNumber());
					allNodes.erase(allNodes.begin() + 1, allNodes.end());
				}
				else
				{
					//트리 재구축
					//한쪽 자식들에 자원을 모두 해재하고 다른 쪽 자식들 자원을 가지고 재구축
					ClearBvhNode(nodeNumber);
					ReBuildBvh(0);
				}
			}
			else
			{
				ClearBvhNode(nodeNumber);
				JBvhNode* grandParentNode = parentNode->GetParentNode();
				if (parentNode->IsLeftNode())
				{
					if (allNodes[nodeNumber]->IsLeftNode())
						grandParentNode->SetLeftNode(parentNode->GetRightNode());
					else
						grandParentNode->SetLeftNode(parentNode->GetLeftNode());
				}
				else
				{
					if (allNodes[nodeNumber]->IsLeftNode())
						grandParentNode->SetRightNode(parentNode->GetRightNode());
					else
						grandParentNode->SetRightNode(parentNode->GetLeftNode());
				}
				parentNode->SetLeftNode(nullptr);
				parentNode->SetRightNode(nullptr);
				ClearBvhNode(parentNode->GetNodeNumber());

				const uint parentNumber = parentNode->GetNodeNumber();
				const uint childNodeNumberEnd = allNodes[nodeNumber]->GetRightNumberEnd();

				allNodes.erase(allNodes.begin() + parentNumber, allNodes.begin() + parentNumber + 1);
				allNodes.erase(allNodes.begin() + nodeNumber - 1, allNodes.begin() + childNodeNumberEnd);

				const uint allNodeCount = (uint)allNodes.size();
				for (uint i = parentNumber; i < allNodeCount; ++i)
					allNodes[i]->SetNodeNumber(i);
			}
		}
	}
}