#pragma once
#include"../../Core/JDataType.h"
#include"../../Utility/JMathHelper.h"
#include<DirectXMath.h>

namespace JinEngine
{
	namespace Graphic
	{
		struct JOutlineConstants
		{
			DirectX::XMFLOAT4X4 world = JMathHelper::Identity4x4();

			//invalid red ornage yellow green blue purple black
			DirectX::XMFLOAT4 colors[8] =
			{
				DirectX::XMFLOAT4(0.0f ,0.0f, 0.0f, 0.0f),
				DirectX::XMFLOAT4(1.0f ,0.0f, 0.0f, 1.0f),
				DirectX::XMFLOAT4(1.0f ,0.5f, 0.0f, 1.0f),
				DirectX::XMFLOAT4(1.0f ,1.0f, 0.0f, 1.0f),
				DirectX::XMFLOAT4(0.0f ,1.0f, 0.0f, 1.0f),
				DirectX::XMFLOAT4(0.0f ,0.0f, 1.0f, 1.0f),
				DirectX::XMFLOAT4(0.5f ,0.0f, 0.75f, 1.0f),
				DirectX::XMFLOAT4(0.05f ,0.05f, 0.05f, 1.0f)
			};
			float threshold = 0.01f;
			float thickness = 4;
			uint offset = 0;
			uint width = 0;
			uint height = 0;
			uint outlinePad00 = 0;
			uint outlinePad01 = 0;
			uint outlinePad02 = 0;
		}; 
	}
}