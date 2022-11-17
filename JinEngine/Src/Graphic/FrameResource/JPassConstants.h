#pragma once
#include"../../Utility/JMathHelper.h" 
#include"../../Core/JDataType.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JPassConstants
		{
			DirectX::XMFLOAT4 ambientLight = { 0.0f, 0.0f, 0.0f, 1.0f };
			float totalTime = 0.0f;
			float deltaTime = 0.0f;
			uint lightMax = 0; 
			uint smLightMax = 0; 	 
			uint passPad00 = 0;
			uint passPad01 = 0;
			uint passPad02 = 0;
			uint passPad03 = 0;
		};
	}
}