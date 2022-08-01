#pragma once
#include"../../../Core/SpaceSpatial/JSceneSpatialStructureType.h"
#include"../../../Core/SpaceSpatial/Bvh/JBvhType.h"
#include"../../../Core/JDataType.h"

namespace JinEngine
{
	class ISceneSpatialStructure
	{  
		friend class JResourceManager;
	public:
		virtual ~ISceneSpatialStructure() = default;
	private:
		virtual void ViewCulling()noexcept = 0;

		virtual void OnSceneSpatialStructure()noexcept = 0;
		virtual void OffSceneSpatialStructure()noexcept = 0;

		virtual void OnDebugBoundingBox(bool onlyLeafNode)noexcept = 0;
		virtual void OffDebugBoundingBox()noexcept = 0;

		virtual void CreateDemoGameObject()noexcept = 0;
		virtual void EraseDemoGameObject()noexcept = 0;

		virtual void BuildOctree(const uint octreeSizeSquare = 16, const float looseFactor = 1.15f, const bool isLooseOctree = false)noexcept = 0;
		virtual void BuildBvh(const Core::J_BVH_BUILD_TYPE bvhBuildType = Core::J_BVH_BUILD_TYPE::TOP_DOWN, 
			const Core::J_BVH_SPLIT_TYPE splitType = Core::J_BVH_SPLIT_TYPE::SAH)noexcept = 0;
	};
}