#include"PixelCommon.hlsl"

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
 
#ifdef USE_DIRECTIONAL_LIGHT_PCM_16_SAMPLE
#define DIRECTIONAL_LIGHT_PCF_SAMPLE 16
#elif USE_DIRECTIONAL_LIGHT_PCM_32_SAMPLE
#define DIRECTIONAL_LIGHT_PCF_SAMPLE 32
#else
#define DIRECTIONAL_LIGHT_PCF_SAMPLE 0
#endif
 
struct PcssCsmPixelInfo
{
	float3 uv;
	float depth;
};
 
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

/*static const float2 poissonDisk64[64] =
{
	float2(0.0617981, 0.07294159),
	float2(0.6470215, 0.7474022),
	float2(-0.5987766, -0.7512833),
	float2(-0.693034, 0.6913887),
	float2(0.6987045, -0.6843052),
	float2(-0.9402866, 0.04474335),
	float2(0.8934509, 0.07369385),
	float2(0.1592735, -0.9686295),
	float2(-0.05664673, 0.995282),
	float2(-0.1203411, -0.1301079),
	float2(0.1741608, -0.1682285),
	float2(-0.09369049, 0.3196758),
	float2(0.185363, 0.3213367),
	float2(-0.1493771, -0.3147511),
	float2(0.4452095, 0.2580113),
	float2(-0.1080467, -0.5329178),
	float2(0.1604507, 0.5460774),
	float2(-0.4037193, -0.2611179),
	float2(0.5947998, -0.2146744),
	float2(0.3276062, 0.9244621),
	float2(-0.6518704, -0.2503952),
	float2(-0.3580975, 0.2806469),
	float2(0.8587891, 0.4838005),
	float2(-0.1596546, -0.8791054),
	float2(-0.3096867, 0.5588146),
	float2(-0.5128918, 0.1448544),
	float2(0.8581337, -0.424046),
	float2(0.1562584, -0.5610626),
	float2(-0.7647934, 0.2709858),
	float2(-0.3090832, 0.9020988),
	float2(0.3935608, 0.4609676),
	float2(0.3929337, -0.5010948),
	float2(-0.8682281, -0.1990303),
	float2(-0.01973724, 0.6478714),
	float2(-0.3897587, -0.4665619),
	float2(-0.7416366, -0.4377831),
	float2(-0.5523247, 0.4272514),
	float2(-0.5325066, 0.8410385),
	float2(0.3085465, -0.7842533),
	float2(0.8400612, -0.200119),
	float2(0.6632416, 0.3067062),
	float2(-0.4462856, -0.04265022),
	float2(0.06892014, 0.812484),
	float2(0.5149567, -0.7502338),
	float2(0.6464897, -0.4666451),
	float2(-0.159861, 0.1038342),
	float2(0.6455986, 0.04419327),
	float2(-0.7445076, 0.5035095),
	float2(0.9430245, 0.3139912),
	float2(0.0349884, -0.7968109),
	float2(-0.9517487, 0.2963554),
	float2(-0.7304786, -0.01006928),
	float2(-0.5862702, -0.5531025),
	float2(0.3029106, 0.09497032),
	float2(0.09025345, -0.3503742),
	float2(0.4356628, -0.0710125),
	float2(0.4112572, 0.7500054),
	float2(0.3401214, -0.3047142),
	float2(-0.2192158, -0.6911137),
	float2(-0.4676369, 0.6570358),
	float2(0.6295372, 0.5629555),
	float2(0.1253822, 0.9892166),
	float2(-0.1154335, 0.8248222),
	float2(-0.4230408, -0.7129914)
};
*/

float2 CalRotationTrig(const float2 uv)
{
	float random = shadowMaps[bluseNoiseTextureIndex].Sample(samPointClamp, uv * invBluseNoiseTextureSize * camInvRenderTargetSize).a;
	random = mad(random, 2.0, -1.0);
	
	float rotationAngle = random * PI;
	return float2(cos(rotationAngle), sin(rotationAngle));
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
float2 FindBlocker(const float2 uv, const float2 searchSize, const float2 rotationTrig, const float depth, const float bias, const int shadowMapIndex)
{
	float2 result = float2(0, 0);
	[unroll]
	for (int i = 0; i < PCSS_BLOCKER_SAMPLE; ++i)
	{
		float2 offset = Rotate(poissonDisk32[i], rotationTrig) * searchSize;
		float shadowMapDepth = shadowMaps[shadowMapIndex].SampleLevel(samPcssBloker, uv + offset, 0.0f).r;
		if (shadowMapDepth < depth + bias)
		{
			result.x += shadowMapDepth;
			result.y++;
		}
	}
	return result;
}
float PCFPoisson(const float2 uv, float2 filterRadiusUv, const float2 rotationTrig, const float depth, const float bias, int shadowMapIndex)
{
	float percentLit = 0.0f;
	[unroll]
	for (int i = 0; i < PCSS_PCF_SAMPLE; ++i)
	{
		float2 offset = Rotate(poissonDisk32[i], rotationTrig) * filterRadiusUv;
		percentLit += shadowMaps[shadowMapIndex].SampleCmpLevelZero(samCmpLinearPointShadow, uv + offset, depth + bias).r;
	}
	return percentLit /= PCSS_PCF_SAMPLE;
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
float2 CsmFindBlocker(PcssCsmPixelInfo curInfo, const float2 searchSize, const float2 rotationTrig, const float bias, const int shadowMapIndex)
{
	float2 result = float2(0, 0);
	for (int i = 0; i < PCSS_BLOCKER_SAMPLE; ++i)
	{
		float2 offset = Rotate(poissonDisk32[i], rotationTrig) * searchSize;
		float shadowMapDepth = shadowArray[shadowMapIndex].SampleLevel(samPcssBloker, curInfo.uv + float3(offset, 0.0f), 0.0f).r;
		if (shadowMapDepth < curInfo.depth + bias)
		{
			result.x += shadowMapDepth;
			++result.y;
		}
	}
	return result;
}
float CsmPCFSampling(const float2 uv, const float depth, const float delta, const int csmIndex, const int shadowMapIndex)
{
	float2 sampleSize = float2(delta, delta);
	float2 rotationTrig = CalRotationTrig(uv);
	float percentLit = 0.0f;
	[unroll]
	for (int i = 0; i < DIRECTIONAL_LIGHT_PCF_SAMPLE; ++i)
	{
#ifdef USE_DIRECTIONAL_LIGHT_PCM_16_SAMPLE
		float2 offset = Rotate(poissonDisk16[i], rotationTrig) * sampleSize;
#elif USE_DIRECTIONAL_LIGHT_PCM_32_SAMPLE
		float2 offset = Rotate(poissonDisk32[i], rotationTrig) * sampleSize;
#else
		float2 offset = float2(0, 0);
#endif
		percentLit += shadowArray[shadowMapIndex].SampleCmpLevelZero(samCmpLinearPointShadow, float3(uv + offset, csmIndex), depth).r;
	}
	return percentLit / DIRECTIONAL_LIGHT_PCF_SAMPLE;
}
float CsmPCSS(PcssCsmPixelInfo curInfo, const float shadowHz, const int lightIndex)
{
	float2 rotationTrig = CalRotationTrig(curInfo.uv.xy);
	int csmIndex = curInfo.uv.z;
	
	int shadowMapIndex = directionalLight[lightIndex].shadowMapIndex + csmLocalIndex;
	int csmDataIndex = directionalLight[lightIndex].csmDataIndex + csmLocalIndex;
	float penumbraScale = directionalLight[lightIndex].penumbraScale;
	float penumbraBlockerScale = directionalLight[lightIndex].penumbraBlockerScale;
	float tanAngle = directionalLight[lightIndex].tanAngle;
	float bias = directionalLight[lightIndex].bias;
	float frustumNear = csmData[csmDataIndex].frustumNear[csmIndex];
	float frustumFar = csmData[csmDataIndex].frustumFar[csmIndex];
	
	const float sampleSize = directionalLight[lightIndex].shadowMapInvSize;
	const float frustumRange = frustumFar - frustumNear;
	float searchSize = sampleSize * penumbraBlockerScale * tanAngle * (shadowHz / frustumRange);
	float2 blockResult = CsmFindBlocker(curInfo, float2(searchSize, searchSize), rotationTrig, bias, shadowMapIndex);

	float avgBlockerDepth = blockResult.x / blockResult.y;
	float numBlockers = blockResult.y;
	if (numBlockers == 0)
		return 1.0f;
 
	float avgBlokerZ = NdcToViewOZ(avgBlockerDepth, frustumNear, frustumFar);
	float penumbraRadius = ((shadowHz - avgBlokerZ) / avgBlokerZ);
	float2 filterRadiusUV = sampleSize * penumbraScale * tanAngle * float2(penumbraRadius, penumbraRadius);
	
	float percentLit = 0.0f;
	[unroll]
	for (int i = 0; i < PCSS_PCF_SAMPLE; ++i)
	{
		float2 offset = Rotate(poissonDisk32[i], rotationTrig) * filterRadiusUV;
		percentLit += shadowArray[shadowMapIndex].SampleCmpLevelZero(samCmpLinearPointShadow, curInfo.uv + float3(offset, 0.0f), curInfo.depth + bias).r;
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
float GetDirectionalSSM(float4 shadowPosH, const int lightIndex)
{
	shadowPosH.xyz /= shadowPosH.w;
	float depth = shadowPosH.z;
	return shadowMaps[directionalLight[lightIndex].shadowMapIndex].SampleCmpLevelZero(samCmpLinearPointShadow, shadowPosH.xy, depth).r;
}
float CalDirectionalSSMUsePCF(float4 shadowPosH, const int lightIndex)
{
	shadowPosH.xyz /= shadowPosH.w;
	float depth = shadowPosH.z;
 
	int shadowMapIndex = directionalLight[lightIndex].shadowMapIndex;
	float2 sampleSize = float2(directionalLight[lightIndex].shadowMapInvSize, directionalLight[lightIndex].shadowMapInvSize);
	float2 rotationTrig = CalRotationTrig(shadowPosH.xy);
	
	float percentLit = 0.0f;
	[unroll]
	for (int i = 0; i < DIRECTIONAL_LIGHT_PCF_SAMPLE; ++i)
	{
#ifdef USE_DIRECTIONAL_LIGHT_PCM_16_SAMPLE
		float2 offset = Rotate(poissonDisk16[i], rotationTrig) * sampleSize;
#elif USE_DIRECTIONAL_LIGHT_PCM_32_SAMPLE
		float2 offset = Rotate(poissonDisk32[i], rotationTrig) * sampleSize;
#else
		float2 offset = float2(0, 0);
#endif
		percentLit += shadowMaps[shadowMapIndex].SampleCmpLevelZero(samCmpLinearPointShadow, shadowPosH.xy + offset, depth).r;
	}
	return percentLit / DIRECTIONAL_LIGHT_PCF_SAMPLE;
}
float CalDirectionalSSMUsePCSS(const float4 shadowPosH, const float3 posW, const int lightIndex)
{
	float2 uv = shadowPosH.xy / shadowPosH.w;
	float depth = shadowPosH.z / shadowPosH.w;
	float2 rotationTrig = CalRotationTrig(uv);
	
	int shadowMapIndex = directionalLight[lightIndex].shadowMapIndex;
	float frustumNear = directionalLight[lightIndex].frustumNear;
	float frustumFar = directionalLight[lightIndex].frustumFar;
	float penumbraScale = directionalLight[lightIndex].penumbraScale;
	float penumbraBlockerScale = directionalLight[lightIndex].penumbraBlockerScale;
	float tanAngle = directionalLight[lightIndex].tanAngle;
	float bias = directionalLight[lightIndex].bias;

	const float sampleSize = directionalLight[lightIndex].shadowMapInvSize;
	const float frustumRange = frustumFar - frustumNear;
	float searchSize = sampleSize * penumbraBlockerScale * tanAngle * (shadowPosH.z / frustumRange);
	float2 blockResult = FindBlocker(uv, float2(searchSize, searchSize), rotationTrig, depth, bias, shadowMapIndex);

	float avgBlockerDepth = blockResult.x / blockResult.y;
	float numBlockers = blockResult.y;
	if (numBlockers == 0)
		return 1.0f;
 
	float avgBlokerZ = NdcToViewOZ(avgBlockerDepth, frustumNear, frustumFar);
	float penumbraRadius = ((shadowPosH.z - avgBlokerZ) / avgBlokerZ);
	float2 filterRadiusUV = sampleSize * penumbraScale * tanAngle * float2(penumbraRadius, penumbraRadius);
	
	float percentLit = 0.0f;
	[unroll]
	for (int i = 0; i < PCSS_PCF_SAMPLE; ++i)
	{
		float2 offset = Rotate(poissonDisk32[i], rotationTrig) * filterRadiusUV;
		percentLit += shadowMaps[shadowMapIndex].SampleCmpLevelZero(samCmpLinearPointShadow, uv + offset, depth + bias).r;
	}
	return percentLit /= PCSS_PCF_SAMPLE;
}
float GetDirectionalCsm(const float4 posV, const int lightIndex)
{
	int shadowMapIndex = directionalLight[lightIndex].shadowMapIndex + csmLocalIndex;
	int csmDataIndex = directionalLight[lightIndex].csmDataIndex + csmLocalIndex;
	
	float4 textureCoord = 0.0f;
	int csmIndex = CalCsmIndex(posV, csmDataIndex, textureCoord);
	float2 uv = textureCoord.xy / textureCoord.w;
	float depth = textureCoord.z / textureCoord.w;
	
	return shadowArray[shadowMapIndex].SampleCmpLevelZero(samCmpLinearPointShadow, float3(uv, csmIndex), depth).r;
}
float CalDirectionalCsmUsePCF(const float4 posV, const int lightIndex)
{
	int shadowMapIndex = directionalLight[lightIndex].shadowMapIndex + csmLocalIndex;
	int csmDataIndex = directionalLight[lightIndex].csmDataIndex + csmLocalIndex;
	
	float4 textureCoord = 0.0f;
	int csmIndex = CalCsmIndex(posV, csmDataIndex, textureCoord);

	float2 uv = textureCoord.xy / textureCoord.w;
	float depth = textureCoord.z / textureCoord.w;

	float percentLit = CsmPCFSampling(uv, depth, directionalLight[lightIndex].shadowMapInvSize, csmIndex, shadowMapIndex);
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
				float nPercentLit = CsmPCFSampling(uv, depth, directionalLight[lightIndex].shadowMapInvSize, nextCsmIndex, shadowMapIndex);
				percentLit = lerp(percentLit, nPercentLit, blendAmount);
			}
		}
	}
	return percentLit;
}
float CalDirectionalCsmUsePCSS(const float4 posV, const float shadowHz, const int lightIndex)
{
	int csmDataIndex = directionalLight[lightIndex].csmDataIndex + csmLocalIndex;
	int shadowMapIndex = directionalLight[lightIndex].shadowMapIndex + csmLocalIndex;

	float4 textureCoord = 0.0f;
	int csmIndex = CalCsmIndex(posV, csmDataIndex, textureCoord);

	PcssCsmPixelInfo curInfo;
	curInfo.uv = float3(textureCoord.xy / textureCoord.w, csmIndex);
	curInfo.depth = textureCoord.z / textureCoord.w;
	
	float percentLit = CsmPCSS(curInfo, shadowHz, lightIndex);
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
		
				PcssCsmPixelInfo nextInfo;
				nextInfo.uv = float3(textureCoord.xy / textureCoord.w, nextCsmIndex);
				nextInfo.depth = textureCoord.z / textureCoord.w;
			
				float nPercentLit = CsmPCSS(nextInfo, shadowHz, lightIndex);
				percentLit = lerp(percentLit, nPercentLit, blendAmount);
			}
		}
	}
	return percentLit;
}

#ifdef USE_POINT_LIGHT_PCM_16_SAMPLE
#define POINT_LIGHT_PCF_SAMPLE 16
#elif USE_POINT_LIGHT_PCM_32_SAMPLE
#define POINT_LIGHT_PCF_SAMPLE 32
#else
#define POINT_LIGHT_PCF_SAMPLE 0
#endif

void DecomposePointVector(const float3 litVec,
const float distance,
const float shadowMapInv,
out float3 nLitVec,
out float3 sideLitVec,
out float3 upLitVec)
{
	nLitVec = litVec / distance;
	sideLitVec = normalize(cross(nLitVec, float3(0, 0, 1)));
	upLitVec = cross(sideLitVec, nLitVec);
	 
	sideLitVec *= shadowMapInv;
	upLitVec *= shadowMapInv;
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
void CalDepthFactor(const float4 shadowCoord, const float shadowMapInv, out float compareDistance, out float shadowDepthBias)
{
	//test 수정필요
	float bias = 0.03f * 512 * shadowMapInv;
	// Calculate the Z buffer value that would have been stored for this position in the shadow map
	compareDistance = shadowCoord.z / shadowCoord.w;
	shadowDepthBias = -bias / shadowCoord.w;
}
float CalPointSSMUsePCSS(const float3 posW, const float3 normal, const int lightIndex)
{
	int shadowMapIndex = pointLight[lightIndex].shadowMapIndex;
	float shadowMapInv = pointLight[lightIndex].shadowMapInvSize;
	float3 litVec = posW - pointLight[lightIndex].position;
	float distance = length(litVec);
	
	float3 nLitVec;
	float3 sideLitVec;
	float3 upLitVec;
	DecomposePointVector(litVec, distance, shadowMapInv, nLitVec, sideLitVec, upLitVec);
	int cubeFaceIndex = FindCubeFace(litVec);
	
	float4 shadowCoord = mul(float4(posW, 1.0f), pointLight[lightIndex].shadowMapTransform[cubeFaceIndex]);
	float compareDistance;
	float shadowDepthBias;
	CalDepthFactor(shadowCoord, shadowMapInv, compareDistance, shadowDepthBias);
 
	float frustumNear = pointLight[lightIndex].frustumNear;
	float frustumFar = pointLight[lightIndex].frustumFar;
	float penumbraScale = pointLight[lightIndex].penumbraScale;
	float penumbraBlockerScale = pointLight[lightIndex].penumbraBlockerScale;
	
	const float frustumRange = frustumFar - frustumNear;
	float searchSize = penumbraBlockerScale * (shadowCoord.z / frustumRange);

	float2 blockResult = float2(0, 0);
	[unroll]
	for (int blockStep = 0; blockStep < PCSS_BLOCKER_SAMPLE; ++blockStep)
	{
		float2 offset = poissonDisk32[blockStep] * searchSize;
		float3 samplePos = (nLitVec + sideLitVec * offset.x + upLitVec * offset.y) * searchSize;
		float shadowMapDepth = shadowCubeMap[shadowMapIndex].SampleLevel(samPcssBloker, samplePos, 0).r;
		if (shadowMapDepth < compareDistance + shadowDepthBias)
		{
			blockResult.x += shadowMapDepth;
			blockResult.y++;
		}
	}
	
	float avgBlockerDepth = blockResult.x / blockResult.y;
	float numBlockers = blockResult.y;
	if (numBlockers == 0)
		return 1.0f;
 
	float avgBlokerZ = NdcToViewOZ(avgBlockerDepth, frustumNear, frustumFar);
	float penumbraRadius = ((shadowCoord.z - avgBlokerZ) / avgBlokerZ);
	float filterRadius = penumbraScale * penumbraRadius;
	
	float percentLit = 0.0f;
	[unroll]
	for (int filterStep = 0; filterStep < PCSS_PCF_SAMPLE; ++filterStep)
	{
		float2 offset = poissonDisk32[filterStep] * filterRadius;
		float3 samplePos = (nLitVec + sideLitVec * offset.x + upLitVec * offset.y);
		percentLit += shadowCubeMap[shadowMapIndex].SampleCmpLevelZero(samCmpLinearPointShadow, samplePos, compareDistance + shadowDepthBias).r;
		//* length(poissonDisk32[filterStep])
	}
	return percentLit /= PCSS_PCF_SAMPLE;
}
float CalPointSSMUsePCF(const float3 posW, const float3 normal, const int lightIndex)
{
	int shadowMapIndex = pointLight[lightIndex].shadowMapIndex;
	float shadowMapInv = pointLight[lightIndex].shadowMapInvSize;
	float3 litVec = posW - pointLight[lightIndex].position;
	float distance = length(litVec);
	
	if (distance * shadowMapInv > 1.0f)
		return 1.0f;

	float3 nLitVec;
	float3 sideLitVec;
	float3 upLitVec;
	DecomposePointVector(litVec, distance, shadowMapInv, nLitVec, sideLitVec, upLitVec);
	int cubeFaceIndex = FindCubeFace(litVec);
	
	float4 shadowCoord = mul(float4(posW, 1.0f), pointLight[lightIndex].shadowMapTransform[cubeFaceIndex]);
	float compareDistance;
	float shadowDepthBias;
	CalDepthFactor(shadowCoord, shadowMapInv, compareDistance, shadowDepthBias);
	
	float percentLit = 0;
	[unroll]
	for (int i = 0; i < POINT_LIGHT_PCF_SAMPLE; ++i)
	{
#ifdef USE_POINT_LIGHT_PCM_16_SAMPLE
		float3 samplePos = nLitVec + sideLitVec * poissonDisk16[i].x + upLitVec * poissonDisk16[i].y;
		float diskLength = length(poissonDisk16[i]);
#elif USE_POINT_LIGHT_PCM_32_SAMPLE
		float3 samplePos = nLitVec + sideLitVec * poissonDisk32[i].x + upLitVec * poissonDisk32[i].y;
		float diskLength = length(poissonDisk32[i]);
#else
		float3 samplePos = nLitVec + upLitVec;
		float diskLength = 1.0f;
#endif
		percentLit += shadowCubeMap[shadowMapIndex].SampleCmpLevelZero(
				samCmpLinearPointShadow,
				samplePos,
				compareDistance + shadowDepthBias * diskLength).r;
	}
	return percentLit / POINT_LIGHT_PCF_SAMPLE;
}
float CalPointSSM(const float3 posW, const int lightIndex)
{
	float3 lightToPos = posW - pointLight[lightIndex].position;
	
	float fNear = pointLight[lightIndex].frustumNear;
	float fFar = pointLight[lightIndex].frustumFar;
	
	float litDepth = NonLinearDepth((length(lightToPos) - fNear) / (fFar - fNear), fNear, fFar);
	float smDepth = shadowCubeMap[pointLight[lightIndex].shadowMapIndex].Sample(samCubeShadow, lightToPos).r;
		
	return smDepth + SHADOW_BIAS < litDepth ? 0.0f : 1.0f;
}
 

#ifdef USE_SPOT_LIGHT_PCM_16_SAMPLE
#define SPOT_LIGHT_PCF_SAMPLE 16
#elif USE_SPOT_LIGHT_PCM_32_SAMPLE
#define SPOT_LIGHT_PCF_SAMPLE 32
#else
#define SPOT_LIGHT_PCF_SAMPLE 0
#endif 
float CalSpotSSMUsePCSS(float4 shadowPosH, const int lightIndex)
{
	float2 uv = shadowPosH.xy / shadowPosH.w;
	float depth = shadowPosH.z / shadowPosH.w;
	float2 rotationTrig = CalRotationTrig(uv);
	
	int shadowMapIndex = spotLight[lightIndex].shadowMapIndex;
	float frustumNear = spotLight[lightIndex].frustumNear;
	float frustumFar = spotLight[lightIndex].frustumFar;
	float penumbraScale = spotLight[lightIndex].penumbraScale;
	float penumbraBlockerScale = spotLight[lightIndex].penumbraBlockerScale;

	const float sampleSize = spotLight[lightIndex].shadowMapInvSize;
	const float frustumRange = frustumFar - frustumNear;
	float searchSize = sampleSize * penumbraBlockerScale * (shadowPosH.z / frustumRange);
	float2 blockResult = FindBlocker(uv, float2(searchSize, searchSize), rotationTrig, depth, 0, shadowMapIndex);

	float avgBlockerDepth = blockResult.x / blockResult.y;
	float numBlockers = blockResult.y;
	if (numBlockers == 0)
		return 1.0f;
 
	float avgBlokerZ = NdcToViewOZ(avgBlockerDepth, frustumNear, frustumFar);
	float penumbraRadius = ((shadowPosH.z - avgBlokerZ) / avgBlokerZ);
	float2 filterRadiusUV = sampleSize * penumbraScale * float2(penumbraRadius, penumbraRadius);
	
	float percentLit = 0.0f;
	[unroll]
	for (int i = 0; i < PCSS_PCF_SAMPLE; ++i)
	{
		float2 offset = Rotate(poissonDisk32[i], rotationTrig) * filterRadiusUV;
		percentLit += shadowMaps[shadowMapIndex].SampleCmpLevelZero(samCmpLinearPointShadow, uv + offset, depth).r;
	}
	return percentLit /= PCSS_PCF_SAMPLE;
}
float CalSpotSSMUsePCF(float4 shadowPosH, const int lightIndex)
{
	const float delta = spotLight[lightIndex].shadowMapInvSize;
	const int shadowMapIndex = spotLight[lightIndex].shadowMapIndex;
	
	shadowPosH.xyz /= shadowPosH.w;
	float depth = shadowPosH.z;
 
	float2 sampleSize = float2(delta, delta);
	float2 rotationTrig = CalRotationTrig(shadowPosH.xy);
	
	float percentLit = 0.0f;
	[unroll]
	for (int i = 0; i < SPOT_LIGHT_PCF_SAMPLE; ++i)
	{
#ifdef USE_SPOT_LIGHT_PCM_16_SAMPLE
		float2 offset = Rotate(poissonDisk16[i], rotationTrig) * sampleSize;
#elif USE_SPOT_LIGHT_PCM_32_SAMPLE
		float2 offset = Rotate(poissonDisk32[i], rotationTrig) * sampleSize;
#else
		float2 offset = float2(0, 0);
#endif
		percentLit += shadowMaps[shadowMapIndex].SampleCmpLevelZero(samCmpLinearPointShadow, shadowPosH.xy + offset, depth).r;
	}
	return percentLit / SPOT_LIGHT_PCF_SAMPLE;
}
float GetSpotSSM(float4 shadowPosH, const int lightIndex)
{
	// Complete projection by doing division by w.
	shadowPosH.xyz /= shadowPosH.w;

	// Depth in NDC space.
	float depth = shadowPosH.z;
	return shadowMaps[spotLight[lightIndex].shadowMapIndex].SampleCmpLevelZero(samCmpLinearPointShadow, shadowPosH.xy, depth).r;
}

float4 CalculateLight(Material mat, float3 posW, float3 normal, float3 tangentW, float3 toEye)
{
	float3 directLight = float3(0, 0, 0);
#ifdef SHADOW
	for (int dLitIndex = directionalLitSt; dLitIndex < directionalLitEd; ++dLitIndex)
	{
		float shadowFactor = 1.0f;
		/**
		* DIRECTONAL_LIGHT_HAS_NORMAL_SHADOW_MAP) shadowPosH = posW * view * proj * textureSpace
		* DIRECTONAL_LIGHT_HAS_CSM) shadowPosH = posW * view	... csm index에 따라 CalDirectionalCsmShadowFactorEx 함수 내부에서
		* textureSpace 변환
		*/
		float4 shadowPosH = mul(float4(posW, 1.0f), directionalLight[dLitIndex].shadowMapTransform);
		if (directionalLight[dLitIndex].shadowMapType == DIRECTONAL_LIGHT_HAS_NORMAL_SHADOW_MAP)
		{
#ifdef USE_DIRECTIONAL_LIGHT_PCSS
			shadowFactor = CalDirectionalSSMUsePCSS(shadowPosH, posW, dLitIndex);
#elif USE_DIRECTIONAL_LIGHT_PCM
			shadowFactor = CalDirectionalSSMUsePCF(shadowPosH, dLitIndex);
#else
			shadowFactor = GetDirectionalSSM(shadowPosH, dLitIndex);
#endif 
			directLight += ComputeDirectionalLight(directionalLight[dLitIndex], mat, normal, tangentW, toEye) * shadowFactor; 
		}
		else if (directionalLight[dLitIndex].shadowMapType == DIRECTONAL_LIGHT_HAS_CSM)
		{				
			float4 posV = mul(float4(posW, 1.0f), directionalLight[dLitIndex].view);
#ifdef USE_DIRECTIONAL_LIGHT_PCSS
			shadowFactor = CalDirectionalCsmUsePCSS(posV, shadowPosH.z, dLitIndex);
#elif USE_DIRECTIONAL_LIGHT_PCM
			shadowFactor = CalDirectionalCsmUsePCF(posV, dLitIndex);
#else
			shadowFactor = GetDirectionalCsm(shadowPosH, dLitIndex);
#endif 
			directLight += ComputeDirectionalLight(directionalLight[dLitIndex], mat, normal, tangentW, toEye) * shadowFactor;	
		} 
		else
			directLight += ComputeDirectionalLight(directionalLight[dLitIndex], mat, normal, tangentW, toEye);
	}
	for (int pLitIndex = pointLitSt; pLitIndex < pointLitEd; ++pLitIndex)
	{
		if (pointLight[pLitIndex].hasShadowMap)
		{
			float shadowFactor = 1.0f;
#ifdef USE_POINT_LIGHT_PCSS
			shadowFactor = CalPointSSMUsePCSS(posW, normal, pLitIndex); 
#elif USE_POINT_LIGHT_PCM
			shadowFactor = CalPointSSMUsePCF(posW, normal, pLitIndex);
#else
			shadowFactor = CalPointSSM(posW, pLitIndex);
#endif 
			directLight += ComputePointLight(pointLight[pLitIndex], mat, posW, normal, tangentW, toEye) * shadowFactor;
		}
		else
			directLight += ComputePointLight(pointLight[pLitIndex], mat, posW, normal, tangentW, toEye);
	}
	for (int sLitIndex = spotLitSt; sLitIndex < spotLitEd; ++sLitIndex)
	{
		if (spotLight[sLitIndex].hasShadowMap)
		{
			float4 shadowPosH = mul(float4(posW, 1.0f), spotLight[sLitIndex].shadowMapTransform);
			float shadowFactor = 1.0f;
#ifdef USE_SPOT_LIGHT_PCSS
			shadowFactor = CalSpotSSMUsePCSS(shadowPosH, sLitIndex);
#elif USE_SPOT_LIGHT_PCM
			shadowFactor = CalSpotSSMUsePCF(shadowPosH, sLitIndex);
#else
			shadowFactor = GetSpotSSM(shadowPosH, sLitIndex);
#endif
			directLight += ComputeSpotLight(spotLight[sLitIndex], mat, posW, normal, tangentW, toEye) * shadowFactor;
		}
		else
			directLight += ComputeSpotLight(spotLight[sLitIndex], mat, posW, normal, tangentW, toEye);
	}
#else  
	for (int dLitIndex = directionalLitSt; dLitIndex < directionalLitEd; ++dLitIndex)
		directLight += ComputeDirectionalLight(directionalLight[dLitIndex], mat, normal, tangentW, toEye);
	for (int pLitIndex = pointLitSt; pLitIndex < pointLitEd; ++pLitIndex)
		directLight += ComputePointLight(pointLight[pLitIndex], mat, posW, normal, tangentW, toEye);
	for (int sLitIndex = spotLitSt; sLitIndex < spotLitEd; ++sLitIndex)
		directLight += ComputeSpotLight(spotLight[sLitIndex], mat, posW, normal, tangentW, toEye);
#endif
	return float4(directLight, 0.0f);
}
