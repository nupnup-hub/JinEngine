#pragma once
#include"../../JDataType.h"
#include<vector>
#include<DirectXCollision.h>

namespace JinEngine
{
	class JGameObject;
	namespace Core
	{
		class JOctreeNode
		{
		private:
			DirectX::BoundingBox boundingBox;
			JOctreeNode* parentNode;
			std::vector<JOctreeNode*> childrenNode;
			std::vector<JOctreeNode*> neighborNode;
			std::vector<JGameObject*> innerGameObject;
			JGameObject* debugGameObject;
		public:
			JOctreeNode(const DirectX::BoundingBox& boundingBox, bool isLooseOctree, JOctreeNode* parentNode = nullptr);
			~JOctreeNode();
			JOctreeNode(const JOctreeNode& rhs) = delete;
			JOctreeNode& operator=(const JOctreeNode& rhs) = delete;
			JOctreeNode(JOctreeNode&& rhs) = default;
			JOctreeNode& operator=(JOctreeNode&& rhs) = default;

			bool AddGameObject(JGameObject* gameObj, bool isLooseOctree)noexcept;
			bool AddNeighborNode(JOctreeNode* octreeNode)noexcept;
			void CreateDebugGameObject(JGameObject* parent, bool onlyLeafNode)noexcept;
			void DestroyDebugGameObject()noexcept;
			void Clear();
			void Culling(const DirectX::BoundingFrustum& camFrustum)noexcept;

			DirectX::BoundingBox GetBoundingBox()const noexcept;
			DirectX::BoundingBox GetInnerGameObjectBoundingBox(const uint index)const noexcept;
			JOctreeNode* GetParentNode()noexcept;
			JOctreeNode* GetChildNode(const uint index)noexcept;
			uint GetInnerGameObjectCount()const noexcept;

			bool IsLeafNode()const noexcept;
			bool IsRootNode()const noexcept;
		private:
			bool RemoveInnerGameObject(JGameObject* gameObject)noexcept;
		};
	}
}