#pragma once 
#include"JSceneSpatialStructureType.h"
#include"../JDataType.h"
#include"Bvh/JBvhType.h"
#include<DirectXMath.h>
#include<DirectXCollision.h>
#include<memory>
#include<vector>

namespace JinEngine
{
	class JGameObject;
	class JScene;
	namespace Core
	{
		class JBvh;
		class JOctree;
		class JCullingFrustum;
		class JSceneSpatialStructure
		{
		private:
			std::unique_ptr<JOctree> octree;
			std::unique_ptr<JBvh> bvh;
			J_SCENE_SPATIAL_STRUCTURE_TYPE spatialStructureType;
		public:
			JSceneSpatialStructure();
			~JSceneSpatialStructure();

			void BuildOctree(std::vector<JGameObject*>& gameObject,
				const uint octreeSizeSquare,
				const float looseFactor,
				const bool isLooseOctree)noexcept;
			void BuildBvh(std::vector<JGameObject*>& gameObject,
				const J_BVH_BUILD_TYPE bvhBuildType,
				const J_BVH_SPLIT_TYPE splitType)noexcept;
			void Clear()noexcept;

			void OnDebugBoundingBox(JGameObject* parent, bool onlyLeafNode)noexcept;
			void OffDebugBoundingBox()noexcept;

			void Culling(const JCullingFrustum& camFrustum)noexcept;
			void Culling(const DirectX::BoundingFrustum& camFrustum)noexcept;

			void UpdateGameObject(const size_t guid)noexcept;

			void AddGameObject(JGameObject* gameObject)noexcept;
			void RemoveGameObject(JGameObject* gameObject)noexcept;
		};
	}
}