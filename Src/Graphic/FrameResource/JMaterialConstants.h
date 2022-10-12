#pragma once 
#include"../../Utility/JMathHelper.h"
#include"../../Core/JDataType.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JMaterialConstants
		{
			DirectX::XMFLOAT4  AlbedoColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			// Used in texture mapping.
			DirectX::XMFLOAT4X4 MatTransform = JMathHelper::Identity4x4();
			float Metalic = 0.75f;
			float Roughness = 0.3f;
			/*
			* srv heap index
			1 = missing
			*/
			uint AlbedoMapIndex = 1;
			uint NormalMapIndex = 1;
			uint HeightMapIndex = 1;
			uint RoughnessMapIndex = 1;
			uint AmbientOcclusionMapIndex = 1;
			uint objPad = 0;
		};
	}
}