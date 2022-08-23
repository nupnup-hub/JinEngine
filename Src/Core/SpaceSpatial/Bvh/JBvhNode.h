#pragma once
#include"../../JDataType.h"
#include"../../DirectXEx/JCullingFrustum.h"
#include<DirectXCollision.h>
#include<unordered_map>

namespace JinEngine
{
	class JGameObject;
	class JScene;
	namespace Core
	{
		enum class J_BVH_NODE_TYPE
		{
			ROOT,
			LEAF,
			NODE,
		};
		class JBvhNode
		{
		private:
			uint nodeNumber;
			J_BVH_NODE_TYPE type;
			DirectX::BoundingBox bv;
			JBvhNode* parent;
			JBvhNode* left;
			JBvhNode* right;
			JGameObject* innerGameObject;
			JGameObject* debugGameObject;
		public:
			JBvhNode(const uint nodeNumber, const J_BVH_NODE_TYPE type, const DirectX::BoundingBox& bv, JBvhNode* parent, JGameObject* innerGameObject, bool isLeftNode);
			~JBvhNode();
			JBvhNode(const JBvhNode& rhs) = delete;
			JBvhNode& operator=(const JBvhNode& rhs) = delete;
			JBvhNode(JBvhNode&& rhs) = default;
			JBvhNode& operator=(JBvhNode&& rhs) = default;

			void CreateDebugGameObject(JGameObject* parent, bool onlyLeafNode)noexcept;
			void DestroyDebugGameObject()noexcept;
			void Clear()noexcept;
			void Culling(const JCullingFrustum& camFrustum, J_CULLING_FLAG flag)noexcept;
			void Culling(const DirectX::BoundingFrustum& camFrustum)noexcept;
			void UpdateInnerGameObject()noexcept;
			bool IsLeftNode()const noexcept;
			bool IsContain(const DirectX::BoundingBox& boundBox)const noexcept;

			uint GetNodeNumber()const noexcept;
			uint GetLeftNumberEnd()const noexcept;
			uint GetRightNumberEnd()const noexcept;
			J_BVH_NODE_TYPE GetNodeType()const noexcept;
			DirectX::BoundingBox GetBoundingBox()const noexcept;
			JBvhNode* GetParentNode()noexcept;
			JBvhNode* GetLeftNode()noexcept;
			JBvhNode* GetRightNode()noexcept;
			JBvhNode* GetContainNodeToRoot(const DirectX::BoundingBox& boundBox)noexcept;
			JBvhNode* GetContainNodeToLeaf(const DirectX::BoundingBox& boundBox)noexcept;
			JGameObject* GetInnerGameObject()noexcept;
			JGameObject* GetDebugGameObject()noexcept;

			void SetNodeNumber(const uint newNumber)noexcept;
			void SetNodeType(const J_BVH_NODE_TYPE newNodeType)noexcept;
			void SetLeftNode(JBvhNode* newLeftNode)noexcept;
			void SetRightNode(JBvhNode* newRightNode)noexcept;
			void SetInnerGameObject(JGameObject* newInnerGameObject)noexcept;
		private:
			void SetVisible()noexcept;
			void SetInVisible()noexcept;
			JBvhNode* FindRightLeafNode()noexcept;
		};
	}
}