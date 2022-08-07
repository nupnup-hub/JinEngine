#pragma once
#include"JOctreeNode.h"
#include"../../JDataType.h"
#include<DirectXCollision.h>
#include<memory>
#include<vector>

namespace JinEngine
{
	//초기화까지만 구현
	//dynamic 추가 삭제 구현 필요
	class JGameObject;
	namespace Core
	{
		class JOctree
		{
		private:
			JOctreeNode* rootNodeCash;
			std::vector<std::unique_ptr<JOctreeNode>> allNode;
			JGameObject* debugRoot;
			uint octreeSize;
			float looseFactor;
			bool isLooseOctree;
			bool isDebugModeActivated;
			static constexpr uint minSize = 1;
			static constexpr uint looseFactorMax = 4;
			static constexpr uint neighBorIndexList[8][3]
			{
				 { 1,2,4 },{ 0,3,5 },{ 0,3,6 },{ 1,2,7 },
				 { 0,5,6 },{ 1,4,7 },{ 2,4,7 },{ 3,5,6 }
			};
		public:
			JOctree(std::vector<JGameObject*>& gameObject, const uint octreeSizeSquare, const float looseFactor, const bool isLooseOctree);
			~JOctree();
			void OnDebugGameObject(JGameObject* newDebugRoot, bool onlyLeafNode)noexcept;
			void OffDebugGameObject()noexcept;
			void Culling(const DirectX::BoundingFrustum& camFrustum)noexcept;
			void Clear()noexcept;
		private:
			void BuildOctree(std::vector<JGameObject*>& gameObject)noexcept;
			void BuildOctreeNode(JOctreeNode* parent, const uint depth)noexcept;
			void FindNeighborOctreeNode(JOctreeNode* node)noexcept;
			JOctreeNode* FindNodeNearCenter(JOctreeNode* node, const DirectX::XMVECTOR point)noexcept;
			uint GetExtentLength(const uint depth)const noexcept;
			uint GetLooseExtentLength(const uint depth)const noexcept;
		};
	}
}