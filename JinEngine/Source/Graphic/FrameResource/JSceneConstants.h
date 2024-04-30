#pragma once
#include"../../Core/Math/JVector.h"
#include"../../Core/JCoreEssential.h"

namespace JinEngine
{
	namespace Graphic
	{ 
		struct JScenePassConstants
		{ 
			float appTotalTime = 0;
			float appDeltaTime = 0;
			float sceneTotalTime = 0;
			float sceneDeltaTime = 0;
			uint directionalLitSt = 0;
			uint directionalLitEd = 0;
			uint pointLitSt = 0;
			uint pointLitEd = 0;
			uint spotLitSt = 0;
			uint spotLitEd = 0;
			uint rectLitSt = 0;
			uint rectLitEd = 0;
			int missingTextureIndex = 0;		//always exist until engine end
			int bluseNoiseTextureIndex = 0;		//always exist until engine end
			JVector2F bluseNoiseTextureSize = JVector2F::Zero();
			JVector2F invBluseNoiseTextureSize = JVector2F::Zero();
			float clusterMinDepth = 0;
			int paddPad00 = 0; 
		};
	}
}