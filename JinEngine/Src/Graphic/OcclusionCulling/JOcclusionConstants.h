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
			DirectX::XMFLOAT4X4 view = JMathHelper::Identity4x4();
			DirectX::XMFLOAT4X4 proj = JMathHelper::Identity4x4(); 
			DirectX::XMFLOAT4 frustumPlane[6];
			float viewWidth = 0;
			float viewHeight = 0;
			float camNear = 0;
			int validQueryCount = 0;
			int occMapCount = 0;
			int maxOccSize = 0;
			int minOccSize = 0;
			int passPad00 = 0;
		};

		struct HZBDebugInfo
		{
			DirectX::XMFLOAT4X4 objWorld = JMathHelper::Identity4x4();

			DirectX::XMFLOAT3 center = DirectX::XMFLOAT3(0, 0, 0);
			DirectX::XMFLOAT3 extents = DirectX::XMFLOAT3(0, 0, 0);

			DirectX::XMFLOAT4 posCW = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT4 posCV = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT4 posEW = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT4 posEV = DirectX::XMFLOAT4(0, 0, 0, 0);
			 
			DirectX::XMFLOAT3 nearPoint0 = DirectX::XMFLOAT3(0, 0, 0);
			DirectX::XMFLOAT3 nearPoint1 = DirectX::XMFLOAT3(0, 0, 0);
			DirectX::XMFLOAT3 nearPoint2 = DirectX::XMFLOAT3(0, 0, 0);
			DirectX::XMFLOAT3 nearPoint3 = DirectX::XMFLOAT3(0, 0, 0);
			DirectX::XMFLOAT3 nearPoint4 = DirectX::XMFLOAT3(0, 0, 0);
			DirectX::XMFLOAT3 nearPoint5 = DirectX::XMFLOAT3(0, 0, 0);

			DirectX::XMFLOAT3 nearPointW = DirectX::XMFLOAT3(0, 0, 0);
			DirectX::XMFLOAT4 nearPointH = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT3 nearPointC = DirectX::XMFLOAT3(0, 0, 0);

			DirectX::XMFLOAT4 bboxPointV0 = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT4 bboxPointV1 = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT4 bboxPointV2 = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT4 bboxPointV3 = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT4 bboxPointV4 = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT4 bboxPointV5 = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT4 bboxPointV6 = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT4 bboxPointV7 = DirectX::XMFLOAT4(0, 0, 0, 0);


			DirectX::XMFLOAT4 bboxPointH0 = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT4 bboxPointH1 = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT4 bboxPointH2 = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT4 bboxPointH3 = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT4 bboxPointH4 = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT4 bboxPointH5 = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT4 bboxPointH6 = DirectX::XMFLOAT4(0, 0, 0, 0);
			DirectX::XMFLOAT4 bboxPointH7 = DirectX::XMFLOAT4(0, 0, 0, 0);

			DirectX::XMFLOAT2 clipFrame0 = DirectX::XMFLOAT2(0, 0);
			DirectX::XMFLOAT2 clipFrame1 = DirectX::XMFLOAT2(0, 0);
			DirectX::XMFLOAT2 clipFrame2 = DirectX::XMFLOAT2(0, 0);
			DirectX::XMFLOAT2 clipFrame3 = DirectX::XMFLOAT2(0, 0);

			float width = 0;
			float height = 0;
			int lod = 0;

			DirectX::XMFLOAT3 uvExtentsMax = DirectX::XMFLOAT3(0, 0, 0);
			DirectX::XMFLOAT3 uvExtentsMin = DirectX::XMFLOAT3(0, 0, 0);

			float centerDepth = 0;
			float copareDepth = 0;

			int threadIndex = -1;
			int queryIndex = -1;
		};

	}
}