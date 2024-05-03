#pragma once
#include"JOctreeNode.h"
#include"JOctreeOption.h"
#include"../JAccelerator.h" 
#include<DirectXCollision.h>
#include<memory>
#include<vector>
#include<unordered_map>

namespace JinEngine
{
	//�ʱ�ȭ������ ����
	//dynamic �߰� ���� ���� �ʿ�
	class JGameObject;
	class JOctree : public JCpuAccelerator
	{
	private:
		JOctreeNode* rootNodeCash = nullptr;
		std::vector<std::unique_ptr<JOctreeNode>> allNode;
		std::unordered_map<size_t, JOctreeNode*> containNodeMap;
		uint minSize = 32;
		uint octreeSizeSquare = 9;
		uint octreeSize = 1 << octreeSizeSquare;
		//2023-12-29 note
		//�ش� node���� �ξ����� object�� ����Ǵ� ��츦 ��������(Contain = pass child, intersect = store this node)
		//loosefactor���
		//node ũ��� ����� object��ũ�Ⱑ ������Ҷ� �������� ȿ���� ���� ����.
		float looseFactor = 2;
		//2022-11-04~ JOctree is looseOctree 
		static constexpr bool isLooseOctree = true;
		static constexpr float looseFactorMin = 2;
		static constexpr float looseFactorMax = 4;
		static constexpr uint neighBorIndexList[8][3]
		{
			 { 1,2,4 },{ 0,3,5 },{ 0,3,6 },{ 1,2,7 },
			 { 0,5,6 },{ 1,4,7 },{ 2,4,7 },{ 3,5,6 }
		};
	public:
		JOctree(const J_ACCELERATOR_LAYER layer);
		~JOctree();
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
		JOctreeOption GetOctreeOption()const noexcept;
	private:
		uint GetExtentLength(const uint depth)const noexcept;
		uint GetLooseExtentLength(const uint depth)const noexcept;
	public:
		void SetOctreeOption(const JOctreeOption& newOption)noexcept;
	private:
		void BuildOctree()noexcept;
		void BuildOctreeNode(JOctreeNode* parent, const uint depth)noexcept;
	private:
		void FindNeighborOctreeNode(JOctreeNode* node)noexcept;
		JOctreeNode* FindNodeNearCenter(JOctreeNode* node, const DirectX::XMVECTOR point)noexcept;
		JOctreeNode* FindOptimalNode(JOctreeNode* node, const DirectX::BoundingOrientedBox& bbox)noexcept;
	};
}