#pragma once
#include"../../JDataType.h"
#include"../../Geometry/JCullingFrustum.h"
#include<vector> 
#include<DirectXCollision.h>

namespace JinEngine
{
	class JGameObject;
	namespace Core
	{
		class JCullingFrustum;
		class JOctreeNode
		{
		private:
			DirectX::BoundingBox boundingBox;
			JOctreeNode* parentNode = nullptr;
			std::vector<JOctreeNode*> childrenNode;
			std::vector<JOctreeNode*> neighborNode;
			std::vector<JGameObject*> innerGameObject;
			JGameObject* debugGameObject = nullptr;
		public:
			JOctreeNode(const DirectX::BoundingBox& boundingBox, bool isLooseOctree, JOctreeNode* parentNode = nullptr);
			~JOctreeNode();
			JOctreeNode(const JOctreeNode& rhs) = delete;
			JOctreeNode& operator=(const JOctreeNode& rhs) = delete;
			JOctreeNode(JOctreeNode&& rhs) = default;
			JOctreeNode& operator=(JOctreeNode&& rhs) = default;
		public:
			void CreateDebugGameObject(JGameObject* parent, bool onlyLeafNode)noexcept;
			void DestroyDebugGameObject()noexcept;
			void Clear();
		public:
			//Culling node bbox
			void Culling(const JCullingFrustum& camFrustum, J_CULLING_FLAG flag)noexcept;
			void Culling(const DirectX::BoundingFrustum& camFrustum, const DirectX::BoundingFrustum& nearFrustum)noexcept;
			JGameObject* IntersectFirst(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir)const noexcept;
			void IntersectAscendingSort(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir, _Out_ std::vector<JGameObject*>& res)const noexcept;
			void IntersectDescendingSort(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir, _Out_ std::vector<JGameObject*>& res)const noexcept;
			void Intersect(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir, _Out_ std::vector<JGameObject*>& res)const noexcept;
			void OffCulling();
		public:
			bool AddGameObject(JGameObject* gameObj, bool isLooseOctree)noexcept;
			bool AddNeighborNode(JOctreeNode* octreeNode)noexcept;
			bool RemoveGameObject(JGameObject* gameObj)noexcept; 
		public:
			bool IsLeafNode()const noexcept;
			bool IsRootNode()const noexcept;
		public:
			DirectX::BoundingBox GetBoundingBox()const noexcept;
			DirectX::BoundingBox GetInnerGameObjectBoundingBox(const uint index)const noexcept;
			JOctreeNode* GetParentNode()noexcept;
			JOctreeNode* GetChildNode(const uint index)noexcept;
			uint GetInnerGameObjectCount()const noexcept;
		private:
			bool RemoveInnerGameObject(JGameObject* gameObject)noexcept;
		private:
			void CullingInnerObject(const JCullingFrustum& camFrustum, J_CULLING_FLAG oriFlag);
			void CullingInnerObject(const DirectX::BoundingFrustum& camFrustum, const DirectX::BoundingFrustum& nearFrustum);
		private:
			void SetVisible(const JCullingFrustum& camFrustum, J_CULLING_FLAG flag)noexcept;
			void SetVisible(const DirectX::BoundingFrustum& camFrustum, const DirectX::BoundingFrustum& nearFrustum)noexcept;
			void SetInVisible()noexcept;
		};
	}
}