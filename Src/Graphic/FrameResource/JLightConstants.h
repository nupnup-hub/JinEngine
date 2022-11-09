#pragma once 
#include"../../Object/Component/Light/JLightStruct.h" 
#include"../../Core/JDataType.h"

namespace JinEngine
{
	namespace Graphic
	{
		//Off Shadow
		struct JLightConstants
		{
		public:
			JDirectionalLight directionalLight[JinEngine::Constant::maxLight]; 
			JPointLight pointLight[JinEngine::Constant::maxLight];
			JSpotLight spotLight[JinEngine::Constant::maxLight];

			uint directionalLightMax = 0;
			uint pointLightMax = 0;
			uint spotLightMax = 0;
			uint litObjPad0 = 0; 
		};

		//On Shadow
		struct JSMLightConstants
		{
		public: 
			JSMDirectionalLight sDirectionalLight[JinEngine::Constant::maxLight];
			JSMPointLight sPointLight[JinEngine::Constant::maxLight];
			JSMSpotLight sSpotLight[JinEngine::Constant::maxLight];

			uint smDirectionalLightMax = 0;
			uint smPointLightMax = 0;
			uint smSpotLightMax = 0;
			uint smLitObjPad0 = 0;
		};
	}
}