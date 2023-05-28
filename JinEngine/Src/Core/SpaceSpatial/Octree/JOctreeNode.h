#pragma once
#include"../JSpaceSpatialNode.h"
#include"../../JDataType.h"
#include"../../Geometry/JCullingFrustum.h"
#include"../../Pointer/JOwnerPtr.h"
#include<vector> 
#include<DirectXCollision.h>

namespace JinEngine
{
	class JGameObject; 
	namespace Core
	{
		class JCullingFrustum;
		class JOctreeNode : public JSpaceSpatialNode
		{
		private:
			DirectX::BoundingBox boundingBox;
			JOctreeNode* parentNode = nullptr;
			std::vector<JOctreeNode*> childrenNode;
			std::vector<JOctreeNode*> neighborNode;
			std::vector<JUserPtr<JGameObject>> innerGameObject;
			JUserPtr<JGameObject> debugGameObject = nullptr;
		public:
			JOctreeNode(const DirectX::BoundingBox& boundingBox, bool isLooseOctree, JOctreeNode* parentNode = nullptr);
			~JOctreeNode();
			JOctreeNode(const JOctreeNode& rhs) = delete;
			JOctreeNode& operator=(const JOctreeNode& rhs) = delete;
			JOctreeNode(JOctreeNode&& rhs) = default;
			JOctreeNode& operator=(JOctreeNode&& rhs) = default;
		public:
			void CreateDebugGameObject(JUserPtr<JGameObject> parent, bool onlyLeafNode)noexcept;
			void DestroyDebugGameObject()noexcept;
			void Clear();
		public:
			//Culling node bbox
			void Culling(Graphic::JCullingUserInterface& cullUser, const JCullingFrustum& camFrustum, J_CULLING_FLAG flag)noexcept;
			void Culling(Graphic::JCullingUserInterface& cullUser, const DirectX::BoundingFrustum& camFrustum, const DirectX::BoundingFrustum& cullingFrustum)noexcept;
			JUserPtr<JGameObject> IntersectFirst(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir)const noexcept;
			void IntersectAscendingSort(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir, _Out_ std::vector<JUserPtr<JGameObject>>& res)const noexcept;
			void IntersectDescendingSort(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir, _Out_ std::vector<JUserPtr<JGameObject>>& res)const noexcept;
			void Intersect(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir, _Out_ std::vector<JUserPtr<JGameObject>>& res)const noexcept;
		public:
			bool AddGameObject(JUserPtr<JGameObject> gameObj, bool isLooseOctree)noexcept;
			bool AddNeighborNode(JOctreeNode* octreeNode)noexcept;
			bool RemoveGameObject(JUserPtr<JGameObject> gameObj)noexcept; 
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
			bool RemoveInnerGameObject(JUserPtr<JGameObject> gameObject)noexcept;
		private:
			void CullingInnerObject(Graphic::JCullingUserInterface& cullUser, const JCullingFrustum& camFrustum, J_CULLING_FLAG oriFlag);
			void CullingInnerObject(Graphic::JCullingUserInterface& cullUser, const DirectX::BoundingFrustum& camFrustum, const DirectX::BoundingFrustum& cullingFrustum);
		private:
			void SetVisible(Graphic::JCullingUserInterface& cullUser, const JCullingFrustum& camFrustum, J_CULLING_FLAG flag)noexcept;
			void SetVisible(Graphic::JCullingUserInterface& cullUser, const DirectX::BoundingFrustum& camFrustum, const DirectX::BoundingFrustum& cullingFrustum)noexcept;
			void SetInVisible(Graphic::JCullingUserInterface& cullUser)noexcept;
		};
	}
}