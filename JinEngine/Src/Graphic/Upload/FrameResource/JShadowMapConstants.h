#pragma once
#include"../../../Utility/JMathHelper.h" 
#include"../../../Core/JDataType.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JShadowMapConstants
		{
		public:
			DirectX::XMFLOAT4X4 viewProj = JMathHelper::Identity4x4();
		};
	}
}