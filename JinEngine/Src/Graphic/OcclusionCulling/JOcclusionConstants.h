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
			int queryResultIndex = 0;
			int objectPad00 = 0;
			int objectPad01 = 0;
			int isValid = 0;
		};
		struct JDepthMapInfoConstants
		{
			int nowWidth = 0;
			int nowHeight = 0;
			int nowIndex = 0;
			int samplePad00 = 0;
		};
		struct JOcclusionPassConstants
		{ 
			DirectX::XMFLOAT4X4 viewProj = JMathHelper::Identity4x4(); 
			DirectX::XMFLOAT4X4 camWorld = JMathHelper::Identity4x4();
			int validQueryCount = 0;
			DirectX::XMFLOAT3 camPos = DirectX::XMFLOAT3(0, 0, 0);
		};

		struct HZBDebugInfo
		{
			DirectX::XMFLOAT4X4 objWorld = JMathHelper::Identity4x4();
			DirectX::XMFLOAT4X4 camWorld = JMathHelper::Identity4x4();
			DirectX::XMFLOAT4 posW = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT4 camW = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT4 posC = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT4 posH = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT3 posS = DirectX::XMFLOAT3(0, 0, 0);
			float radius = 0;
			DirectX::XMFLOAT4 additionalPosW = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT2 texSize = DirectX::XMFLOAT2(0, 0);
			float centerDepth = 0;
			float copareDepth = 0;
			int lod = 0;
			int hzbDebugPad00 = 0;
			int hzbDebugPad01 = 0;
			int hzbDebugPad02 = 0;
		};

	}
}