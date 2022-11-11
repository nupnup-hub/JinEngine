#pragma once
#include<DirectXMath.h> 
#include"../../Utility/JMathHelper.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JBoundingObjectConstants
		{
			DirectX::XMFLOAT4X4 boundWorld = JMathHelper::Identity4x4();
		};
	}
}