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
			//red ornage yellow green blue purple black
			DirectX::XMFLOAT4 colors[7] =
			{
				DirectX::XMFLOAT4(1.0f ,0.0f, 0.0f, 0.8f),
				DirectX::XMFLOAT4(1.0f ,0.5f, 0.0f, 0.8f),
				DirectX::XMFLOAT4(1.0f ,1.0f, 0.0f, 0.8f),
				DirectX::XMFLOAT4(0.0f ,1.0f, 0.0f, 0.8f),
				DirectX::XMFLOAT4(0.0f ,0.0f, 1.0f, 0.8f),
				DirectX::XMFLOAT4(0.5f ,0.0f, 0.75f, 0.8f),
				DirectX::XMFLOAT4(0.05f ,0.05f, 0.05f, 0.8f)
			};
			uint offset = 0;
			uint width = 0;
			uint height = 0;
			uint outlinePad00 = 0;
		}; 
	}
}