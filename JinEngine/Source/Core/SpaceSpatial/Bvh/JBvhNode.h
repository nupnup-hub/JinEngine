#pragma once
#include"../JSpaceSpatialNode.h"
#include"../../JDataType.h"
#include"../../Geometry/JCullingFrustum.h" 
#include"../../Pointer/JOwnerPtr.h"
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
		class JBvhNode : public JSpaceSpatialNode
		{
		private:
			uint nodeNumber;
			J_BVH_NODE_TYPE type;
			DirectX::BoundingBox bbox;	//BoundingOriented가 더 정확하지만 느리므로 leaf type Culling에서만 사용하고 그외에는 BoundingBox를 사용한다	
			JBvhNode* parent = nullptr;
			JBvhNode* left = nullptr;
			JBvhNode* right = nullptr;
			JUserPtr<JGameObject> innerGameObject = nullptr;
			JUserPtr<JGameObject> debugGameObject = nullptr;
		public:
			JBvhNode(const uint nodeNumber, const J_BVH_NODE_TYPE type, const DirectX::BoundingBox& bbox, JBvhNode* parent, const JUserPtr<JGameObject>& innerGameObject, bool isLeftNode);
			~JBvhNode();
			JBvhNode(const JBvhNode& rhs) = delete;
			JBvhNode& operator=(const JBvhNode& rhs) = delete;
			JBvhNode(JBvhNode&& rhs) = default;
			JBvhNode& operator=(JBvhNode&& rhs) = default;
		public:
			void CreateDebugGameObject(const JUserPtr<JGameObject>& parent, bool onlyLeafNode)noexcept;
			void DestroyDebugGameObject()noexcept;
			void Clear()noexcept;
			void Culling(Graphic::JCullingUserInterface& cullUser, const JCullingFrustum& camFrustum, J_CULLING_FLAG flag)noexcept; 
			void Culling(Graphic::JCullingUserInterface& cullUser, const DirectX::BoundingFrustum& camFrustum, const DirectX::BoundingFrustum& cullingFrustum)noexcept;
			JUserPtr<JGameObject> IntersectFirst(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir, const bool allowContainRayPos)const noexcept;
			void IntersectAscendingSort(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir, _Out_ std::vector<JUserPtr<JGameObject>>& res)const noexcept;
			void IntersectDescendingSort(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir, _Out_ std::vector<JUserPtr<JGameObject>>& res)const noexcept;
			//AscendingSort
			void Intersect(const DirectX::FXMVECTOR ori, const DirectX::FXMVECTOR dir, _Out_ std::vector<JUserPtr<JGameObject>>& res)const noexcept;
			void UpdateInnerGameObject()noexcept; 
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
			JUserPtr<JGameObject> GetInnerGameObject()const noexcept;
			JUserPtr<JGameObject> GetDebugGameObject()const noexcept;
		public:
			void SetNodeNumber(const uint newNumber)noexcept;
			void SetNodeType(const J_BVH_NODE_TYPE newNodeType)noexcept;
			void SetLeftNode(JBvhNode* newLeftNode)noexcept;
			void SetRightNode(JBvhNode* newRightNode)noexcept;
			void SetInnerGameObject(const JUserPtr<JGameObject>& newInnerGameObject)noexcept;
		private:
			void SetVisible(Graphic::JCullingUserInterface& cullUser)noexcept;
			void SetVisible(Graphic::JCullingUserInterface& cullUser, const DirectX::BoundingFrustum& cullingFrustum, const bool camInParentBBox)noexcept;
			void SetInVisible(Graphic::JCullingUserInterface& cullUser)noexcept;
			JBvhNode* FindRightLeafNode()noexcept;
		private:
			void SetDebugObjectTransform()noexcept;
		public:
			void BuildDebugNode(Editor::JEditorBinaryTreeView& treeView);
		};
	}
}