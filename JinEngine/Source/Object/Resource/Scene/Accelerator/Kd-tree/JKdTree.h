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
#include"JKdTreeNode.h"
#include"JKdTreeOption.h"
#include"../JAccelerator.h"
#include"../JAcceleratorType.h"
#include"../JAcceleratorVisualizeInterface.h"  
#include<DirectXCollision.h>

namespace JinEngine
{
	class JGameObject;
	struct BoundEdge;
	namespace Editor
	{
		class JEditorBinaryTreeView;
	} 
	class JKdTree : public JCpuAccelerator
	{
	private:
		JKdTreeNode* root = nullptr;
		std::vector<std::unique_ptr<JKdTreeNode>> allNodes;
		J_ACCELERATOR_BUILD_TYPE buildType = J_ACCELERATOR_BUILD_TYPE::TOP_DOWN;
		J_ACCELERATOR_SPLIT_TYPE splitType = J_ACCELERATOR_SPLIT_TYPE::SAH;
		float isectCost = 80;
		float traversalCost = 1;
		float emptyBonus = 0.5f;
		uint leastObj = 2;
		uint defaultAlignLeafNodeCount = 32;
		uint leafNodeCount = 0; 
	public:
		JKdTree(const J_ACCELERATOR_LAYER layer);
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
		JKdTreeOption GetKdTreeOption()const noexcept;
	public:
		void SetKdTreeOption(const JKdTreeOption& newOption)noexcept;
	private:
		void BuildKdTree(JKdTreeNode* parentNode, const std::vector<JUserPtr<JGameObject>>& objList, const uint nodeNumOffsset);
		void BuildTopDownKdTree(JKdTreeNode* parentNode,
			const Core::JBBox nodeBounds,
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
		void FindBestValue(const Core::JBBox& nodeBounds,
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
		Core::JBBox CalculateObjectBBox(const std::vector<JUserPtr<JGameObject>>& objList, const uint st, const uint ed)const noexcept;
	public:
		void BuildDebugTree(JAcceleratorVisualizeInterface* treeView);
	};
}