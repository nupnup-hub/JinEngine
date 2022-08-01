#pragma once 
#include"../../Object/Component/Light/JLightStruct.h" 
#include"../../Core/JDataType.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JLightConstants
		{
			JDirectionalLight directionalLight[maxLight];
			S_DirectionalLight s_directionalLight[maxLight];

			JPointLight pointLight[maxLight];
			S_PointLight s_pointLight[maxLight];

			JSpotLight spotLight[maxLight];
			S_SpotLight s_spotLight[maxLight];

			int directionalLightMax = 0;
			int s_directionalLightMax = 0;

			int pointLightMax = 0;
			int s_pointLightMax = 0;

			int spotLightMax = 0;
			int s_spotLightMax = 0;

			uint objPad0 = 0;
			uint objPad1 = 0;
		};
	}
}