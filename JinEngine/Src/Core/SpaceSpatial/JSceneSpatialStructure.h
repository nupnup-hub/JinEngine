#pragma once 
#include"JSpaceSpatialType.h"  
#include"../JDataType.h" 
#include"Octree/JOctreeOption.h"
#include"Bvh/JBvhOption.h"
#include"Kd-tree/JKdTreeOption.h"
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
	namespace Core
	{
		class JOctree;
		class JBvh;
		class JKdTree;
		class JSpaceSpatial;
		class JCullingFrustum;
		class JSceneSpatialStructure
		{ 
		private:
			struct ActivatedOptionCash
			{
			private:
				static constexpr uint8 octreeIndex = (int)J_SPACE_SPATIAL_TYPE::OCTREE;
				static constexpr uint8 bvhIndex = (int)J_SPACE_SPATIAL_TYPE::BVH;
				static constexpr uint8 kdTreeIndex = (int)J_SPACE_SPATIAL_TYPE::KD_TREE;
			public:
				JOctreeOption preOctreeOption;
				JBvhOption preBvhOption;
				JKdTreeOption preKdTreeOption;
			public:
				bool preHasInnerRoot[(int)J_SPACE_SPATIAL_TYPE::COUNT];
				bool preHasDebugRoot[(int)J_SPACE_SPATIAL_TYPE::COUNT];
				size_t preInnerRootGuid[(int)J_SPACE_SPATIAL_TYPE::COUNT];
				size_t preDebugRootGuid[(int)J_SPACE_SPATIAL_TYPE::COUNT];
			public:
				ActivatedOptionCash(const JOctreeOption& octreeOption, const JBvhOption& bvhOption, const JKdTreeOption& kdTreeOption);
			public:
				JOctreeOption GetOctreeOption()const noexcept;
				JBvhOption GetBvhOption()const noexcept;
				JKdTreeOption GetKdTreeOption()const noexcept;
			public:
				void SetOctreeOption(const JOctreeOption& octreeOption);
				void SetBvhOption(const JBvhOption& bvhOption);
				void SetKdTreeOption(const JKdTreeOption& kdTreeOption);
			};
		private:
			std::unique_ptr<JOctree> octree;
			std::unique_ptr<JBvh> bvh;   
			std::unique_ptr<JKdTree> kdTree;
		private:
			std::vector<JSpaceSpatial*> spaceSpatialVec;
		private:
			std::unique_ptr<ActivatedOptionCash> optionCash;				//cash predata
		private:
			bool activateTrigger = true;
			bool initTrigger = false; 
		public:
			JSceneSpatialStructure();
			~JSceneSpatialStructure();
		public: 
			void Clear()noexcept;
		public:
			void Culling(const JCullingFrustum& camFrustum)noexcept;  
			void Culling(const DirectX::BoundingFrustum& camFrustum)noexcept;
			void UpdateGameObject(JGameObject* gameObject)noexcept;
		public:
			void AddGameObject(JGameObject* gameObject)noexcept;
			void RemoveGameObject(JGameObject* gameObject)noexcept;
		public:
			std::vector<JGameObject*> GetAlignedObject(const DirectX::BoundingFrustum& camFrustum)const noexcept;
		public:
			//Option 
			JOctreeOption GetOctreeOption()const noexcept;
			JBvhOption GetBvhOption()const noexcept;
			JKdTreeOption GetKdTreeOption()const noexcept;
			void SetOctreeOption(const JOctreeOption& option);
			void SetBvhOption(const JBvhOption& option);
			void SetKdTreeOption(const JKdTreeOption& option);
			void SetInitTrigger(const bool value)noexcept; 
		public:
			bool IsInitTriggerActivated()const noexcept;
		public:
			void Activate()noexcept;
			void DeAcitvate()noexcept;
		public:
			void BuildDebugTree(Core::J_SPACE_SPATIAL_TYPE type, Editor::JEditorBinaryTreeView& tree)noexcept; 
		};
	}
}