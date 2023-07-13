#pragma once  
#include"../../../Utility/JMathHelper.h"
#include"../../../Core/JDataType.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JObjectConstants
		{
			DirectX::XMFLOAT4X4 World = JMathHelper::Identity4x4();
			DirectX::XMFLOAT4X4 TexTransform = JMathHelper::Identity4x4();
			uint     MaterialIndex = 0;
			uint     ObjPad0 = 0;
			uint     ObjPad1 = 0;
			uint     ObjPad2 = 0;
		};
	}
}