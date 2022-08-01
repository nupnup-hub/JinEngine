#pragma once
#include"../../../Utility/JMathHelper.h"
#include"../../../Core/JDataType.h"

namespace JinEngine
{
	struct JShadow
	{
		DirectX::XMFLOAT4X4 shadowTransform = JMathHelper::Identity4x4();
		int shadowMapIndex = -1;
		uint shadowPad00 = 0;
		uint shadowPad01 = 0;
		uint shadowPad02 = 0;
	}; 
}