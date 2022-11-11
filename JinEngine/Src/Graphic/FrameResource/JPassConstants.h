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
			uint objPad0 = 0;
			uint objPad1 = 0;
		};
	}
}