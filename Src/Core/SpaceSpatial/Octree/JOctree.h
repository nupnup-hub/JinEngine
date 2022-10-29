#pragma once
#include"JOctreeNode.h"
#include"../../JDataType.h"
#include<DirectXCollision.h>
#include<memory>
#include<vector>
#include<unordered_map>

namespace JinEngine
{
	//초기화까지만 구현
	//dynamic 추가 삭제 구현 필요
	class JGameObject;
	namespace Core
	{
		class JCullingFrustum;
		class JOctree
		{
		private:
			JOctreeNode* rootNodeCash = nullptr;
			std::vector<std::unique_ptr<JOctreeNode>> allNode;
			std::unordered_map<size_t, JOctreeNode*> containNodeMap;
			JGameObject* debugRoot;
			uint octreeSize;
			float looseFactor;
			bool isLooseOctree;
			bool isDebugActivated = false;
			bool isDebugLeafOnly = true;
			uint minSize = 1;
			static constexpr uint looseFactorMax = 4;
			static constexpr uint neighBorIndexList[8][3]
			{
				 { 1,2,4 },{ 0,3,5 },{ 0,3,6 },{ 1,2,7 },
				 { 0,5,6 },{ 1,4,7 },{ 2,4,7 },{ 3,5,6 }
			};
		public:
			JOctree(std::vector<JGameObject*>& gameObject, const uint minSize, const uint octreeSizeSquare, const float looseFactor, const bool isLooseOctree);
			~JOctree();
		public:
			void Clear()noexcept;
		public:
			void OnDebugGameObject(JGameObject* newDebugRoot)noexcept;
			void OffDebugGameObject()noexcept;
			void Culling(const JCullingFrustum& camFrustum)noexcept;
			void Culling(const DirectX::BoundingFrustum& camFrustum)noexcept;
			void UpdateGameObject(JGameObject* gameObject)noexcept;
		public:
			bool AddGameObject(JGameObject* newGameObject)noexcept;
			bool RemoveGameObject(JGameObject* gameObj)noexcept;
		public:
			void SetDebugOnlyLeaf(bool value)noexcept;
			bool IsDebugActivated()const noexcept;
			bool IsDebugLeafOnly()const noexcept;
		private:
			void BuildOctree(std::vector<JGameObject*>& gameObject)noexcept;
			void BuildOctreeNode(JOctreeNode* parent, const uint depth)noexcept;
		private:
			void FindNeighborOctreeNode(JOctreeNode* node)noexcept;
			JOctreeNode* FindNodeNearCenter(JOctreeNode* node, const DirectX::XMVECTOR point)noexcept;
			JOctreeNode* FindOptimalNode(JOctreeNode* node, const DirectX::BoundingBox& bbox, const DirectX::ContainmentType condition)noexcept;
		private:
			uint GetExtentLength(const uint depth)const noexcept;
			uint GetLooseExtentLength(const uint depth)const noexcept;
		};
	}
}