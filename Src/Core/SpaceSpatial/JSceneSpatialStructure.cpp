#include"JSceneSpatialStructure.h"
#include"Octree/JOctree.h"
#include"Bvh/JBvh.h"

namespace JinEngine
{
	namespace Core
	{
		JSceneSpatialStructure::JSceneSpatialStructure() {}
		JSceneSpatialStructure::~JSceneSpatialStructure() {}

		void JSceneSpatialStructure::BuildOctree(std::vector<JGameObject*>& gameObject,
			const uint octreeSizeSquare,
			const float looseFactor,
			const bool isLooseOctree)noexcept
		{
			Clear();
			octree = std::make_unique<JOctree>(gameObject, octreeSizeSquare, looseFactor, isLooseOctree);
			spatialStructureType = J_SCENE_SPATIAL_STRUCTURE_TYPE::OCTREE;
		}
		void JSceneSpatialStructure::BuildBvh(std::vector<JGameObject*>& gameObject,
			const J_BVH_BUILD_TYPE bvhBuildType,
			const J_BVH_SPLIT_TYPE splitType)noexcept
		{
			Clear();
			bvh = std::make_unique<JBvh>(gameObject, bvhBuildType, splitType);
			spatialStructureType = J_SCENE_SPATIAL_STRUCTURE_TYPE::BVH;
		}
		void JSceneSpatialStructure::Clear()noexcept
		{
			if (octree != nullptr)
			{
				octree->Clear();
				octree.reset();
			}

			if (bvh != nullptr)
			{
				bvh->Clear();
				bvh.reset();
			}
		}
		void JSceneSpatialStructure::OnDebugBoundingBox(JGameObject* parent, bool onlyLeafNode)noexcept
		{
			if (parent == nullptr)
				return;

			switch (spatialStructureType)
			{
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::OCTREE:
				if (octree != nullptr)
					octree->OnDebugGameObject(parent, onlyLeafNode);
				break;
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::BVH:
				if (bvh != nullptr)
					bvh->OnDebugGameObject(parent, onlyLeafNode);
				break;
			default:
				break;
			}
		}
		void JSceneSpatialStructure::OffDebugBoundingBox()noexcept
		{
			switch (spatialStructureType)
			{
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::OCTREE:
				if (octree != nullptr)
					octree->OffDebugGameObject();
				break;
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::BVH:
				if (bvh != nullptr)
					bvh->OffDebugGameObject();
				break;
			default:
				break;
			}
		}
		void JSceneSpatialStructure::Culling(const JCullingFrustum& camFrustum)noexcept
		{
			switch (spatialStructureType)
			{
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::OCTREE:
				if (octree != nullptr)
					;//octree->Culling(camFrustum);
				break;
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::BVH:
				if (bvh != nullptr)
					bvh->Culling(camFrustum);
				break;
			default:
				break;
			}
		}
		void JSceneSpatialStructure::Culling(const DirectX::BoundingFrustum& camFrustum)noexcept
		{
			switch (spatialStructureType)
			{
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::OCTREE:
				if (octree != nullptr)
					octree->Culling(camFrustum);
				break;
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::BVH:
				if (bvh != nullptr)
					bvh->Culling(camFrustum);
				break;
			default:
				break;
			}
		}
		void JSceneSpatialStructure::UpdateGameObject(const size_t guid)noexcept
		{
			switch (spatialStructureType)
			{
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::OCTREE:
				if (octree != nullptr)
					//octree->Culling(camFrustum);
					break;
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::BVH:
				if (bvh != nullptr)
					bvh->UpdateGameObject(guid);
				break;
			default:
				break;
			}
		}
		void JSceneSpatialStructure::AddGameObject(JGameObject* gameObject)noexcept
		{
			switch (spatialStructureType)
			{
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::OCTREE:
				if (octree != nullptr)
					//octree->Culling(camFrustum);
					break;
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::BVH:
				if (bvh != nullptr)
					bvh->AddGameObject(gameObject);
				break;
			default:
				break;
			}
		}
		void JSceneSpatialStructure::RemoveGameObject(JGameObject* gameObject)noexcept
		{
			switch (spatialStructureType)
			{
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::OCTREE:
				if (octree != nullptr)
					//octree->Culling(camFrustum);
					break;
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::BVH:
				if (bvh != nullptr)
					bvh->RemoveGameObject(gameObject);
				break;
			default:
				break;
			}
		}
	}
}