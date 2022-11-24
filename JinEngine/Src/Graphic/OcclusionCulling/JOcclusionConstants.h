#pragma once
#include"../../Core/JDataType.h"
#include"../../Utility/JMathHelper.h"
#include<DirectXMath.h>

namespace JinEngine
{
	namespace Graphic
	{
		struct JOcclusionObjectConstants
		{
			DirectX::XMFLOAT4X4 objWorld = JMathHelper::Identity4x4();
			DirectX::XMFLOAT3 center = DirectX::XMFLOAT3(0, 0, 0);
			float radius = 0;
			uint queryResultIndex = 0;
			uint objectPad00 = 0;
			uint objectPad01 = 0;
			uint isValid = 0;
		};
		struct JDepthMapInfoConstants
		{
			uint maxWidth = 0;
			uint maxHeight = 0;
			uint validMapCount = 0;
			uint samplePad00 = 0;
		};
		struct JOcclusionPassConstants
		{ 
			DirectX::XMFLOAT4X4 viewProj = JMathHelper::Identity4x4(); 
			DirectX::XMFLOAT4X4 camWorld = JMathHelper::Identity4x4();
			uint validQueryCount = 0;
			DirectX::XMFLOAT3 camPos = DirectX::XMFLOAT3(0, 0, 0);
		};
	}
}