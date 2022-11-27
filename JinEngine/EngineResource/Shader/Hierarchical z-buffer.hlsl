
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
	float4 posCH;
	float3 posCC;

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
	float3 nearPointF;

	float width;
	float height;

	float2 uv;
	float2 uvExtentsMax;
	float2 uvExtentsMin;

	float centerDepth;
	float finalDepth;
};


Texture2D depthMap: register(t0);
RWTexture2D<float> lastMipmap: register(u0);

#ifdef DOWN_SAMPLING_COUNT
Texture2D mipmap[DOWN_SAMPLING_COUNT] : register(t1, space1);
#endif

StructuredBuffer<ObjectInfo> object : register(t2);
RWStructuredBuffer<float> queryResult : register(u1, space1);
RWStructuredBuffer<HZBDebugInfo> hzbDebugInfo : register(u2, space1);

SamplerState downSam : register(s0);
SamplerState occSam : register(s1);

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
	int validQueryCount;
	int passPad00;
};

#if defined (DIMX) && defined (DIMY)
[numthreads(DIMX, DIMY, 1)]
void HZBDownSampling(int3 dispatchThreadID : SV_DispatchThreadID)
{
	//Thread dim X is texture width
   //gorup dim Y is texture height

	if ((nowWidth / 2) <= dispatchThreadID.x || (nowHeight / 2) <= dispatchThreadID.y)
		return;

	int mipLevel = 0;
	if (nowIndex > 0)
		mipLevel = nowIndex - 1;

	const float2 baseIndex = float2(dispatchThreadID.x, dispatchThreadID.y);
	const float2 uv = float2((baseIndex.x * 2) / nowWidth, (baseIndex.y * 2) / nowHeight);

	const float color00 = depthMap.SampleLevel(downSam, uv, 0).r;
	const float color01 = depthMap.SampleLevel(downSam, uv + float2(1 / nowWidth, 0), 0).r;
	const float color02 = depthMap.SampleLevel(downSam, uv + float2(1 / nowWidth, 1), 0).r;
	const float color03 = depthMap.SampleLevel(downSam, uv + float2(0, 1 / nowHeight), 0).r;

	float finalColor = min(color00, min(color01, min(color02, color03)));

	bool shouldIncludeExtraColumnFromPreviousLevel = ((nowWidth & 1) != 0);
	bool shouldIncludeExtraRowFromPreviousLevel = ((nowHeight & 1) != 0);

	if (shouldIncludeExtraColumnFromPreviousLevel)
	{
		const float extraColor00 = depthMap.SampleLevel(downSam, uv + float2(2 / nowWidth, 0), 0).r;
		const float extraColor01 = depthMap.SampleLevel(downSam, uv + float2(2 / nowWidth, 1 / nowHeight), 0).r;

		// In the case where the width and height are both odd, need to include the
		// 'corner' value as well.
		if (shouldIncludeExtraRowFromPreviousLevel)
		{
			const float extraColor02 = depthMap.SampleLevel(downSam, uv + float2(2 / nowWidth, 2), 0).r;
			finalColor = min(finalColor, extraColor02);
		}
		finalColor = min(finalColor, min(extraColor00, extraColor01));
	}
	if (shouldIncludeExtraRowFromPreviousLevel)
	{
		const float extraColor00 = depthMap.SampleLevel(downSam, uv + float2(0, 2 / nowHeight), 0).r;
		const float extraColor01 = depthMap.SampleLevel(downSam, uv + float2(1 / nowWidth, 2), 0).r;

		finalColor = min(finalColor, min(extraColor00, extraColor01));
	}

	lastMipmap[int2(dispatchThreadID.x, dispatchThreadID.y)].r = finalColor;

	//const float color = depthMap.SampleLevel(downSam, uv, 0).r;
}
#endif

/*
const float3 pNormal = normalize(cross((p1 - p0), (p2 - p0)));
	const float3 dist = dot(-pNormal, p0);
	const float dotCoord = dot(camPos, pNormal) + dist;
	return camPos - dot(camPos, pNormal) * pNormal + dist * pNormal;
*/
float3 CalNearPoint(const float3 p0, const float3 p1, const float3 p2)
{
	const float3 camPos = float3(0, 0, 0);
	const float3 pNormal = normalize(cross((p1 - p0), (p2 - p0)));
	const float3 dist = dot(-pNormal, p0);
	const float dotCoord = dot(camPos, pNormal) + dist;
	return camPos - dot(camPos, pNormal) * pNormal + dist * pNormal;
}

#if defined (DIMX) && defined (DIMY) && defined (DIMZ)
[numthreads(DIMX, DIMY, DIMZ)]
void HZBOcclusion(int3 dispatchThreadID : SV_DispatchThreadID)
{
	//Thread dim X is objectList x
   //gorup dim X is objectList x

	if (dispatchThreadID.x >= validQueryCount || !object[dispatchThreadID.x].isValid)
		return;

	const float4 posCW = mul(float4(object[dispatchThreadID.x].center, 1.0f), object[dispatchThreadID.x].objWorld);
	const float4 posCV = mul(posCW, camView);
	const float4 posCH = mul(posCV, camProj);
	const float3 posCC = posCH.xyz / posCH.w;

	const float4 posEW = mul(float4(object[dispatchThreadID.x].extents, 1.0f), object[dispatchThreadID.x].objWorld) - posCW + float4(0, 0, 0, 1);
	const float3 camW = mul(float4(0.0f, 0.0f, 0.0f, 1.0f), camWorld).xyz;

	const float4 bboxPointH[8] =
	{
		mul(float4(posCW.xyz + float3(posEW.x, posEW.y, posEW.z), 1.0f),camViewProj),
		mul(float4(posCW.xyz + float3(posEW.x, posEW.y, -posEW.z), 1.0f),camViewProj),
		mul(float4(posCW.xyz + float3(posEW.x, -posEW.y, posEW.z), 1.0f),camViewProj),
		mul(float4(posCW.xyz + float3(posEW.x, -posEW.y, -posEW.z), 1.0f),camViewProj),
		mul(float4(posCW.xyz + float3(-posEW.x, posEW.y, posEW.z), 1.0f),camViewProj),
		mul(float4(posCW.xyz + float3(-posEW.x, posEW.y, -posEW.z), 1.0f),camViewProj),
		mul(float4(posCW.xyz + float3(-posEW.x, -posEW.y, posEW.z), 1.0f),camViewProj),
		mul(float4(posCW.xyz + float3(-posEW.x, -posEW.y, -posEW.z), 1.0f),camViewProj)
	};

	const float3 bboxPointC[8] =
	{
		bboxPointH[0].xyz / bboxPointH[0].w,
		bboxPointH[1].xyz / bboxPointH[1].w,
		bboxPointH[2].xyz / bboxPointH[2].w,
		bboxPointH[3].xyz / bboxPointH[3].w,
		bboxPointH[4].xyz / bboxPointH[4].w,
		bboxPointH[5].xyz / bboxPointH[5].w,
		bboxPointH[6].xyz / bboxPointH[6].w,
		bboxPointH[7].xyz / bboxPointH[7].w,
	};

	static const float maxDistance = 100000;
	static const float minDistance = -100000;

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

	maxX = (maxX + 1) * 0.5f;
	minX = (minX + 1) * 0.5f;
	maxY = (maxY + 1) * 0.5f;
	minY = (minY + 1) * 0.5f;

	const float width = viewWidth * (maxX - minX);
	const float height = viewHeight * (maxY - minY);

	int lod = DOWN_SAMPLING_COUNT - (ceil(log2(max(width, height)) + 1));
	if (lod < 0)
		lod = 0;

	const float3 nearPoint[6] =
	{
		CalNearPoint(bboxPointC[0], bboxPointC[1], bboxPointC[2]),
		CalNearPoint(bboxPointC[0], bboxPointC[1], bboxPointC[4]),
		CalNearPoint(bboxPointC[0], bboxPointC[2], bboxPointC[4]),
		CalNearPoint(bboxPointC[7], bboxPointC[5], bboxPointC[6]),
		CalNearPoint(bboxPointC[1], bboxPointC[5], bboxPointC[3]),
		CalNearPoint(bboxPointC[7], bboxPointC[3], bboxPointC[6])
	};

	float minDist = 10000;
	uint minIndex = 0;
	for (uint i = 0; i < 6; ++i)
	{
		float dist = length(nearPoint[i]);
		if (dist < minDist)
		{
			minDist = dist;
			minIndex = i;
		}
	}
	 
	const float2 clipNear = float2((nearPoint[minIndex].x + 1) * 0.5f, (nearPoint[minIndex].y + 1) * 0.5f);

	const float2 clipPoint00 = float2(maxX, maxY);
	const float2 clipPoint01 = float2(maxX, minY);
	const float2 clipPoint02 = float2(minX, maxY);
	const float2 clipPoint03 = float2(minX, minY);

	const float centerDepth = mipmap[lod].SampleLevel(occSam, clipNear, 0).r;
	const float compareDepth00 = mipmap[lod].SampleLevel(occSam, clipPoint00, 0).r;
	const float compareDepth01 = mipmap[lod].SampleLevel(occSam, clipPoint01, 0).r;
	const float compareDepth02 = mipmap[lod].SampleLevel(occSam, clipPoint02, 0).r;
	const float compareDepth03 = mipmap[lod].SampleLevel(occSam, clipPoint03, 0).r;

	const float finalCompareDepth = max(compareDepth00, max(compareDepth01, max(compareDepth02, compareDepth03)));

	//HZBDebugInfo info;
	hzbDebugInfo[dispatchThreadID.x].objWorld = object[dispatchThreadID.x].objWorld;

	hzbDebugInfo[dispatchThreadID.x].center = object[dispatchThreadID.x].center;
	hzbDebugInfo[dispatchThreadID.x].extents = object[dispatchThreadID.x].extents;

	hzbDebugInfo[dispatchThreadID.x].posCW = posCW;
	hzbDebugInfo[dispatchThreadID.x].posCV = posCV;
	hzbDebugInfo[dispatchThreadID.x].posCH = posCH;
	hzbDebugInfo[dispatchThreadID.x].posCC = posCC;

	hzbDebugInfo[dispatchThreadID.x].posEW = posEW; 

	hzbDebugInfo[dispatchThreadID.x].camPos = camW;

	hzbDebugInfo[dispatchThreadID.x].bboxPoint0 = bboxPointC[0];
	hzbDebugInfo[dispatchThreadID.x].bboxPoint1 = bboxPointC[1];
	hzbDebugInfo[dispatchThreadID.x].bboxPoint2 = bboxPointC[2];
	hzbDebugInfo[dispatchThreadID.x].bboxPoint3 = bboxPointC[3];
	hzbDebugInfo[dispatchThreadID.x].bboxPoint4 = bboxPointC[4];
	hzbDebugInfo[dispatchThreadID.x].bboxPoint5 = bboxPointC[5];
	hzbDebugInfo[dispatchThreadID.x].bboxPoint6 = bboxPointC[6];
	hzbDebugInfo[dispatchThreadID.x].bboxPoint7 = bboxPointC[7];

	hzbDebugInfo[dispatchThreadID.x].nearPoint0 = nearPoint[0];
	hzbDebugInfo[dispatchThreadID.x].nearPoint1 = nearPoint[1];
	hzbDebugInfo[dispatchThreadID.x].nearPoint2 = nearPoint[2];
	hzbDebugInfo[dispatchThreadID.x].nearPoint3 = nearPoint[3];
	hzbDebugInfo[dispatchThreadID.x].nearPoint4 = nearPoint[4];
	hzbDebugInfo[dispatchThreadID.x].nearPoint5 = nearPoint[5];
	hzbDebugInfo[dispatchThreadID.x].nearPointF = nearPoint[minIndex];

	hzbDebugInfo[dispatchThreadID.x].width = width;
	hzbDebugInfo[dispatchThreadID.x].height = height;

	hzbDebugInfo[dispatchThreadID.x].uv = clipNear;
	hzbDebugInfo[dispatchThreadID.x].uvExtentsMax = float2(maxX, maxY);
	hzbDebugInfo[dispatchThreadID.x].uvExtentsMin = float2(minX, minY);

	hzbDebugInfo[dispatchThreadID.x].centerDepth = centerDepth;
	hzbDebugInfo[dispatchThreadID.x].finalDepth = finalCompareDepth;

	if (centerDepth <= finalCompareDepth)
		queryResult[dispatchThreadID.x] = 0;
	else
		queryResult[dispatchThreadID.x] = 1;
}
#endif
 
/*

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
	float4 posCH;
	float3 posCC;

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
	float3 nearPointF;

	float width;
	float height;

	float2 uv;
	float2 uvExtentsMax;
	float2 uvExtentsMin;

	float centerDepth;
	float finalDepth;
};


Texture2D depthMap: register(t0);
RWTexture2D<float> lastMipmap: register(u0);

#ifdef DOWN_SAMPLING_COUNT
Texture2D mipmap[DOWN_SAMPLING_COUNT] : register(t1, space1);
#endif

StructuredBuffer<ObjectInfo> object : register(t2);
RWStructuredBuffer<float> queryResult : register(u1, space1);
RWStructuredBuffer<HZBDebugInfo> hzbDebugInfo : register(u2, space1);

SamplerState downSam : register(s0);
SamplerState occSam : register(s1);

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
	int validQueryCount;
	int passPad00;
};

#if defined (DIMX) && defined (DIMY)
[numthreads(DIMX, DIMY, 1)]
void HZBDownSampling(int3 dispatchThreadID : SV_DispatchThreadID)
{
	//Thread dim X is texture width
   //gorup dim Y is texture height

	if ((nowWidth / 2) <= dispatchThreadID.x || (nowHeight / 2) <= dispatchThreadID.y)
		return;

	int mipLevel = 0;
	if (nowIndex > 0)
		mipLevel = nowIndex - 1;

	const float2 baseIndex = float2(dispatchThreadID.x, dispatchThreadID.y);
	const float2 uv = float2((baseIndex.x * 2) / nowWidth, (baseIndex.y * 2) / nowHeight);

	const float color00 = depthMap.SampleLevel(downSam, uv, 0).r;
	const float color01 = depthMap.SampleLevel(downSam, uv + float2(1 / nowWidth, 0), 0).r;
	const float color02 = depthMap.SampleLevel(downSam, uv + float2(1 / nowWidth, 1), 0).r;
	const float color03 = depthMap.SampleLevel(downSam, uv + float2(0, 1 / nowHeight), 0).r;

	float finalColor = min(color00, min(color01, min(color02, color03)));

	bool shouldIncludeExtraColumnFromPreviousLevel = ((nowWidth & 1) != 0);
	bool shouldIncludeExtraRowFromPreviousLevel = ((nowHeight & 1) != 0);

	if (shouldIncludeExtraColumnFromPreviousLevel)
	{
		const float extraColor00 = depthMap.SampleLevel(downSam, uv + float2(2 / nowWidth, 0), 0).r;
		const float extraColor01 = depthMap.SampleLevel(downSam, uv + float2(2 / nowWidth, 1 / nowHeight), 0).r;

		// In the case where the width and height are both odd, need to include the
		// 'corner' value as well.
		if (shouldIncludeExtraRowFromPreviousLevel)
		{
			const float extraColor02 = depthMap.SampleLevel(downSam, uv + float2(2 / nowWidth, 2), 0).r;
			finalColor = min(finalColor, extraColor02);
		}
		finalColor = min(finalColor, min(extraColor00, extraColor01));
	}
	if (shouldIncludeExtraRowFromPreviousLevel)
	{
		const float extraColor00 = depthMap.SampleLevel(downSam, uv + float2(0, 2 / nowHeight), 0).r;
		const float extraColor01 = depthMap.SampleLevel(downSam, uv + float2(1 / nowWidth, 2), 0).r;

		finalColor = min(finalColor, min(extraColor00, extraColor01));
	}

	lastMipmap[int2(dispatchThreadID.x, dispatchThreadID.y)].r = finalColor;

	//const float color = depthMap.SampleLevel(downSam, uv, 0).r;
}
#endif

/*
const float3 pNormal = normalize(cross((p1 - p0), (p2 - p0)));
	const float3 dist = dot(-pNormal, p0);
	const float dotCoord = dot(camPos, pNormal) + dist;
	return camPos - dot(camPos, pNormal) * pNormal + dist * pNormal;
*/
float3 CalNearPoint(const float3 p0, const float3 p1, const float3 p2)
{
	const float3 camPos = float3(0, 0, 0);
	const float3 pNormal = normalize(cross((p1 - p0), (p2 - p0)));
	const float3 dist = dot(-pNormal, p0);
	const float dotCoord = dot(camPos, pNormal) + dist;
	return camPos - dot(camPos, pNormal) * pNormal + dist * pNormal;
}

#if defined (DIMX) && defined (DIMY) && defined (DIMZ)
[numthreads(DIMX, DIMY, DIMZ)]
void HZBOcclusion(int3 dispatchThreadID : SV_DispatchThreadID)
{
	//Thread dim X is objectList x
   //gorup dim X is objectList x

	if (dispatchThreadID.x >= validQueryCount || !object[dispatchThreadID.x].isValid)
		return;

	const float4 posCW = mul(float4(object[dispatchThreadID.x].center, 1.0f), object[dispatchThreadID.x].objWorld);
	const float4 posCV = mul(posCW, camView);
	const float4 posCH = mul(posCV, camProj);
	const float3 posCC = posCH.xyz / posCH.w;

	const float4 posEW = mul(float4(object[dispatchThreadID.x].extents, 1.0f), object[dispatchThreadID.x].objWorld) - posCW + float4(0, 0, 0, 1);
	const float3 camW = mul(float4(0.0f, 0.0f, 0.0f, 1.0f), camWorld).xyz;

	const float4 bboxPointH[8] =
	{
		mul(float4(posCW.xyz + float3(posEW.x, posEW.y, posEW.z), 1.0f),camViewProj),
		mul(float4(posCW.xyz + float3(posEW.x, posEW.y, -posEW.z), 1.0f),camViewProj),
		mul(float4(posCW.xyz + float3(posEW.x, -posEW.y, posEW.z), 1.0f),camViewProj),
		mul(float4(posCW.xyz + float3(posEW.x, -posEW.y, -posEW.z), 1.0f),camViewProj),
		mul(float4(posCW.xyz + float3(-posEW.x, posEW.y, posEW.z), 1.0f),camViewProj),
		mul(float4(posCW.xyz + float3(-posEW.x, posEW.y, -posEW.z), 1.0f),camViewProj),
		mul(float4(posCW.xyz + float3(-posEW.x, -posEW.y, posEW.z), 1.0f),camViewProj),
		mul(float4(posCW.xyz + float3(-posEW.x, -posEW.y, -posEW.z), 1.0f),camViewProj)
	};

	const float3 bboxPointC[8] =
	{
		bboxPointH[0].xyz / bboxPointH[0].w,
		bboxPointH[1].xyz / bboxPointH[1].w,
		bboxPointH[2].xyz / bboxPointH[2].w,
		bboxPointH[3].xyz / bboxPointH[3].w,
		bboxPointH[4].xyz / bboxPointH[4].w,
		bboxPointH[5].xyz / bboxPointH[5].w,
		bboxPointH[6].xyz / bboxPointH[6].w,
		bboxPointH[7].xyz / bboxPointH[7].w,
	};

	static const float maxDistance = 100000;
	static const float minDistance = -100000;

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

	maxX = (maxX + 1) * 0.5f;
	minX = (minX + 1) * 0.5f;
	maxY = (maxY + 1) * 0.5f;
	minY = (minY + 1) * 0.5f;

	const float width = viewWidth * (maxX - minX);
	const float height = viewHeight * (maxY - minY);

	int lod = DOWN_SAMPLING_COUNT - (ceil(log2(max(width, height)) + 1));
	if (lod < 0)
		lod = 0;

	const float3 nearPoint[6] =
	{
		CalNearPoint(bboxPointC[0], bboxPointC[1], bboxPointC[2]),
		CalNearPoint(bboxPointC[0], bboxPointC[1], bboxPointC[4]),
		CalNearPoint(bboxPointC[0], bboxPointC[2], bboxPointC[4]),
		CalNearPoint(bboxPointC[7], bboxPointC[5], bboxPointC[6]),
		CalNearPoint(bboxPointC[1], bboxPointC[5], bboxPointC[3]),
		CalNearPoint(bboxPointC[7], bboxPointC[3], bboxPointC[6])
	};

	float minDist = 10000;
	uint minIndex = 0;
	for (uint i = 0; i < 6; ++i)
	{
		float dist = length(nearPoint[i]);
		if (dist < minDist)
		{
			minDist = dist;
			minIndex = i;
		}
	}

	const float2 clipNear = float2((nearPoint[minIndex].x + 1) * 0.5f, (nearPoint[minIndex].y + 1) * 0.5f);

	const float2 clipPoint00 = float2(maxX, maxY);
	const float2 clipPoint01 = float2(maxX, minY);
	const float2 clipPoint02 = float2(minX, maxY);
	const float2 clipPoint03 = float2(minX, minY);

	const float centerDepth = mipmap[lod].SampleLevel(occSam, clipNear, 0).r;
	const float compareDepth00 = mipmap[lod].SampleLevel(occSam, clipPoint00, 0).r;
	const float compareDepth01 = mipmap[lod].SampleLevel(occSam, clipPoint01, 0).r;
	const float compareDepth02 = mipmap[lod].SampleLevel(occSam, clipPoint02, 0).r;
	const float compareDepth03 = mipmap[lod].SampleLevel(occSam, clipPoint03, 0).r;

	const float finalCompareDepth = max(compareDepth00, max(compareDepth01, max(compareDepth02, compareDepth03)));

	//HZBDebugInfo info;
	hzbDebugInfo[dispatchThreadID.x].objWorld = object[dispatchThreadID.x].objWorld;

	hzbDebugInfo[dispatchThreadID.x].center = object[dispatchThreadID.x].center;
	hzbDebugInfo[dispatchThreadID.x].extents = object[dispatchThreadID.x].extents;

	hzbDebugInfo[dispatchThreadID.x].posCW = posCW;
	hzbDebugInfo[dispatchThreadID.x].posCV = posCV;
	hzbDebugInfo[dispatchThreadID.x].posCH = posCH;
	hzbDebugInfo[dispatchThreadID.x].posCC = posCC;

	hzbDebugInfo[dispatchThreadID.x].posEW = posEW;

	hzbDebugInfo[dispatchThreadID.x].camPos = camW;

	hzbDebugInfo[dispatchThreadID.x].bboxPoint0 = bboxPointC[0];
	hzbDebugInfo[dispatchThreadID.x].bboxPoint1 = bboxPointC[1];
	hzbDebugInfo[dispatchThreadID.x].bboxPoint2 = bboxPointC[2];
	hzbDebugInfo[dispatchThreadID.x].bboxPoint3 = bboxPointC[3];
	hzbDebugInfo[dispatchThreadID.x].bboxPoint4 = bboxPointC[4];
	hzbDebugInfo[dispatchThreadID.x].bboxPoint5 = bboxPointC[5];
	hzbDebugInfo[dispatchThreadID.x].bboxPoint6 = bboxPointC[6];
	hzbDebugInfo[dispatchThreadID.x].bboxPoint7 = bboxPointC[7];

	hzbDebugInfo[dispatchThreadID.x].nearPoint0 = nearPoint[0];
	hzbDebugInfo[dispatchThreadID.x].nearPoint1 = nearPoint[1];
	hzbDebugInfo[dispatchThreadID.x].nearPoint2 = nearPoint[2];
	hzbDebugInfo[dispatchThreadID.x].nearPoint3 = nearPoint[3];
	hzbDebugInfo[dispatchThreadID.x].nearPoint4 = nearPoint[4];
	hzbDebugInfo[dispatchThreadID.x].nearPoint5 = nearPoint[5];
	hzbDebugInfo[dispatchThreadID.x].nearPointF = nearPoint[minIndex];

	hzbDebugInfo[dispatchThreadID.x].width = width;
	hzbDebugInfo[dispatchThreadID.x].height = height;

	hzbDebugInfo[dispatchThreadID.x].uv = clipNear;
	hzbDebugInfo[dispatchThreadID.x].uvExtentsMax = float2(maxX, maxY);
	hzbDebugInfo[dispatchThreadID.x].uvExtentsMin = float2(minX, minY);

	hzbDebugInfo[dispatchThreadID.x].centerDepth = centerDepth;
	hzbDebugInfo[dispatchThreadID.x].finalDepth = finalCompareDepth;

	if (centerDepth <= finalCompareDepth)
		queryResult[dispatchThreadID.x] = 0;
	else
		queryResult[dispatchThreadID.x] = 1;
}
#endif

/*
#if defined (DIMX) && defined (DIMY) && defined (DIMZ)
[numthreads(DIMX, DIMY, DIMZ)]
void HZBOcclusion(int3 dispatchThreadID : SV_DispatchThreadID)
{
	//Thread dim X is objectList x
   //gorup dim X is objectList x

	if (dispatchThreadID.x >= validQueryCount || !object[dispatchThreadID.x].isValid)
		return;

	const float4 posCW = mul(float4(object[dispatchThreadID.x].center, 1.0f), object[dispatchThreadID.x].objWorld);
	const float4 posCV = mul(posCW, camView);
	const float4 posCH = mul(posCV, camProj);
	const float3 posCC = posCH.xyz / posCH.w;

	const float4 posEW = mul(float4(object[dispatchThreadID.x].extents, 1.0f), object[dispatchThreadID.x].objWorld) - posCW + float4(0, 0, 0, 1);
	const float4 posEV = mul(posEW, camView);
	const float4 posEH = mul(posEV, camProj);
	const float3 posEC = posEH.xyz / posEH.w;

	const float3 camW = mul(float4(0.0f, 0.0f, 0.0f, 1.0f), camWorld).xyz;

	const float4 bboxPointH[8] =
	{
		mul(float4(posCW.xyz + float3(posEW.x, posEW.y, posEW.z), 1.0f),camViewProj),
		mul(float4(posCW.xyz + float3(posEW.x, posEW.y, -posEW.z), 1.0f),camViewProj),
		mul(float4(posCW.xyz + float3(posEW.x, -posEW.y, posEW.z), 1.0f),camViewProj),
		mul(float4(posCW.xyz + float3(posEW.x, -posEW.y, -posEW.z), 1.0f),camViewProj),
		mul(float4(posCW.xyz + float3(-posEW.x, posEW.y, posEW.z), 1.0f),camViewProj),
		mul(float4(posCW.xyz + float3(-posEW.x, posEW.y, -posEW.z), 1.0f),camViewProj),
		mul(float4(posCW.xyz + float3(-posEW.x, -posEW.y, posEW.z), 1.0f),camViewProj),
		mul(float4(posCW.xyz + float3(-posEW.x, -posEW.y, -posEW.z), 1.0f),camViewProj)
	};

	const float3 bboxPointC[8] =
	{
		bboxPointH[0].xyz / bboxPointH[0].w,
		bboxPointH[1].xyz / bboxPointH[1].w,
		bboxPointH[2].xyz / bboxPointH[2].w,
		bboxPointH[3].xyz / bboxPointH[3].w,
		bboxPointH[4].xyz / bboxPointH[4].w,
		bboxPointH[5].xyz / bboxPointH[5].w,
		bboxPointH[6].xyz / bboxPointH[6].w,
		bboxPointH[7].xyz / bboxPointH[7].w,
	};

	static const float maxDistance = 100000;
	static const float minDistance = 0;

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

	const float width = maxY - minX;
	const float height = maxY - minY;

	int lod = DOWN_SAMPLING_COUNT - (ceil(log2(max(width, height)) + 1));
	if (lod < 0)
		lod = 0;

	const float3 nearPoint[6] =
	{
		CalNearPoint(bboxPointC[0], bboxPointC[1], bboxPointC[2]),
		CalNearPoint(bboxPointC[0], bboxPointC[1], bboxPointC[4]),
		CalNearPoint(bboxPointC[0], bboxPointC[2], bboxPointC[4]),
		CalNearPoint(bboxPointC[7], bboxPointC[5], bboxPointC[6]),
		CalNearPoint(bboxPointC[1], bboxPointC[5], bboxPointC[3]),
		CalNearPoint(bboxPointC[7], bboxPointC[3], bboxPointC[6])
	};

	float minDist = 10000;
	uint minIndex = 0;
	for (uint i = 0; i < 6; ++i)
	{
		float dist = length(nearPoint[i]);
		if (dist < minDist)
		{
			minDist = dist;
			minIndex = i;
		}
	}

	const float4 nearH = mul(float4(nearPoint[minIndex], 1.0f), camViewProj);
	const float3 nearC = nearH.xyz / nearH.w;

	const float2 clipNear = float2((nearC.x + 1) * 0.5f, (nearC.y + 1) * 0.5f);

	const float2 clipPoint00 = float2(maxX, maxY));
	const float2 clipPoint01 = float2(maxX, minY));
	const float2 clipPoint02 = float2(minX, maxY));
	const float2 clipPoint03 = float2(minX, minY));

	const float centerDepth = mipmap[lod].SampleLevel(occSam, clipNear, 0).r;
	const float compareDepth00 = mipmap[lod].SampleLevel(occSam, clipPoint00, 0).r;
	const float compareDepth01 = mipmap[lod].SampleLevel(occSam, clipPoint01, 0).r;
	const float compareDepth02 = mipmap[lod].SampleLevel(occSam, clipPoint02, 0).r;
	const float compareDepth03 = mipmap[lod].SampleLevel(occSam, clipPoint03, 0).r;

	const float finalCompareDepth = min(compareDepth00, min(compareDepth01, min(compareDepth02, compareDepth03)));

	//HZBDebugInfo info;
	hzbDebugInfo[dispatchThreadID.x].objWorld = object[dispatchThreadID.x].objWorld;

	hzbDebugInfo[dispatchThreadID.x].center = object[dispatchThreadID.x].center;
	hzbDebugInfo[dispatchThreadID.x].extents = object[dispatchThreadID.x].extents;

	hzbDebugInfo[dispatchThreadID.x].posCW = posCW;
	hzbDebugInfo[dispatchThreadID.x].posCV = posCV;
	hzbDebugInfo[dispatchThreadID.x].posCH = posCH;
	hzbDebugInfo[dispatchThreadID.x].posCC = posCC;

	hzbDebugInfo[dispatchThreadID.x].posEW = posEW;
	hzbDebugInfo[dispatchThreadID.x].posEV = posEV;
	hzbDebugInfo[dispatchThreadID.x].posEH = posEH;
	hzbDebugInfo[dispatchThreadID.x].posEC = posEC;

	hzbDebugInfo[dispatchThreadID.x].camPos = camW;

	hzbDebugInfo[dispatchThreadID.x].bboxPoint0 = bboxPoint[0];
	hzbDebugInfo[dispatchThreadID.x].bboxPoint1 = bboxPoint[1];
	hzbDebugInfo[dispatchThreadID.x].bboxPoint2 = bboxPoint[2];
	hzbDebugInfo[dispatchThreadID.x].bboxPoint3 = bboxPoint[3];
	hzbDebugInfo[dispatchThreadID.x].bboxPoint4 = bboxPoint[4];
	hzbDebugInfo[dispatchThreadID.x].bboxPoint5 = bboxPoint[5];
	hzbDebugInfo[dispatchThreadID.x].bboxPoint6 = bboxPoint[6];
	hzbDebugInfo[dispatchThreadID.x].bboxPoint7 = bboxPoint[7];

	hzbDebugInfo[dispatchThreadID.x].nearPoint0 = nearPoint[0];
	hzbDebugInfo[dispatchThreadID.x].nearPoint1 = nearPoint[1];
	hzbDebugInfo[dispatchThreadID.x].nearPoint2 = nearPoint[2];
	hzbDebugInfo[dispatchThreadID.x].nearPoint3 = nearPoint[3];
	hzbDebugInfo[dispatchThreadID.x].nearPoint4 = nearPoint[4];
	hzbDebugInfo[dispatchThreadID.x].nearPoint5 = nearPoint[5];
	hzbDebugInfo[dispatchThreadID.x].nearPointF = nearPoint[minIndex];

	hzbDebugInfo[dispatchThreadID.x].width = width;
	hzbDebugInfo[dispatchThreadID.x].height = height;

	hzbDebugInfo[dispatchThreadID.x].uv = clipNear;
	hzbDebugInfo[dispatchThreadID.x].uvExtentsMax = float2(maxX, maxY);
	hzbDebugInfo[dispatchThreadID.x].uvExtentsMin = float2(minX, minY);

	hzbDebugInfo[dispatchThreadID.x].centerDepth = centerDepth;
	hzbDebugInfo[dispatchThreadID.x].finalDepth = finalCompareDepth;

	if (centerDepth <= finalCompareDepth)
		queryResult[dispatchThreadID.x] = 0;
	else
		queryResult[dispatchThreadID.x] = 1;
}
#endif
*/
*/