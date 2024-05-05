#pragma once 
#include"JBvhNode.h" 
#include"JBvhOption.h"
#include"../JAccelerator.h"
#include"../JAcceleratorType.h"   
#include<DirectXCollision.h>

namespace JinEngine
{
	class JGameObject; 
	class JBvh : public JCpuAccelerator
	{
	private:
		//root node allways existing until call unbuld or clear
		JBvhNode* root = nullptr;
		std::vector<std::unique_ptr<JBvhNode>> allNodes;
		std::unordered_map<size_t, JBvhNode*> leafNodeMap;
		//Bvh에 JGameObject가 1개일시 트리를 생성하지 않고 캐싱
		//이 후 JGameObject가 2개이상이될시 트리 생성
		JUserPtr<JGameObject> innerGameObjectCandidate = nullptr;
		J_ACCELERATOR_BUILD_TYPE buildType = J_ACCELERATOR_BUILD_TYPE::TOP_DOWN;
		J_ACCELERATOR_SPLIT_TYPE splitType = J_ACCELERATOR_SPLIT_TYPE::SAH;
	private:
		static constexpr uint bucketCount = 12;
	public:
		JBvh(const J_ACCELERATOR_LAYER layer);
		~JBvh();
	protected:
		void Build()noexcept final;
		void UnBuild()noexcept final;
	public:
		void Clear()noexcept final;
	protected:
		void OnDebugGameObject()noexcept final;
		void OffDebugGameObject()noexcept final;
	public:
		void Culling(JAcceleratorCullingInfo& info)noexcept final;
		void Intersect(JAcceleratorIntersectInfo& info)const noexcept final;
		void Contain(JAcceleratorContainInfo& info)const noexcept final;
		void AlignedObject(JAcceleratorAlignInfo& info, _Out_ std::vector<JUserPtr<JGameObject>>& aligned, _Out_ int& validCount)const noexcept;
	public:
		void UpdateGameObject(const JUserPtr<JGameObject>& gameObject)noexcept final;
	public:
		void AddGameObject(const JUserPtr<JGameObject>& newGameObject)noexcept final;
		void RemoveGameObject(const JUserPtr<JGameObject>& gameObj)noexcept final;
	public:
		J_ACCELERATOR_TYPE GetType()const noexcept final;
		Core::JBBox GetSceneBBox()const noexcept final;
		uint GetNodeCount()const noexcept;
		JBvhOption GetBvhOption()const noexcept; 
	private:
		DirectX::BoundingBox GetRootBoundingBox()const noexcept;
		float GetDimensionValue(const DirectX::XMFLOAT3& point, const int dim)const noexcept;
	public:
		void SetBvhOption(const JBvhOption& newOption)noexcept;
	private:
		void BuildTopdownBvh(JBvhNode* parent,
			std::vector<JUserPtr<JGameObject>>& objectList,
			std::vector<std::unique_ptr<JBvhNode>>& nodeVec,
			const int start,
			const int end,
			const int numberOffset)noexcept;
		void ReBuildBvh(const uint nodeNumber, const JUserPtr<JGameObject>& additionalGameObj = nullptr)noexcept;
	private:
		void ClearBvhNode(const uint nodeNumber)noexcept;
		void DestroyBvhNode(const uint nodeNumber)noexcept;
	public:
		void BuildDebugTree(JAcceleratorVisualizeInterface* treeView);
	};
}