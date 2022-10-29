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
			const uint minSize,
			const uint octreeSizeSquare,
			const float looseFactor,
			const bool isLooseOctree)noexcept
		{
			Clear();
			octree = std::make_unique<JOctree>(gameObject, minSize, octreeSizeSquare, looseFactor, isLooseOctree);
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
		void JSceneSpatialStructure::OnDebugBoundingBox(JGameObject* parent)noexcept
		{
			if (parent == nullptr)
				return;

			switch (spatialStructureType)
			{
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::OCTREE:
				if (octree != nullptr)
					octree->OnDebugGameObject(parent);
				break;
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::BVH:
				if (bvh != nullptr)
					bvh->OnDebugGameObject(parent);
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
		void JSceneSpatialStructure::UpdateGameObject(JGameObject* gameObject)noexcept
		{
			switch (spatialStructureType)
			{
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::OCTREE:
				if (octree != nullptr)
					octree->UpdateGameObject(gameObject);
				break;
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::BVH:
				if (bvh != nullptr)
					bvh->UpdateGameObject(gameObject);
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
					octree->AddGameObject(gameObject);
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
					octree->RemoveGameObject(gameObject);
				break;
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::BVH:
				if (bvh != nullptr)
					bvh->RemoveGameObject(gameObject);
				break;
			default:
				break;
			}
		}
		JSceneSpatialStructureOption JSceneSpatialStructure::GetOption()const noexcept
		{
			return JSceneSpatialStructureOption(spatialStructureType, IsSpatialStructureActivated(), IsDebugActivated(), IsDebugLeafOnly());
		}
		J_SCENE_SPATIAL_STRUCTURE_TYPE JSceneSpatialStructure::GetSpatialStructureType()const noexcept
		{
			return spatialStructureType;
		}
		void JSceneSpatialStructure::SetSpatialStructureType(const J_SCENE_SPATIAL_STRUCTURE_TYPE newType)noexcept
		{
			spatialStructureType = newType;
		}
		void JSceneSpatialStructure::SetDebugOnlyLeaf(bool value)noexcept
		{
			switch (spatialStructureType)
			{
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::OCTREE:
				if (octree != nullptr)
					octree->SetDebugOnlyLeaf(value);
				break;
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::BVH:
				if (bvh != nullptr)
					bvh->SetDebugOnlyLeaf(value);
				break;
			default:
				break;
			}
		}
		bool JSceneSpatialStructure::IsSpatialStructureActivated()const noexcept
		{
			return octree != nullptr || bvh != nullptr;
		}
		bool JSceneSpatialStructure::IsDebugActivated()const noexcept
		{
			switch (spatialStructureType)
			{
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::OCTREE:
				if (octree != nullptr)
					return octree->IsDebugActivated();
				break;
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::BVH:
				if (bvh != nullptr)
					return bvh->IsDebugActivated();
				break;
			default:
				break;
			}
			return false;
		}
		bool JSceneSpatialStructure::IsDebugLeafOnly()const noexcept
		{
			switch (spatialStructureType)
			{
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::OCTREE:
				if (octree != nullptr)
					return octree->IsDebugLeafOnly();
				break;
			case J_SCENE_SPATIAL_STRUCTURE_TYPE::BVH:
				if (bvh != nullptr)
					return bvh->IsDebugLeafOnly();
				break;
			default:
				break;
			}
			return false;
		}
	}
}