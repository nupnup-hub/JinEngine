#pragma once
#include"../../JDataType.h"
#include"../../Geometry/JCullingFrustum.h"
#include"../../../Utility/JVector.h"
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
	namespace Core
	{ 
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
		class JKdTreeNode
		{
		private:
			uint nodeNumber; 
			J_KDTREE_NODE_TYPE nodeType; 
			J_KDTREE_NODE_SPLIT_AXIS splitType;
			DirectX::BoundingBox bbox;
			JKdTreeNode* parent = nullptr;
			JKdTreeNode* left = nullptr;
			JKdTreeNode* right = nullptr; 
			JGameObject* debugGameObject = nullptr;
			std::vector<JGameObject*> innerGameObject;
		private:
			//Debug
			//std::string nodeLog;
		public:
			JKdTreeNode(const uint nodeNumber, const J_KDTREE_NODE_TYPE nodeType, const DirectX::BoundingBox& bbox, JKdTreeNode* parent);
			~JKdTreeNode();
		public:
			void CreateDebugGameObject(JGameObject* parent, bool onlyLeafNode)noexcept;
			void DestroyDebugGameObject()noexcept;
			void Clear()noexcept;
			void Culling(const JCullingFrustum& camFrustum, J_CULLING_FLAG flag)noexcept;
			void Culling(const DirectX::BoundingFrustum& camFrustum)noexcept;
			void OffCulling()noexcept;
		public:
			bool IsLeftNode()const noexcept;
			bool IsChildNode(const uint pNodeNumber)const noexcept;
		public:
			uint GetNodeNumber()const noexcept;
			uint GetLeftNumberEnd()const noexcept;
			uint GetRightNumberEnd()const noexcept;
			J_KDTREE_NODE_TYPE GetNodeType()const noexcept;
			DirectX::BoundingBox GetBoundingBox()const noexcept;
			JKdTreeNode* GetParentNode()noexcept;
			JKdTreeNode* GetLeftNode()const noexcept;
			JKdTreeNode* GetRightNode()const noexcept;		
			std::vector<JGameObject*> GetInnerGameObject()const noexcept;
			std::vector<JGameObject*> GetAreaInnerGameObject()const noexcept;
			uint GetInnerGameObjectCount()const noexcept;
			uint GetDepth()const noexcept;
			void GetAlignLeafNode(const JVector3<float>& pos, std::vector<JKdTreeNode*>& alignNode, uint& index)noexcept;
			void GetAlignLeafNode(const DirectX::BoundingFrustum& camFrustum, std::vector<JKdTreeNode*>& alignNode, uint& index)noexcept;
			void SetNodeNumber(const uint newNodeNumber)noexcept; 
			void SetSplitType(const uint newSplitType)noexcept;
			void SetSplitType(const J_KDTREE_NODE_SPLIT_AXIS newSplitType)noexcept;
			void SetLeftNode(JKdTreeNode* newLeftNode)noexcept;
			void SetRightNode(JKdTreeNode* newRightNode)noexcept;
		public:
			bool HasGameObject(const size_t guid)noexcept;
			void AddInnerGameObject(JGameObject* newInnerGameObject)noexcept;
			void AddInnerGameObject(const std::vector<JGameObject*>& newInnerGameObject)noexcept;
			void RemoveInnerGameObject(const size_t guid)noexcept;
			void StuffInnerGameObject(std::vector<JGameObject*>& objList, uint& offset);
		private: 
			void SetVisible()noexcept;
			void SetInVisible()noexcept;
			JKdTreeNode* FindRightLeafNode() noexcept;
		public:
			//Debug
			void BuildDebugNode(Editor::JEditorBinaryTreeView& treeView);
			//void AddLog(const std::string& newNodeLog)noexcept;
		};
	}
}