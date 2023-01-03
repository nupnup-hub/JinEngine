#include"DepthFunc.hlsl"

struct ObjectInfo
{ 
	float3 coners[8];
	float3 center;
	float3 extents;
	int queryResultIndex;
	int isValid; 
};

//Debug
/*
struct HZBDebugInfo
{
	float4 planes[6];
	float3 center;
	float3 extents;
	int cullingRes;
	float centerDepth;
	float finalCompareDepth;

	int threadIndex;
	int queryIndex;
	int debugPad00;
};
RWStructuredBuffer<HZBDebugInfo> hzbDebugInfo : register(u2, space1);
*/

Texture2D<float> depthMap: register(t0);
Texture2D mipmap: register(t1);
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
	float4x4 camViewProj;
	float4 frustumPlane[6];
	float4 frustumDir;
	float3 frustumPos;

	float viewWidth;
	float viewHeight;
	float camNear;
	float camFar;
	int validQueryCount;
	int occMapCount; 
	int occIndexOffset;
	int correctFailTrigger; 
	int cbPassPad00;
};

static const float maxDistance = 100000;
static const float minDistance = -100000;
static const float correctFailDeltaRate = 0.04f;
static const uint correctFailLoopCount = 25;
//occMap size max is 512
//512 is less than thread and group max dim

#if defined (DIMX) && defined (DIMY)
[numthreads(DIMX, DIMY, 1)]
void HZBCopyDepthMap(int3 dispatchThreadID : SV_DispatchThreadID)
{
	if (nowWidth <= dispatchThreadID.x || nowHeight <= dispatchThreadID.y)
		return;
	 
	lastMipmap[int2(dispatchThreadID.x, dispatchThreadID.y)].r = ToLinearZValue(depthMap.Load(int3(dispatchThreadID.x, dispatchThreadID.y, 0)), camNear, camFar);
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

	const float color00 = mipmap.SampleLevel(downSam, uv, 0).r;
	const float color01 = mipmap.SampleLevel(downSam, uv + float2(xOffset, 0), 0).r;
	const float color02 = mipmap.SampleLevel(downSam, uv + float2(xOffset, yOffset), 0).r;
	const float color03 = mipmap.SampleLevel(downSam, uv + float2(0, yOffset), 0).r;

	float finalColor = max(color00, max(color01, max(color02, color03)));

	bool shouldIncludeExtraColumnFromPreviousLevel = ((nowWidth & 1) != 0);
	bool shouldIncludeExtraRowFromPreviousLevel = ((nowHeight & 1) != 0);

	if (shouldIncludeExtraColumnFromPreviousLevel)
	{
		const float extraColor00 = mipmap.SampleLevel(downSam, uv + float2(xOffset * 2, 0), 0).r;
		const float extraColor01 = mipmap.SampleLevel(downSam, uv + float2(xOffset * 2, yOffset), 0).r;

		// In the case where the width and height are both odd, need to include the
		// 'corner' value as well.
		if (shouldIncludeExtraRowFromPreviousLevel)
		{
			const float extraColor02 = mipmap.SampleLevel(downSam, uv + float2(xOffset * 2, yOffset * 2), 0).r;
			finalColor = max(finalColor, extraColor02);
		}
		finalColor = max(finalColor, max(extraColor00, extraColor01));
	}
	if (shouldIncludeExtraRowFromPreviousLevel)
	{
		const float extraColor00 = mipmap.SampleLevel(downSam, uv + float2(0, yOffset * 2), 0).r;
		const float extraColor01 = mipmap.SampleLevel(downSam, uv + float2(xOffset, yOffset * 2), 0).r;

		finalColor = max(finalColor, max(extraColor00, extraColor01));
	}
	lastMipmap[int2(baseIndex.x, baseIndex.y)].r = finalColor;

#elif DOWN_SAMPLEING_BY_LOAD

	const int3 baseIndex = int3(dispatchThreadID.x * 2, dispatchThreadID.y * 2, 0);
	const float color00 = mipmap.Load(baseIndex).r;
	const float color01 = mipmap.Load(baseIndex, int2(1, 0)).r;
	const float color02 = mipmap.Load(baseIndex, int2(1, 1)).r;
	const float color03 = mipmap.Load(baseIndex, int2(0, 1)).r;

	float finalColor = max(color00, max(color01, max(color02, color03)));

	bool shouldIncludeExtraColumnFromPreviousLevel = ((nowWidth & 1) != 0);
	bool shouldIncludeExtraRowFromPreviousLevel = ((nowHeight & 1) != 0);

	if (shouldIncludeExtraColumnFromPreviousLevel)
	{
		const float extraColor00 = mipmap.Load(baseIndex, int2(2, 0)).r;
		const float extraColor01 = mipmap.Load(baseIndex, int2(2, 1)).r;

		// In the case where the width and height are both odd, need to include the
		// 'corner' value as well.
		if (shouldIncludeExtraRowFromPreviousLevel)
		{
			const float extraColor02 = mipmap.Load(baseIndex, int2(2, 2)).r;
			finalColor = max(finalColor, extraColor02);
		}
		finalColor = max(finalColor, max(extraColor00, extraColor01));
	}
	if (shouldIncludeExtraRowFromPreviousLevel)
	{
		const float extraColor00 = mipmap.Load(baseIndex, int2(0, 2)).r;
		const float extraColor01 = mipmap.Load(baseIndex, int2(1, 2)).r;

		finalColor = max(finalColor, max(extraColor00, extraColor01));
	}
	lastMipmap[int2(dispatchThreadID.x, dispatchThreadID.y)].r = finalColor;
#endif 

}
#endif
  
int CullBBox(const float3 center, const float3 extents)
{
	int isOut = (int)(dot(float4(center, 1.0f), frustumPlane[0]) > dot(extents, abs(frustumPlane[0].xyz)));
	isOut += (int)(dot(float4(center, 1.0f), frustumPlane[1]) > dot(extents, abs(frustumPlane[1].xyz)));
	isOut += (int)(dot(float4(center, 1.0f), frustumPlane[2]) > dot(extents, abs(frustumPlane[2].xyz)));
	isOut += (int)(dot(float4(center, 1.0f), frustumPlane[3]) > dot(extents, abs(frustumPlane[3].xyz)));
	isOut += (int)(dot(float4(center, 1.0f), frustumPlane[4]) > dot(extents, abs(frustumPlane[4].xyz)));
	isOut += (int)(dot(float4(center, 1.0f), frustumPlane[5]) > dot(extents, abs(frustumPlane[5].xyz)));

	return isOut;
}

float3 CalNearPoint(const float3 p0, const float3 p1, const float3 p2, const float3 p3)
{
	const float3 camPos = float3(0, 0, 0);
	const float3 pNormal = normalize(cross(p1 - p0, p2 - p0));
	const float3 dist = dot(-pNormal, p0);
	const float dotCoord = dot(camPos, pNormal) + dist;
	float3 nearPoint = camPos - dotCoord * pNormal;

	const float2 xFactor = float2(min(min(p0.x, p1.x), min(p2.x, p3.x)), max(max(p0.x, p1.x), max(p2.x, p3.x)));
	const float2 yFactor = float2(min(min(p0.y, p1.y), min(p2.y, p3.y)), max(max(p0.y, p1.y), max(p2.y, p3.y)));
	const float2 zFactor = float2(min(min(p0.z, p1.z), min(p2.z, p3.z)), max(max(p0.z, p1.z), max(p2.z, p3.z)));

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
 
	const int threadIndex = dispatchThreadID.x;
	const int queryIndex = object[threadIndex].queryResultIndex;

	if (validQueryCount <= threadIndex || !object[threadIndex].isValid)
		return;
    
	if (CullBBox(object[threadIndex].center, object[threadIndex].extents) > 0)
	{
		queryResult[queryIndex] = 1;
		return;
	}
	 
	const float4 bboxPointV[8] =
	{
		mul(float4(object[threadIndex].coners[0], 1.0f), camView),
		mul(float4(object[threadIndex].coners[1], 1.0f), camView),
		mul(float4(object[threadIndex].coners[2], 1.0f), camView),
		mul(float4(object[threadIndex].coners[3], 1.0f), camView),
		mul(float4(object[threadIndex].coners[4], 1.0f), camView),
		mul(float4(object[threadIndex].coners[5], 1.0f), camView),
		mul(float4(object[threadIndex].coners[6], 1.0f), camView),
		mul(float4(object[threadIndex].coners[7], 1.0f), camView)
	};

	/*
	{ { { -1.0f, -1.0f,  1.0f, 0.0f } } },
	{ { {  1.0f, -1.0f,  1.0f, 0.0f } } },
	{ { {  1.0f,  1.0f,  1.0f, 0.0f } } },
	{ { { -1.0f,  1.0f,  1.0f, 0.0f } } },
	{ { { -1.0f, -1.0f, -1.0f, 0.0f } } },
	{ { {  1.0f, -1.0f, -1.0f, 0.0f } } },
	{ { {  1.0f,  1.0f, -1.0f, 0.0f } } },
	{ { { -1.0f,  1.0f, -1.0f, 0.0f } } },
	*/
	const float3 nearPoint[6] =
	{
		CalNearPoint(bboxPointV[2].xyz, bboxPointV[6].xyz, bboxPointV[1].xyz, bboxPointV[5].xyz),
		CalNearPoint(bboxPointV[2].xyz, bboxPointV[6].xyz, bboxPointV[3].xyz, bboxPointV[7].xyz),
		CalNearPoint(bboxPointV[2].xyz, bboxPointV[1].xyz, bboxPointV[3].xyz, bboxPointV[0].xyz),
		CalNearPoint(bboxPointV[4].xyz, bboxPointV[7].xyz, bboxPointV[0].xyz, bboxPointV[3].xyz),
		CalNearPoint(bboxPointV[4].xyz, bboxPointV[5].xyz, bboxPointV[7].xyz, bboxPointV[6].xyz),
		CalNearPoint(bboxPointV[4].xyz, bboxPointV[5].xyz, bboxPointV[0].xyz, bboxPointV[1].xyz)
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

	const float2 bboxPointNdc[8] =
	{
		(bboxPointH[0].xy / bboxPointH[0].w) * float2(0.5f, -0.5f) + float2(0.5f, 0.5f),
		(bboxPointH[1].xy / bboxPointH[1].w) * float2(0.5f, -0.5f) + float2(0.5f, 0.5f),
		(bboxPointH[2].xy / bboxPointH[2].w) * float2(0.5f, -0.5f) + float2(0.5f, 0.5f),
		(bboxPointH[3].xy / bboxPointH[3].w) * float2(0.5f, -0.5f) + float2(0.5f, 0.5f),
		(bboxPointH[4].xy / bboxPointH[4].w) * float2(0.5f, -0.5f) + float2(0.5f, 0.5f),
		(bboxPointH[5].xy / bboxPointH[5].w) * float2(0.5f, -0.5f) + float2(0.5f, 0.5f),
		(bboxPointH[6].xy / bboxPointH[6].w) * float2(0.5f, -0.5f) + float2(0.5f, 0.5f),
		(bboxPointH[7].xy / bboxPointH[7].w) * float2(0.5f, -0.5f) + float2(0.5f, 0.5f)
	};

	float minX = maxDistance;
	float maxX = minDistance;
	float minY = maxDistance;
	float maxY = minDistance;

	for (uint i = 0; i < 8; ++i)
	{
		if (bboxPointNdc[i].x < minX)
			minX = bboxPointNdc[i].x;
		if (bboxPointNdc[i].x > maxX)
			maxX = bboxPointNdc[i].x;
		if (bboxPointNdc[i].y < minY)
			minY = bboxPointNdc[i].y;
		if (bboxPointNdc[i].y > maxY)
			maxY = bboxPointNdc[i].y;
	}

	const float width = viewWidth * (maxX - minX);
	const float height = viewHeight * (maxY - minY);

	int lodFactor = ceil(log2(max(width, height))) - occIndexOffset;
	if (lodFactor < 0)
		lodFactor = 0;

	const int lod = clamp((occMapCount - 1) - lodFactor, 0, occMapCount - 1);

	const float compareDepth00 = ToNoLinearZValue(mipmap.SampleLevel(occFrameSam, bboxPointNdc[0], lod).r, camNear, camFar);
	const float compareDepth01 = ToNoLinearZValue(mipmap.SampleLevel(occFrameSam, bboxPointNdc[1], lod).r, camNear, camFar);
	const float compareDepth02 = ToNoLinearZValue(mipmap.SampleLevel(occFrameSam, bboxPointNdc[2], lod).r, camNear, camFar);
	const float compareDepth03 = ToNoLinearZValue(mipmap.SampleLevel(occFrameSam, bboxPointNdc[3], lod).r, camNear, camFar);
	const float compareDepth04 = ToNoLinearZValue(mipmap.SampleLevel(occFrameSam, bboxPointNdc[4], lod).r, camNear, camFar);
	const float compareDepth05 = ToNoLinearZValue(mipmap.SampleLevel(occFrameSam, bboxPointNdc[5], lod).r, camNear, camFar);
	const float compareDepth06 = ToNoLinearZValue(mipmap.SampleLevel(occFrameSam, bboxPointNdc[6], lod).r, camNear, camFar);
	const float compareDepth07 = ToNoLinearZValue(mipmap.SampleLevel(occFrameSam, bboxPointNdc[7], lod).r, camNear, camFar);

	float finalCompareDepth =  max(max(compareDepth00, max(compareDepth01, max(compareDepth02, compareDepth03))),
		max(compareDepth04, max(compareDepth05, max(compareDepth06, compareDepth07))));
	 
	if (centerDepth > finalCompareDepth && correctFailTrigger > 0)
	{
		//uint textureWidth = 0;
		//uint textureHeight = 0;
		//uint numberOfLevels = 0;
		//mipmap.GetDimensions(lod, textureWidth, textureHeight, numberOfLevels);

		const float xGap = maxX - minX;
		const float yGap = maxY - minY;

		const float dx = xGap * correctFailDeltaRate;
		const float dy = yGap * correctFailDeltaRate;
 
		for (uint i = 0; i < correctFailLoopCount; ++i)
		{
			for (uint j = 0; j < correctFailLoopCount; ++j)
			{
				const float compareDepth = ToNoLinearZValue(mipmap.SampleLevel(occFrameSam,
					float2(minX + (dx * j), minY + (dy * i)), lod).r, camNear, camFar);
				if (centerDepth <= compareDepth)
				{
					finalCompareDepth = compareDepth;
					i = correctFailLoopCount;
					j = correctFailLoopCount; 
				}
			}
		} 
	}

	if (centerDepth <= finalCompareDepth)
		queryResult[queryIndex] = 0;
	else
		queryResult[queryIndex] = 1;
   
	//hzbDebugInfo[threadIndex].centerDepth = centerDepth;
	//hzbDebugInfo[threadIndex].finalCompareDepth = finalCompareDepth;
	//Debug
	/*
	*
	hzbDebugInfo[threadIndex].centerDepth = -1;
	hzbDebugInfo[threadIndex].finalCompareDepth = -1;
	hzbDebugInfo[threadIndex].threadIndex = threadIndex;
	hzbDebugInfo[threadIndex].queryIndex = queryIndex;

	hzbDebugInfo[threadIndex].center = object[threadIndex].center;
	hzbDebugInfo[threadIndex].extents = object[threadIndex].extents;
	*/
}
#endif