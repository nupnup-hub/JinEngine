
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
	float4 posEW;

	float3 camPos;

	float3 bboxPoint0;
	float3 bboxPoint1;
	float3 bboxPoint2;
	float3 bboxPoint3;
	float3 bboxPoint4;
	float3 bboxPoint5;
	float3 bboxPoint6;
	float3 bboxPoint7;

	float3 nearPoint0;
	float3 nearPoint1;
	float3 nearPoint2;
	float3 nearPoint3;
	float3 nearPoint4;
	float3 nearPoint5;

	float3 clipFrame0;
	float3 clipFrame1;
	float3 clipFrame2;
	float3 clipFrame3;
	float3 clipNearW;
	float3 clipNearC;
	float3 clipNearS;

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
RWTexture2D<float> lastMipmap: register(u0);

#ifdef DOWN_SAMPLING_COUNT
Texture2D mipmap[DOWN_SAMPLING_COUNT] : register(t1, space1);
#endif

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
	float4x4 camViewProj;
	float viewWidth;
	float viewHeight;
	float camNear;
	int validQueryCount;
};

#if defined (DIMX) && defined (DIMY)
[numthreads(DIMX, DIMY, 1)]
void HZBDownSampling(int3 dispatchThreadID : SV_DispatchThreadID)
{
	//Thread dim X is texture width
   //gorup dim Y is texture height

	if ((nowWidth / 2) <= dispatchThreadID.x || (nowHeight / 2) <= dispatchThreadID.y)
		return;
 
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
			finalColor = min(finalColor, extraColor02);
		}
		finalColor = max(finalColor, min(extraColor00, extraColor01));
	}
	if (shouldIncludeExtraRowFromPreviousLevel)
	{
		const float extraColor00 = depthMap.SampleLevel(downSam, uv + float2(0, yOffset * 2), 0).r;
		const float extraColor01 = depthMap.SampleLevel(downSam, uv + float2(xOffset, yOffset * 2), 0).r;

		finalColor = max(finalColor, min(extraColor00, extraColor01));
	}

	lastMipmap[int2(dispatchThreadID.x, dispatchThreadID.y)].r = finalColor;

	//const float color = depthMap.SampleLevel(downSam, uv, 0).r;
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
	const float4 posEW = mul(float4(object[threadIndex].extents, 1.0f), object[threadIndex].objWorld) - posCW + float4(0, 0, 0, 1);
	const float4 camW = mul(float4(0.0f, 0.0f, camNear, 1.0f), camWorld);

	const float4 bboxPointW[8] =
	{
		float4(posCW.xyz + float3(posEW.x, posEW.y, posEW.z), 1.0f),
		float4(posCW.xyz + float3(posEW.x, posEW.y, -posEW.z), 1.0f),
		float4(posCW.xyz + float3(posEW.x, -posEW.y, posEW.z), 1.0f),
		float4(posCW.xyz + float3(posEW.x, -posEW.y, -posEW.z), 1.0f),
		float4(posCW.xyz + float3(-posEW.x, posEW.y, posEW.z), 1.0f),
		float4(posCW.xyz + float3(-posEW.x, posEW.y, -posEW.z), 1.0f),
		float4(posCW.xyz + float3(-posEW.x, -posEW.y, posEW.z), 1.0f),
		float4(posCW.xyz + float3(-posEW.x, -posEW.y, -posEW.z), 1.0f)
	};

	const float3 nearPoint[6] =
	{
		CalNearPoint(camW, bboxPointW[0].xyz, bboxPointW[1].xyz, bboxPointW[2].xyz),
		CalNearPoint(camW, bboxPointW[0].xyz, bboxPointW[1].xyz, bboxPointW[4].xyz),
		CalNearPoint(camW, bboxPointW[0].xyz, bboxPointW[2].xyz, bboxPointW[4].xyz),
		CalNearPoint(camW, bboxPointW[7].xyz, bboxPointW[5].xyz, bboxPointW[6].xyz),
		CalNearPoint(camW, bboxPointW[7].xyz, bboxPointW[3].xyz, bboxPointW[5].xyz),
		CalNearPoint(camW, bboxPointW[7].xyz, bboxPointW[3].xyz, bboxPointW[6].xyz)
	};

	float nowMinDist = maxDistance;
	uint minIndex = 0;
	for (uint i = 0; i < 6; ++i)
	{
		float dist = length(nearPoint[i] - camW);
		if (dist < nowMinDist)
		{
			nowMinDist = dist;
			minIndex = i;
		}
	}

	const float4 clipNearH = mul(float4(nearPoint[minIndex], 1.0f), camViewProj);
	const float3 clipNearC = clipNearH.xyz / clipNearH.w;

	const float4 bboxPointH[8] =
	{
		mul(bboxPointW[0],camViewProj),
		mul(bboxPointW[1],camViewProj),
		mul(bboxPointW[2],camViewProj),
		mul(bboxPointW[3],camViewProj),
		mul(bboxPointW[4],camViewProj),
		mul(bboxPointW[5],camViewProj),
		mul(bboxPointW[6],camViewProj),
		mul(bboxPointW[7],camViewProj),
	};

	static const float4x4 ndcTransform = float4x4
		(0.5f, 0, 0, 0,
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

	const float3 clipFrame[4] =
	{
		float3(maxX, maxY, 0),
		float3(maxX, minY, 0),
		float3(minX, maxY, 0),
		float3(minX, minY, 0)
	};

	const float width = viewWidth * (maxX - minX);
	const float height = viewHeight * (maxY - minY);

	int lod = DOWN_SAMPLING_COUNT - (ceil(log2(max(width, height)) + 1));
	if (lod < 0)
		lod = 0; 

	const float centerDepth = clipNearC.z;
	const float compareDepth00 = mipmap[lod].SampleLevel(occFrameSam, clipFrame[0].xy, 0).r;
	const float compareDepth01 = mipmap[lod].SampleLevel(occFrameSam, clipFrame[1].xy, 0).r;
	const float compareDepth02 = mipmap[lod].SampleLevel(occFrameSam, clipFrame[2].xy, 0).r;
	const float compareDepth03 = mipmap[lod].SampleLevel(occFrameSam, clipFrame[3].xy, 0).r;

	const float finalCompareDepth = max(compareDepth00, max(compareDepth01, max(compareDepth02, compareDepth03)));
	const int queryIndex = object[threadIndex].queryResultIndex;

	if (centerDepth <= finalCompareDepth)
		queryResult[queryIndex] = 0;
	else
		queryResult[queryIndex] = 1;

	hzbDebugInfo[threadIndex].objWorld = object[threadIndex].objWorld;

	hzbDebugInfo[threadIndex].center = object[threadIndex].center;
	hzbDebugInfo[threadIndex].extents = object[threadIndex].extents;

	hzbDebugInfo[threadIndex].posCW = posCW;
	hzbDebugInfo[threadIndex].posEW = posEW;

	hzbDebugInfo[threadIndex].camPos = camW;

	hzbDebugInfo[threadIndex].bboxPoint0 = bboxPointC[0].xyz;
	hzbDebugInfo[threadIndex].bboxPoint1 = bboxPointC[1].xyz;
	hzbDebugInfo[threadIndex].bboxPoint2 = bboxPointC[2].xyz;
	hzbDebugInfo[threadIndex].bboxPoint3 = bboxPointC[3].xyz;
	hzbDebugInfo[threadIndex].bboxPoint4 = bboxPointC[4].xyz;
	hzbDebugInfo[threadIndex].bboxPoint5 = bboxPointC[5].xyz;
	hzbDebugInfo[threadIndex].bboxPoint6 = bboxPointC[6].xyz;
	hzbDebugInfo[threadIndex].bboxPoint7 = bboxPointC[7].xyz;

	hzbDebugInfo[threadIndex].nearPoint0 = nearPoint[0];
	hzbDebugInfo[threadIndex].nearPoint1 = nearPoint[1];
	hzbDebugInfo[threadIndex].nearPoint2 = nearPoint[2];
	hzbDebugInfo[threadIndex].nearPoint3 = nearPoint[3];
	hzbDebugInfo[threadIndex].nearPoint4 = nearPoint[4];
	hzbDebugInfo[threadIndex].nearPoint5 = nearPoint[5];

	hzbDebugInfo[threadIndex].clipFrame0 = clipFrame[0];
	hzbDebugInfo[threadIndex].clipFrame1 = clipFrame[1];
	hzbDebugInfo[threadIndex].clipFrame2 = clipFrame[2];
	hzbDebugInfo[threadIndex].clipFrame3 = clipFrame[3];
	hzbDebugInfo[threadIndex].clipNearW = nearPoint[minIndex];
	hzbDebugInfo[threadIndex].clipNearC = clipNearC;
	float3 clipNearS = clipNearC;
	clipNearS.x = clamp((clipNearS.x + 1) * 0.5f, -0.001f, 1.001f);
	clipNearS.y = clamp((clipNearS.y + 1) * 0.5f, -0.001f, 1.001f);
	hzbDebugInfo[threadIndex].clipNearS = clipNearS;

	hzbDebugInfo[threadIndex].width = width;
	hzbDebugInfo[threadIndex].height = height;

	hzbDebugInfo[threadIndex].uvExtentsMax = float3(maxX, maxY, 0);
	hzbDebugInfo[threadIndex].uvExtentsMin = float3(minX, minY, 0);

	hzbDebugInfo[threadIndex].centerDepth = centerDepth;
	hzbDebugInfo[threadIndex].finalDepth = finalCompareDepth;

	hzbDebugInfo[threadIndex].threadIndex = threadIndex;
	hzbDebugInfo[threadIndex].queryIndex = queryIndex;
}
#endif
