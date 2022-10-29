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
			JDirectionalLight directionalLight[maxLight]; 
			JPointLight pointLight[maxLight]; 
			JSpotLight spotLight[maxLight]; 

			uint directionalLightMax = 0;
			uint pointLightMax = 0;
			uint spotLightMax = 0;
			uint litObjPad0 = 0; 
		};

		//On Shadow
		struct JSMLightConstants
		{
		public: 
			JSMDirectionalLight sDirectionalLight[maxLight];
			JSMPointLight sPointLight[maxLight];
			JSMSpotLight sSpotLight[maxLight];

			uint smDirectionalLightMax = 0;
			uint smPointLightMax = 0;
			uint smSpotLightMax = 0;
			uint smLitObjPad0 = 0;
		};
	}
}