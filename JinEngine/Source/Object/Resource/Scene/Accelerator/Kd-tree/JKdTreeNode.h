#pragma once 
#include"../JAcceleratorNode.h"
#include"../JAcceleratorOption.h" 
#include"../../../../../Core/Geometry/JCullingFrustum.h"
#include"../../../../../Core/Geometry/JRay.h" 
#include"../../../../../Core/Math/JVector.h"
#include<DirectXCollision.h> 
#include<vector>
#include<string>

namespace JinEngine
{
	class JGameObject;
	namespace Editor
	{
		class JEditorBinaryTreeView;
	} 
	enum class J_KDTREE_NODE_TYPE
	{
		ROOT,
		LEAF,
		NODE,
	};
	enum class J_KDTREE_NODE_SPLIT_AXIS
	{
		X = 0,
		Y,
		Z
	};
	class JKdTreeNode : public JAcceleratorNode
	{
	private:
		uint nodeNumber; 
		J_KDTREE_NODE_TYPE nodeType;
		J_KDTREE_NODE_SPLIT_AXIS splitType;
		DirectX::BoundingBox bbox;
		JKdTreeNode* parent = nullptr;
		JKdTreeNode* left = nullptr;
		JKdTreeNode* right = nullptr;
		JUserPtr<JGameObject> debugGameObject = nullptr;
		std::vector<JUserPtr<JGameObject>> innerGameObject;
	private:
		//Debug
		//std::string nodeLog;
	public:
		JKdTreeNode(const uint nodeNumber, const J_KDTREE_NODE_TYPE nodeType, const DirectX::BoundingBox& bbox, JKdTreeNode* parent);
		~JKdTreeNode();
	public:
		void CreateDebugGameObject(const JUserPtr<JGameObject>& parent, bool onlyLeafNode)noexcept;
		void DestroyDebugGameObject()noexcept;
		void Clear()noexcept;
	public:
		void Culling(JAcceleratorCullingInfo& info, Core::J_CULLING_FLAG flag)noexcept;
		void Culling(JAcceleratorCullingInfo& info)noexcept;
		void FindIntersect(JAcceleratorIntersectInfo& info)const noexcept;
		void FindContain(JAcceleratorContainInfo& info)const noexcept;
		void AlignLeafNode(const JAcceleratorAlignInfo& info, std::vector<JKdTreeNode*>& alignNode, uint& index, const uint depth)noexcept;
	public:
		uint GetNodeNumber()const noexcept;
		uint GetLeftNumberEnd()const noexcept;
		uint GetRightNumberEnd()const noexcept; 
		J_KDTREE_NODE_TYPE GetNodeType()const noexcept;
		DirectX::BoundingBox GetBoundingBox()const noexcept;
		JKdTreeNode* GetParentNode()noexcept;
		JKdTreeNode* GetLeftNode()const noexcept;
		JKdTreeNode* GetRightNode()const noexcept;
		std::vector<JUserPtr<JGameObject>> GetInnerGameObject()const noexcept;
		std::vector<JUserPtr<JGameObject>> GetAreaInnerGameObject()const noexcept;
		uint GetInnerGameObjectCount()const noexcept;
		uint GetDepth()const noexcept; 
	public:
		void SetNodeNumber(const uint newNodeNumber)noexcept;
		void SetSplitType(const uint newSplitType)noexcept;
		void SetSplitType(const J_KDTREE_NODE_SPLIT_AXIS newSplitType)noexcept;
		void SetLeftNode(JKdTreeNode* newLeftNode)noexcept;
		void SetRightNode(JKdTreeNode* newRightNode)noexcept;
	private:
		void SetVisible(JAcceleratorCullingInfo& info)noexcept;
		void SetInVisible(JAcceleratorCullingInfo& info)noexcept;
	public:
		bool IsLeftNode()const noexcept;
		bool IsChildNode(const uint pNodeNumber)const noexcept;
		bool HasGameObject(const size_t guid)noexcept;
	private:
		bool IsNearRight(const JAcceleratorAlignInfo& info);
	public:
		void AddInnerGameObject(const JUserPtr<JGameObject>& newInnerGameObject)noexcept;
		void AddInnerGameObject(const std::vector<JUserPtr<JGameObject>>& newInnerGameObject)noexcept;
		void RemoveInnerGameObject(const size_t guid)noexcept;
		void StuffInnerGameObject(std::vector<JUserPtr<JGameObject>>& objList,
			uint& listIndex,
			JAcceleratorAlignInfo::AlignPassCondPtr cond = nullptr);
	private:
		JKdTreeNode* FindRightLeafNode() noexcept; 
		JAcceleratorIntersectInfo::Result FindFirstntersect(JAcceleratorIntersectInfo& info)const noexcept;
		void FindIntersectNotSort(JAcceleratorIntersectInfo& info)const noexcept;
		void FindContainNotSort(JAcceleratorContainInfo& info)const noexcept;
	public:
		//Debug
		void BuildDebugNode(Editor::JEditorBinaryTreeView& treeView);
		//void AddLog(const std::string& newNodeLog)noexcept;
	};
}