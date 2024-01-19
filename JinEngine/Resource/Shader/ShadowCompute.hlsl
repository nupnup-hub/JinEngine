#include"LightCompute.hlsl"
/**
* PCCS referenced by NVIDIAGameWorks-GraphicsSamples-master-softshadow 
*/
//Erase VSM function  --2023-11-01-- 

/**
* Common
*/
 
#if !defined(SHADOW_SLOPE_SCALE)
#define SHADOW_SLOPE_SCALE 1.2f		//bias = DepthBias / (2^24)
#endif

#if !defined(SHADOW_BIAS)
#define SHADOW_BIAS  0.03f //0.0015f		//bias = DepthBias / (2^24)
#endif
 
#if !defined(USE_CSM_LEVEL_BLEND)
#define USE_CSM_LEVEL_BLEND 1
#endif

#if !defined(PCSS_SAMPLE_COUNT)
#define PCSS_SAMPLE_COUNT 1.0f
#endif

#if !defined(PCSS_BLOCKER_SAMPLE)
#define PCSS_BLOCKER_SAMPLE 32
#endif
 
#if !defined(PCSS_PCF_SAMPLE)
#define PCSS_PCF_SAMPLE 32
#endif
  
#ifdef USE_PCF_16_SAMPLE
#define LIGHT_PCF_SAMPLE 16
#elif USE_PCF_32_SAMPLE
#define LIGHT_PCF_SAMPLE 32
#else
#define LIGHT_PCF_SAMPLE 0
#endif

struct PCFData
{
	float bias;
	float sampleSize;
};
struct PCSSData
{
	float frustumNear;
	float frustumFar;
	float penumbraScale;
	float penumbraBlockerScale;
	float tanAngle;
	float bias;
	float sampleSize;
};
struct ShadowMapCubeData
{
	float3 nLitVec;
	float3 sideLitVec;
	float3 upLitVec;
	float compareDistance;
	float shadowDepthBias;
};
#ifdef USE_PCF_16_SAMPLE
static const float2 poissonDisk16[16] =
{
	float2(-0.94201624, -0.39906216),
	float2(0.94558609, -0.76890725),
	float2(-0.094184101, -0.92938870),
	float2(0.34495938, 0.29387760),
	float2(-0.91588581, 0.45771432),
	 float2(-0.81544232, -0.87912464),
	float2(-0.38277543, 0.27676845),
	 float2(0.97484398, 0.75648379),
	float2(0.44323325, -0.97511554),
	float2(0.53742981, -0.47373420),
	float2(-0.26496911, -0.41893023),
	float2(0.79197514, 0.19090188),
	 float2(-0.24188840, 0.99706507),
	float2(-0.81409955, 0.91437590),
	float2(0.19984126, 0.78641367),
	float2(0.14383161, -0.14100790)
};
#endif

static const float2 poissonDisk32[32] =
{
	float2(-0.975402, -0.0711386),
    float2(-0.920347, -0.41142),
    float2(-0.883908, 0.217872),
    float2(-0.884518, 0.568041),
    float2(-0.811945, 0.90521),
    float2(-0.792474, -0.779962),
    float2(-0.614856, 0.386578),
    float2(-0.580859, -0.208777),
    float2(-0.53795, 0.716666),
    float2(-0.515427, 0.0899991),
    float2(-0.454634, -0.707938),
    float2(-0.420942, 0.991272),
    float2(-0.261147, 0.588488),
    float2(-0.211219, 0.114841),
    float2(-0.146336, -0.259194),
    float2(-0.139439, -0.888668),
    float2(0.0116886, 0.326395),
    float2(0.0380566, 0.625477),
    float2(0.0625935, -0.50853),
    float2(0.125584, 0.0469069),
    float2(0.169469, -0.997253),
    float2(0.320597, 0.291055),
    float2(0.359172, -0.633717),
    float2(0.435713, -0.250832),
    float2(0.507797, -0.916562),
    float2(0.545763, 0.730216),
    float2(0.56859, 0.11655),
    float2(0.743156, -0.505173),
    float2(0.736442, -0.189734),
    float2(0.843562, 0.357036),
    float2(0.865413, 0.763726),
    float2(0.872005, -0.927)
};

float2 CalRotationTrig(const float2 uv)
{
	float random = textureMaps[bluseNoiseTextureIndex].Sample(samPointWrap, uv * invBluseNoiseTextureSize * camInvRenderTargetSize).a;
	return RandomRotationTrig(random);
}
float2 Rotate(float2 pos, float2 rotationTrig)
{
	return float2(pos.x * rotationTrig.x - pos.y * rotationTrig.y, pos.y * rotationTrig.x + pos.x * rotationTrig.y);
}

float2 DepthGradient(const float2 uv, const float z)
{
	float2 dz_duv = float2(0.0f, 0.0f);
	 
	float3 duvdist_dx = ddx(float3(uv, z));
	float3 duvdist_dy = ddy(float3(uv, z));
 	
	dz_duv.x = duvdist_dy.y * duvdist_dx.z;
	dz_duv.x -= duvdist_dx.y * duvdist_dy.z;

	dz_duv.y = duvdist_dx.x * duvdist_dy.z;
	dz_duv.y -= duvdist_dy.x * duvdist_dx.z;

	float det = (duvdist_dx.x * duvdist_dy.y) - (duvdist_dx.y * duvdist_dy.x);
	dz_duv /= det;

	return dz_duv;
}
float BiasedZ(float2 dz_duv, float2 offset, float z0)
{
	return z0 + dot(dz_duv, offset);
}
float CalBias(float2 uv, float z)
{
	return SHADOW_SLOPE_SCALE * max(abs(ddx(float3(uv, z))), abs(ddy(float3(uv, z)))) + SHADOW_BIAS * pow(exp2(1.0f) - 32.0f, 2);
}
void CalBlendAmount(float4 textureCoord, float blendRate, in out float currentPixelsBlendBandLocation, out float blendBetweenCascadesAmount)
{
	float2 distanceToOne = float2(1.0f - textureCoord.x, 1.0f - textureCoord.y);
	currentPixelsBlendBandLocation = min(textureCoord.x, textureCoord.y);
	currentPixelsBlendBandLocation = min(currentPixelsBlendBandLocation, min(distanceToOne.x, distanceToOne.y));
	blendBetweenCascadesAmount = 1.0f - (currentPixelsBlendBandLocation / blendRate);
}
float4 GetCsmColor(const int index)
{
	if (index == 0)
		return float4(1.0f, 0.0f, 0.0f, 1.0f);
	else if (index == 1)
		return float4(0.0f, 1.0f, 0.0f, 1.0f);
	else if (index == 2)
		return float4(0.0f, 0.0f, 1.0f, 1.0f);
	else if (index == 3)
		return float4(1.0f, 1.0f, 0.0f, 1.0f);
	else if (index == 4)
		return float4(1.0f, 0.0f, 1.0f, 1.0f);
	else if (index == 5)
		return float4(0.0f, 1.0f, 1.0f, 1.0f);
	else
		return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
int CalCsmIndex(float4 posV, int csmDataIndex, out float4 textureCoord)
{
	textureCoord = 0.0f;
	const int validCount = csmData[csmDataIndex].count;
	for (int i = 0; i < validCount; ++i)
	{
		textureCoord = posV * csmData[csmDataIndex].scale[i];
		textureCoord += csmData[csmDataIndex].posOffset[i];

		if (min(textureCoord.x, textureCoord.y) > csmData[csmDataIndex].mapMinBorder &&
			max(textureCoord.x, textureCoord.y) < csmData[csmDataIndex].mapMaxBorder)
		{
			return i;
		}
	}
	return validCount - 1;
}
float CsmPCFSampling(PCFData pcfData, Texture2DArray shadowM, const float2 uv, const float depth, const int csmIndex)
{
	float2 rotationTrig = CalRotationTrig(uv);
	
	float percentLit = 0.0f;
	float compareDepth = depth + pcfData.bias;
	[unroll]
	for (int i = 0; i < LIGHT_PCF_SAMPLE; ++i)
	{
#ifdef USE_PCF_16_SAMPLE
		float2 offset = Rotate(poissonDisk16[i], rotationTrig) * pcfData.sampleSize;
#elif USE_PCF_32_SAMPLE
		float2 offset = Rotate(poissonDisk32[i], rotationTrig) * pcfData.sampleSize;
#else
		float2 offset = float2(0, 0);
#endif
		percentLit += shadowM.SampleCmpLevelZero(samCmpLinearPointShadow, float3(uv + offset, csmIndex), compareDepth).r;
	}
	return percentLit / LIGHT_PCF_SAMPLE;
}
float CsmPCSS(PCSSData data, Texture2DArray shadowM, const float3 uv, const float depth, const float shadowHz)
{
	float2 rotationTrig = CalRotationTrig(uv.xy);

	const float frustumRange = data.frustumFar - data.frustumNear;
	float searchSize = data.sampleSize * data.penumbraBlockerScale * data.tanAngle * (shadowHz / frustumRange);
	
	float2 blockResult = float2(0, 0);
	float compareDepth = depth + data.bias;
	
	[unroll]
	for (int bStep = 0; bStep < PCSS_BLOCKER_SAMPLE; ++bStep)
	{
		float2 offset = Rotate(poissonDisk32[bStep], rotationTrig) * searchSize;
		float shadowMapDepth = shadowM.SampleLevel(samPcssBloker, uv + float3(offset, 0.0f), 0.0f).r;
		if (shadowMapDepth < compareDepth)
		{
			blockResult.x += shadowMapDepth;
			++blockResult.y;
		}
	}

	float avgBlockerDepth = blockResult.x / blockResult.y;
	float numBlockers = blockResult.y;
	if (numBlockers == 0)
		return 1.0f;
 
	float avgBlokerZ = NdcToViewOZ(avgBlockerDepth, data.frustumNear, data.frustumFar);
	float penumbraRadius = ((shadowHz - avgBlokerZ) / avgBlokerZ);
	float2 filterRadiusUV = data.sampleSize * data.penumbraScale * data.tanAngle * float2(penumbraRadius, penumbraRadius);
	
	float percentLit = 0.0f;
	[unroll]
	for (int pStep = 0; pStep < PCSS_PCF_SAMPLE; ++pStep)
	{
		float2 offset = Rotate(poissonDisk32[pStep], rotationTrig) * filterRadiusUV;
		percentLit += shadowM.SampleCmpLevelZero(samCmpLinearPointShadow, uv + float3(offset, 0.0f), compareDepth).r;
	}
	return percentLit / PCSS_PCF_SAMPLE;
}
float4 CalCascadeDebugFactor(const float4 posV, const int shadowMapIndex, const int csmDataIndex)
{
	float4 textureCoord = 0.0f;
	int csmIndex = CalCsmIndex(posV, csmDataIndex, textureCoord);

	float4 csmColor = GetCsmColor(csmIndex);
	if (USE_CSM_LEVEL_BLEND)
	{
		float blendLocation = 1.0f;
		float blendAmount = 1.0f;

		CalBlendAmount(textureCoord, csmData[csmDataIndex].levelBlendRate, blendLocation, blendAmount);
		if (blendLocation < csmData[csmDataIndex].levelBlendRate)
		{
			int nextCsmIndex = min(CSM_MAX_COUNT - 1, csmIndex + 1);
			if (csmData[csmDataIndex].count > nextCsmIndex)
			{
				textureCoord = posV * csmData[csmDataIndex].scale[nextCsmIndex];
				textureCoord += csmData[csmDataIndex].posOffset[nextCsmIndex];
			}
			csmColor = lerp(GetCsmColor(nextCsmIndex), csmColor, blendAmount);
		}
	}
	return csmColor;
}
float GetSSM(Texture2D shadowM, float4 shadowPosH, const float bias)
{
	shadowPosH.xyz /= shadowPosH.w;
	float depth = shadowPosH.z;
	return shadowM.SampleCmpLevelZero(samCmpLinearPointShadow, shadowPosH.xy, depth + bias).r;
}
float CalSSMUsePCF(PCFData pcfData, Texture2D shadowM, float4 shadowPosH)
{
	shadowPosH.xyz /= shadowPosH.w;
	float depth = shadowPosH.z;
   
	float2 rotationTrig = CalRotationTrig(shadowPosH.xy);
	
	float percentLit = 0.0f;
	float compareDepth = depth + pcfData.bias;
	[unroll]
	for (int i = 0; i < LIGHT_PCF_SAMPLE; ++i)
	{
#ifdef USE_PCF_16_SAMPLE
		float2 offset = Rotate(poissonDisk16[i], rotationTrig) * pcfData.sampleSize;
#elif USE_PCF_32_SAMPLE
		float2 offset = Rotate(poissonDisk32[i], rotationTrig) * pcfData.sampleSize;
#else
		float2 offset = float2(0, 0);
#endif
		percentLit += shadowM.SampleCmpLevelZero(samCmpLinearPointShadow, shadowPosH.xy + offset, compareDepth).r;
	}
	return percentLit / LIGHT_PCF_SAMPLE;
}
float CalSSMUsePCSS(PCSSData data, Texture2D shadowM, const float4 shadowPosH, const float3 posW)
{
	float2 uv = shadowPosH.xy / shadowPosH.w;
	float depth = shadowPosH.z / shadowPosH.w;
	float2 rotationTrig = CalRotationTrig(uv);

	const float frustumRange = data.frustumFar - data.frustumNear;
	float searchSize = data.sampleSize * data.penumbraBlockerScale * data.tanAngle * (shadowPosH.z / frustumRange);
	
	float2 blockResult = float2(0, 0);
	float compareDepth = depth + data.bias;
	
	[unroll]
	for (int bStep = 0; bStep < PCSS_BLOCKER_SAMPLE; ++bStep)
	{
		float2 offset = Rotate(poissonDisk32[bStep], rotationTrig) * searchSize;
		float shadowMapDepth = shadowM.SampleLevel(samPcssBloker, uv + offset, 0.0f).r;
		if (shadowMapDepth < compareDepth)
		{
			blockResult.x += shadowMapDepth;
			++blockResult.y;
		}
	}

	float avgBlockerDepth = blockResult.x / blockResult.y;
	float numBlockers = blockResult.y;
	if (numBlockers == 0)
		return 1.0f;
 
	float avgBlokerZ = NdcToViewOZ(avgBlockerDepth, data.frustumNear, data.frustumFar);
	float penumbraRadius = ((shadowPosH.z - avgBlokerZ) / avgBlokerZ);
	float2 filterRadiusUV = data.sampleSize * data.penumbraScale * data.tanAngle * float2(penumbraRadius, penumbraRadius);
	
	float percentLit = 0.0f;
	[unroll]
	for (int pStep = 0; pStep < PCSS_PCF_SAMPLE; ++pStep)
	{
		float2 offset = Rotate(poissonDisk32[pStep], rotationTrig) * filterRadiusUV;
		percentLit += shadowM.SampleCmpLevelZero(samCmpLinearPointShadow, uv + offset, compareDepth).r;
	}
	return percentLit / PCSS_PCF_SAMPLE;
}
float GetCsm(Texture2DArray shadowM, const float4 posV, const float bias, const int csmDataIndex)
{
	float4 textureCoord = 0.0f;
	int csmIndex = CalCsmIndex(posV, csmDataIndex, textureCoord);
	float2 uv = textureCoord.xy / textureCoord.w;
	float depth = textureCoord.z / textureCoord.w;
	
	return shadowM.SampleCmpLevelZero(samCmpLinearPointShadow, float3(uv, csmIndex), depth + bias).r;
}
float CalCsmUsePCF(PCFData pcfData, Texture2DArray shadowM, const float4 posV, const int csmDataIndex)
{
	float4 textureCoord = 0.0f;
	int csmIndex = CalCsmIndex(posV, csmDataIndex, textureCoord);

	float2 uv = textureCoord.xy / textureCoord.w;
	float depth = textureCoord.z / textureCoord.w;

	float percentLit = CsmPCFSampling(pcfData, shadowM, uv, depth, csmIndex);
	if (USE_CSM_LEVEL_BLEND)
	{
		float blendLocation = 1.0f;
		float blendAmount = 1.0f;

		CalBlendAmount(textureCoord, csmData[csmDataIndex].levelBlendRate, blendLocation, blendAmount);
		if (blendLocation < csmData[csmDataIndex].levelBlendRate)
		{
			int nextCsmIndex = min(CSM_MAX_COUNT - 1, csmIndex + 1);
			if (csmIndex != nextCsmIndex)
			{
				textureCoord = posV * csmData[csmDataIndex].scale[nextCsmIndex];
				textureCoord += csmData[csmDataIndex].posOffset[nextCsmIndex];
				uv = textureCoord.xy / textureCoord.w;
				depth = textureCoord.z / textureCoord.w;
				float nPercentLit = CsmPCFSampling(pcfData, shadowM, uv, depth, nextCsmIndex);
				percentLit = lerp(percentLit, nPercentLit, blendAmount);
			}
		}
	}
	return percentLit;
}
float CalCsmUsePCSS(PCSSData data, Texture2DArray shadowM, const float4 posV, const float shadowHz, const int csmDataIndex)
{
	float4 textureCoord = 0.0f;
	int csmIndex = CalCsmIndex(posV, csmDataIndex, textureCoord);
	 
	float3 uv = float3(textureCoord.xy / textureCoord.w, csmIndex);
	float depth = textureCoord.z / textureCoord.w;
	
	float percentLit = CsmPCSS(data, shadowM, uv, depth, shadowHz);
	if (USE_CSM_LEVEL_BLEND)
	{
		float blendLocation = 1.0f;
		float blendAmount = 1.0f;
		CalBlendAmount(textureCoord, csmData[csmDataIndex].levelBlendRate, blendLocation, blendAmount);
		if (blendLocation < csmData[csmDataIndex].levelBlendRate)
		{
			int nextCsmIndex = min(CSM_MAX_COUNT - 1, csmIndex + 1);
			if (csmIndex != nextCsmIndex)
			{
				textureCoord = posV * csmData[csmDataIndex].scale[nextCsmIndex];
				textureCoord += csmData[csmDataIndex].posOffset[nextCsmIndex];
		 
				uv = float3(textureCoord.xy / textureCoord.w, nextCsmIndex);
				depth = textureCoord.z / textureCoord.w;
			
				float nPercentLit = CsmPCSS(data, shadowM, uv, depth, shadowHz);
				percentLit = lerp(percentLit, nPercentLit, blendAmount);
			}
		}
	}
	return percentLit;
}

int FindCubeFace(const float3 litVec)
{
	float3 absLitvec = abs(litVec);
	float maxCoord = max(absLitvec.x, max(absLitvec.y, absLitvec.z));

	//가장큰 factor에 면에서 그림자 추출
	int cubeFaceIndex = 0;
	if (maxCoord == absLitvec.x)
	{
		cubeFaceIndex = absLitvec.x == litVec.x ? 0 : 1;
	}
	else if (maxCoord == absLitvec.y)
	{
		cubeFaceIndex = absLitvec.y == litVec.y ? 2 : 3;
	}
	else
	{
		cubeFaceIndex = absLitvec.z == litVec.z ? 4 : 5;
	}
	return cubeFaceIndex;
}
float CalCubeSSMUsePCSS(PCSSData data, ShadowMapCubeData smData, TextureCube shadowM, float4 shadowCoord)
{
	smData.shadowDepthBias += data.bias;
	const float frustumRange = data.frustumFar - data.frustumNear;
	float searchSize = data.penumbraBlockerScale * (shadowCoord.z / frustumRange);

	float2 blockResult = float2(0, 0);
	[unroll]
	for (int blockStep = 0; blockStep < PCSS_BLOCKER_SAMPLE; ++blockStep)
	{
		float2 offset = poissonDisk32[blockStep] * searchSize;
		float3 samplePos = (smData.nLitVec + smData.sideLitVec * offset.x + smData.upLitVec * offset.y) * searchSize;
		float shadowMapDepth = shadowM.SampleLevel(samPcssBloker, samplePos, 0).r;
		float diskLength = length(poissonDisk32[blockStep]);
		if (shadowMapDepth < smData.compareDistance + smData.shadowDepthBias * diskLength)
		{
			blockResult.x += shadowMapDepth;
			blockResult.y++;
		}
	}
	
	float avgBlockerDepth = blockResult.x / blockResult.y;
	float numBlockers = blockResult.y;
	if (numBlockers == 0)
		return 1.0f;
 
	float avgBlokerZ = NdcToViewOZ(avgBlockerDepth, data.frustumNear, data.frustumFar);
	float penumbraRadius = ((shadowCoord.z - avgBlokerZ) / avgBlokerZ);
	float filterRadius = data.penumbraScale * penumbraRadius;
	
	float percentLit = 0.0f;
	[unroll]
	for (int filterStep = 0; filterStep < PCSS_PCF_SAMPLE; ++filterStep)
	{
		float2 offset = poissonDisk32[filterStep] * filterRadius;
		float3 samplePos = (smData.nLitVec + smData.sideLitVec * offset.x + smData.upLitVec * offset.y);
		float diskLength = length(poissonDisk32[filterStep]);
		percentLit += shadowM.SampleCmpLevelZero(samCmpLinearPointShadow, samplePos, smData.compareDistance + smData.shadowDepthBias * diskLength).r;
		//* length(poissonDisk32[filterStep])
	}
	return percentLit /= PCSS_PCF_SAMPLE;
}
float CalCubeSSMUsePCF(PCFData data, ShadowMapCubeData smData, TextureCube shadowM, float distance)
{
	if (distance * data.sampleSize > 1.0f)
		return 1.0f;
	
	smData.shadowDepthBias += data.bias;
	float percentLit = 0;
	[unroll]
	for (int i = 0; i < LIGHT_PCF_SAMPLE; ++i)
	{
#ifdef USE_PCF_16_SAMPLE
		float3 samplePos = smData.nLitVec + smData.sideLitVec * poissonDisk16[i].x + smData.upLitVec * poissonDisk16[i].y;
		float diskLength = length(poissonDisk16[i]);
#elif USE_PCF_32_SAMPLE
		float3 samplePos = smData.nLitVec + smData.sideLitVec * poissonDisk32[i].x + smData.upLitVec * poissonDisk32[i].y;
		float diskLength = length(poissonDisk32[i]);
#else
		float3 samplePos = smData.nLitVec + smData.sideLitVec + smData.upLitVec;
		float diskLength = 1.0f;
#endif
		percentLit += shadowM.SampleCmpLevelZero(
				samCmpLinearPointShadow,
				samplePos,
				smData.compareDistance + smData.shadowDepthBias * diskLength).r;
	}
	return percentLit / LIGHT_PCF_SAMPLE;
}
float GetCubeSSM(ShadowMapCubeData smData, TextureCube shadowM)
{
	float3 samplePos = smData.nLitVec + smData.upLitVec;
	float compareDistance = smData.compareDistance + smData.shadowDepthBias;
	return shadowM.SampleCmpLevelZero(samCmpLinearPointShadow, samplePos, compareDistance).r;
}

float3 ComputeDirectionalLight(Material mat, float3 posW, float3 normalW, float3 tangentW, float3 toEye, int index)
{
	DirectionalLight dLitCalData;
	dLitCalData.direction = directionalLight[index].direction;
	dLitCalData.color = directionalLight[index].color;
	dLitCalData.power = directionalLight[index].power;
	
	float shadowFactor = 1.0f;
#ifdef SHADOW 
	if (directionalLight[index].shadowMapType == DIRECTONAL_LIGHT_HAS_NORMAL_SHADOW_MAP)
	{
		float4 shadowPosH = mul(float4(posW, 1.0f), directionalLight[index].shadowMapTransform);
#ifdef USE_DIRECTIONAL_LIGHT_PCSS	
		PCSSData pcssData;
		pcssData.frustumNear = directionalLight[index].frustumNear;
		pcssData.frustumFar = directionalLight[index].frustumFar;
		pcssData.penumbraScale = directionalLight[index].penumbraScale;
		pcssData.penumbraBlockerScale = directionalLight[index].penumbraBlockerScale;
		pcssData.tanAngle = directionalLight[index].tanAngle;
		pcssData.bias = directionalLight[index].bias;
		pcssData.sampleSize = directionalLight[index].shadowMapInvSize; 
	
		shadowFactor = CalSSMUsePCSS(pcssData, shadowMaps[directionalLight[index].shadowMapIndex], shadowPosH, posW);
#elif USE_DIRECTIONAL_LIGHT_PCF
		PCFData pcfData;
		pcfData.bias = directionalLight[index].bias;
		pcfData.sampleSize = directionalLight[index].shadowMapInvSize;
	
		shadowFactor = CalSSMUsePCF(pcfData, shadowMaps[directionalLight[index].shadowMapIndex], shadowPosH);
#else
		shadowFactor = GetSSM(shadowMaps[directionalLight[index].shadowMapIndex], shadowPosH, directionalLight[index].bias);
#endif 
	}
	else if (directionalLight[index].shadowMapType == DIRECTONAL_LIGHT_HAS_CSM)
	{
		float4 posV = mul(float4(posW, 1.0f), directionalLight[index].view);
		int csmDataIndex = directionalLight[index].csmDataIndex + csmLocalIndex;
		int shadowMapIndex = directionalLight[index].shadowMapIndex + csmLocalIndex;
#ifdef USE_DIRECTIONAL_LIGHT_PCSS
		PCSSData pcssData;
		pcssData.frustumNear = directionalLight[index].frustumNear;
		pcssData.frustumFar = directionalLight[index].frustumFar;
		pcssData.penumbraScale = directionalLight[index].penumbraScale;
		pcssData.penumbraBlockerScale = directionalLight[index].penumbraBlockerScale;
		pcssData.tanAngle = directionalLight[index].tanAngle;
		pcssData.bias = directionalLight[index].bias;
		pcssData.sampleSize = directionalLight[index].shadowMapInvSize;
	 
		float4 shadowPosH = mul(float4(posW, 1.0f), directionalLight[index].shadowMapTransform);
		shadowFactor = CalCsmUsePCSS(pcssData, shadowArray[shadowMapIndex], posV, shadowPosH.z, csmDataIndex);
#elif USE_DIRECTIONAL_LIGHT_PCF
		PCFData pcfData;
		pcfData.bias = directionalLight[index].bias;
		pcfData.sampleSize = directionalLight[index].shadowMapInvSize;
		shadowFactor = CalCsmUsePCF(pcfData, shadowArray[shadowMapIndex], posV, csmDataIndex);
#else
		shadowFactor = GetCsm(shadowArray[shadowMapIndex], posV, directionalLight[index].bias, csmDataIndex);
#endif  
	} 
#endif 
	return ComputeDirectionalLight(dLitCalData, mat, normalW, tangentW, toEye) * shadowFactor;
}
float3 ComputePointLight(Material mat, float3 posW, float3 normalW, float3 tangentW, float3 toEye, int index)
{
	PointLight pLitCalData;
	pLitCalData.midPosition = pointLight[index].midPosition;
	pLitCalData.sidePosition[0] = pointLight[index].sidePosition[0];
	pLitCalData.sidePosition[1] = pointLight[index].sidePosition[1];
	pLitCalData.color = pointLight[index].color;
	pLitCalData.power = pointLight[index].power;
	pLitCalData.range = pointLight[index].frustumFar;
		
	float shadowFactor = 1.0f;
#ifdef SHADOW
	float3 litVec =  posW - pointLight[index].midPosition; 
	if (pointLight[index].hasShadowMap)
	{		
		int shadowMapIndex = pointLight[index].shadowMapIndex; 
		float distance = length(litVec);
						
	 	ShadowMapCubeData smData;
		smData.nLitVec = litVec / distance;
		smData.sideLitVec = normalize(cross(smData.nLitVec, float3(0, 0, 1)));
		smData.upLitVec = cross(smData.sideLitVec, smData.nLitVec);
		smData.sideLitVec *= pointLight[index].shadowMapInvSize;
		smData.upLitVec *= pointLight[index].shadowMapInvSize;
			
		float4 shadowCoord = mul(float4(posW, 1.0f), pointLight[index].shadowMapTransform[FindCubeFace(litVec)]);
		//test 수정필요
		float bias = 0.03f * 512 * pointLight[index].shadowMapInvSize; 
		smData.compareDistance = shadowCoord.z / shadowCoord.w;
		smData.shadowDepthBias = -bias / shadowCoord.w;
	
#ifdef USE_POINT_LIGHT_PCSS
		PCSSData pcssData;
		pcssData.frustumNear = pointLight[index].frustumNear;
		pcssData.frustumFar = pointLight[index].frustumFar;
		pcssData.penumbraScale = pointLight[index].penumbraScale;
		pcssData.penumbraBlockerScale = pointLight[index].penumbraBlockerScale;
		pcssData.bias = pointLight[index].bias;
		pcssData.sampleSize = pointLight[index].shadowMapInvSize;
		 
		shadowFactor = CalCubeSSMUsePCSS(pcssData, smData, shadowCubeMap[shadowMapIndex], shadowCoord);
#elif USE_POINT_LIGHT_PCF
		PCFData pcfData;
		pcfData.bias = pointLight[index].bias;
		pcfData.sampleSize = pointLight[index].shadowMapInvSize;		
		shadowFactor =  CalCubeSSMUsePCF(pcfData, smData, shadowCubeMap[shadowMapIndex], distance);
#else
		shadowFactor = GetCubeSSM(smData, shadowCubeMap[shadowMapIndex]);
#endif  
	}
#endif 
	return ComputePointLight(pLitCalData, mat, posW, normalW, tangentW, toEye) * shadowFactor;
}
float3 ComputeSpotLight(Material mat, float3 posW, float3 normalW, float3 tangentW, float3 toEye, int index)
{
	SpotLight sLitCalData;
	sLitCalData.position = spotLight[index].position;
	sLitCalData.direction = spotLight[index].direction;
	sLitCalData.color = spotLight[index].color;
	sLitCalData.power = spotLight[index].power;
	sLitCalData.range = spotLight[index].frustumFar;
	sLitCalData.innerConeCosAngle = spotLight[index].innerConeCosAngle;
	sLitCalData.outerConeCosAngle = spotLight[index].outerConeCosAngle;
	
	float shadowFactor = 1.0f;
#ifdef SHADOW
	if (spotLight[index].hasShadowMap)
	{
		float4 shadowPosH = mul(float4(posW, 1.0f), spotLight[index].shadowMapTransform);
#ifdef USE_SPOT_LIGHT_PCSS
		PCSSData pcssData;
		pcssData.frustumNear = spotLight[index].frustumNear;
		pcssData.frustumFar = spotLight[index].frustumFar;
		pcssData.penumbraScale = spotLight[index].penumbraScale;
		pcssData.penumbraBlockerScale = spotLight[index].penumbraBlockerScale;
		pcssData.bias = spotLight[index].bias;
		pcssData.sampleSize = spotLight[index].shadowMapInvSize; 
	
		shadowFactor = CalSSMUsePCSS(csmData, shadowMaps[spotLight[index].shadowMapIndex], shadowPosH, posW);
#elif USE_SPOT_LIGHT_PCF
		PCFData pcfData;
		pcfData.bias = spotLight[index].bias;
		pcfData.sampleSize = spotLight[index].shadowMapInvSize;
	
		shadowFactor = CalSSMUsePCF(pcfData, shadowMaps[spotLight[index].shadowMapIndex], shadowPosH);
#else
		shadowFactor = GetSSM(shadowMaps[spotLight[index].shadowMapIndex], shadowPosH, spotLight[index].bias);
#endif
	}
#endif
	return ComputeSpotLight(sLitCalData, mat, posW, normalW, tangentW, toEye) * shadowFactor;
}
float3 ComputeRectLight(Material mat, float3 posW, float3 normalW, float3 tangentW, float3 toEye, int index)
{
	RectLight rect = ComputeRect(rectLight[index], posW);
	//미구현
	//if (rectLight[index].hasShadowMap)	
	//	;
	return ComputeRectLight(rect, mat, posW, normalW, tangentW, toEye);
}
 
#ifdef LIGHT_CLUSTER
float3 ComputeLight(Material mat, float3 posW, float3 normalW, float3 tangentW, float3 toEye, int2 screenPos, float nonLinearDepth)
{  
	float3 lightColor = float3(0, 0, 0);
	for (uint dLitIndex = directionalLitSt; dLitIndex < directionalLitEd; ++dLitIndex)
		lightColor += ComputeDirectionalLight(mat, posW, normalW, tangentW, toEye, dLitIndex);
  
	const uint rangeX = camRenderTargetSize.x / CLUSTER_DIM_X;
	const uint rangeY = camRenderTargetSize.y / CLUSTER_DIM_Y; 
	const float minDepth = clusterMinDepth; //log2(NEAR_CLUST);
	const float maxDepth = log2(camFarZ);

	const float scale = 1.0f / (maxDepth - minDepth) * (CLUSTER_DIM_Z - 1.0f);
	const float bias = 1.0f - minDepth * scale;  
	const float depth = NdcToViewPZ(nonLinearDepth, camNearZ, camFarZ);
	if (depth == camFarZ)
		return lightColor; 
	   
	int dep = int(max(log2(depth) * scale + bias, 0.0f)); 
	int2 clusterPos = int2(screenPos.x / rangeX, screenPos.y / rangeY);
	uint lightIndex = (startOffsetBuffer.Load((clusterPos.x + CLUSTER_DIM_X * clusterPos.y + CLUSTER_DIM_X * CLUSTER_DIM_Y * dep) * LINKED_LIST_INDEX_PER_BYTE) & CLUSTER_LIGHT_INVALID_ID);
  
	LinkedLightID linkedLight;
	if (lightIndex != CLUSTER_LIGHT_INVALID_ID)
	{
		linkedLight = linkedLightList[lightIndex];
		//Rect light
		while ((linkedLight.lightID >> CLUSTER_LIGHT_ID_PER_BIT) == 2)
		{ 
			uint lightID = (linkedLight.lightID & CLUSTER_LIGHT_ID_RANGE);
			lightColor += ComputeRectLight(mat, posW, normalW, tangentW, toEye, lightID); 
			lightIndex = linkedLight.link;				
			if (lightIndex == CLUSTER_LIGHT_INVALID_ID)
				break;
			linkedLight = linkedLightList[lightIndex];
		}

		//Spot light
		while ((linkedLight.lightID >> CLUSTER_LIGHT_ID_PER_BIT) == 1)
		{	 
			uint lightID = (linkedLight.lightID & CLUSTER_LIGHT_ID_RANGE);
			lightColor += ComputeSpotLight(mat, posW, normalW, tangentW, toEye, lightID); 
			lightIndex = linkedLight.link;					
			if (lightIndex == CLUSTER_LIGHT_INVALID_ID)
				break;
			linkedLight = linkedLightList[lightIndex];
		}
			 
		//Point light
		while ((linkedLight.lightID >> CLUSTER_LIGHT_ID_PER_BIT) == 0)
		{ 
			uint lightID = (linkedLight.lightID & CLUSTER_LIGHT_ID_RANGE);
			lightColor += ComputePointLight(mat, posW, normalW, tangentW, toEye, lightID);
			lightIndex = linkedLight.link;
			if (lightIndex == CLUSTER_LIGHT_INVALID_ID)
				break;
			linkedLight = linkedLightList[lightIndex];
		}
	}
	return lightColor; 
}
#else
#endif
float3 ComputeLight(Material mat, float3 posW, float3 normalW, float3 tangentW, float3 toEye)
{ 
	float3 lightColor = float3(0, 0, 0);
	for (uint dLitIndex = directionalLitSt; dLitIndex < directionalLitEd; ++dLitIndex)
		lightColor += ComputeDirectionalLight(mat, posW, normalW, tangentW, toEye, dLitIndex);
	for (uint pLitIndex = pointLitSt; pLitIndex < pointLitEd; ++pLitIndex) 
		lightColor += ComputePointLight(mat, posW, normalW, tangentW, toEye, pLitIndex);
	for (uint sLitIndex = spotLitSt; sLitIndex < spotLitEd; ++sLitIndex)
		lightColor += ComputeSpotLight(mat, posW, normalW, tangentW, toEye, sLitIndex);
	for (uint rLitIndex = rectLitSt; rLitIndex < rectLitEd; ++rLitIndex)
		lightColor += ComputeRectLight(mat, posW, normalW, tangentW, toEye, rLitIndex);
	return lightColor;
}
 
 
