#include"JBvh.h"
#include"../../Geometry/JBBox.h"
#include"../../Geometry/JDirectXCollisionEx.h"
#include"../../../Object/GameObject/JGameObject.h"
#include"../../../Object/Component/RenderItem/JRenderItem.h"
#include"../../../Editor/Utility/JEditorBinaryTreeView.h"
#include <algorithm>

namespace JinEngine
{
	using namespace DirectX;
	namespace Core
	{
		struct BucketInfo
		{
			int count = 0;
			JBBox bounds = JBBox::InfBBox();
		};

		JBvh::JBvh(const J_SPACE_SPATIAL_LAYER layer)
			:JSpaceSpatial(layer)
		{}
		JBvh::~JBvh() {}
		void JBvh::Build()noexcept
		{
			if (root != nullptr)
				assert("JBvh Build Error");

			allNodes.emplace_back(std::make_unique<JBvhNode>(0,
				J_BVH_NODE_TYPE::ROOT, GetRootBoundingBox(),
				nullptr,
				nullptr,
				false));
			root = allNodes[0].get();

			std::vector<JGameObject*> objectList = GetInnerObject();
			if (objectList.size() > 0)
			{
				if (objectList.size() == 1)
					innerGameObjectCandidate = objectList[0];
				else
				{
					switch (buildType)
					{
					case J_SPACE_SPATIAL_BUILD_TYPE::TOP_DOWN:
					{
						BuildTopdownBvh(root, objectList, allNodes, 0, (int)objectList.size(), 0);
						break;
					}
					//case J_SPACE_SPATIAL_BUILD_TYPE::BOTTOP_UP:
					//	break;
					default:
						break;
					}

					const uint allNodeCount = (uint)allNodes.size();
					if (IsDebugActivated())
					{
						for (uint i = 0; i < allNodeCount; ++i)
							allNodes[i]->CreateDebugGameObject(GetDebugRoot(), IsDebugLeafOnly());
					}
				}
			}
		}
		void JBvh::UnBuild()noexcept
		{
			const int allNodeCount = (int)allNodes.size();
			for (int i = 0; i < allNodeCount; ++i)
				allNodes[i]->Clear();
			root = nullptr;
			allNodes.clear();
			leafNodeMap.clear();
			innerGameObjectCandidate = nullptr;
		}
		void JBvh::Clear()noexcept
		{
			UnBuild();
			JSpaceSpatial::Clear();
		}
		void JBvh::OnDebugGameObject()noexcept
		{
			const int allNodeCount = (int)allNodes.size();
			for (int i = 0; i < allNodeCount; ++i)
				allNodes[i]->CreateDebugGameObject(GetDebugRoot(), IsDebugLeafOnly());
		}
		void JBvh::OffDebugGameObject()noexcept
		{
			const int allNodeCount = (int)allNodes.size();
			for (int i = 0; i < allNodeCount; ++i)
				allNodes[i]->DestroyDebugGameObject();
		}
		void JBvh::OffCulling()noexcept
		{
			if (root != nullptr)
				root->OffCulling();
		}
		void JBvh::Culling(const JCullingFrustum& camFrustum)noexcept
		{
			if (allNodes.size() > 1)
				root->Culling(camFrustum, J_CULLING_FLAG::NONE);

			if (innerGameObjectCandidate != nullptr)
			{
				J_CULLING_FLAG flag = J_CULLING_FLAG::NONE;
				JRenderItem* rItem = innerGameObjectCandidate->GetRenderItem();
				J_CULLING_RESULT res = camFrustum.IsBoundingBoxIn(rItem->GetBoundingBox(), flag);
				if (res == J_CULLING_RESULT::CONTAIN)
					rItem->SetRenderVisibility(J_RENDER_VISIBILITY::VISIBLE);
				else if (res == J_CULLING_RESULT::DISJOINT)
					rItem->SetRenderVisibility(J_RENDER_VISIBILITY::INVISIBLE);
			}
		} 
		void JBvh::Culling(const DirectX::BoundingFrustum& camFrustum, const DirectX::FXMVECTOR camPos)noexcept
		{
			if (allNodes.size() > 1)
				root->Culling(camFrustum, camPos);

			if (innerGameObjectCandidate != nullptr)
			{
				J_CULLING_FLAG flag = J_CULLING_FLAG::NONE;
				JRenderItem* rItem = innerGameObjectCandidate->GetRenderItem();
				ContainmentType res = camFrustum.Contains(rItem->GetBoundingBox());
				if (res == ContainmentType::CONTAINS)
					rItem->SetRenderVisibility(J_RENDER_VISIBILITY::VISIBLE);
				else if (res == ContainmentType::DISJOINT)
					rItem->SetRenderVisibility(J_RENDER_VISIBILITY::INVISIBLE);
			}
		}
		JGameObject* JBvh::IntersectFirst(const JRay& ray)const noexcept
		{
			if (root != nullptr)
				return root->IntersectFirst(ray.GetPosV(), ray.GetDirV());
			else
				return nullptr;
		}
		void JBvh::Intersect(const JRay& ray, const J_SPACE_SPATIAL_SORT_TYPE sortType, _Out_ std::vector<JGameObject*>& res)const noexcept
		{
			if (root != nullptr)
			{ 
				if (sortType == J_SPACE_SPATIAL_SORT_TYPE::ASCENDING)
					root->IntersectAscendingSort(ray.GetPosV(), ray.GetDirV(), res);
				else if (sortType == J_SPACE_SPATIAL_SORT_TYPE::DESCENDING)
					root->IntersectDescendingSort(ray.GetPosV(), ray.GetDirV(), res);
				else
					root->Intersect(ray.GetPosV(), ray.GetDirV(), res);
			}
		}
		void JBvh::UpdateGameObject(JGameObject* gameObject)noexcept
		{ 
			auto leafNode = leafNodeMap.find(gameObject->GetGuid());
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
		void JBvh::RemoveGameObject(JGameObject* gameObj)noexcept
		{
			auto leafNode = leafNodeMap.find(gameObj->GetGuid());
			if (leafNode != leafNodeMap.end())
				DestroyBvhNode(leafNode->second->GetNodeNumber());
			else if (innerGameObjectCandidate != nullptr && innerGameObjectCandidate->GetGuid() == gameObj->GetGuid())
				innerGameObjectCandidate = nullptr;

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
		J_SPACE_SPATIAL_TYPE JBvh::GetType()const noexcept
		{
			return J_SPACE_SPATIAL_TYPE::BVH;
		}
		uint JBvh::GetNodeCount()const noexcept
		{
			return allNodes.size();
		}
		JBvhOption JBvh::GetBvhOption()const noexcept
		{
			return JBvhOption(buildType, splitType, JSpaceSpatial::GetCommonOption());
		}
		void JBvh::SetBvhOption(const JBvhOption& newOption)noexcept
		{
			JBvhOption preOption = GetBvhOption();
			buildType = newOption.buildType;
			splitType = newOption.splitType;

			if (!preOption.EqualCommonOption(newOption))
				SetCommonOption(newOption.commonOption);
			else
			{
				if (!preOption.EqualBvhOption(newOption))
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
		DirectX::BoundingBox JBvh::GetRootBoundingBox()const noexcept
		{
			constexpr int rootBvSize = 1 << 16;
			return BoundingBox(XMFLOAT3(0, 0, 0), XMFLOAT3(rootBvSize, rootBvSize, rootBvSize));
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
		void JBvh::BuildTopdownBvh(JBvhNode* parent,
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
				JBBox bound = JBBox::InfBBox();
				for (int i = start; i < end; ++i)
					bound = JBBox::Union(bound, objectList[i]->GetRenderItem()->GetBoundingBox(true));

				JBBox centroidBound = JBBox::InfBBox();
				for (int i = start; i < end; ++i)
					centroidBound = JBBox::Union(centroidBound, objectList[i]->GetRenderItem()->GetBoundingBox().Center);

				int dim = centroidBound.MaxDimension();
				constexpr int bucketCount = 12;
				BucketInfo buckets[bucketCount];

				float centroidMinDim = centroidBound.min[dim];
				float centroidMaxDim = centroidBound.max[dim];
				float centroidSub = centroidMaxDim - centroidMinDim;

				switch (splitType)
				{
				case JinEngine::Core::J_SPACE_SPATIAL_SPLIT_TYPE::SAH:
				{
					//인접한 bbox들에 center를 포함하는 bbox생성
					for (int i = start; i < end; ++i)
					{
						float centroid = GetDimensionValue(objectList[i]->GetRenderItem()->GetBoundingBox().Center, dim);
						int b = 0;
						if (centroidSub != 0)
						{
							//b is 0 ~ (bucketCount - 1) 
							b = bucketCount * ((centroid - centroidMinDim) / centroidSub);
						}
						if (b >= bucketCount)
							b = bucketCount - 1;
						buckets[b].count++;
						buckets[b].bounds = JBBox::Union(buckets[b].bounds, objectList[i]->GetRenderItem()->GetBoundingBox());
					}
					break;
				}
				default:
				{
					return;
				}
				}
				constexpr float traverseRate = 0.125f;

				float boundSurface = bound.Surface();
				float cost[bucketCount - 1];

				//최적에 중앙값 결정
				for (int i = 0; i < bucketCount - 1; ++i)
				{
					JBBox b0 = JBBox::InfBBox();
					JBBox b1 = JBBox::InfBBox();

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

				JBBox leftBv = JBBox::InfBBox();
				JBBox rightBv = JBBox::InfBBox();
				for (int i = start; i < mid; ++i)
					leftBv = JBBox::Union(leftBv, objectList[i]->GetRenderItem()->GetBoundingBox());
				for (int i = mid; i < end; ++i)
					rightBv = JBBox::Union(rightBv, objectList[i]->GetRenderItem()->GetBoundingBox());

				if (mid - start <= minObjectPerLeaf)
					nodeVec.emplace_back(std::make_unique<JBvhNode>((uint)nodeVec.size() + numberOffset, J_BVH_NODE_TYPE::LEAF, leftBv.Convert(), parent, objectList[start], true));
				else
					nodeVec.emplace_back(std::make_unique<JBvhNode>((uint)nodeVec.size() + numberOffset, J_BVH_NODE_TYPE::NODE, leftBv.Convert(), parent, nullptr, true));
				BuildTopdownBvh(parent->GetLeftNode(), objectList, nodeVec, start, mid, numberOffset);

				if (end - mid <= minObjectPerLeaf)
					nodeVec.emplace_back(std::make_unique<JBvhNode>((uint)nodeVec.size() + numberOffset, J_BVH_NODE_TYPE::LEAF, rightBv.Convert(), parent, objectList[mid], false));
				else
					nodeVec.emplace_back(std::make_unique<JBvhNode>((uint)nodeVec.size() + numberOffset, J_BVH_NODE_TYPE::NODE, rightBv.Convert(), parent, nullptr, false));
				BuildTopdownBvh(parent->GetRightNode(), objectList, nodeVec, mid, end, numberOffset);

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
				leafNodeMap.erase(tarNode->GetInnerGameObject()->GetGuid());
				objectList.push_back(tarNode->GetInnerGameObject());
				tarNode->SetNodeType(J_BVH_NODE_TYPE::NODE);
				tarNode->SetInnerGameObject(nullptr);
				tarNode->DestroyDebugGameObject();
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
			case J_SPACE_SPATIAL_BUILD_TYPE::TOP_DOWN:
			{
				BuildTopdownBvh(tarNode, objectList, newNodeVec, 0, (int)objectList.size(), tarNode->GetNodeNumber() + 1);
				break;
			}
			break;
			//case J_SPACE_SPATIAL_BUILD_TYPE::BOTTOP_UP:
			//	break;
			default:
				break;
			}
			const uint newNodeCount = (uint)newNodeVec.size();
			if (IsDebugActivated())
			{
				for (uint i = 0; i < newNodeCount; ++i)
					newNodeVec[i]->CreateDebugGameObject(GetDebugRoot(), IsDebugLeafOnly());
			}

			allNodes.insert(allNodes.begin() + tarNodeStNumber,
				std::make_move_iterator(newNodeVec.begin()),
				std::make_move_iterator(newNodeVec.end()));

			const uint allNodeCount = (uint)allNodes.size();
			for (uint i = 0; i < allNodeCount; ++i)
				allNodes[i]->SetNodeNumber(i);
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
		void JBvh::DestroyBvhNode(const uint nodeNumber)noexcept
		{
			if (nodeNumber == 0)
				return;

			JBvhNode* parentNode = allNodes[nodeNumber]->GetParentNode();
			if (parentNode->GetNodeType() == J_BVH_NODE_TYPE::ROOT)
			{
				const uint allNodeCount = (uint)allNodes.size();
				if (allNodeCount == 3)
				{
					//이진트리 유지를 위해 루트 자식 노드를 삭제하고
					//포함된 게임오브젝트를 후보로 캐싱
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
				JBvhNode* grandParentNode = parentNode->GetParentNode();
				if (parentNode->IsLeftNode())
				{
					if (allNodes[nodeNumber]->IsLeftNode())
					{
						grandParentNode->SetLeftNode(parentNode->GetRightNode());
						parentNode->SetRightNode(nullptr);
					}
					else
					{
						grandParentNode->SetLeftNode(parentNode->GetLeftNode());
						parentNode->SetLeftNode(nullptr);
					}
				}
				else
				{
					if (allNodes[nodeNumber]->IsLeftNode())
					{
						grandParentNode->SetRightNode(parentNode->GetRightNode());
						parentNode->SetRightNode(nullptr);
					}
					else
					{
						grandParentNode->SetRightNode(parentNode->GetLeftNode());
						parentNode->SetLeftNode(nullptr);
					}
				}
				ClearBvhNode(nodeNumber);
				parentNode->Clear();
				//parentNode->SetLeftNode(nullptr);
				//parentNode->SetRightNode(nullptr);
				//ClearBvhNode(parentNode->GetNodeNumber());

				const uint parentNumber = parentNode->GetNodeNumber();
				const uint childNodeNumberEnd = allNodes[nodeNumber]->GetRightNumberEnd();

				allNodes.erase(allNodes.begin() + nodeNumber, allNodes.begin() + childNodeNumberEnd + 1);
				allNodes.erase(allNodes.begin() + parentNumber, allNodes.begin() + parentNumber + 1);

				const uint allNodeCount = (uint)allNodes.size();
				for (uint i = parentNumber; i < allNodeCount; ++i)
					allNodes[i]->SetNodeNumber(i);
			}
		}
		void JBvh::BuildDebugTree(Editor::JEditorBinaryTreeView& treeView)
		{
			if (root == nullptr)
				return;

			treeView.Initialize(allNodes.size());
			root->BuildDebugNode(treeView);
		}
	}
}