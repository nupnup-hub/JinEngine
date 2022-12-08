struct ObjectInfo
{
	float4x4 objWorld;
	float3 center;
	int queryResultIndex;
	float3 extents;
	int isValid;
};

//Debug
/*
struct HZBDebugInfo
{
	float4x4 objWorld;

	float3 center;
	float3 extents;

	float4 posCW;
	float4 posCV;
	float4 posEW;
	float4 posEV;

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
	int lod;

	float3 uvExtentsMax;
	float3 uvExtentsMin;

	float centerDepth;
	float finalDepth;

	int threadIndex;
	int queryIndex;
};
RWStructuredBuffer<HZBDebugInfo> hzbDebugInfo : register(u2, space1);
*/

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
	float4x4 camView;
	float4x4 camProj; 
	float4 frustumPlane[6];
	float viewWidth;
	float viewHeight;
	float camNear;
	float camFar;
	int validQueryCount;
	int occMapCount; 
	int occIndexOffset;
	int correctFailTrigger; 
};

static const float maxDistance = 100000;
static const float minDistance = -100000;

//occMap size max is 512
//512 is less than thread and group max dim

float ToLinearZValue(const float v)
{
	return (2.0f * camNear) / (camNear + camFar - v * (camFar - camNear));
}
float ToNoLinearZValue(const float v)
{
	return  -((camNear + camFar) * v - (2 * camNear)) / ((camNear - camFar) * v);
}

#if defined (DIMX) && defined (DIMY)
[numthreads(DIMX, DIMY, 1)]
void HZBCopyDepthMap(int3 dispatchThreadID : SV_DispatchThreadID)
{
	if (nowWidth <= dispatchThreadID.x || nowHeight <= dispatchThreadID.y)
		return;
	 
	lastMipmap[int2(dispatchThreadID.x, dispatchThreadID.y)].r = ToLinearZValue(depthMap.Load(int3(dispatchThreadID.x, dispatchThreadID.y, 0)).r);
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
 
int CullBBox(const float3 center, const float3 extents)
{
	int isOut = dot(float4(center, 1.0f), frustumPlane[0]) > dot(extents, abs(frustumPlane[0].xyz));
	isOut += dot(float4(center, 1.0f), frustumPlane[1]) > dot(extents, abs(frustumPlane[1].xyz));
	isOut += dot(float4(center, 1.0f), frustumPlane[2]) > dot(extents, abs(frustumPlane[2].xyz));
	isOut += dot(float4(center, 1.0f), frustumPlane[3]) > dot(extents, abs(frustumPlane[3].xyz));
	isOut += dot(float4(center, 1.0f), frustumPlane[4]) > dot(extents, abs(frustumPlane[4].xyz));
	isOut += dot(float4(center, 1.0f), frustumPlane[5]) > dot(extents, abs(frustumPlane[5].xyz));

	return isOut;
}

float3 CalNearPoint(const float3 p0, const float3 p1, const float3 p2)
{
	const float3 camPos = float3(0, 0, 0);
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

float VertexSampleDepth(const float4 posCW, float4 posEW)
{
	const float4 bboxPointV[8] =
	{
		mul(float4(posCW.xyz + float3(posEW.x, posEW.y, posEW.z), 1.0f), camView),
		mul(float4(posCW.xyz + float3(posEW.x, posEW.y, -posEW.z), 1.0f), camView),
		mul(float4(posCW.xyz + float3(posEW.x, -posEW.y, posEW.z), 1.0f), camView),
		mul(float4(posCW.xyz + float3(posEW.x, -posEW.y, -posEW.z), 1.0f), camView),
		mul(float4(posCW.xyz + float3(-posEW.x, posEW.y, posEW.z), 1.0f), camView),
		mul(float4(posCW.xyz + float3(-posEW.x, posEW.y, -posEW.z), 1.0f), camView),
		mul(float4(posCW.xyz + float3(-posEW.x, -posEW.y, posEW.z), 1.0f), camView),
		mul(float4(posCW.xyz + float3(-posEW.x, -posEW.y, -posEW.z), 1.0f), camView)
	};

	const float4 bboxPointH[8] =
	{
		mul(bboxPointV[0], camProj),
		mul(bboxPointV[1], camProj),
		mul(bboxPointV[2], camProj),
		mul(bboxPointV[3], camProj),
		mul(bboxPointV[4], camProj),
		mul(bboxPointV[5], camProj),
		mul(bboxPointV[6], camProj),
		mul(bboxPointV[7], camProj)
	};

	const float3 bboxPointNdc[8] =
	{
		(bboxPointH[0].xyz / bboxPointH[0].w) * float3(0.5f, -0.5f, 1.0f) + float3(0.5f, 0.5f, 0.0f),
		(bboxPointH[1].xyz / bboxPointH[1].w) * float3(0.5f, -0.5f, 1.0f) + float3(0.5f, 0.5f, 0.0f),
		(bboxPointH[2].xyz / bboxPointH[2].w) * float3(0.5f, -0.5f, 1.0f) + float3(0.5f, 0.5f, 0.0f),
		(bboxPointH[3].xyz / bboxPointH[3].w) * float3(0.5f, -0.5f, 1.0f) + float3(0.5f, 0.5f, 0.0f),
		(bboxPointH[4].xyz / bboxPointH[4].w) * float3(0.5f, -0.5f, 1.0f) + float3(0.5f, 0.5f, 0.0f),
		(bboxPointH[5].xyz / bboxPointH[5].w) * float3(0.5f, -0.5f, 1.0f) + float3(0.5f, 0.5f, 0.0f),
		(bboxPointH[6].xyz / bboxPointH[6].w) * float3(0.5f, -0.5f, 1.0f) + float3(0.5f, 0.5f, 0.0f),
		(bboxPointH[7].xyz / bboxPointH[7].w) * float3(0.5f, -0.5f, 1.0f) + float3(0.5f, 0.5f, 0.0f)
	};


	float3 minXNdc = float3(maxDistance, 0, 0);
	float3 maxXNdc = float3(minDistance, 0, 0);
	float3 minYNdc = float3(0, maxDistance, 0);
	float3 maxYNdc = float3(0, minDistance, 0);
	float3 minZNdc = float3(0, 0, maxDistance);

	for (uint i = 0; i < 8; ++i)
	{
		if (bboxPointNdc[i].x < minXNdc.x)
			minXNdc = bboxPointNdc[i];
		if (bboxPointNdc[i].x > maxXNdc.x)
			maxXNdc = bboxPointNdc[i];
		if (bboxPointNdc[i].y < minYNdc.y)
			minYNdc = bboxPointNdc[i];
		if (bboxPointNdc[i].y > maxYNdc.y)
			maxYNdc = bboxPointNdc[i];
		if (bboxPointNdc[i].z < minZNdc.z)
			minZNdc = bboxPointNdc[i];
	}

	const float width = viewWidth * (maxXNdc.x - minXNdc.x);
	const float height = viewHeight * (maxYNdc.y - minYNdc.y);

	int lodFactor = ceil(log2(max(width, height))) - occIndexOffset;
	if (lodFactor < 0)
		lodFactor = 0;

	const int lod = clamp((occMapCount - 1) - lodFactor, 0, occMapCount - 1);

	const float compareDepth00 = ToNoLinearZValue(depthMipmap.SampleLevel(occFrameSam, float2(maxXNdc.x, maxYNdc.y), lod).r);
	const float compareDepth01 = ToNoLinearZValue(depthMipmap.SampleLevel(occFrameSam, float2(maxXNdc.x, minYNdc.y), lod).r);
	const float compareDepth02 = ToNoLinearZValue(depthMipmap.SampleLevel(occFrameSam, float2(minXNdc.x, maxYNdc.y), lod).r);
	const float compareDepth03 = ToNoLinearZValue(depthMipmap.SampleLevel(occFrameSam, float2(minXNdc.x, minYNdc.y), lod).r);

	return max(compareDepth00, max(compareDepth01, max(compareDepth02, compareDepth03)));
}
float RasterizeSampleDepth(const float centerDepth, const float4 posCW, float4 posEW)
{
	const float4 bboxPointV[8] =
	{
		mul(float4(posCW.xyz + float3(posEW.x, posEW.y, posEW.z), 1.0f), camView),
		mul(float4(posCW.xyz + float3(posEW.x, posEW.y, -posEW.z), 1.0f), camView),
		mul(float4(posCW.xyz + float3(posEW.x, -posEW.y, posEW.z), 1.0f), camView),
		mul(float4(posCW.xyz + float3(posEW.x, -posEW.y, -posEW.z), 1.0f), camView),
		mul(float4(posCW.xyz + float3(-posEW.x, posEW.y, posEW.z), 1.0f), camView),
		mul(float4(posCW.xyz + float3(-posEW.x, posEW.y, -posEW.z), 1.0f), camView),
		mul(float4(posCW.xyz + float3(-posEW.x, -posEW.y, posEW.z), 1.0f), camView),
		mul(float4(posCW.xyz + float3(-posEW.x, -posEW.y, -posEW.z), 1.0f), camView)
	};

	const float4 bboxPointH[8] =
	{
		mul(bboxPointV[0], camProj),
		mul(bboxPointV[1], camProj),
		mul(bboxPointV[2], camProj),
		mul(bboxPointV[3], camProj),
		mul(bboxPointV[4], camProj),
		mul(bboxPointV[5], camProj),
		mul(bboxPointV[6], camProj),
		mul(bboxPointV[7], camProj)
	};

	const float3 bboxPointNdc[8] =
	{
		(bboxPointH[0].xyz / bboxPointH[0].w) * float3(0.5f, -0.5f, 1.0f) + float3(0.5f, 0.5f, 0.0f),
		(bboxPointH[1].xyz / bboxPointH[1].w) * float3(0.5f, -0.5f, 1.0f) + float3(0.5f, 0.5f, 0.0f),
		(bboxPointH[2].xyz / bboxPointH[2].w) * float3(0.5f, -0.5f, 1.0f) + float3(0.5f, 0.5f, 0.0f),
		(bboxPointH[3].xyz / bboxPointH[3].w) * float3(0.5f, -0.5f, 1.0f) + float3(0.5f, 0.5f, 0.0f),
		(bboxPointH[4].xyz / bboxPointH[4].w) * float3(0.5f, -0.5f, 1.0f) + float3(0.5f, 0.5f, 0.0f),
		(bboxPointH[5].xyz / bboxPointH[5].w) * float3(0.5f, -0.5f, 1.0f) + float3(0.5f, 0.5f, 0.0f),
		(bboxPointH[6].xyz / bboxPointH[6].w) * float3(0.5f, -0.5f, 1.0f) + float3(0.5f, 0.5f, 0.0f),
		(bboxPointH[7].xyz / bboxPointH[7].w) * float3(0.5f, -0.5f, 1.0f) + float3(0.5f, 0.5f, 0.0f)
	};
 
 
	float3 minXNdc = float3(maxDistance, 0, 0);
	float3 maxXNdc = float3(minDistance, 0, 0);
	float3 minYNdc = float3(0, maxDistance, 0);
	float3 maxYNdc = float3(0, minDistance, 0);
	float3 minZNdc = float3(0, 0, maxDistance);

	for (uint i = 0; i < 8; ++i)
	{
		if (bboxPointNdc[i].x < minXNdc.x)
			minXNdc = bboxPointNdc[i];
		if (bboxPointNdc[i].x > maxXNdc.x)
			maxXNdc = bboxPointNdc[i];
		if (bboxPointNdc[i].y < minYNdc.y)
			minYNdc = bboxPointNdc[i];
		if (bboxPointNdc[i].y > maxYNdc.y)
			maxYNdc = bboxPointNdc[i];
		if (bboxPointNdc[i].z < minZNdc.z)
			minZNdc = bboxPointNdc[i];
	}

	const float width = viewWidth * (maxXNdc.x - minXNdc.x);
	const float height = viewHeight * (maxYNdc.y - minYNdc.y);

	//int lodFactor = ceil(log2(max(width, height))) - ceil(log2(minOccSize)) + 1;
	int lodFactor = ceil(log2(max(width, height))) - occIndexOffset;
	if (lodFactor < 0)
		lodFactor = 0;

	const int lod = clamp((occMapCount - 1) - lodFactor, 0, occMapCount - 1);
	uint textureWidth = 0;
	uint textureHeight = 0;
	uint numberOfLevels = 0;
	depthMipmap.GetDimensions(lod, textureWidth, textureHeight, numberOfLevels);

	const float dx = (maxXNdc.x - minXNdc.x) / textureWidth;
	const float dy = (maxYNdc.y - minYNdc.y) / textureHeight;
	float finalCompareDepth = minDistance;

	for (uint i = 0; i < textureHeight; ++i)
	{
		for (uint j = 0; j < textureWidth; ++j)
		{
			const float compareDepth = ToNoLinearZValue(depthMipmap.SampleLevel(occFrameSam, float2(minXNdc.x + (dx * j), minYNdc.y + (dy * i)), lod).r);
			if (centerDepth <= compareDepth)
				return compareDepth;
		}
	}
	return 0;
}
#if defined (DIMX) && defined (DIMY) && defined (DIMZ)
[numthreads(DIMX, DIMY, DIMZ)]
void HZBOcclusion(int3 dispatchThreadID : SV_DispatchThreadID)
{
	//Thread dim X is objectList x
   //gorup dim X is objectList x

	//dispatchThreadID.x >= validQueryCount || 

	const int threadIndex = dispatchThreadID.x;
	const int queryIndex = object[threadIndex].queryResultIndex;

	if (validQueryCount <= threadIndex || !object[threadIndex].isValid)
		return;

	const float4x4 extentWM = float4x4(object[threadIndex].objWorld._m00_m01_m02_m03,
		object[threadIndex].objWorld._m10_m11_m12_m13,
		object[threadIndex].objWorld._m20_m21_m22_m23,
		0.0f, 0.0f, 0.0f, 1.0f);

	const float4 posCW = mul(float4(object[threadIndex].center, 1.0f), object[threadIndex].objWorld);
	const float4 posEW = mul(float4(object[threadIndex].extents, 1.0f), extentWM);

	if (CullBBox(posCW.xyz, posEW.xyz) > 0)
	{
		queryResult[queryIndex] = 1;
		return;
	}

	const float4 bboxPointV[8] =
	{
		mul(float4(posCW.xyz + float3(posEW.x, posEW.y, posEW.z), 1.0f), camView),
		mul(float4(posCW.xyz + float3(posEW.x, posEW.y, -posEW.z), 1.0f), camView),
		mul(float4(posCW.xyz + float3(posEW.x, -posEW.y, posEW.z), 1.0f), camView),
		mul(float4(posCW.xyz + float3(posEW.x, -posEW.y, -posEW.z), 1.0f), camView),
		mul(float4(posCW.xyz + float3(-posEW.x, posEW.y, posEW.z), 1.0f), camView),
		mul(float4(posCW.xyz + float3(-posEW.x, posEW.y, -posEW.z), 1.0f), camView),
		mul(float4(posCW.xyz + float3(-posEW.x, -posEW.y, posEW.z), 1.0f), camView),
		mul(float4(posCW.xyz + float3(-posEW.x, -posEW.y, -posEW.z), 1.0f), camView)
	};

	const float3 nearPoint[6] =
	{
		CalNearPoint(bboxPointV[0].xyz, bboxPointV[1].xyz, bboxPointV[2].xyz),
		CalNearPoint(bboxPointV[0].xyz, bboxPointV[1].xyz, bboxPointV[4].xyz),
		CalNearPoint(bboxPointV[0].xyz, bboxPointV[2].xyz, bboxPointV[4].xyz),
		CalNearPoint(bboxPointV[7].xyz, bboxPointV[5].xyz, bboxPointV[6].xyz),
		CalNearPoint(bboxPointV[7].xyz, bboxPointV[3].xyz, bboxPointV[5].xyz),
		CalNearPoint(bboxPointV[7].xyz, bboxPointV[3].xyz, bboxPointV[6].xyz)
	};

	float nowMinDist = maxDistance;
	uint minIndex = 0;
	for (uint i = 0; i < 6; ++i)
	{
		float dist = length(nearPoint[i]);
		if (dist < nowMinDist)
		{
			nowMinDist = dist;
			minIndex = i;
		}
	}

	const float4 clipNearH = mul(float4(nearPoint[minIndex], 1.0f), camProj);
	const float3 clipNearC = clipNearH.xyz / clipNearH.w;
	const float centerDepth = clipNearC.z;
	 
	float finalCompareDepth = VertexSampleDepth(posCW, posEW);
	if (centerDepth > finalCompareDepth && correctFailTrigger > 0)
		finalCompareDepth = RasterizeSampleDepth(centerDepth, posCW, posEW);

	if (centerDepth <= finalCompareDepth)
		queryResult[queryIndex] = 0;
	else
		queryResult[queryIndex] = 1;
 
	/*
	hzbDebugInfo[threadIndex].objWorld = object[threadIndex].objWorld;

	hzbDebugInfo[threadIndex].center = object[threadIndex].center;
	hzbDebugInfo[threadIndex].extents = object[threadIndex].extents;

	hzbDebugInfo[threadIndex].posCW = posCW;
	//hzbDebugInfo[threadIndex].posCV = posCV;
	hzbDebugInfo[threadIndex].posEW = posEW;
	//hzbDebugInfo[threadIndex].posEV = posEV;

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

	hzbDebugInfo[threadIndex].clipFrame0 = float2(maxXNdc.x, maxYNdc.y);
	hzbDebugInfo[threadIndex].clipFrame1 = float2(maxXNdc.x, minYNdc.y);
	hzbDebugInfo[threadIndex].clipFrame2 = float2(minXNdc.x, maxYNdc.y);
	hzbDebugInfo[threadIndex].clipFrame3 = float2(minXNdc.x, minYNdc.y);

	hzbDebugInfo[threadIndex].width = width;
	hzbDebugInfo[threadIndex].height = height;
	hzbDebugInfo[threadIndex].lod = lod;

	hzbDebugInfo[threadIndex].uvExtentsMax = float3(maxXNdc.x, maxYNdc.y, 0);
	hzbDebugInfo[threadIndex].uvExtentsMin = float3(minXNdc.x, minYNdc.y, minZNdc.z);

	hzbDebugInfo[threadIndex].centerDepth = centerDepth;
	hzbDebugInfo[threadIndex].finalDepth = finalCompareDepth;

	hzbDebugInfo[threadIndex].threadIndex = threadIndex;
	hzbDebugInfo[threadIndex].queryIndex = queryIndex;
	*/

}
#endif