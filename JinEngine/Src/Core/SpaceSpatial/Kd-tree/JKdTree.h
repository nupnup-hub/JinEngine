#pragma once
#include"JKdTreeNode.h"
#include"JKdTreeOption.h"
#include"../JSpaceSpatial.h"
#include"../JSpaceSpatialType.h"
#include<memory>
#include<vector>
#include<unordered_map>
#include<DirectXCollision.h>

namespace JinEngine
{
	class JGameObject;
	namespace Editor
	{
		class JEditorBinaryTreeView;
	}
	namespace Core
	{
		struct BoundEdge;
		class JCullingFrustum;
		class JBBox; 
		class JKdTree : public JSpaceSpatial
		{
		private:
			JKdTreeNode* root = nullptr;	
			std::vector<std::unique_ptr<JKdTreeNode>> allNodes; 
			J_SPACE_SPATIAL_BUILD_TYPE buildType = J_SPACE_SPATIAL_BUILD_TYPE::TOP_DOWN;
			J_SPACE_SPATIAL_SPLIT_TYPE splitType = J_SPACE_SPATIAL_SPLIT_TYPE::SAH; 
			float isectCost = 80;
			float traversalCost = 1;
			float emptyBonus = 0.5f;
			uint leastObj = 2; 
			uint alignLeafNodeCount = 64;
		public:
			JKdTree(const J_SPACE_SPATIAL_LAYER layer);
			~JKdTree();
		protected:
			void Build()noexcept final;
			void UnBuild()noexcept final;
		public:
			void Clear()noexcept final;
		protected:
			void OnDebugGameObject()noexcept final;
			void OffDebugGameObject()noexcept final; 
		public:
			void Culling(Graphic::JCullingUserInterface& cullUser, const JCullingFrustum& camFrustum)noexcept final;
			void Culling(Graphic::JCullingUserInterface& cullUser, const DirectX::BoundingFrustum& camFrustum, const DirectX::BoundingFrustum& cullingFrustum)noexcept final;
			JUserPtr<JGameObject> IntersectFirst(const JRay& ray, const bool allowContainRayPos)const noexcept final;
			void Intersect(const JRay& ray, const J_SPACE_SPATIAL_SORT_TYPE sortType, _Out_ std::vector<JUserPtr<JGameObject>>& res)const noexcept final;
			void UpdateGameObject(const JUserPtr<JGameObject>& gameObject)noexcept final;
		public:
			void AddGameObject(const JUserPtr<JGameObject>& newGameObject)noexcept final;
			void RemoveGameObject(const JUserPtr<JGameObject>& gameObj)noexcept final;
		public:
			J_SPACE_SPATIAL_TYPE GetType()const noexcept final;
			uint GetNodeCount()const noexcept;
			JKdTreeOption GetKdTreeOption()const noexcept;
			std::vector<JUserPtr<JGameObject>> GetAlignedObject(const JVector3<float>& pos)const noexcept;
			std::vector<JUserPtr<JGameObject>> GetAlignedObject(const DirectX::BoundingFrustum& camFrustum)const noexcept;
			void SetKdTreeOption(const JKdTreeOption& newOption)noexcept;
		private: 
			void BuildKdTree(JKdTreeNode* parentNode, const std::vector<JUserPtr<JGameObject>>& objList, const uint nodeNumOffsset);
			void BuildTopDownKdTree(JKdTreeNode* parentNode,
				const JBBox nodeBounds,
				const std::vector<JUserPtr<JGameObject>>& objList,
				uint nObject,
				uint* objIndexList,
				uint* objIndexListBufL,
				uint* objIndexListBufR,
				std::vector<std::unique_ptr<JKdTreeNode>>& additionalNode,
				std::vector<std::vector<BoundEdge>>& edge,
				int badRefines,
				int depth,
				const uint nodeNumOffset)noexcept; 
			void FindBestValue(const JBBox& nodeBounds,
				const std::vector<JUserPtr<JGameObject>>& objList,
				uint nObject,
				uint* objIndexList,
				std::vector<BoundEdge>& edge, 
				const uint dim,
				_Out_ int& bestDim,
				_Out_ int& bestOffset,
				_Out_ float& bestCost)noexcept;
			void InitLeafNode(JKdTreeNode* node, const std::vector<JUserPtr<JGameObject>>& objList, uint nObject, uint* objIndexList)noexcept;
		private:
			void ClearKdTreeNode(const uint nodeNumber);
			void ReBuildKdTreeNode(const uint nodeNumber, JUserPtr<JGameObject> additionalObj = nullptr);
		private:
			void FindHasNode(JKdTreeNode* node, const JUserPtr<JGameObject>& gobj, const DirectX::BoundingBox& bbox, std::vector<JKdTreeNode*>& hitNode);
			JKdTreeNode* FindContainNode(JKdTreeNode* node, const DirectX::BoundingBox& bbox);
		private: 
			uint CalculateMaxDepth(const uint objCount)const noexcept;
			JBBox CalculateObjectBBox(const std::vector<JUserPtr<JGameObject>>& objList, const uint st, const uint ed)const noexcept;
		public:
			void BuildDebugTree(Editor::JEditorBinaryTreeView& treeView);
		};
	}
}