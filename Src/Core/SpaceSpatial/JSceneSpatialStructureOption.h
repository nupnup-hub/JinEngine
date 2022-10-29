#pragma once
#include"JSceneSpatialStructureType.h"
namespace JinEngine
{
	namespace Core
	{
		class JSceneSpatialStructureOption
		{
		public:
			const J_SCENE_SPATIAL_STRUCTURE_TYPE type;
			const bool isSpatialStructureActivated;
			const bool isDebugActivated;
			const bool isDebugOnlyLeaf;
		public:
			JSceneSpatialStructureOption(const J_SCENE_SPATIAL_STRUCTURE_TYPE type,
				const bool isSpatialStructureActivated,
				const bool isDebugActivated,
				const bool isDebugOnlyLeaf);
		};
	}
}