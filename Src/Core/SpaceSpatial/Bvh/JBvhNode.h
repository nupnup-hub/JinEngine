#pragma once
#include"../../JDataType.h"
#include"../../Geometry/JCullingFrustum.h" 
#include<unordered_map>

namespace JinEngine
{
	class JGameObject; 
	namespace Editor
	{
		class JEditorBinaryTreeView;
	}
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
			DirectX::BoundingBox bbox;
			JBvhNode* parent = nullptr;
			JBvhNode* left = nullptr;
			JBvhNode* right = nullptr;
			JGameObject* innerGameObject = nullptr;
			JGameObject* debugGameObject = nullptr;
		public:
			JBvhNode(const uint nodeNumber, const J_BVH_NODE_TYPE type, const DirectX::BoundingBox& bbox, JBvhNode* parent, JGameObject* innerGameObject, bool isLeftNode);
			~JBvhNode();
			JBvhNode(const JBvhNode& rhs) = delete;
			JBvhNode& operator=(const JBvhNode& rhs) = delete;
			JBvhNode(JBvhNode&& rhs) = default;
			JBvhNode& operator=(JBvhNode&& rhs) = default;
		public:
			void CreateDebugGameObject(JGameObject* parent, bool onlyLeafNode)noexcept;
			void DestroyDebugGameObject()noexcept;
			void Clear()noexcept;
			void Culling(const JCullingFrustum& camFrustum, J_CULLING_FLAG flag)noexcept;
			void Culling(const DirectX::BoundingFrustum& camFrustum)noexcept;
			void UpdateInnerGameObject()noexcept;
			void OffCulling()noexcept;
		public:
			bool IsLeftNode()const noexcept;
			bool IsContain(const DirectX::BoundingBox& boundBox)const noexcept;
		public:
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
			JGameObject* GetInnerGameObject()const noexcept;
			JGameObject* GetDebugGameObject()const noexcept;

			void SetNodeNumber(const uint newNumber)noexcept;
			void SetNodeType(const J_BVH_NODE_TYPE newNodeType)noexcept;
			void SetLeftNode(JBvhNode* newLeftNode)noexcept;
			void SetRightNode(JBvhNode* newRightNode)noexcept;
			void SetInnerGameObject(JGameObject* newInnerGameObject)noexcept;
		private:
			void SetVisible()noexcept;  
			void SetInVisible()noexcept;
			JBvhNode* FindRightLeafNode()noexcept;
		public:
			void BuildDebugNode(Editor::JEditorBinaryTreeView& treeView);
		};
	}
}