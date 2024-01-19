#pragma once
#include"../JAcceleratorNode.h"
#include"../JAcceleratorOption.h"  
#include<vector> 
#include<DirectXCollision.h>

namespace JinEngine
{
	class JGameObject; 
	class JOctreeNode : public JAcceleratorNode
	{
	private:
		static constexpr uint childCount = 8;
	private:
		DirectX::BoundingBox boundingBox;
		JOctreeNode* parentNode = nullptr;
		JOctreeNode* childrenNode[childCount] = { nullptr, };
		std::vector<JOctreeNode*> neighborNode;
		std::vector<JUserPtr<JGameObject>> innerGameObject;
		JUserPtr<JGameObject> debugGameObject = nullptr;
	public:
		JOctreeNode(const DirectX::BoundingBox& boundingBox, bool isLooseOctree, JOctreeNode* parentNode = nullptr);
		~JOctreeNode();
		JOctreeNode(const JOctreeNode& rhs) = delete;
		JOctreeNode& operator=(const JOctreeNode& rhs) = delete;
		JOctreeNode(JOctreeNode&& rhs) = default;
		JOctreeNode& operator=(JOctreeNode&& rhs) = default;
	public:
		void CreateDebugGameObject(JUserPtr<JGameObject> parent, bool onlyLeafNode)noexcept;
		void DestroyDebugGameObject()noexcept;
		void Clear();
	public:
		//Culling node bbox
		//void Culling(JAcceleratorCullingInfo& info, Core::J_CULLING_FLAG flag)noexcept;
		void Culling(JAcceleratorCullingInfo& info)noexcept;
		void FindIntersect(JAcceleratorIntersectInfo& info)const noexcept;
		void FindContain(JAcceleratorContainInfo& info)const noexcept;
		void AlignLeafNode(const JAcceleratorAlignInfo& info, std::vector<JUserPtr<JGameObject>>& alignGameObject, uint& index)noexcept;
	public:
		bool AddGameObject(JUserPtr<JGameObject> gameObj, bool isLooseOctree)noexcept;
		bool AddNeighborNode(JOctreeNode* octreeNode)noexcept;
		bool RemoveGameObject(JUserPtr<JGameObject> gameObj)noexcept;
	public:
		bool IsLeafNode()const noexcept;
		bool IsRootNode()const noexcept;
	public:
		DirectX::BoundingBox GetBoundingBox()const noexcept;
		DirectX::BoundingBox GetInnerGameObjectBoundingBox(const uint index)const noexcept;
		JOctreeNode* GetParentNode()noexcept;
		JOctreeNode* GetChildNode(const uint index)noexcept;
		uint GetInnerGameObjectCount()const noexcept;
	private:
		bool RemoveInnerGameObject(JUserPtr<JGameObject> gameObject)noexcept;
	private:
		//void CullingInnerObject(JAcceleratorCullingInfo& info, Core::J_CULLING_FLAG oriFlag);
		void CullingInnerObject(JAcceleratorCullingInfo& info);
	private:
		//void SetVisible(JAcceleratorCullingInfo& info, Core::J_CULLING_FLAG oriFlag)noexcept;
		void SetVisible(JAcceleratorCullingInfo& info)noexcept;
		void SetInVisible(JAcceleratorCullingInfo& info)noexcept;
	private:
		JAcceleratorIntersectInfo::Result FindIntersectFirst(JAcceleratorIntersectInfo& info)const noexcept;
		void FindIntersectNotSort(JAcceleratorIntersectInfo& info)const noexcept;
		void FindContainNotSort(JAcceleratorContainInfo& info)const noexcept; 
	private:
		void StuffChildIntermediateData(JAcceleratorIntersectInfo& info, JAcceleratorIntersectInfo::Intermediate* ptr)const noexcept;
	};
}