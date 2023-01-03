#include"JKdTree.h"
#include"../../Geometry/JBBox.h" 
#include"../../Geometry/JRay.h"
#include"../../../Utility/JMathHelper.h"
#include"../../../Utility/JVector.h"
#include"../../../Object/GameObject/JGameObject.h"
#include"../../../Object/Component/RenderItem/JRenderItem.h"
#include"../../../Editor/Utility/JEditorBinaryTreeView.h"
#include<algorithm>

//Debug
//#include"../../File/JFileIOHelper.h"
//#include"../../../Debug/JDebugTimer.h"

using namespace DirectX;
namespace JinEngine
{
	namespace Core
	{
		enum class J_BOUND_EDGE_TYPE
		{
			START,
			END
		};

		struct BoundEdge
		{
		public:
			float p;
			uint objNumber;
			J_BOUND_EDGE_TYPE edgeType;
		public:
			BoundEdge() = default;
			BoundEdge(const float p, const uint objNumber, const J_BOUND_EDGE_TYPE edgeType)
				:p(p), objNumber(objNumber), edgeType(edgeType)
			{ }
		};

		JKdTree::JKdTree(const J_SPACE_SPATIAL_LAYER layer)
			:JSpaceSpatial(layer)
		{} 
		JKdTree::~JKdTree() {}
		void JKdTree::Build()noexcept
		{
			if (root != nullptr)
				assert("JKdTree Build Error");

			std::vector<JGameObject*> objList = GetInnerObject();
			if (objList.size() > 1)
				BuildKdTree(root, objList, 0);
		}
		void JKdTree::UnBuild()noexcept
		{
			const int allNodeCount = (int)allNodes.size();
			for (int i = 0; i < allNodeCount; ++i)
				allNodes[i]->Clear();
			root = nullptr;
			allNodes.clear();  
		}
		void JKdTree::Clear()noexcept
		{
			UnBuild();
			JSpaceSpatial::Clear();
		}
		void JKdTree::OnDebugGameObject()noexcept
		{
			const int allNodeCount = (int)allNodes.size();
			for (int i = 0; i < allNodeCount; ++i)
				allNodes[i]->CreateDebugGameObject(GetDebugRoot(), IsDebugLeafOnly());
		}
		void JKdTree::OffDebugGameObject()noexcept
		{
			const int allNodeCount = (int)allNodes.size();
			for (int i = 0; i < allNodeCount; ++i)
				allNodes[i]->DestroyDebugGameObject();
		}
		void JKdTree::OffCulling()noexcept
		{
			if (root != nullptr)
				root->OffCulling();
		}
		void JKdTree::Culling(const JCullingFrustum& camFrustum)noexcept
		{
			if (root != nullptr)
				root->Culling(camFrustum, J_CULLING_FLAG::NONE);
		}
		void JKdTree::Culling(const DirectX::BoundingFrustum& camFrustum, const DirectX::FXMVECTOR camPos)noexcept
		{
			if (root != nullptr)
				root->Culling(camFrustum, camPos);
		}
		JGameObject* JKdTree::IntersectFirst(const JRay& ray)const noexcept
		{
			if (root != nullptr)
				return root->IntersectFirst(ray.GetPosV(), ray.GetDirV());
			else
				return nullptr;
		}
		void JKdTree::Intersect(const JRay& ray, const J_SPACE_SPATIAL_SORT_TYPE sortType, _Out_ std::vector<JGameObject*>& res)const noexcept
		{
			if (root != nullptr)
			{  
				std::vector<JGameObject*> innerList;
				std::vector<float> distVec;
				if (sortType == J_SPACE_SPATIAL_SORT_TYPE::ASCENDING)
					root->IntersectAscendingSort(ray.GetPosV(), ray.GetDirV(), innerList, distVec);
				else if (sortType == J_SPACE_SPATIAL_SORT_TYPE::DESCENDING)
					root->IntersectDescendingSort(ray.GetPosV(), ray.GetDirV(), innerList, distVec);
				else
					root->Intersect(ray.GetPosV(), ray.GetDirV(), innerList);

				const uint innerCount = (uint)innerList.size();
				for (uint i = 0; i < innerCount; ++i)
				{
					bool isOverlap = false;
					const size_t guid = innerList[i]->GetGuid();
					const uint resCount = (uint)res.size();
					for (uint j = 0; j < resCount; ++j)
					{
						if (res[j]->GetGuid() == guid)
						{
							isOverlap = true;
							break;
						}
					}
					if (!isOverlap)
						res.push_back(innerList[i]);
				}
			}
		}
		void JKdTree::UpdateGameObject(JGameObject* gameObj)noexcept
		{ 
			if (!IsValidLayer(gameObj->GetRenderItem()->GetRenderLayer()))
				return;
			 
			const BoundingBox bbox = gameObj->GetRenderItem()->GetBoundingBox();
			std::vector<JKdTreeNode*> hitNode;
			FindHasNode(root, gameObj, bbox, hitNode);

			bool hasChanged = false;
			const uint hitNodeCount = (uint)hitNode.size();
			for (uint i = 0; i < hitNodeCount; ++i)
			{
				if (hitNode[i]->GetBoundingBox().Contains(bbox) == ContainmentType::DISJOINT)
				{
					hasChanged = true;
					break;
				}
			}

			if (hasChanged)
			{
				UnBuild();
				Build();
			}
			 
			/*
			std::wofstream stream;
			stream.open(L"D:\\JinWooJung\\KdTreeLog.txt", std::ios::out | std::ios::app);
			JDebugTimer::StartGameTimer();
			JDebugTimer::StopGameTimer();
			JFileIOHelper::StoreAtomicData(stream, L"Find ChangeTime", JDebugTimer::GetElapsedMilliTime());
			if (hasChanged)
			{
				JDebugTimer::StartGameTimer();

				JDebugTimer::StopGameTimer();
				JFileIOHelper::StoreAtomicData(stream, L"Tree Build ms:", JDebugTimer::GetElapsedMilliTime());
			}
			else
				JFileIOHelper::StoreJString(stream, L"don't Build", L" ");
			JFileIOHelper::InputSpace(stream, 1);
			stream.close();
			*/
		}
		void JKdTree::AddGameObject(JGameObject* newGameObject)noexcept
		{  
			JKdTreeNode* containNode = FindContainNode(root, newGameObject->GetRenderItem()->GetBoundingBox());
			if (containNode == nullptr)
			{
				UnBuild();
				Build();
			}
			else
			{
				if (containNode->GetNodeType() == J_KDTREE_NODE_TYPE::LEAF)
					containNode->AddInnerGameObject(newGameObject);
				else
					ReBuildKdTreeNode(containNode->GetNodeNumber(), newGameObject);
			}
		}
		void JKdTree::RemoveGameObject(JGameObject* gameObj)noexcept
		{		 
			const BoundingBox bbox = gameObj->GetRenderItem()->GetBoundingBox();
			std::vector<JKdTreeNode*> hitNode;
			FindHasNode(root, gameObj, bbox, hitNode);

			bool hasZero = false;
			const uint hitNodeCount = (uint)hitNode.size();
			for (uint i = 0; i < hitNodeCount; ++i)
			{
				hitNode[i]->RemoveInnerGameObject(gameObj->GetGuid());
				if (hitNode[i]->GetInnerGameObjectCount() == 0)
					hasZero = true;
			}
		 
			if (hasZero)
			{
				UnBuild();
				Build();
			}
		}
		J_SPACE_SPATIAL_TYPE JKdTree::GetType()const noexcept
		{
			return J_SPACE_SPATIAL_TYPE::KD_TREE;
		}
		uint JKdTree::GetNodeCount()const noexcept
		{
			return (uint)allNodes.size();
		}
		JKdTreeOption JKdTree::GetKdTreeOption()const noexcept
		{
			return JKdTreeOption(buildType, splitType, GetCommonOption());
		}
		std::vector<JGameObject*> JKdTree::GetAlignedObject(const JVector3<float>& pos)const noexcept
		{
			std::vector<JKdTreeNode*> alignNode(alignLeafNodeCount);
			uint index = 0;
			root->GetAlignLeafNode(pos, alignNode, index);

			uint totalObjCount = 0;
			for (uint i = 0; i < alignLeafNodeCount; ++i)
				totalObjCount += alignNode[i]->GetInnerGameObjectCount();

			std::vector<JGameObject*> alignObject(totalObjCount);
			totalObjCount = 0;
			for (uint i = 0; i < alignLeafNodeCount; ++i)
				alignNode[i]->StuffInnerGameObject(alignObject, totalObjCount);
			return alignObject;
		}
		std::vector<JGameObject*> JKdTree::GetAlignedObject(const DirectX::BoundingFrustum& camFrustum)const noexcept
		{
			if (allNodes.size() < 3)
				return std::vector<JGameObject*>();

			std::vector<JKdTreeNode*> alignNode(alignLeafNodeCount);
			uint index = 0;
			root->GetAlignLeafNode(camFrustum, alignNode, index);

			uint totalObjCount = 0;
			for (uint i = 0; i < index; ++i)
				totalObjCount += alignNode[i]->GetInnerGameObjectCount();

			std::vector<JGameObject*> alignObject(totalObjCount);
			totalObjCount = 0;
			for (uint i = 0; i < index; ++i)
				alignNode[i]->StuffInnerGameObject(alignObject, totalObjCount);		
			alignObject.resize(totalObjCount);
			return alignObject;
		}
		void JKdTree::SetKdTreeOption(const JKdTreeOption& newOption)noexcept
		{
			JKdTreeOption preOption = GetKdTreeOption();
			buildType = newOption.buildType;
			splitType = newOption.splitType; 

			if (!preOption.EqualCommonOption(newOption))
				SetCommonOption(newOption.commonOption);
			else
			{
				if (!preOption.EqualKdTreeOption(newOption))
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
		void JKdTree::BuildKdTree(JKdTreeNode* parentNode, const std::vector<JGameObject*>& objList, const uint nodeNumOffsset)
		{
			//JDebugTimer::StartGameTimer();
			std::vector<std::unique_ptr<JKdTreeNode>> additionalNode;
			switch (buildType)
			{
			case J_SPACE_SPATIAL_BUILD_TYPE::TOP_DOWN:
			{
				JBBox nodeBound = JBBox::InfBBox();
				for (const auto& data : objList)
					nodeBound = JBBox::Union(nodeBound, data->GetRenderItem()->GetBoundingBox());

				const uint objCount = (uint)objList.size();
				std::unique_ptr<uint[]> objIndexList(new uint[objCount]);
				for (uint i = 0; i < objCount; ++i)
					objIndexList[i] = i;

				uint maxDepth = CalculateMaxDepth(objCount);
				std::unique_ptr<uint[]> objIndexListBufL(new uint[objCount]);
				std::unique_ptr<uint[]> objIndexLisBufR(new uint[(maxDepth + 1) * objCount]);

				std::vector<std::vector<BoundEdge>> edge;
				for (uint i = 0; i < 3; ++i)
					edge.push_back(std::vector<BoundEdge>(objCount * 2));

				BuildTopDownKdTree(parentNode, nodeBound, objList, objCount, objIndexList.get(), objIndexListBufL.get(), objIndexLisBufR.get(), additionalNode, edge, 0, maxDepth, nodeNumOffsset);
				break;
			}
			default:
				return;
			}
			 
			const uint newNodeCount = (uint)additionalNode.size();
			if (IsDebugActivated())
			{
				for (uint i = 0; i < newNodeCount; ++i)
					additionalNode[i]->CreateDebugGameObject(GetDebugRoot(), IsDebugLeafOnly());
			}

			const uint allNodeCount = (uint)allNodes.size();
			for (uint i = nodeNumOffsset; i < allNodeCount; ++i)
				allNodes[i]->SetNodeNumber(i + newNodeCount);

			allNodes.insert(allNodes.begin() + nodeNumOffsset,
				std::make_move_iterator(additionalNode.begin()),
				std::make_move_iterator(additionalNode.end()));

			if (root == nullptr)
				root = allNodes[0].get();

			/*
			   JDebugTimer::StopGameTimer();
			   std::wofstream stream;
			   stream.open(L"D:\\JinWooJung\\KdTreeLog.txt", std::ios::out | std::ios::app);
			   JFileIOHelper::StoreJString(stream, L"Tree Log", L"");
			   JFileIOHelper::StoreAtomicData(stream, L"newNodeCount:", newNodeCount);
			   JFileIOHelper::StoreAtomicData(stream, L"objListCount:", objList.size());
			   JFileIOHelper::StoreAtomicData(stream, L"ElapsedTime na:", JDebugTimer::GetElapsedNanoTime());
			   JFileIOHelper::StoreAtomicData(stream, L"ElapsedTime ms:", JDebugTimer::GetElapsedMilliTime());
			   JFileIOHelper::StoreAtomicData(stream, L"ElapsedTime s:", JDebugTimer::GetElapsedSecondTime());
			   JFileIOHelper::InputSpace(stream, 1);
			   stream.close();
			*/
		}
		void JKdTree::BuildTopDownKdTree(JKdTreeNode* parentNode,
			const JBBox nodeBounds,
			const std::vector<JGameObject*>& objList,
			uint nObject,
			uint* objIndexList,
			uint* objIndexListBufL,
			uint* objIndexListBufR,
			std::vector<std::unique_ptr<JKdTreeNode>>& additionalNode,
			std::vector<std::vector<BoundEdge>>& edge,
			int badRefines,
			int depth,
			const uint nodeNumOffset)noexcept
		{
			/*
				"BBox: " + std::to_string(nodeBounds.min.x) + ',' + std::to_string(nodeBounds.min.y) + ',' + std::to_string(nodeBounds.min.z) + '\n' +
				std::to_string(nodeBounds.max.x) + ',' + std::to_string(nodeBounds.max.y) + ',' + std::to_string(nodeBounds.max.z) + '\n';
				std::string kLog = "nObject: " + std::to_string(objectCount) + '\n' +
					"listSt: " + std::to_string(listSt) + '\n' +
					"listEd: " + std::to_string(listEd) + '\n';

				for (uint i = listSt; i < listEd; ++i)
					kLog += "i: " + std::to_string(objIndexList[i]) + '\n';
			*/

			if (nObject < leastObj || depth == 0)
			{
				//Build Leaf
				additionalNode.emplace_back(std::make_unique<JKdTreeNode>(additionalNode.size() + nodeNumOffset, J_KDTREE_NODE_TYPE::LEAF, nodeBounds.Convert(), parentNode));
				InitLeafNode(additionalNode[additionalNode.size() - 1].get(), objList, nObject, objIndexList);
				//additionalNode[additionalNode.size() - 1].get()->AddLog(kLog);
				return;
			}

			uint dim = nodeBounds.MaxDimension();
			uint edgeCount = nObject * 2;

			int bestDim = -1;
			int bestOffset = -1;
			float bestCost = JMathHelper::Infinity;

			uint count = 0;
			while (bestDim == -1 && count < 3)
			{
				FindBestValue(nodeBounds, objList, nObject, objIndexList, edge[dim], dim, bestDim, bestOffset, bestCost);
				dim = (dim + 1) % 3;
				++count;
			}

			//kLog += "bestOffset: " + std::to_string(bestOffset) + '\n';

			if (bestCost > (isectCost * nObject))
				++badRefines;

			if ((bestCost > 4 * (isectCost * nObject) && nObject < 16) || bestDim == -1 || badRefines >= 3)
			{
				additionalNode.emplace_back(std::make_unique<JKdTreeNode>(additionalNode.size() + nodeNumOffset, J_KDTREE_NODE_TYPE::LEAF, nodeBounds.Convert(), parentNode));
				InitLeafNode(additionalNode[additionalNode.size() - 1].get(), objList, nObject, objIndexList);
				//additionalNode[additionalNode.size() - 1].get()->AddLog(kLog);
				return;
			}
			if (root == nullptr && parentNode == nullptr)
				additionalNode.emplace_back(std::make_unique<JKdTreeNode>(additionalNode.size() + nodeNumOffset, J_KDTREE_NODE_TYPE::ROOT, nodeBounds.Convert(), parentNode));
			else
				additionalNode.emplace_back(std::make_unique<JKdTreeNode>(additionalNode.size() + nodeNumOffset, J_KDTREE_NODE_TYPE::NODE, nodeBounds.Convert(), parentNode));
			JKdTreeNode* nowNode = additionalNode[additionalNode.size() - 1].get();

			JBBox bounds0 = nodeBounds;
			JBBox bounds1 = nodeBounds;

			float tSplit = edge[bestDim][bestOffset].p;
			bounds0.max[bestDim] = tSplit;
			bounds1.min[bestDim] = tSplit;

			int n0 = 0;
			int n1 = 0;

			for (uint i = 0; i < bestOffset; ++i)
			{
				if (edge[bestDim][i].edgeType == J_BOUND_EDGE_TYPE::START)
					objIndexListBufL[n0++] = edge[bestDim][i].objNumber;
			}

			for (uint i = bestOffset + 1; i < edgeCount; ++i)
			{
				if (edge[bestDim][i].edgeType == J_BOUND_EDGE_TYPE::END)
					objIndexListBufR[n1++] = edge[bestDim][i].objNumber;
			}

			/*for (uint i = 0; i < edgeCount; ++i)
				kLog += "e: " + std::to_string(edge[bestDim][i].objNumber) + '\n';
			for (uint i = 0; i < n0; ++i)
				kLog += "iL: " + std::to_string(objIndexListBufL[listSt + i]) + '\n';
			for (uint i = 0; i < n1; ++i)
				kLog += "iR: " + std::to_string(objIndexListBufR[listSt + n0 + i]) + '\n';
			kLog += "Edge: " + std::to_string(edgeCount) + '\n' +
				"Left N: " + std::to_string(n0) + '\n' +
				"Right N: " + std::to_string(n1) + '\n';*/

				//nowNode->AddLog(kLog);
			BuildTopDownKdTree(nowNode, bounds0, objList,
				n0, objIndexListBufL, objIndexListBufL, objIndexListBufR + nObject,
				additionalNode, edge,
				badRefines, depth - 1, nodeNumOffset);

			nowNode->SetSplitType(bestDim);

			BuildTopDownKdTree(nowNode, bounds1, objList,
				n1, objIndexListBufR, objIndexListBufL, objIndexListBufR + nObject,
				additionalNode, edge,
				badRefines, depth - 1, nodeNumOffset);
		}
		void JKdTree::FindBestValue(const JBBox& nodeBounds,
			const std::vector<JGameObject*>& objList,
			uint nObject,
			uint* objIndexList,
			std::vector<BoundEdge>& edge,
			const uint dim,
			_Out_ int& bestDim,
			_Out_ int& bestOffset,
			_Out_ float& bestCost)noexcept
		{
			for (uint i = 0; i < nObject; ++i)
			{
				const JBBox bbox = objList[objIndexList[i]]->GetRenderItem()->GetBoundingBox();
				edge[i * 2] = BoundEdge(bbox.min[dim], objIndexList[i], J_BOUND_EDGE_TYPE::START);
				edge[i * 2 + 1] = BoundEdge(bbox.max[dim], objIndexList[i], J_BOUND_EDGE_TYPE::END);
			}

			//거리가 가까운 순으로 정렬

			const uint edgeCount = nObject * 2;
			std::sort(&edge[0], &edge[edgeCount], [](const BoundEdge& e0, const BoundEdge& e1)
				{
					if (e0.p == e1.p)
						return (int)e0.edgeType < (int)e1.edgeType;
					else
						return e0.p < e1.p;
				});

			float oldCost = isectCost * (float)nObject;
			float totalSA = nodeBounds.Surface();
			float invTotalSA = 1 / totalSA;

			switch (splitType)
			{
			case JinEngine::Core::J_SPACE_SPATIAL_SPLIT_TYPE::SAH:
			{
				//두 영역에 남아있는 edge Factor
				uint nBelow = 0;
				uint nAbove = nObject;

				float pMinP = nodeBounds.min[dim];
				float pMaxP = nodeBounds.max[dim];

				JVector3<float> pBBDistance = nodeBounds.DistanceVector();

				for (uint i = 0; i < edgeCount; ++i)
				{
					if (edge[i].edgeType == J_BOUND_EDGE_TYPE::END)
						--nAbove;
					float edgeP = edge[i].p;
					if (edgeP > pMinP && edgeP < pMaxP)
					{
						// Compute child surface areas for split at _edgeT_
						int otherAxis0 = (dim + 1) % 3;
						int otherAxis1 = (dim + 2) % 3;

						float belowSurface = 2 * (pBBDistance[otherAxis0] * pBBDistance[otherAxis1] +
							(edgeP - pMinP) *
							(pBBDistance[otherAxis0] + pBBDistance[otherAxis1]));
						float aboveSurface = 2 * (pBBDistance[otherAxis0] * pBBDistance[otherAxis1] +
							(pMaxP - edgeP) *
							(pBBDistance[otherAxis0] + pBBDistance[otherAxis1]));

						float pBelow = belowSurface * invTotalSA;
						float pAbove = aboveSurface * invTotalSA;
						float eb = (nAbove == 0 || nBelow == 0) ? emptyBonus : 0;
						float cost = traversalCost + isectCost * (1 - eb) * (pBelow * nBelow + pAbove * nAbove);

						if (cost < bestCost)
						{
							bestCost = cost;
							bestDim = dim;
							bestOffset = i;
						}
					}
					if (edge[i].edgeType == J_BOUND_EDGE_TYPE::START)
						++nBelow;
				}
				break;
			}
			default:
				break;
			}
		}
		void JKdTree::InitLeafNode(JKdTreeNode* node, const std::vector<JGameObject*>& objList, uint nObject, uint* objIndexList)noexcept
		{ 
			std::vector<JGameObject*> innerGameObj(nObject);
			for (uint i = 0; i < nObject; ++i)
				innerGameObj[i] = objList[objIndexList[i]];
			node->AddInnerGameObject(innerGameObj);
		}
		void JKdTree::ClearKdTreeNode(const uint nodeNumber)
		{
			const uint nodeNumberEnd = allNodes[nodeNumber]->GetRightNumberEnd();
			for (uint i = nodeNumber; i <= nodeNumberEnd; ++i)
				allNodes[i]->Clear();
		}
		void JKdTree::ReBuildKdTreeNode(const uint nodeNumber, JGameObject* additionalObj)
		{
			if (nodeNumber == 0)
			{
				UnBuild();
				Build();
				return;
			}

			std::vector<JGameObject*> validInnerVec = allNodes[nodeNumber]->GetAreaInnerGameObject();
			if (additionalObj != nullptr)
				validInnerVec.push_back(additionalObj);

			JKdTreeNode* pNode = allNodes[nodeNumber]->GetParentNode();
			if (allNodes[nodeNumber]->IsLeftNode())
				pNode->SetLeftNode(nullptr);
			else
				pNode->SetRightNode(nullptr);

			const uint nodeREnd = allNodes[nodeNumber]->GetRightNumberEnd();
			ClearKdTreeNode(nodeNumber);
			if (pNode->GetNodeType() == J_KDTREE_NODE_TYPE::ROOT)
				root = nullptr;
			allNodes.erase(allNodes.begin() + nodeNumber, allNodes.begin() + nodeREnd + 1);
			BuildKdTree(pNode, validInnerVec, nodeNumber);
		}
		void JKdTree::FindHasNode(JKdTreeNode* node, const JGameObject* gobj, const DirectX::BoundingBox& bbox, std::vector<JKdTreeNode*>& hitNode)
		{
			if (node == nullptr)
				return;

			if (node->GetNodeType() != J_KDTREE_NODE_TYPE::LEAF)
			{
				FindHasNode(node->GetLeftNode(), gobj, bbox, hitNode);
				FindHasNode(node->GetRightNode(), gobj, bbox, hitNode);
			}
			else
			{
				if (node->HasGameObject(gobj->GetGuid()))
					hitNode.push_back(node);
			}
		}
		JKdTreeNode* JKdTree::FindContainNode(JKdTreeNode* node, const DirectX::BoundingBox& bbox)
		{
			if (node == nullptr)
				return nullptr;

			ContainmentType res = node->GetBoundingBox().Contains(bbox);
			if (res != ContainmentType::CONTAINS)
				return nullptr;

			if (node->GetNodeType() == J_KDTREE_NODE_TYPE::LEAF)
				return node;

			JKdTreeNode* leftRes = FindContainNode(node->GetLeftNode(), bbox);
			JKdTreeNode* rightRes = FindContainNode(node->GetRightNode(), bbox);
			if (leftRes == nullptr && rightRes == nullptr)
				return node;

			else if (leftRes != nullptr && rightRes != nullptr)
			{
				const XMVECTOR point = XMLoadFloat3(&bbox.Center);
				XMFLOAT3 leftExtents = leftRes->GetBoundingBox().Extents;
				XMFLOAT3 rightExtents = rightRes->GetBoundingBox().Extents;

				XMFLOAT3 leftDisF;
				XMFLOAT3 rightDisF;

				XMStoreFloat3(&leftDisF, XMVector3Length(XMVectorSubtract(point, XMLoadFloat3(&leftExtents))));
				XMStoreFloat3(&rightDisF, XMVector3Length(XMVectorSubtract(point, XMLoadFloat3(&rightExtents))));

				return leftDisF.x < rightDisF.x ? leftRes : rightRes;
			}
			else
				return leftRes != nullptr ? leftRes : rightRes;
		}
		uint JKdTree::CalculateMaxDepth(const uint objCount)const noexcept
		{
			return std::round(8 + 1.3f * JMathHelper::Log2Int(objCount));
		}
		JBBox JKdTree::CalculateObjectBBox(const std::vector<JGameObject*>& objList, const uint st, const uint ed)const noexcept
		{
			JBBox result;
			const uint objCount = (uint)objList.size();
			for (uint i = st; i < ed; ++i)
				result = JBBox::Union(result, objList[i]->GetRenderItem()->GetBoundingBox());
			return result;
		}
		void JKdTree::BuildDebugTree(Editor::JEditorBinaryTreeView& treeView)
		{
			treeView.Initialize(allNodes.size());
			if (root != nullptr)
				root->BuildDebugNode(treeView);
		}
	}
}
/*
		void JKdTree::CreateRootGameObject(const std::vector<JGameObject*>& objList)
		{
			if (root != nullptr)
				return;

			J_KDTREE_NODE_TYPE nodeType = objList.size() > 1 ? J_KDTREE_NODE_TYPE::ROOT : J_KDTREE_NODE_TYPE::LEAF;
			allNodes.emplace_back(std::make_unique<JKdTreeNode>(0,
				nodeType,
				CalculateObjectBBox(objList, 0, (uint)objList.size()).Convert(),
				nullptr));
			root = allNodes[0].get();
		}
*/