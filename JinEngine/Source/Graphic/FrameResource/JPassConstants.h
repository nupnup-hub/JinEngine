#pragma once
#include"../../Core/Math/JVector.h"
#include"../../Core/JCoreEssential.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JEnginePassConstants
		{ 
			float appTotalTime = 0.0f;
			float appDeltaTime = 0.0f;	  
			int missingTextureIndex = 0;		//always exist until engine end
			int bluseNoiseTextureIndex = 0;		//always exist until engine end
			JVector2F bluseNoiseTextureSize;
			JVector2F invBluseNoiseTextureSize;
			int ltcMatTextureIndex = 0;
			int ltcAmpTextureIndex = 0;
			int passPad00 = 0;
			int paddPad01 = 0;
		};
		struct JScenePassConstants
		{ 
			float sceneTotalTime = 0.0f; 
			float sceneDeltaTime = 0.0f;
			uint directionalLitSt = 0;
			uint directionalLitEd = 0;
			uint pointLitSt = 0;
			uint pointLitEd = 0;
			uint spotLitSt = 0;
			uint spotLitEd = 0;
			uint rectLitSt = 0;
			uint rectLitEd = 0;
			uint scenePassPad00 = 0;
			uint scenePassPad01 = 0;
		};
	}
}