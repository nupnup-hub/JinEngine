struct ObjectInfo
{
	float4x4 objWorld;
	float3 center;
	int queryResultIndex;
	float3 extents;
	int isValid;
};

struct HZBDebugInfo
{
	float4x4 objWorld;

	float3 center;
	float3 extents;

	float4 posCW;
	float4 posCV;
	float4 posEW;
	float4 posEV;

	float3 camPos;

	float3 nearPoint0;
	float3 nearPoint1;
	float3 nearPoint2;
	float3 nearPoint3;
	float3 nearPoint4;
	float3 nearPoint5;

	float3 nearPointW;
	float4 nearPointH;
	float3 nearPointC;

	float4 bboxPointV0;
	float4 bboxPointV1;
	float4 bboxPointV2;
	float4 bboxPointV3;
	float4 bboxPointV4;
	float4 bboxPointV5;
	float4 bboxPointV6;
	float4 bboxPointV7;

	float4 bboxPointH0;
	float4 bboxPointH1;
	float4 bboxPointH2;
	float4 bboxPointH3;
	float4 bboxPointH4;
	float4 bboxPointH5;
	float4 bboxPointH6;
	float4 bboxPointH7;

	float2 clipFrame0;
	float2 clipFrame1;
	float2 clipFrame2;
	float2 clipFrame3;

	float width;
	float height;

	float3 uvExtentsMax;
	float3 uvExtentsMin;

	float centerDepth;
	float finalDepth;

	int threadIndex;
	int queryIndex;
};

RWStructuredBuffer<HZBDebugInfo> hzbDebugInfo : register(u2, space1);

Texture2D depthMap: register(t0);
Texture2D depthMipmap: register(t1);
RWTexture2D<float> lastMipmap: register(u0);

StructuredBuffer<ObjectInfo> object : register(t2);
RWStructuredBuffer<float> queryResult : register(u1, space1);

SamplerState downSam : register(s0);
SamplerState occFrameSam : register(s1);

cbuffer cbDepthMapInfo : register(b0)
{
	int nowWidth;
	int nowHeight;
	int nowIndex;
	int samplePad00;
};

cbuffer cbPass : register(b1)
{
	float4x4 camWorld;
	float4x4 camView;
	float4x4 camProj;
	float4x4 camViewProj;
	float viewWidth;
	float viewHeight;
	float camNear;
	int validQueryCount;
};

#if defined (DIMX) && defined (DIMY)
[numthreads(DIMX, DIMY, 1)]
void HZBCopyDepthMap(int3 dispatchThreadID : SV_DispatchThreadID)
{
	if (nowWidth <= dispatchThreadID.x || nowHeight <= dispatchThreadID.y)
		return;

	lastMipmap[int2(dispatchThreadID.x, dispatchThreadID.y)].r = depthMap.Load(int3(dispatchThreadID.x, dispatchThreadID.y, 0)).r;
} 
#endif

#define DOWN_SAMPLEING_BY_LOAD 1
//#define DOWN_SAMPLEING_BY_SAMPLE_LEVEL

#if defined (DIMX) && defined (DIMY)
[numthreads(DIMX, DIMY, 1)]
void HZBDownSampling(int3 dispatchThreadID : SV_DispatchThreadID)
{
	if ((nowWidth / 2) <= dispatchThreadID.x || (nowHeight / 2) <= dispatchThreadID.y)
		return;

#ifdef DOWN_SAMPLEING_BY_SAMPLE_LEVEL

	const float2 baseIndex = float2(dispatchThreadID.x, dispatchThreadID.y);
	const float2 uv = float2((baseIndex.x * 2 + 1) / nowWidth, (baseIndex.y * 2 + 1) / nowHeight);
	const float xOffset = 1 / nowWidth;
	const float yOffset = 1 / nowHeight;

	const float color00 = depthMap.SampleLevel(downSam, uv, 0).r;
	const float color01 = depthMap.SampleLevel(downSam, uv + float2(xOffset, 0), 0).r;
	const float color02 = depthMap.SampleLevel(downSam, uv + float2(xOffset, yOffset), 0).r;
	const float color03 = depthMap.SampleLevel(downSam, uv + float2(0, yOffset), 0).r;

	float finalColor = max(color00, max(color01, max(color02, color03)));

	bool shouldIncludeExtraColumnFromPreviousLevel = ((nowWidth & 1) != 0);
	bool shouldIncludeExtraRowFromPreviousLevel = ((nowHeight & 1) != 0);

	if (shouldIncludeExtraColumnFromPreviousLevel)
	{
		const float extraColor00 = depthMap.SampleLevel(downSam, uv + float2(xOffset * 2, 0), 0).r;
		const float extraColor01 = depthMap.SampleLevel(downSam, uv + float2(xOffset * 2, yOffset), 0).r;

		// In the case where the width and height are both odd, need to include the
		// 'corner' value as well.
		if (shouldIncludeExtraRowFromPreviousLevel)
		{
			const float extraColor02 = depthMap.SampleLevel(downSam, uv + float2(xOffset * 2, yOffset * 2), 0).r;
			finalColor = max(finalColor, extraColor02);
		}
		finalColor = max(finalColor, max(extraColor00, extraColor01));
	}
	if (shouldIncludeExtraRowFromPreviousLevel)
	{
		const float extraColor00 = depthMap.SampleLevel(downSam, uv + float2(0, yOffset * 2), 0).r;
		const float extraColor01 = depthMap.SampleLevel(downSam, uv + float2(xOffset, yOffset * 2), 0).r;

		finalColor = max(finalColor, max(extraColor00, extraColor01));
	}
	lastMipmap[int2(baseIndex.x, baseIndex.y)].r = finalColor;

#elif DOWN_SAMPLEING_BY_LOAD

	const int3 baseIndex = int3(dispatchThreadID.x * 2, dispatchThreadID.y * 2, 0);
	const float color00 = depthMap.Load(baseIndex).r;
	const float color01 = depthMap.Load(baseIndex, int2(1, 0)).r;
	const float color02 = depthMap.Load(baseIndex, int2(1, 1)).r;
	const float color03 = depthMap.Load(baseIndex, int2(0, 1)).r;

	float finalColor = max(color00, max(color01, max(color02, color03)));

	bool shouldIncludeExtraColumnFromPreviousLevel = ((nowWidth & 1) != 0);
	bool shouldIncludeExtraRowFromPreviousLevel = ((nowHeight & 1) != 0);

	if (shouldIncludeExtraColumnFromPreviousLevel)
	{
		const float extraColor00 = depthMap.Load(baseIndex, int2(2, 0)).r;
		const float extraColor01 = depthMap.Load(baseIndex, int2(2, 1)).r;

		// In the case where the width and height are both odd, need to include the
		// 'corner' value as well.
		if (shouldIncludeExtraRowFromPreviousLevel)
		{
			const float extraColor02 = depthMap.Load(baseIndex, int2(2, 2)).r;
			finalColor = max(finalColor, extraColor02);
		}
		finalColor = max(finalColor, max(extraColor00, extraColor01));
	}
	if (shouldIncludeExtraRowFromPreviousLevel)
	{
		const float extraColor00 = depthMap.Load(baseIndex, int2(0, 2)).r;
		const float extraColor01 = depthMap.Load(baseIndex, int2(1, 2)).r;

		finalColor = max(finalColor, max(extraColor00, extraColor01));
	}
	lastMipmap[int2(dispatchThreadID.x, dispatchThreadID.y)].r = finalColor;
#endif 

}
#endif

float3 CalNearPoint(const float3 camPos, float3 p0, float3 p1, float3 p2)
{
	const float3 pNormal = normalize(cross(p1 - p0, p2 - p0));
	const float3 dist = dot(-pNormal, p0);
	const float dotCoord = dot(camPos, pNormal) + dist;
	float3 nearPoint = camPos - dotCoord * pNormal;

	const float2 xFactor = float2(min(min(p0.x, p1.x), p2.x), max(max(p0.x, p1.x), p2.x));
	const float2 yFactor = float2(min(min(p0.y, p1.y), p2.y), max(max(p0.y, p1.y), p2.y));
	const float2 zFactor = float2(min(min(p0.z, p1.z), p2.z), max(max(p0.z, p1.z), p2.z));

	nearPoint.x = clamp(nearPoint.x, xFactor.x, xFactor.y);
	nearPoint.y = clamp(nearPoint.y, yFactor.x, yFactor.y);
	nearPoint.z = clamp(nearPoint.z, zFactor.x, zFactor.y);

	return nearPoint;
}

#if defined (DIMX) && defined (DIMY) && defined (DIMZ)
[numthreads(DIMX, DIMY, DIMZ)]
void HZBOcclusion(int3 dispatchThreadID : SV_DispatchThreadID)
{
	//Thread dim X is objectList x
   //gorup dim X is objectList x

	//dispatchThreadID.x >= validQueryCount || 
	static const float maxDistance = 100000;
	static const float minDistance = -100000;

	const int threadIndex = dispatchThreadID.x;

	if (validQueryCount >= threadIndex && !object[threadIndex].isValid)
		return;

	const float4 posCW = mul(float4(object[threadIndex].center, 1.0f), object[threadIndex].objWorld);
	const float4 posCV = mul(posCW, camView);

	const float4x4 extentWM = float4x4(object[threadIndex].objWorld._m00_m01_m02_m03,
		object[threadIndex].objWorld._m10_m11_m12_m13,
		object[threadIndex].objWorld._m20_m21_m22_m23,
		0.0f, 0.0f, 0.0f, 1.0f);
	const float4x4 extentCM = float4x4(camView._m00_m01_m02_m03,
		camView._m10_m11_m12_m13,
		camView._m20_m21_m22_m23,
		0.0f, 0.0f, 0.0f, 1.0f);

	const float4 posEW = mul(float4(object[threadIndex].extents, 1.0f), extentWM);
	const float4 posEV = mul(posEW, extentCM);
	const float3 camC = float3(0.0f, 0.0f, 0.0f);
	//const float3 camC = float3(0.0f, 0.0f, camNear);

	const float4 bboxPointV[8] =
	{
		float4(posCV.xyz + float3(posEV.x, posEV.y, posEV.z), 1.0f),
		float4(posCV.xyz + float3(posEV.x, posEV.y, -posEV.z), 1.0f),
		float4(posCV.xyz + float3(posEV.x, -posEV.y, posEV.z), 1.0f),
		float4(posCV.xyz + float3(posEV.x, -posEV.y, -posEV.z), 1.0f),
		float4(posCV.xyz + float3(-posEV.x, posEV.y, posEV.z), 1.0f),
		float4(posCV.xyz + float3(-posEV.x, posEV.y, -posEV.z), 1.0f),
		float4(posCV.xyz + float3(-posEV.x, -posEV.y, posEV.z), 1.0f),
		float4(posCV.xyz + float3(-posEV.x, -posEV.y, -posEV.z), 1.0f)
	};

	const float3 nearPoint[6] =
	{
		CalNearPoint(camC, bboxPointV[0].xyz, bboxPointV[1].xyz, bboxPointV[2].xyz),
		CalNearPoint(camC, bboxPointV[0].xyz, bboxPointV[1].xyz, bboxPointV[4].xyz),
		CalNearPoint(camC, bboxPointV[0].xyz, bboxPointV[2].xyz, bboxPointV[4].xyz),
		CalNearPoint(camC, bboxPointV[7].xyz, bboxPointV[5].xyz, bboxPointV[6].xyz),
		CalNearPoint(camC, bboxPointV[7].xyz, bboxPointV[3].xyz, bboxPointV[5].xyz),
		CalNearPoint(camC, bboxPointV[7].xyz, bboxPointV[3].xyz, bboxPointV[6].xyz)
	};

	float nowMinDist = maxDistance;
	uint minIndex = 0;
	for (uint i = 0; i < 6; ++i)
	{
		float dist = length(nearPoint[i] - camC);
		if (dist < nowMinDist)
		{
			nowMinDist = dist;
			minIndex = i;
		}
	}

	const float4 clipNearH = mul(float4(nearPoint[minIndex], 1.0f), camProj);
	const float3 clipNearC = clipNearH.xyz / clipNearH.w;

	const float4 bboxPointH[8] =
	{
		mul(bboxPointV[0], camProj),
		mul(bboxPointV[1], camProj),
		mul(bboxPointV[2], camProj),
		mul(bboxPointV[3], camProj),
		mul(bboxPointV[4], camProj),
		mul(bboxPointV[5], camProj),
		mul(bboxPointV[6], camProj),
		mul(bboxPointV[7], camProj),
	};

	static const float4x4 ndcTransform = float4x4(0.5f, 0, 0, 0,
		0, -0.5f, 0, 0,
		0, 0, 1.0f, 0,
		0.5f, 0.5f, 0, 1.0f);

	const float3 bboxPointC[8] =
	{
		mul(bboxPointH[0] / bboxPointH[0].w, ndcTransform).xyz,
		mul(bboxPointH[1] / bboxPointH[1].w, ndcTransform).xyz,
		mul(bboxPointH[2] / bboxPointH[2].w, ndcTransform).xyz,
		mul(bboxPointH[3] / bboxPointH[3].w, ndcTransform).xyz,
		mul(bboxPointH[4] / bboxPointH[4].w, ndcTransform).xyz,
		mul(bboxPointH[5] / bboxPointH[5].w, ndcTransform).xyz,
		mul(bboxPointH[6] / bboxPointH[6].w, ndcTransform).xyz,
		mul(bboxPointH[7] / bboxPointH[7].w, ndcTransform).xyz,
	};

	float minX = maxDistance;
	float maxX = minDistance;
	float minY = maxDistance;
	float maxY = minDistance;

	for (uint i = 0; i < 8; ++i)
	{
		if (bboxPointC[i].x > maxX)
			maxX = bboxPointC[i].x;
		if (bboxPointC[i].x < minX)
			minX = bboxPointC[i].x;
		if (bboxPointC[i].y > maxY)
			maxY = bboxPointC[i].y;
		if (bboxPointC[i].y < minY)
			minY = bboxPointC[i].y;
	}

	//maxX = maxX * 0.5f + 0.5f;
	//minX = minX * 0.5f + 0.5f;
	//maxY = maxY * -0.5f + 0.5f;
	//minY = minY * -0.5f + 0.5f;

	const float2 clipFrame[4] =
	{
		float2(maxX, maxY),
		float2(maxX, minY),
		float2(minX, maxY),
		float2(minX, minY)
	};

	const float width = viewWidth * (maxX - minX);
	const float height = viewHeight * (maxY - minY);

	int lod = 10 - (ceil(log2(max(width, height)) + 1));
	if (lod < 0)
		lod = 0;

	const float centerDepth = clipNearC.z;
	const float compareDepth00 = depthMipmap.SampleLevel(occFrameSam, clipFrame[0], lod).r;
	const float compareDepth01 = depthMipmap.SampleLevel(occFrameSam, clipFrame[1], lod).r;
	const float compareDepth02 = depthMipmap.SampleLevel(occFrameSam, clipFrame[2], lod).r;
	const float compareDepth03 = depthMipmap.SampleLevel(occFrameSam, clipFrame[3], lod).r;

	const float finalCompareDepth = max(compareDepth00, max(compareDepth01, max(compareDepth02, compareDepth03)));
	const int queryIndex = object[threadIndex].queryResultIndex;

	if (centerDepth <= finalCompareDepth)
		queryResult[queryIndex] = 0;
	else
		queryResult[queryIndex] = 1;

 /*
 	hzbDebugInfo[threadIndex].objWorld = object[threadIndex].objWorld;

	hzbDebugInfo[threadIndex].center = object[threadIndex].center;
	hzbDebugInfo[threadIndex].extents = object[threadIndex].extents;

	hzbDebugInfo[threadIndex].posCW = posCW;
	hzbDebugInfo[threadIndex].posCV = posCV;
	hzbDebugInfo[threadIndex].posEW = posEW;
	hzbDebugInfo[threadIndex].posEV = posEV;

	hzbDebugInfo[threadIndex].camPos = camC;

	hzbDebugInfo[threadIndex].nearPoint0 = nearPoint[0];
	hzbDebugInfo[threadIndex].nearPoint1 = nearPoint[1];
	hzbDebugInfo[threadIndex].nearPoint2 = nearPoint[2];
	hzbDebugInfo[threadIndex].nearPoint3 = nearPoint[3];
	hzbDebugInfo[threadIndex].nearPoint4 = nearPoint[4];
	hzbDebugInfo[threadIndex].nearPoint5 = nearPoint[5];

	hzbDebugInfo[threadIndex].nearPointW = nearPoint[minIndex];
	hzbDebugInfo[threadIndex].nearPointH = clipNearH;
	hzbDebugInfo[threadIndex].nearPointC = clipNearC;

	hzbDebugInfo[threadIndex].bboxPointV0 = bboxPointV[0];
	hzbDebugInfo[threadIndex].bboxPointV1 = bboxPointV[1];
	hzbDebugInfo[threadIndex].bboxPointV2 = bboxPointV[2];
	hzbDebugInfo[threadIndex].bboxPointV3 = bboxPointV[3];
	hzbDebugInfo[threadIndex].bboxPointV4 = bboxPointV[4];
	hzbDebugInfo[threadIndex].bboxPointV5 = bboxPointV[5];
	hzbDebugInfo[threadIndex].bboxPointV6 = bboxPointV[6];
	hzbDebugInfo[threadIndex].bboxPointV7 = bboxPointV[7];

	hzbDebugInfo[threadIndex].bboxPointH0 = bboxPointH[0];
	hzbDebugInfo[threadIndex].bboxPointH1 = bboxPointH[1];
	hzbDebugInfo[threadIndex].bboxPointH2 = bboxPointH[2];
	hzbDebugInfo[threadIndex].bboxPointH3 = bboxPointH[3];
	hzbDebugInfo[threadIndex].bboxPointH4 = bboxPointH[4];
	hzbDebugInfo[threadIndex].bboxPointH5 = bboxPointH[5];
	hzbDebugInfo[threadIndex].bboxPointH6 = bboxPointH[6];
	hzbDebugInfo[threadIndex].bboxPointH7 = bboxPointH[7];

	hzbDebugInfo[threadIndex].clipFrame0 = clipFrame[0];
	hzbDebugInfo[threadIndex].clipFrame1 = clipFrame[1];
	hzbDebugInfo[threadIndex].clipFrame2 = clipFrame[2];
	hzbDebugInfo[threadIndex].clipFrame3 = clipFrame[3];

	hzbDebugInfo[threadIndex].width = width;
	hzbDebugInfo[threadIndex].height = height;

	hzbDebugInfo[threadIndex].uvExtentsMax = float3(maxX, maxY, 0);
	hzbDebugInfo[threadIndex].uvExtentsMin = float3(minX, minY, 0);

	hzbDebugInfo[threadIndex].centerDepth = centerDepth;
	hzbDebugInfo[threadIndex].finalDepth = finalCompareDepth;

	hzbDebugInfo[threadIndex].threadIndex = threadIndex;
	hzbDebugInfo[threadIndex].queryIndex = queryIndex;
 */
}
#endif