/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once
#include"../JAcceleratorNode.h"
#include"../JAcceleratorOption.h"   
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