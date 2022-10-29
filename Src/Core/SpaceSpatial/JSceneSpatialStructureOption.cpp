#include"JSceneSpatialStructureOption.h"

namespace JinEngine
{
	namespace Core
	{
		JSceneSpatialStructureOption::JSceneSpatialStructureOption(const J_SCENE_SPATIAL_STRUCTURE_TYPE type,
			const bool isSpatialStructureActivated,
			const bool isDebugActivated,
			const bool isDebugOnlyLeaf)
			:type(type), 
			isSpatialStructureActivated(isSpatialStructureActivated),
			isDebugActivated(isDebugActivated), 
			isDebugOnlyLeaf(isDebugOnlyLeaf)
		{}
	}
}