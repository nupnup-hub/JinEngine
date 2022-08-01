#pragma once
#include"ISceneFrameDirty.h"
#include"ISceneGameObjectEvent.h"
#include"ISceneSpatialStructure.h"
#include"ISceneAnimator.h"

namespace JinEngine
{
	class IScene abstract : public ISceneFrameDirty, 
		public ISceneGameObjectEvent, 
		public ISceneSpatialStructure,
		public ISceneAnimator
	{

	};
}