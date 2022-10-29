#pragma once
#include"JBvhType.h"
#include"JBvhNode.h" 
#include"../../JDataType.h"
#include<vector>
#include<unordered_map>
#include<memory>
#include<DirectXCollision.h>

namespace JinEngine
{
	class JGameObject;
	namespace Core
	{
		class JCullingFrustum;
		class JBvh
		{
		private:
			JBvhNode* root;
			std::vector<std::unique_ptr<JBvhNode>> allNodes;
			std::unordered_map<size_t, JBvhNode*> leafNodeMap;
			JGameObject* debugRoot;
			//Bvh에 JGameObject가 1개일시 트리를 생성하지 않고 캐싱
			//이 후 JGameObject가 2개이상이될시 트리 생성
			JGameObject* innerGameObjectCandidate;
			bool isDebugActivated = false;
			bool isDebugLeafOnly = true;
			const J_BVH_BUILD_TYPE buildType;
			const J_BVH_SPLIT_TYPE splitType;
		public:
			JBvh(std::vector<JGameObject*> objectList, const J_BVH_BUILD_TYPE buildType = J_BVH_BUILD_TYPE::TOP_DOWN, const J_BVH_SPLIT_TYPE splitType = J_BVH_SPLIT_TYPE::SAH);
			~JBvh();
		public:
			void Clear()noexcept;
		public:
			void OnDebugGameObject(JGameObject* newDebugRoot)noexcept;
			void OffDebugGameObject()noexcept;
			void Culling(const JCullingFrustum& camFrustum)noexcept;
			void Culling(const DirectX::BoundingFrustum& camFrustum)noexcept;
			void UpdateGameObject(JGameObject* gameObject)noexcept;
		public:
			void AddGameObject(JGameObject* newGameObject)noexcept;
			void RemoveGameObject(JGameObject* gameObj)noexcept; 
		public:
			void SetDebugOnlyLeaf(bool value)noexcept;
			bool IsDebugActivated()const noexcept;
			bool IsDebugLeafOnly()const noexcept;
		private:
			DirectX::BoundingBox GetRootBoundingBox()const noexcept;
			int GetMaximumDimension(const DirectX::BoundingBox& box)const noexcept;
			float GetDimensionValue(const DirectX::XMFLOAT3& point, const int dim)const noexcept;
		private:
			void BuildTopdownBvhBySAH(JBvhNode* parent,
				std::vector<JGameObject*>& objectList,
				std::vector<std::unique_ptr<JBvhNode>>& nodeVec,
				const int start,
				const int end,
				const int numberOffset)noexcept;
			/*
			Dimension
			x = 0, y = 1, z = 2
			*/

			void ReBuildBvh(const uint nodeNumber, JGameObject* additionalGameObj = nullptr)noexcept;
			void ClearBvhNode(const uint nodeNumber)noexcept;
			void DestroyBvhNode(const uint nodeNumber)noexcept;
		};
	}
}