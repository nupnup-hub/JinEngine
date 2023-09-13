#pragma once
#include"../../Core/Math/JMathHelper.h"
#include"../../Core/JCoreEssential.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JEnginePassConstants
		{ 
			float appTotalTime = 0.0f;
			float appDeltaTime = 0.0f;	 
			uint ePassPad00 = 0;
			uint ePassPad01 = 0;
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
		};
	}
}