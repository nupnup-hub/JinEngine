#include"JBvh.h"
#include"../../../../GameObject/JGameObject.h"
#include"../../../../Component/RenderItem/JRenderItem.h"
#include"../../../../Component/RenderItem/JRenderItemPrivate.h"
#include"../../../../../Core/Geometry/JBBox.h"
#include"../../../../../Core/Geometry/JDirectXCollisionEx.h"
#include"../../../../../Core/Math/JMathHelper.h"  
#include"../../../../../Editor/EditTool/JEditorViewStructure.h"
#include"../../../../../Graphic/Culling/JCullingInterface.h"
#include <algorithm>

namespace JinEngine
{
	using namespace DirectX;
	namespace
	{
		using RItemFrameIndexInteface = JRenderItemPrivate::FrameIndexInterface;
		static constexpr uint useCandidateCount = 3;
	}

	struct BucketInfo
	{
		int count = 0;
		Core::JBBox bounds = Core::JBBox::InfBBox();
	};

	JBvh::JBvh(const J_ACCELERATOR_LAYER layer)
		:JAccelerator(layer)
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

		std::vector<JUserPtr<JGameObject>> objectList = GetInnerObject();
		if (objectList.size() > 0)
		{
			if (objectList.size() == 1)
				innerGameObjectCandidate = objectList[0];
			else
			{
				switch (buildType)
				{
				case J_ACCELERATOR_BUILD_TYPE::TOP_DOWN:
				{
					BuildTopdownBvh(root, objectList, allNodes, 0, (int)objectList.size(), 0);
					break;
				}
				//case J_ACCELERATOR_BUILD_TYPE::BOTTOP_UP:
				//	break;
				default:
					break;
				}

				const uint allNodeCount = (uint)allNodes.size();
				for (uint i = 0; i < allNodeCount; ++i)
				{
					if (IsDebugActivated())
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
		JAccelerator::Clear();
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
	void JBvh::Culling(JAcceleratorCullingInfo& info)noexcept
	{ 
		if (allNodes.size() > 1)
		{
			if (info.useJFrustum)
				root->Culling(info, Core::J_CULLING_FLAG::NONE);
			else
				root->Culling(info);
		}

		if (innerGameObjectCandidate != nullptr)
		{
			Core::J_CULLING_FLAG flag = Core::J_CULLING_FLAG::NONE;
			JUserPtr<JRenderItem> rItem = innerGameObjectCandidate->GetRenderItem();
			if (info.useJFrustum)
			{
				Core::J_CULLING_RESULT res = info.jFrustum.Contain(rItem->GetBoundingBox(), flag);
				if (res == Core::J_CULLING_RESULT::CONTAIN || res == Core::J_CULLING_RESULT::INTERSECT)
					info.cullUser.OffCulling(Graphic::J_CULLING_TYPE::FRUSTUM, RItemFrameIndexInteface::GetBoundingFrameIndex(rItem.Get()));
				else if (res == Core::J_CULLING_RESULT::DISJOINT)
					info.cullUser.SetCulling(Graphic::J_CULLING_TYPE::FRUSTUM, RItemFrameIndexInteface::GetBoundingFrameIndex(rItem.Get()));
			}
			else
			{
				ContainmentType res = info.frustum.Contains(rItem->GetBoundingBox());
				if (res == ContainmentType::CONTAINS || res == ContainmentType::INTERSECTS)
					info.cullUser.OffCulling(Graphic::J_CULLING_TYPE::FRUSTUM, RItemFrameIndexInteface::GetBoundingFrameIndex(rItem.Get()));
				else if (res == ContainmentType::DISJOINT)
					info.cullUser.SetCulling(Graphic::J_CULLING_TYPE::FRUSTUM, RItemFrameIndexInteface::GetBoundingFrameIndex(rItem.Get()));
			}
		}
	}
	void JBvh::Intersect(JAcceleratorIntersectInfo& info)const noexcept
	{
		if (allNodes.size() > 1)
			root->FindIntersect(info);
		else if (innerGameObjectCandidate != nullptr)
		{
			float leftDist = FLT_MAX;
			auto oriBBox = innerGameObjectCandidate->GetRenderItem()->GetOrientedBoundingBox();
			if (!info.allowContainRayPos && oriBBox.Contains(info.ray.PosV()) == DirectX::CONTAINS)
				return;

			if (innerGameObjectCandidate->GetRenderItem()->GetBoundingBox().Intersects(info.ray.PosV(), info.ray.DirV(), leftDist))
			{
				if (info.untilFirst)
					info.firstResult = innerGameObjectCandidate;
				else
					info.resultObjVec.push_back(innerGameObjectCandidate);
			}
		}
	}
	void JBvh::Contain(JAcceleratorContainInfo& info)const noexcept
	{
		if (allNodes.size() > 1)
			root->FindContain(info);
		else if (innerGameObjectCandidate != nullptr)
		{
			auto oriBBox = innerGameObjectCandidate->GetRenderItem()->GetOrientedBoundingBox();
			ContainmentType res = ContainmentType::DISJOINT;
			if (info.toolType == JAcceleratorContainInfo::CONTAIN_TOOL::FRUSTUM)
				res = info.frustum.Contains(oriBBox);
			else if (info.toolType == JAcceleratorContainInfo::CONTAIN_TOOL::OBB)
				res = info.obb.Contains(oriBBox);

			const bool canAdd = info.allowIntersect ? res != ContainmentType::DISJOINT : res == ContainmentType::CONTAINS;
			if (canAdd)
				info.resultObjVec.push_back(innerGameObjectCandidate);
		}
	}
	void JBvh::AlignedObject(JAcceleratorAlignInfo& info, _Out_ std::vector<JUserPtr<JGameObject>>& aligned, _Out_ int& validCount)const noexcept
	{
		if (leafNodeMap.size() != aligned.size())
			aligned.resize(leafNodeMap.size());
		 
		info.acceleratorMaxDepth = JMathHelper::PowerOfTwoExponent(allNodes.size());
		info.alignMaxDepth = info.acceleratorMaxDepth * info.AlignRangeRate();
		validCount = leafNodeMap.size();
		if (allNodes.size() > 1)
		{
			uint index = 0;
			root->AlignLeafNode(info, aligned, index, 0);
		}
		else if (innerGameObjectCandidate != nullptr)
			aligned[0] = innerGameObjectCandidate;
	}
	void JBvh::UpdateGameObject(const JUserPtr<JGameObject>& gameObject)noexcept
	{
		auto leafNode = leafNodeMap.find(gameObject->GetGuid());
		if (leafNode != leafNodeMap.end())
		{
			leafNode->second->UpdateInnerGameObject();
			BoundingBox boundBox = leafNode->second->GetInnerGameObject()->GetRenderItem()->GetBoundingBox();

			if (!leafNode->second->GetParentNode()->IsContainNode(boundBox))
			{
				JBvhNode* containNode = leafNode->second->GetParentNode()->GetContainNodeToRoot(boundBox);
				if (containNode == nullptr)
					ReBuildBvh(0);
				else
					ReBuildBvh(containNode->GetNodeNumber());
			}
		}
	}
	void JBvh::AddGameObject(const JUserPtr<JGameObject>& newGameObject)noexcept
	{
		JBvhNode* containNode = root->GetContainNodeToLeaf(newGameObject->GetRenderItem()->GetBoundingBox());
		if (containNode != nullptr)
			ReBuildBvh(containNode->GetNodeNumber(), newGameObject);
	}
	void JBvh::RemoveGameObject(const JUserPtr<JGameObject>& gameObj)noexcept
	{
		auto leafNode = leafNodeMap.find(gameObj->GetGuid());
		if (leafNode != leafNodeMap.end())
			DestroyBvhNode(leafNode->second->GetNodeNumber());
		else if (innerGameObjectCandidate != nullptr && innerGameObjectCandidate->GetGuid() == gameObj->GetGuid())
			innerGameObjectCandidate = nullptr;

		if (IsDebugRoot(gameObj))
		{
			JAcceleratorOption option = GetCommonOption();
			option.debugRoot = nullptr;
			SetCommonOption(option);
		}
		if (IsInnerRoot(gameObj))
		{
			JAcceleratorOption option = GetCommonOption();
			option.innerRoot = nullptr;
			SetCommonOption(option);
		}
	}
	J_ACCELERATOR_TYPE JBvh::GetType()const noexcept
	{
		return J_ACCELERATOR_TYPE::BVH;
	}
	Core::JBBox JBvh::GetSceneBBox()const noexcept
	{
		if (root == nullptr)
			return Core::JBBox();
		else
		{
			if (allNodes.size() > useCandidateCount)
				return Core::JBBox::Union(root->GetLeftNode()->GetBoundingBox(), root->GetRightNode()->GetBoundingBox());
			else
			{
				if (innerGameObjectCandidate != nullptr)
					return innerGameObjectCandidate->GetRenderItem()->GetBoundingBox();
				else
					return Core::JBBox();
			}
		}
	}
	uint JBvh::GetNodeCount()const noexcept
	{
		return (uint)allNodes.size();
	}
	JBvhOption JBvh::GetBvhOption()const noexcept
	{
		return JBvhOption(buildType, splitType, JAccelerator::GetCommonOption());
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
				if (IsAcceleratorActivated())
				{
					UnBuild();
					Build();
					if (IsDebugActivated())
						OnDebugGameObject();
				}
			}
		}
	}
	void JBvh::BuildTopdownBvh(JBvhNode* parent,
		std::vector<JUserPtr<JGameObject>>& objectList,
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
			Core::JBBox bound = Core::JBBox::InfBBox();
			for (int i = start; i < end; ++i)
				bound = Core::JBBox::Union(bound, objectList[i]->GetRenderItem()->GetBoundingBox());

			Core::JBBox centroidBound = Core::JBBox::InfBBox();
			for (int i = start; i < end; ++i)
				centroidBound = Core::JBBox::Union(centroidBound, objectList[i]->GetRenderItem()->GetBoundingBox().Center);

			int dim = centroidBound.MaxDimension();
			constexpr int bucketCount = 12;
			BucketInfo buckets[bucketCount];

			float centroidMinDim = centroidBound.minP[dim];
			float centroidMaxDim = centroidBound.maxP[dim];
			float centroidSub = centroidMaxDim - centroidMinDim;

			switch (splitType)
			{
			case JinEngine::J_ACCELERATOR_SPLIT_TYPE::SAH:
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
					buckets[b].bounds = Core::JBBox::Union(buckets[b].bounds, objectList[i]->GetRenderItem()->GetBoundingBox());
				}
				break;
			}
			default:
			{
				return;
			}
			}
			const float traverseRate = 0.125f;
			const float nanFactor = 1.0f;
			const float maxCost = FLT_MAX;
			float boundSurface = bound.Surface();
			float cost[bucketCount - 1];

			//최적에 중앙값 결정
			for (int i = 0; i < bucketCount - 1; ++i)
			{
				Core::JBBox b0 = Core::JBBox::InfBBox();
				Core::JBBox b1 = Core::JBBox::InfBBox();

				int count0 = 0;
				int count1 = 0;

				for (int j = 0; j <= i; ++j)
				{
					b0 = Core::JBBox::Union(b0, buckets[j].bounds);
					count0 += buckets[j].count;
				}
				for (int j = i + 1; j < bucketCount; ++j)
				{
					b1 = Core::JBBox::Union(b1, buckets[j].bounds);
					count1 += buckets[j].count;
				}
				//교차값 = 1, 횡단값 = 1 / 8
				cost[i] = traverseRate + (count0 * b0.Surface() + count1 * b1.Surface()) / boundSurface;
				if (std::isnan(cost[i]))
					cost[i] = maxCost;
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

			JUserPtr<JGameObject>* pmid = std::partition(&objectList[start], &objectList[end - 1] + 1,
				[=](JUserPtr<JGameObject> gameObj)
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

			Core::JBBox leftBv = Core::JBBox::InfBBox();
			Core::JBBox rightBv = Core::JBBox::InfBBox();
			for (int i = start; i < mid; ++i)
				leftBv = Core::JBBox::Union(leftBv, objectList[i]->GetRenderItem()->GetBoundingBox());
			for (int i = mid; i < end; ++i)
				rightBv = Core::JBBox::Union(rightBv, objectList[i]->GetRenderItem()->GetBoundingBox());

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
	void JBvh::ReBuildBvh(const uint nodeNumber, const JUserPtr<JGameObject>& additionalGameObj)noexcept
	{
		std::vector<JUserPtr<JGameObject>> objectList;
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
						JUserPtr<JGameObject> innerGameObject = allNodes[i]->GetInnerGameObject();
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

		if (objectList.size() > 0)
		{
			std::vector<std::unique_ptr<JBvhNode>> newNodeVec;
			switch (buildType)
			{
			case J_ACCELERATOR_BUILD_TYPE::TOP_DOWN:
			{
				BuildTopdownBvh(tarNode, objectList, newNodeVec, 0, (int)objectList.size(), tarNode->GetNodeNumber() + 1);
				break;
			}
			break;
			//case J_ACCELERATOR_BUILD_TYPE::BOTTOP_UP:
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
		} 
		 
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
				JUserPtr<JGameObject> innerGameObject = allNodes[i]->GetInnerGameObject();
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
			if (allNodeCount == useCandidateCount)
			{
				//이진트리 유지를 위해 루트 자식 노드를 삭제하고
				//포함된 게임오브젝트를 후보로 캐싱
				if (allNodes[nodeNumber]->IsLeftNode())
					innerGameObjectCandidate = parentNode->GetRightNode()->GetInnerGameObject();
				else
					innerGameObjectCandidate = parentNode->GetLeftNode()->GetInnerGameObject();

				ClearBvhNode(root->GetLeftNode()->GetNodeNumber());
				ClearBvhNode(root->GetRightNode()->GetNodeNumber());
				root->SetLeftNode(nullptr);
				root->SetRightNode(nullptr);
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

		treeView.ClearNode();
		treeView.SetGridSize(2500);
		root->BuildDebugNode(treeView);
	}
}