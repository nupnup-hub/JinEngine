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
			int queryResultIndex = 0;
			DirectX::XMFLOAT3 extents = DirectX::XMFLOAT3(0, 0, 0);
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
			DirectX::XMFLOAT4X4 camWorld = JMathHelper::Identity4x4();
			DirectX::XMFLOAT4X4 view = JMathHelper::Identity4x4();
			DirectX::XMFLOAT4X4 proj = JMathHelper::Identity4x4();
			DirectX::XMFLOAT4X4 viewProj= JMathHelper::Identity4x4(); 
			float viewWidth = 0;
			float viewHeight= 0;
			int validQueryCount = 0; 
			int passPad00 = 0;
		};

		struct HZBDebugInfo
		{
			DirectX::XMFLOAT4X4 objWorld = JMathHelper::Identity4x4(); 

			DirectX::XMFLOAT3 center = DirectX::XMFLOAT3(0, 0, 0);
			DirectX::XMFLOAT3 extents = DirectX::XMFLOAT3(0, 0, 0);

			DirectX::XMFLOAT4 posCW = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT4 posCV = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT4 posCH = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT3 posCC = DirectX::XMFLOAT3(0, 0, 0);

			DirectX::XMFLOAT4 posEW = DirectX::XMFLOAT4(0, 0, 0, 0); 

			DirectX::XMFLOAT3 camPos = DirectX::XMFLOAT3(0, 0, 0);

			DirectX::XMFLOAT3 bboxPoint0 = DirectX::XMFLOAT3(0, 0, 0);
			DirectX::XMFLOAT3 bboxPoint1 = DirectX::XMFLOAT3(0, 0, 0);
			DirectX::XMFLOAT3 bboxPoint2 = DirectX::XMFLOAT3(0, 0, 0);
			DirectX::XMFLOAT3 bboxPoint3 = DirectX::XMFLOAT3(0, 0, 0);
			DirectX::XMFLOAT3 bboxPoint4 = DirectX::XMFLOAT3(0, 0, 0);
			DirectX::XMFLOAT3 bboxPoint5 = DirectX::XMFLOAT3(0, 0, 0);
			DirectX::XMFLOAT3 bboxPoint6 = DirectX::XMFLOAT3(0, 0, 0);
			DirectX::XMFLOAT3 bboxPoint7 = DirectX::XMFLOAT3(0, 0, 0);

			DirectX::XMFLOAT3 nearPoint0 = DirectX::XMFLOAT3(0, 0, 0);
			DirectX::XMFLOAT3 nearPoint1 = DirectX::XMFLOAT3(0, 0, 0);
			DirectX::XMFLOAT3 nearPoint2 = DirectX::XMFLOAT3(0, 0, 0);
			DirectX::XMFLOAT3 nearPoint3 = DirectX::XMFLOAT3(0, 0, 0);
			DirectX::XMFLOAT3 nearPoint4 = DirectX::XMFLOAT3(0, 0, 0);
			DirectX::XMFLOAT3 nearPoint5 = DirectX::XMFLOAT3(0, 0, 0);

			DirectX::XMFLOAT3 nearPointF = DirectX::XMFLOAT3(0, 0, 0);
			float width = 0;
			float height = 0;

			DirectX::XMFLOAT2 uv = DirectX::XMFLOAT2(0, 0);
			DirectX::XMFLOAT2 uvExtentsMax = DirectX::XMFLOAT2(0, 0);
			DirectX::XMFLOAT2 uvExtentsMin = DirectX::XMFLOAT2(0, 0);

			float centerDepth = 0;
			float copareDepth = 0;
		};

	}
}