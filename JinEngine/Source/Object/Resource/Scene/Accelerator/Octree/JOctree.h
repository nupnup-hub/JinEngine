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
#include"JOctreeNode.h"
#include"JOctreeOption.h"
#include"../JAccelerator.h" 
#include<DirectXCollision.h> 

namespace JinEngine
{
	//초기화까지만 구현
	//dynamic 추가 삭제 구현 필요
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
		//해당 node보다 훨씬작은 object가 저장되는 경우를 막기위해(Contain = pass child, intersect = store this node)
		//loosefactor사용
		//node 크기와 저장될 object에크기가 엇비슷할때 공간구조 효율이 가장 좋다.
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