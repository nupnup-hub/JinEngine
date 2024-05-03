#include"JOctree.h"  
#include"../../../../GameObject/JGameObject.h" 
#include"../../../../Component/RenderItem/JRenderItem.h"
#include"../../../../../Core/Geometry/JBBox.h"
#include"../../../../../Core/Math/JMathHelper.h"  
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

	JOctree::JOctree(const J_ACCELERATOR_LAYER layer)
		:JCpuAccelerator(layer)
	{}
	JOctree::~JOctree() {}
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
		JCpuAccelerator::Clear();
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
	void JOctree::Culling(JAcceleratorCullingInfo& info)noexcept
	{
		if (rootNodeCash != nullptr)
		{
			if (info.useJFrustum)
				;// rootNodeCash->Culling(info, Core::J_CULLING_FLAG::NONE);
			else
				rootNodeCash->Culling(info);
		}
	}
	void JOctree::Intersect(JAcceleratorIntersectInfo& info)const noexcept
	{
		if (rootNodeCash == nullptr)
			return;

		rootNodeCash->FindIntersect(info);
	}
	void JOctree::Contain(JAcceleratorContainInfo& info)const noexcept
	{
		if (rootNodeCash == nullptr)
			return;

		rootNodeCash->FindContain(info);
	}
	void JOctree::AlignedObject(JAcceleratorAlignInfo& info, _Out_ std::vector<JUserPtr<JGameObject>>& aligned, _Out_ int& validCount)const noexcept
	{
		if (rootNodeCash == nullptr)
			return; 

		uint index = 0;
		rootNodeCash->AlignLeafNode(info, aligned, index);
		validCount = index;
	}
	void JOctree::UpdateGameObject(const JUserPtr<JGameObject>& gameObject)noexcept
	{
		if (!IsValidLayer(gameObject->GetRenderItem()->GetRenderLayer()))
			return;

		auto octNode = containNodeMap.find(gameObject->GetGuid());
		if (octNode != containNodeMap.end())
		{
			RemoveGameObject(gameObject);
			AddGameObject(gameObject);
		}
	}
	void JOctree::AddGameObject(const JUserPtr<JGameObject>& newGameObject)noexcept
	{
		JUserPtr<JRenderItem> rItem = newGameObject->GetRenderItem();
		JOctreeNode* octNode = FindOptimalNode(rootNodeCash, rItem->GetOrientedBoundingBox());
		if (octNode == nullptr)
			return;

		if (octNode->AddGameObject(newGameObject, isLooseOctree))
		{
			containNodeMap.emplace(newGameObject->GetGuid(), octNode);
			if (IsDebugActivated())
				octNode->CreateDebugGameObject(GetDebugRoot(), IsDebugLeafOnly());
		}
	}
	void JOctree::RemoveGameObject(const JUserPtr<JGameObject>& gameObj)noexcept
	{
		auto octNode = containNodeMap.find(gameObj->GetGuid());
		if (octNode != containNodeMap.end())
		{
			octNode->second->RemoveGameObject(gameObj);
			containNodeMap.erase(gameObj->GetGuid());
		}
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
	J_ACCELERATOR_TYPE JOctree::GetType()const noexcept
	{
		return J_ACCELERATOR_TYPE::OCTREE;
	}
	Core::JBBox JOctree::GetSceneBBox()const noexcept
	{
		if (rootNodeCash == nullptr)
			return Core::JBBox();
		else
			return rootNodeCash->GetBoundingBox();
	}
	JOctreeOption JOctree::GetOctreeOption()const noexcept
	{
		return JOctreeOption(minSize, octreeSizeSquare, looseFactor, GetCommonOption());
	}
	uint JOctree::GetExtentLength(const uint depth)const noexcept
	{
		return (uint)(octreeSize / (1 << depth));
	}
	uint JOctree::GetLooseExtentLength(const uint depth)const noexcept
	{
		return (uint)((looseFactor * octreeSize) / (1 << depth));
	}
	void JOctree::SetOctreeOption(const JOctreeOption& newOption)noexcept
	{
		JOctreeOption preOption = GetOctreeOption();
		minSize = newOption.minSize;
		octreeSizeSquare = newOption.octreeSizeSquare;
		octreeSize = 1 << octreeSizeSquare;
		looseFactor = std::clamp(newOption.looseFactor, looseFactorMin, looseFactorMax);

		if (!preOption.EqualCommonOption(newOption))
			SetCommonOption(newOption.commonOption);
		else
		{
			if (!preOption.EqualOctreeOption(newOption))
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
	void JOctree::BuildOctree()noexcept
	{
		std::vector<JUserPtr<JGameObject>> gameObject = GetInnerObject();
		if (rootNodeCash != nullptr)
		{
			rootNodeCash->Clear();
			rootNodeCash = nullptr;
		}
		allNode.clear();

		/* old version root bbox
		float octreeHalfSize = octreeSize * 0.5f;
		BoundingBox rootBoundingBox;

		XMFLOAT3 maxVector(octreeHalfSize, octreeHalfSize, octreeHalfSize);
		XMFLOAT3 minVector(-octreeHalfSize, -octreeHalfSize, -octreeHalfSize);

		XMStoreFloat3(&rootBoundingBox.Center, 0.5f * (XMLoadFloat3(&maxVector) + XMLoadFloat3(&minVector)));
		XMStoreFloat3(&rootBoundingBox.Extents, 0.5f * (XMLoadFloat3(&maxVector) - XMLoadFloat3(&minVector)));
		*/

		Core::JBBox rootBound = Core::JBBox::InfBBox();
		for (const auto& data : gameObject)
			rootBound = Core::JBBox::Union(rootBound, data->GetRenderItem()->GetBoundingBox());

		const uint xPower = JMathHelper::PowerOfTwoExponent(rootBound.DistancePoint(0));
		const uint yPower = JMathHelper::PowerOfTwoExponent(rootBound.DistancePoint(1));
		const uint zPower = JMathHelper::PowerOfTwoExponent(rootBound.DistancePoint(2));
		const uint base = pow(2, max(max(xPower, yPower), zPower) + 1);
		const float additionalX = (base - rootBound.DistancePoint(0)) / 2.0f;
		const float additionalY = (base - rootBound.DistancePoint(1)) / 2.0f;
		const float additionalZ = (base - rootBound.DistancePoint(2)) / 2.0f;

		rootBound.Lengthen(JVector3<float>(additionalX, additionalY, additionalZ));
		std::unique_ptr<JOctreeNode> root = std::make_unique<JOctreeNode>(rootBound.Convert(), isLooseOctree);
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
			auto raw = cNode[i].get();
			allNode.push_back(std::move(cNode[i]));
			BuildOctreeNode(raw, depth + 1);
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
	JOctreeNode* JOctree::FindOptimalNode(JOctreeNode* node, const DirectX::BoundingOrientedBox& tarBBox)noexcept
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
}
