#pragma once 
#include"JAcceleratorType.h"     
#include"Octree/JOctreeOption.h"
#include"Bvh/JBvhOption.h"
#include"Kd-tree/JKdTreeOption.h"
#include"../../../../Core/JCoreEssential.h" 
#include"../../../../Core/Geometry/JBBox.h"
#include"../../../../Graphic/Culling/JCullingInterface.h"
#include<DirectXMath.h>
#include<DirectXCollision.h>
#include<memory>
#include<vector>

namespace JinEngine
{
	class JGameObject;
	class JScene;
	namespace Editor
	{ 
		class JEditorBinaryTreeView;
	}

	class JOctree;
	class JBvh;
	class JKdTree;
	class JAccelerator;
	namespace Core
	{
		class JCullingFrustum;
		class JRay;		 
	}
	class JSceneAcceleratorStructure
	{
	private:
		struct ActivatedOptionCash
		{
		public:
			JOctreeOption preOctreeOption[(int)J_ACCELERATOR_LAYER::COUNT];
			JBvhOption preBvhOption[(int)J_ACCELERATOR_LAYER::COUNT];
			JKdTreeOption preKdTreeOption[(int)J_ACCELERATOR_LAYER::COUNT];
		public:
			bool preHasInnerRoot[(int)J_ACCELERATOR_TYPE::COUNT][(int)J_ACCELERATOR_LAYER::COUNT];
			bool preHasDebugRoot[(int)J_ACCELERATOR_TYPE::COUNT][(int)J_ACCELERATOR_LAYER::COUNT];
			size_t preInnerRootGuid[(int)J_ACCELERATOR_TYPE::COUNT][(int)J_ACCELERATOR_LAYER::COUNT];
			size_t preDebugRootGuid[(int)J_ACCELERATOR_TYPE::COUNT][(int)J_ACCELERATOR_LAYER::COUNT];
		public:
			JOctreeOption GetOctreeOption(const J_ACCELERATOR_LAYER layer)const noexcept;
			JBvhOption GetBvhOption(const J_ACCELERATOR_LAYER layer)const noexcept;
			JKdTreeOption GetKdTreeOption(const J_ACCELERATOR_LAYER layer)const noexcept;
		public:
			void SetOctreeOption(const J_ACCELERATOR_LAYER layer, const JOctreeOption& octreeOption);
			void SetBvhOption(const J_ACCELERATOR_LAYER layer, const JBvhOption& bvhOption);
			void SetKdTreeOption(const J_ACCELERATOR_LAYER layer, const JKdTreeOption& kdTreeOption);
		private:
			void LoadRoot(JAcceleratorOption& option, const J_ACCELERATOR_TYPE type, const J_ACCELERATOR_LAYER layer)const noexcept;
			void StoreRoot(const JAcceleratorOption& option, const J_ACCELERATOR_TYPE type, const J_ACCELERATOR_LAYER layer)noexcept;
		};
	private:
		std::unique_ptr<JOctree> octree[(uint)J_ACCELERATOR_LAYER::COUNT];
		std::unique_ptr<JBvh> bvh[(uint)J_ACCELERATOR_LAYER::COUNT];
		std::unique_ptr<JKdTree> kdTree[(uint)J_ACCELERATOR_LAYER::COUNT];
	private:
		std::vector<JAccelerator*> spaceSpatialVec;
	private:
		std::unique_ptr<ActivatedOptionCash> optionCash;
	private:
		JBvhOption debugOptionCash;
	private:
		bool activateTrigger = true;
	public:
		JSceneAcceleratorStructure();
		~JSceneAcceleratorStructure();
	public:
		void Clear()noexcept;
	public:
		//void Culling(const Graphic::JCullingUserInterface& cullUser, const JCullingFrustum& camFrustum)noexcept;  //unuse
		void Culling(JAcceleratorCullingInfo& info)noexcept;
		void Intersect(JAcceleratorIntersectInfo& info)const noexcept;
		void Contain(JAcceleratorContainInfo& info)const noexcept;
		/**
		* prior condition: kdtree is activated
		*/
		std::vector<JUserPtr<JGameObject>> AlignedObject(JAcceleratorAlignInfo& info)const noexcept;
		/**
		* fast version
		* aligned vector는 algined할 layer에 render item만큼 크기를 갖고있어야한다.
		* 크기가 부족할시 함수내부에서 재할당한다.
		*/
		void AlignedObjectF(JAcceleratorAlignInfo& info, _Out_ std::vector<JUserPtr<JGameObject>>& aligned, _Out_ int& validCount)const noexcept;
	public:
		void UpdateGameObject(const JUserPtr<JGameObject>& gameObject)noexcept;
	public:
		void AddGameObject(const JUserPtr<JGameObject>& gameObject)noexcept;
		void RemoveGameObject(const JUserPtr<JGameObject>& gameObject)noexcept;
	public:
		//Option 
		JOctreeOption GetOctreeOption(const J_ACCELERATOR_LAYER layer)const noexcept;
		JBvhOption GetBvhOption(const J_ACCELERATOR_LAYER layer)const noexcept;
		JKdTreeOption GetKdTreeOption(const J_ACCELERATOR_LAYER layer)const noexcept;
		Core::JBBox GetSceneBBox(const J_ACCELERATOR_LAYER layer, _Out_ bool& isValidBBox)const noexcept;
	public:
		void SetOctreeOption(const J_ACCELERATOR_LAYER layer, const JOctreeOption& option);
		void SetBvhOption(const J_ACCELERATOR_LAYER layer, const JBvhOption& option);
		void SetKdTreeOption(const J_ACCELERATOR_LAYER layer, const JKdTreeOption& option);
		//void SetDebugKdTreeOption(const JKdTreeOption& option);
	public:
		bool IsActivated(const J_ACCELERATOR_LAYER layer, const J_ACCELERATOR_TYPE type);
		bool IsActivated(const J_ACCELERATOR_LAYER layer);
	public:
		void Activate()noexcept;
		void DeAcitvate()noexcept;
	public:
		void BuildDebugTree(const J_ACCELERATOR_TYPE type,
			const J_ACCELERATOR_LAYER layer,
			Editor::JEditorBinaryTreeView& tree)noexcept;
	};
}