#pragma once
#include"JOctreeNode.h"
#include"JOctreeOption.h"
#include"../JSpaceSpatial.h"
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
		class JOctree : public JSpaceSpatial
		{
		private:
			JOctreeNode* rootNodeCash = nullptr;
			std::vector<std::unique_ptr<JOctreeNode>> allNode;
			std::unordered_map<size_t, JOctreeNode*> containNodeMap;
			uint minSize = 32;
			uint octreeSizeSquare = 9;
			uint octreeSize = 1 << 9;
			float looseFactor = 2;
			//2022-11-04~ JOctree is looseOctree 
			static constexpr bool isLooseOctree = true; 
			static constexpr float looseFactorMin = 2;
			static constexpr float looseFactorMax = 4;
			static constexpr uint neighBorIndexList[8][3]
			{
				 { 1,2,4 },{ 0,3,5 },{ 0,3,6 },{ 1,2,7 },
				 { 0,5,6 },{ 1,4,7 },{ 2,4,7 },{ 3,5,6 }
			};
		public:
			JOctree(const J_SPACE_SPATIAL_LAYER layer);
			~JOctree();
		protected:
			void Build()noexcept final;
			void UnBuild()noexcept final;
		public:
			void Clear()noexcept final;
		protected:
			void OnDebugGameObject()noexcept final;
			void OffDebugGameObject()noexcept final;
			void OffCulling()noexcept final;
		public:
			void Culling(const JCullingFrustum& camFrustum)noexcept final;
			void Culling(const DirectX::BoundingFrustum& camFrustum)noexcept final;
			JGameObject* IntersectFirst(const JRay& ray)const noexcept final;
			void Intersect(const JRay& ray, const J_SPACE_SPATIAL_SORT_TYPE sortType, _Out_ std::vector<JGameObject*>& res)const noexcept final;
			void UpdateGameObject(JGameObject* gameObject)noexcept final;
		public:
			void AddGameObject(JGameObject* newGameObject)noexcept final;
			void RemoveGameObject(JGameObject* gameObj)noexcept final;
		public:
			J_SPACE_SPATIAL_TYPE GetType()const noexcept;
			JOctreeOption GetOctreeOption()const noexcept;
			void SetOctreeOption(const JOctreeOption& newOption)noexcept;
		private:
			void BuildOctree()noexcept;
			void BuildOctreeNode(JOctreeNode* parent, const uint depth)noexcept;
		private:
			void FindNeighborOctreeNode(JOctreeNode* node)noexcept;
			JOctreeNode* FindNodeNearCenter(JOctreeNode* node, const DirectX::XMVECTOR point)noexcept;
			JOctreeNode* FindOptimalNode(JOctreeNode* node, const DirectX::BoundingBox& bbox)noexcept;
		private:
			uint GetExtentLength(const uint depth)const noexcept;
			uint GetLooseExtentLength(const uint depth)const noexcept;
		};
	}
}