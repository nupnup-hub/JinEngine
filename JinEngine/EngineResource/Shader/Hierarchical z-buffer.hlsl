
struct ObjectInfo
{
	float4x4 objWorld;
	float3 center;
	float radius;
	int queryResultIndex;
	int objectPad00;
	int objectPad01;
	int isValid;
};

struct HZBDebugInfo
{
	float4x4 objWorld;
	float4x4 camWorld;
	float4 posW;
	float4 camW;
	float4 posC;
	float4 posH;
	float3 posS;
	float radius;
	float4 additionalPosW;
	float2 texSize;
	float centerDepth;
	float copareDepth;
	int lod;
	int hzbDebugPad01;
	int hzbDebugPad02;
	int hzbDebugPad03;
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
	float4x4 camViewProj;
	float4x4 camWorld;
	int validQueryCount;
	float3 camPos;
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

#if defined (DIMX) && defined (DIMY) && defined (DIMZ)
[numthreads(DIMX, DIMY, DIMZ)]
void HZBOcclusion(int3 dispatchThreadID : SV_DispatchThreadID)
{
	//Thread dim X is objectList x
   //gorup dim X is objectList x

	if (dispatchThreadID.x >= validQueryCount || !object[dispatchThreadID.x].isValid)
		return;

	int lod = DOWN_SAMPLING_COUNT - (ceil(log2(object[dispatchThreadID.x].radius * 2)) + 1);
	if (lod < 0)
		lod = 0;

	float2 texSize = float2(0, 0);
	mipmap[lod].GetDimensions(texSize.x, texSize.y);

	float4 posW = mul(float4(object[dispatchThreadID.x].center, 1.0f), object[dispatchThreadID.x].objWorld);
	float4 camW = mul(float4(0.0f, 0.0f, 0.0f, 1.0f), camWorld);
	float4 posC = posW + normalize(camW - posW) * object[dispatchThreadID.x].radius;
	float4 posH = mul(posW, camViewProj);
	float3 posS = posH.xyz / posH.w;

	//const int2 centerIndex = int2(posS.x * texSize.x, posS.y * texSize.y);
	const float2 uv = float2(posS.x, posS.y);

	const float centerDepth = mipmap[lod].SampleLevel(occSam, uv, 0).r;
	const float compareDepth00 = mipmap[lod].SampleLevel(occSam, uv + float2(1 / texSize.x, 1 / texSize.y), 0).r;
	const float compareDepth01 = mipmap[lod].SampleLevel(occSam, uv + float2(-1 / texSize.x, -1 / texSize.y), 0).r;
	const float compareDepth02 = mipmap[lod].SampleLevel(occSam, uv + float2(1 / texSize.x, -1 / texSize.y), 0).r;
	const float compareDepth03 = mipmap[lod].SampleLevel(occSam, uv + float2(-1 / texSize.x, 1 / texSize.y), 0).r;

	const float finalCompareDepth = min(compareDepth00, min(compareDepth01, min(compareDepth02, compareDepth03)));
 
	//HZBDebugInfo info;
	hzbDebugInfo[dispatchThreadID.x].objWorld = object[dispatchThreadID.x].objWorld;
	hzbDebugInfo[dispatchThreadID.x].camWorld = camWorld;
	hzbDebugInfo[dispatchThreadID.x].posW = posW;
	hzbDebugInfo[dispatchThreadID.x].camW = camW;
	hzbDebugInfo[dispatchThreadID.x].posC = posC;
	hzbDebugInfo[dispatchThreadID.x].posH = posH;
	hzbDebugInfo[dispatchThreadID.x].posS = posS;
	hzbDebugInfo[dispatchThreadID.x].radius = object[dispatchThreadID.x].radius;
	hzbDebugInfo[dispatchThreadID.x].additionalPosW = normalize(camW - posW) * object[dispatchThreadID.x].radius;
	hzbDebugInfo[dispatchThreadID.x].texSize = texSize;
	hzbDebugInfo[dispatchThreadID.x].centerDepth = centerDepth;
	hzbDebugInfo[dispatchThreadID.x].copareDepth = finalCompareDepth;
	hzbDebugInfo[dispatchThreadID.x].lod = lod; 

	if (centerDepth <= finalCompareDepth)
		queryResult[dispatchThreadID.x] = 0;
	else
		queryResult[dispatchThreadID.x] = 1;
}
#endif
