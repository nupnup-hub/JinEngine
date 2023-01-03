#pragma once 
#include"JBvhNode.h" 
#include"JBvhOption.h"
#include"../JSpaceSpatial.h"
#include"../JSpaceSpatialType.h"
#include"../../JDataType.h"
#include<vector>
#include<unordered_map>
#include<memory>
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
		class JCullingFrustum;
		class JBBox;
		class JBvh : public JSpaceSpatial
		{
		private:
			JBvhNode* root = nullptr;
			std::vector<std::unique_ptr<JBvhNode>> allNodes;
			std::unordered_map<size_t, JBvhNode*> leafNodeMap;
			//Bvh�� JGameObject�� 1���Ͻ� Ʈ���� �������� �ʰ� ĳ��
			//�� �� JGameObject�� 2���̻��̵ɽ� Ʈ�� ����
			JGameObject* innerGameObjectCandidate = nullptr; 
			J_SPACE_SPATIAL_BUILD_TYPE buildType = J_SPACE_SPATIAL_BUILD_TYPE::TOP_DOWN;
			J_SPACE_SPATIAL_SPLIT_TYPE splitType = J_SPACE_SPATIAL_SPLIT_TYPE::SAH;
		private:
			static constexpr uint bucketCount = 12;
		public:
			JBvh(const J_SPACE_SPATIAL_LAYER layer);
			~JBvh();
		protected:
			void Build()noexcept final;
			void UnBuild()noexcept final;
		public:
			void Clear()noexcept final;
		protected:
			void OnDebugGameObject()noexcept final;
			void OffDebugGameObject()noexcept final;
			void OffCulling()noexcept final;
		public:
			void Culling(const JCullingFrustum& camFrustum)noexcept final; 
			void Culling(const DirectX::BoundingFrustum& camFrustum, const DirectX::FXMVECTOR camPos)noexcept final;
			JGameObject* IntersectFirst(const JRay& ray)const noexcept final;
			void Intersect(const JRay& ray, const J_SPACE_SPATIAL_SORT_TYPE sortType, _Out_ std::vector<JGameObject*>& res)const noexcept final;
			void UpdateGameObject(JGameObject* gameObject)noexcept final;
		public:
			void AddGameObject(JGameObject* newGameObject)noexcept final;
			void RemoveGameObject(JGameObject* gameObj)noexcept final;
		public:
			J_SPACE_SPATIAL_TYPE GetType()const noexcept final;
			uint GetNodeCount()const noexcept;
			JBvhOption GetBvhOption()const noexcept;
			void SetBvhOption(const JBvhOption& newOption)noexcept;
		private:
			DirectX::BoundingBox GetRootBoundingBox()const noexcept; 
			float GetDimensionValue(const DirectX::XMFLOAT3& point, const int dim)const noexcept;
		private:
			void BuildTopdownBvh(JBvhNode* parent,
				std::vector<JGameObject*>& objectList,
				std::vector<std::unique_ptr<JBvhNode>>& nodeVec,
				const int start,
				const int end,
				const int numberOffset)noexcept;
			void ReBuildBvh(const uint nodeNumber, JGameObject* additionalGameObj = nullptr)noexcept;
		private:
			void ClearBvhNode(const uint nodeNumber)noexcept;
			void DestroyBvhNode(const uint nodeNumber)noexcept;
		public:
			void BuildDebugTree(Editor::JEditorBinaryTreeView& treeView);
		};
	}
}