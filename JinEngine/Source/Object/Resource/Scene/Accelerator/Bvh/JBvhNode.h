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

namespace JinEngine
{
	class JGameObject; 
	class JAcceleratorVisualizeInterface;
	struct JAcceleratorCullingInfo;
	struct JAcceleratorIntersectInfo; 
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
		DirectX::BoundingBox bbox;	//BoundingOriented�� �� ��Ȯ������ �����Ƿ� leaf type Culling������ ����ϰ� �׿ܿ��� BoundingBox�� ����Ѵ�	
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
		//void Culling(JAcceleratorCullingInfo& info, Core::J_CULLING_FLAG flag)noexcept;
		void Culling(JAcceleratorCullingInfo& info)noexcept;
		void FindIntersect(JAcceleratorIntersectInfo& info)const noexcept;
		void FindContain(JAcceleratorContainInfo& info)const noexcept;
		/**
		* @brief alignGameObject�� �׻� �� gameObject ������ŭ resize ���־�� �Ѵ�.
		*/
		void AlignLeafNode(const JAcceleratorAlignInfo& info, std::vector<JUserPtr<JGameObject>>& alignGameObject, uint& index, const uint depth)noexcept;
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
		bool IsNearRight(const JAcceleratorCullingInfo& info);
		bool IsNearRight(const JAcceleratorAlignInfo& info);
	private:
		JBvhNode* FindRightLeafNode()noexcept;
		JAcceleratorIntersectInfo::Result FindFirstIntersect(JAcceleratorIntersectInfo& info, const float dist)const noexcept;
		void FindIntersectNotSort(JAcceleratorIntersectInfo& info, const float dist)const noexcept;
		void FindContainNotSort(JAcceleratorContainInfo& info)const noexcept;
	public:
		void BuildDebugNode(JAcceleratorVisualizeInterface* treeView);
	};
}