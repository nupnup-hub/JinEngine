#pragma once
#include"../JAcceleratorNode.h" 
#include"../../../../../Core/Geometry/JCullingFrustum.h"  
#include<unordered_map>

namespace JinEngine
{
	class JGameObject; 
	struct JAcceleratorCullingInfo;
	struct JAcceleratorIntersectInfo;
	namespace Editor
	{
		class JEditorBinaryTreeView;
	}
	enum class J_BVH_NODE_TYPE
	{
		ROOT,
		LEAF,
		NODE,
	};
	class JBvhNode : public JAcceleratorNode
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
	public:
		void Culling(JAcceleratorCullingInfo& info, Core::J_CULLING_FLAG flag)noexcept;
		void Culling(JAcceleratorCullingInfo& info)noexcept;
		void FindIntersect(JAcceleratorIntersectInfo& info)const noexcept;
		void FindContain(JAcceleratorContainInfo& info)const noexcept;
		/**
		* @brief alignGameObject은 항상 총 gameObject 갯수만큼 resize 되있어야 한다.
		*/
		void AlignLeafNode(const JAcceleratorAlignInfo& info, std::vector<	JUserPtr<JGameObject>>& alignGameObject, uint& index, const uint depth)noexcept;
	public:
		void UpdateInnerGameObject()noexcept;
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
		void SetVisible(JAcceleratorCullingInfo& info)noexcept;
		void SetVisible(JAcceleratorCullingInfo& info, const bool camInParentBBox)noexcept;
		void SetInVisible(JAcceleratorCullingInfo& info)noexcept;
		void SetDebugObjectTransform()noexcept;
	public:
		bool IsLeftNode()const noexcept;
		bool IsContainNode(const DirectX::BoundingBox& boundBox)const noexcept;
	private:
		bool IsNearRight(const JAcceleratorAlignInfo& info);
	private:
		JBvhNode* FindRightLeafNode()noexcept;
		JUserPtr<JGameObject> FindFirstIntersect(JAcceleratorIntersectInfo& info)const noexcept;
		void FindIntersectAscendingSort(JAcceleratorIntersectInfo& info)const noexcept;
		void FindContainNotSort(JAcceleratorContainInfo& info)const noexcept;
	public:
		void BuildDebugNode(Editor::JEditorBinaryTreeView& treeView);
	};
}