#pragma once 
#include"JSpaceSpatialType.h"  
#include"Octree/JOctreeOption.h"
#include"Bvh/JBvhOption.h"
#include"Kd-tree/JKdTreeOption.h"
#include"../JDataType.h" 
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
		class JRay;
		class JSceneSpatialStructure
		{ 
		private:
			struct ActivatedOptionCash
			{
			public:
				JOctreeOption preOctreeOption[(int)J_SPACE_SPATIAL_LAYER::COUNT];
				JBvhOption preBvhOption[(int)J_SPACE_SPATIAL_LAYER::COUNT];
				JKdTreeOption preKdTreeOption[(int)J_SPACE_SPATIAL_LAYER::COUNT];
			public:
				bool preHasInnerRoot[(int)J_SPACE_SPATIAL_TYPE::COUNT][(int)J_SPACE_SPATIAL_LAYER::COUNT];
				bool preHasDebugRoot[(int)J_SPACE_SPATIAL_TYPE::COUNT][(int)J_SPACE_SPATIAL_LAYER::COUNT];
				size_t preInnerRootGuid[(int)J_SPACE_SPATIAL_TYPE::COUNT][(int)J_SPACE_SPATIAL_LAYER::COUNT];
				size_t preDebugRootGuid[(int)J_SPACE_SPATIAL_TYPE::COUNT][(int)J_SPACE_SPATIAL_LAYER::COUNT];
			public:
				JOctreeOption GetOctreeOption(const J_SPACE_SPATIAL_LAYER layer)const noexcept;
				JBvhOption GetBvhOption(const J_SPACE_SPATIAL_LAYER layer)const noexcept;
				JKdTreeOption GetKdTreeOption(const J_SPACE_SPATIAL_LAYER layer)const noexcept;
			public:
				void SetOctreeOption(const J_SPACE_SPATIAL_LAYER layer, const JOctreeOption& octreeOption);
				void SetBvhOption(const J_SPACE_SPATIAL_LAYER layer, const JBvhOption& bvhOption);
				void SetKdTreeOption(const J_SPACE_SPATIAL_LAYER layer, const JKdTreeOption& kdTreeOption);
			private:
				void LoadRoot(JSpaceSpatialOption& option, const J_SPACE_SPATIAL_TYPE type, const J_SPACE_SPATIAL_LAYER layer)const noexcept;
				void StoreRoot(const JSpaceSpatialOption& option, const J_SPACE_SPATIAL_TYPE type, const J_SPACE_SPATIAL_LAYER layer)noexcept;
			};
		private:
			std::unique_ptr<JOctree> octree[(uint)J_SPACE_SPATIAL_LAYER::COUNT];
			std::unique_ptr<JBvh> bvh[(uint)J_SPACE_SPATIAL_LAYER::COUNT];
			std::unique_ptr<JKdTree> kdTree[(uint)J_SPACE_SPATIAL_LAYER::COUNT];
		private:
			std::vector<JSpaceSpatial*> spaceSpatialVec;
		private:
			std::unique_ptr<ActivatedOptionCash> optionCash;				 
		private:
			JBvhOption debugOptionCash;
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
			JGameObject* Intersect(const J_SPACE_SPATIAL_LAYER layer, const Core::JRay& ray)const noexcept;
			void UpdateGameObject(JGameObject* gameObject)noexcept;
		public:
			void AddGameObject(JGameObject* gameObject)noexcept;
			void RemoveGameObject(JGameObject* gameObject)noexcept;
		public:
			std::vector<JGameObject*> GetAlignedObject(const J_SPACE_SPATIAL_LAYER layer, const DirectX::BoundingFrustum& camFrustum)const noexcept;
		public:
			//Option 
			JOctreeOption GetOctreeOption(const J_SPACE_SPATIAL_LAYER layer)const noexcept;
			JBvhOption GetBvhOption(const J_SPACE_SPATIAL_LAYER layer)const noexcept;
			JKdTreeOption GetKdTreeOption(const J_SPACE_SPATIAL_LAYER layer)const noexcept;
			void SetOctreeOption(const J_SPACE_SPATIAL_LAYER layer, const JOctreeOption& option);
			void SetBvhOption(const J_SPACE_SPATIAL_LAYER layer, const JBvhOption& option);
			void SetKdTreeOption(const J_SPACE_SPATIAL_LAYER layer, const JKdTreeOption& option);
			//void SetDebugKdTreeOption(const JKdTreeOption& option);
			void SetInitTrigger(const bool value)noexcept; 
		public:
			bool IsInitTriggerActivated()const noexcept;
		public:
			void Activate(JGameObject* sceneRoot, JGameObject* sceneDebugRoot)noexcept;
			void DeAcitvate()noexcept;
		public:
			void BuildDebugTree(const Core::J_SPACE_SPATIAL_TYPE type,
				const J_SPACE_SPATIAL_LAYER layer,
				Editor::JEditorBinaryTreeView& tree)noexcept;
		};
	}
}