#include"Common.hlsl"
#include "Math.hlsl"

/**
* Common
*/

#if !defined(SHADOW_BIAS)
#define SHADOW_BIAS 0.0015f		//bias = DepthBias / (2^24)
#endif

#if !defined(PCF_SAMPLE_COUNT)
#define PCF_SAMPLE_COUNT 2.5f		
#endif

#if !defined(PCF_STEP)
#define PCF_STEP 1.0f		
#endif
 
#if !defined(USE_CSM_LEVEL_BLEND)
#define USE_CSM_LEVEL_BLEND 1
#endif

#if !defined(PCSS_SOFT_FACTOR)
#define PCSS_SOFT_FACTOR 1.0f
#endif

#if !defined(PCSS_SAMPLE_COUNT)
#define PCSS_SAMPLE_COUNT 4.0f
#endif

#if !defined(PCSS_BLOCKER_SAMPLE)
#define PCSS_BLOCKER_SAMPLE 16
#endif
 
#if !defined(PCSS_PCF_SAMPLE)
#define PCSS_PCF_SAMPLE PCSS_BLOCKER_SAMPLE
#endif

#if !defined(PCSS_BLOCKER_STEP)
#define PCSS_BLOCKER_STEP 2
#endif
 
#if !defined(PCSS_PCF_STEP)
#define PCSS_PCF_STEP PCSS_BLOCKER_STEP
#endif

#define PCF_STEP_COUNT ((PCF_STEP * 2 + 1) * (PCF_STEP * 2 + 1))
#define PCSS_BLOCKER_STEP_COUNT ((PCSS_BLOCKER_STEP * 2 + 1) * (PCSS_BLOCKER_STEP * 2 + 1))
#define PCSS_PCF_STEP_COUNT PCSS_BLOCKER_STEP_COUNT

/**
* PCCS referenced by NVIDIAGameWorks-GraphicsSamples-master-softshadow.sln
*/
struct PcssSsmPixelInfo
{
	float2 uv;
	float2 dz;
	float depth;
};
struct PcssCsmPixelInfo
{
	float3 uv;
	float2 dz;
	float depth;
};

static float2 poissonDisk16[16] =
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
static float2 poissonDisk32[32] =
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
float BorderDepthTexture(const float2 uv, const int shadowMapIndex)
{
	return ((uv.x <= 1.0) && (uv.y <= 1.0) &&
		(uv.x >= 0.0) && (uv.y >= 0.0)) ? shadowMaps[shadowMapIndex].SampleLevel(samPcssBloker, uv, 0.0f).r : 1.0;
}
float BorderDepthTextureArray(const float3 uv, const int shadowMapIndex)
{
	return ((uv.x <= 1.0) && (uv.y <= 1.0) &&
		(uv.x >= 0.0) && (uv.y >= 0.0)) ? shadowArray[shadowMapIndex].SampleLevel(samPcssBloker, uv, 0.0f).r : 1.0f;
}
float BorderDepthConnetedTextureArray(const float3 uv, const float3 nextUv, const int shadowMapIndex)
{
	return ((uv.x <= 1.0) && (uv.y <= 1.0) &&
		(uv.x >= 0.0) && (uv.y >= 0.0)) ? shadowArray[shadowMapIndex].SampleLevel(samPcssBloker, uv, 0.0f).r :
	(((nextUv.x <= 1.0) && (nextUv.y <= 1.0) &&
		(nextUv.x >= 0.0) && (nextUv.y >= 0.0)) ? shadowArray[shadowMapIndex].SampleLevel(samPcssBloker, nextUv, 0.0f).r : 1.0f);
}
float BorderDepthPCFTexture(const float3 uvz, const int shadowMapIndex)
{
	return ((uvz.x <= 1.0f) && (uvz.y <= 1.0f) &&
		(uvz.x >= 0.0f) && (uvz.y >= 0.0f)) ? shadowMaps[shadowMapIndex].SampleCmpLevelZero(samCmpPcssFilter, uvz.xy, uvz.z).r : ((uvz.z <= 1.0f) ? 1.0f : 0.0f);
}
float BorderDepthPCFTextureArray(const float3 uv, const float z, const int shadowMapIndex)
{
	return ((uv.x <= 1.0) && (uv.y <= 1.0) &&
		(uv.x >= 0.0) && (uv.y >= 0.0)) ? shadowArray[shadowMapIndex].SampleCmpLevelZero(samCmpPcssFilter, uv, z).r : ((z <= 1.0) ? 1.0 : 0.0);
}
float BorderDepthPCFConnectedTextureArray(const float3 uv, const float3 nextUv, const float z, const float nz, const int shadowMapIndex)
{
	return ((uv.x <= 1.0) && (uv.y <= 1.0) &&
		(uv.x >= 0.0) && (uv.y >= 0.0)) ? shadowArray[shadowMapIndex].SampleCmpLevelZero(samCmpPcssFilter, uv, z).r :
	(((nextUv.x <= 1.0) && (nextUv.y <= 1.0) &&
		(nextUv.x >= 0.0) && (nextUv.y >= 0.0)) ? shadowArray[shadowMapIndex].SampleCmpLevelZero(samCmpPcssFilter, nextUv, nz).r : ((nz <= 1.0) ? 1.0 : 0.0));
}
float BiasedZ(float2 dz_duv, float2 offset, float z0)
{
	return z0 + dot(dz_duv, offset);
}

void CalBlendAmount(float4 textureCoord, float blendRate, in out float currentPixelsBlendBandLocation, out float blendBetweenCascadesAmount)
{
	float2 distanceToOne = float2(1.0f - textureCoord.x, 1.0f - textureCoord.y);
	currentPixelsBlendBandLocation = min(textureCoord.x, textureCoord.y);
	currentPixelsBlendBandLocation = min(currentPixelsBlendBandLocation, min(distanceToOne.x, distanceToOne.y));
	blendBetweenCascadesAmount = 1.0f - (currentPixelsBlendBandLocation / blendRate);
}
/**
* ref
* https://developer.download.nvidia.com/whitepapers/2008/PCSS_Integration.pdf
* lightRadiusUv = lightSize / frstrumSize	... 
* receiverZ = (posW * lightView)
* nearPlane = light near plane factor ... frustum near로 설정시 하나의 object에 position변경에 따라 다른 object들의 그림자들의 soft정도가 달라지므로
* 임의의 고정된 값을 사용
*/
// Using similar triangles from the surface point to the area light
float2 CalBlockerSearchUv(const float2 sampleSize, const float scale, const float nearPlane, const float receiverZ)
{
	return ((scale * PCSS_SOFT_FACTOR * (receiverZ - nearPlane)) / receiverZ) * sampleSize;

}
// Using similar triangles between the area light, the blocking plane and the surface point
float CalPenumbraRadius(const float scale, float receiverZ, float blockerZ)
{
	return ((scale * (receiverZ - blockerZ)) / blockerZ);
	//return ((scale * (receiverZ - blockerZ)) / blockerZ);
}
/*
*	in penumbraRadius - world 
*	out penumbraUv
*/ 
float2 CalPenumbraUvE(const float2 sampleCount, const float penumbraRadius, const float nearPlane, const float receiverZ, const float blockerZ)
{
	//penumbra = ((scale * (receiverZ - blockerZ)) / blockerZ)
	float rate = nearPlane / receiverZ;
	return penumbraRadius * sampleCount;
}
float2 FindBlocker(const float2 uv, const float2 searchSize, const float2 dz, const float2 rotationTrig, const float z0, const int shadowMapIndex)
{
	float2 result = float2(0, 0);
	for (int i = 0; i < PCSS_BLOCKER_SAMPLE; ++i)
	{
		float2 offset = poissonDisk16[i] * searchSize;
		offset = Rotate(offset, rotationTrig);
		
		float shadowMapDepth = BorderDepthTexture(uv + offset, shadowMapIndex);
		float z = BiasedZ(dz, offset, z0);
		if (shadowMapDepth < z)
		{
			result.x += shadowMapDepth;
			result.y++;
		}
	}
	return result;
}

/**
* Directional Light
*/

//ssm
float PCFNeighborAverage(float4 shadowPosH, int shadowMapIndex)
{
	shadowPosH.xyz /= shadowPosH.w;
	float depth = shadowPosH.z;
	
	uint width, height;
	shadowMaps[shadowMapIndex].GetDimensions(width, height);
	
	float dx = 1.0f / (float) width;
	float dy = 1.0f / (float) height;

	float percentLit = 0.0f;
	[unroll]
	for (int i = -PCF_STEP; i <= PCF_STEP; ++i)
	{
		[unroll]
		for (int j = -PCF_STEP; j <= PCF_STEP; ++j)
			percentLit += shadowMaps[shadowMapIndex].SampleCmpLevelZero(samCmpLinearPointShadow, shadowPosH.xy + float2(i * dx, j * dy), depth).r;
	}
	return percentLit / PCF_STEP_COUNT;
}
float PCFSampling(float4 shadowPosH, int shadowMapIndex)
{
	shadowPosH.xyz /= shadowPosH.w;
	float depth = shadowPosH.z;

	float width, height;
	shadowMaps[shadowMapIndex].GetDimensions(width, height);
	
	float2 sampleSize = float2(1.0f / width, 1.0f / height) * PCF_SAMPLE_COUNT;
	float random = shadowMaps[bluseNoiseTextureIndex].Sample(samPointClamp, shadowPosH.xy * invBluseNoiseTextureSize * camInvRenderTargetSize).a;
	random = mad(random, 2.0, -1.0);
	
	float rotationAngle = random * PI;
	float2 rotationTrig = float2(cos(rotationAngle), sin(rotationAngle));
	
	float percentLit = 0.0f;
	[unroll]
	for (int i = 0; i < PCSS_PCF_SAMPLE; ++i)
	{
		float2 offset = Rotate(poissonDisk16[i] * sampleSize, rotationTrig);
		percentLit += shadowMaps[shadowMapIndex].SampleCmpLevelZero(samCmpLinearPointShadow, shadowPosH.xy + offset, depth).r;
	}
	return percentLit / PCSS_PCF_SAMPLE;
}
float PCFPoisson(const float2 uv, float2 filterRadiusUv, const float2 dz, const float2 rotationTrig, const float z0, int shadowMapIndex)
{
	float percentLit = 0.0f;
	for (int i = 0; i < PCSS_PCF_SAMPLE; ++i)
	{
		float2 offset = Rotate(poissonDisk16[i] * filterRadiusUv, rotationTrig);
		float z = BiasedZ(dz, offset, z0);
		percentLit += BorderDepthPCFTexture(float3(uv + offset, z), shadowMapIndex);
	}
	return percentLit / PCSS_PCF_SAMPLE;
}
float DirectionalPCSS(float2 uv, float2 dz, float depth, float vPosZ, int lightIndex)
{
	float random = shadowMaps[bluseNoiseTextureIndex].Sample(samPointClamp, uv * invBluseNoiseTextureSize * camInvRenderTargetSize).a;
	random = mad(random, 2.0, -1.0);
	
	float rotationAngle = random * PI;
	float2 rotationTrig = float2(cos(rotationAngle), sin(rotationAngle));
	
	int shadowMapIndex = directionalLight[lightIndex].shadowMapIndex;
	float frustumNear = directionalLight[lightIndex].frustumNear;
	float frustumFar = directionalLight[lightIndex].frustumFar;
	float penumbraScale = directionalLight[lightIndex].punumbraScale;
	float penumNearPlane = directionalLight[lightIndex].penumbraNearPlane;
	/*
	frustum은 항상 scene에 bbox을 view로 변환환 min, max이며
	pcss계산시 일정한 그림자를 계산하기위해  (frustumNear - penumNearPlane) * 0.5f 만큼 vPosZ값에서 빼준다.
	*/
	float halfNearGap = (frustumNear - penumNearPlane) * 0.5f;
	vPosZ -= halfNearGap;
	
	float width, height;
	shadowMaps[shadowMapIndex].GetDimensions(width, height);
	float2 texelSize = float2(1.0f / width, 1.0f / height);
	float sampleSize = texelSize * PCSS_SAMPLE_COUNT;
	 
	float2 searchSize = CalBlockerSearchUv(sampleSize, penumbraScale, penumNearPlane, vPosZ);
	float2 blockResult = FindBlocker(uv, searchSize, dz, rotationTrig, depth, shadowMapIndex);
 
	float avgBlockerDepth = blockResult.x / blockResult.y;
	float numBlockers = blockResult.y;
	if (numBlockers == 0)
		return 1.0f;
	//else if (numBlockers == PCSS_BLOCKER_SAMPLE)
	//	return 0.0f; 
	    
	float avgBlockerDepthWorld = NdcToViewOZ(avgBlockerDepth, frustumNear, frustumFar);
	avgBlockerDepthWorld -= halfNearGap;
	
	float penumbraRadius = CalPenumbraRadius(penumbraScale, vPosZ, avgBlockerDepthWorld);
	float2 filterRadiusUV = penumbraRadius * sampleSize;
	// CalPenumbraUvE(sampleSize, penumbraRadius, penumNearPlane, vPosZ, avgBlockerDepthWorld);
	//CalPenumbraUv(sampleSize, penumbraRadius, vPosZ, avgBlockerDepthWorld);
	return PCFPoisson(uv, filterRadiusUV, dz, rotationTrig, depth, shadowMapIndex);
}
float GetDirectionalSSM(float4 shadowPosH, int shadowMapIndex)
{
	shadowPosH.xyz /= shadowPosH.w;
	float depth = shadowPosH.z;
	return shadowMaps[shadowMapIndex].SampleCmpLevelZero(samCmpLinearPointShadow, shadowPosH.xy, depth).r;
}
float CalDirectionalSSMUsePCF(float4 shadowPosH, int shadowMapIndex)
{
	return PCFSampling(shadowPosH, shadowMapIndex);
	//return PCFNeighborAverage(shadowPosH, shadowMapIndex);
}
float CalDirectionalSSMUsePCSS(const float4 shadowPosH, const float3 posW, const int lightIndex)
{
	float2 uv = shadowPosH.xy / shadowPosH.w;
	float depth = shadowPosH.z / shadowPosH.w;

	float2 dz = DepthGradient(uv, depth);
	float3 posV = mul(float4(posW, 1.0f), directionalLight[lightIndex].view);
	return DirectionalPCSS(uv, dz, depth, posV.z, lightIndex);
}

//csm
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
float2 CsmFindBlocker(PcssCsmPixelInfo curInfo, const float2 searchSize, const float2 rotationTrig, const int shadowMapIndex)
{
	float2 result = float2(0, 0);
	for (int i = 0; i < PCSS_BLOCKER_SAMPLE; ++i)
	{
		float2 offset = Rotate(poissonDisk16[i] * searchSize, rotationTrig);
		float shadowMapDepth = BorderDepthTextureArray(curInfo.uv + float3(offset, 0.0f), shadowMapIndex);
		float z = BiasedZ(curInfo.dz, offset, curInfo.depth);

		if (shadowMapDepth < z)
		{
			result.x += shadowMapDepth;
			++result.y;
		}
	}
	return result;
}
float2 CsmFindBlocker(PcssCsmPixelInfo curInfo, PcssCsmPixelInfo nextInfo, const float2 searchSize, const float2 rotationTrig, const int shadowMapIndex)
{
	float2 result = float2(0, 0);
	for (int i = 0; i < PCSS_BLOCKER_SAMPLE; ++i)
	{
		float2 offset = Rotate(poissonDisk16[i] * searchSize, rotationTrig);
		float shadowMapDepth = BorderDepthConnetedTextureArray(curInfo.uv + float3(offset, 0.0f), nextInfo.uv + float3(offset, 0.0f), shadowMapIndex);
		float z = BiasedZ(curInfo.dz, offset, curInfo.depth);

		if (shadowMapDepth < z)
		{
			result.x += shadowMapDepth;
			++result.y;
		}
	}
	return result;
}
float CsmPCFNeighborAverage(const float2 uv, const float depth, const float2 delta, int csmIndex, int shadowMapIndex)
{
	float percentLit = 0.0f;
	[unroll]
	for (int i = -PCF_STEP; i <= PCF_STEP; ++i)
	{
		[unroll]
		for (int j = -PCF_STEP; j <= PCF_STEP; ++j)
			percentLit += shadowArray[shadowMapIndex].SampleCmpLevelZero(samCmpLinearPointShadow, float3(uv + float2(i * delta.x, j * delta.y), csmIndex), depth).r;
	}
	return percentLit / PCF_STEP_COUNT;
}
float CsmPCFSampling(const float2 uv, const float depth, int csmIndex, int shadowMapIndex)
{
	uint width, height, count, numMips;
	shadowArray[shadowMapIndex].GetDimensions(0, width, height, count, numMips);
	
	float2 sampleSize = float2(1.0f / (float) width, 1.0f / (float) height) * PCF_SAMPLE_COUNT;
	float random = shadowMaps[bluseNoiseTextureIndex].Sample(samPointClamp, uv * invBluseNoiseTextureSize * camInvRenderTargetSize).a;
	random = mad(random, 2.0, -1.0);
	
	float rotationAngle = random * PI;
	float2 rotationTrig = float2(cos(rotationAngle), sin(rotationAngle));
	
	float percentLit = 0.0f;
	[unroll]
	for (int i = 0; i < PCSS_PCF_SAMPLE; ++i)
	{
		float2 offset = Rotate(poissonDisk16[i] * sampleSize, rotationTrig);
		percentLit += shadowArray[shadowMapIndex].SampleCmpLevelZero(samCmpLinearPointShadow, float3(uv + offset, csmIndex), depth).r;
	}
	return percentLit / PCSS_PCF_SAMPLE;
}
float CsmPCFPoisson(PcssCsmPixelInfo curInfo, float2 filterRadiusUv, const float2 rotationTrig, int shadowMapIndex)
{
	float percentLit = 0.0f;
	for (int i = 0; i < PCSS_PCF_SAMPLE; ++i)
	{
		float2 offset = Rotate(poissonDisk16[i] * filterRadiusUv, rotationTrig);
		float z = BiasedZ(curInfo.dz, offset, curInfo.depth);
		percentLit += BorderDepthPCFTextureArray(curInfo.uv + float3(offset, 0.0f), z, shadowMapIndex);
	}
	return percentLit / PCSS_PCF_SAMPLE;
}
float CsmPCFPoisson(PcssCsmPixelInfo curInfo, PcssCsmPixelInfo nextInfo, float2 filterRadiusUv, const float2 rotationTrig, int shadowMapIndex)
{
	float percentLit = 0.0f;
	for (int i = 0; i < PCSS_PCF_SAMPLE; ++i)
	{
		float2 offset = Rotate(poissonDisk16[i] * filterRadiusUv, rotationTrig);
		float z = BiasedZ(curInfo.dz, offset, curInfo.depth);
		float nz = BiasedZ(nextInfo.dz, offset, nextInfo.depth);
		percentLit += BorderDepthPCFConnectedTextureArray(curInfo.uv + float3(offset, 0.0f), nextInfo.uv + float3(offset, 0.0f), z, nz, shadowMapIndex);
	}
	return percentLit / PCSS_PCF_SAMPLE;
}
float CsmPCSS(PcssCsmPixelInfo curInfo, float vPosZ, int csmDataIndex, int shadowMapIndex, int lightIndex)
{
	float random = shadowMaps[bluseNoiseTextureIndex].Sample(samPointClamp, curInfo.uv.xy * invBluseNoiseTextureSize * camInvRenderTargetSize).a;
	random = mad(random, 2.0, -1.0);
	
	float rotationAngle = random * PI;
	float2 rotationTrig = float2(cos(rotationAngle), sin(rotationAngle));
	
	int csmIndex = curInfo.uv.z;
	float subFrustumNear = csmData[csmDataIndex].frustumNear[csmIndex];
	float subFrustumFar = csmData[csmDataIndex].frustumFar[csmIndex];
	float punumbraScale = directionalLight[lightIndex].punumbraScale;
	float penumNearPlane = directionalLight[lightIndex].penumbraNearPlane;
	 
	float halfNearGap = (subFrustumNear - penumNearPlane) * 0.5f;
	vPosZ -= halfNearGap;
	
	uint width, height, count, numMips;
	shadowArray[shadowMapIndex].GetDimensions(0, width, height, count, numMips);
	
	float2 texelSize = float2(1.0f / (float) width, 1.0f / (float) height);
	float sampleSize = texelSize * PCSS_SAMPLE_COUNT;
	float2 searchSize = CalBlockerSearchUv(sampleSize, punumbraScale, penumNearPlane, vPosZ);
	float2 blockResult = CsmFindBlocker(curInfo, searchSize, rotationTrig, shadowMapIndex);

	float avgBlockerDepth = blockResult.x / blockResult.y;
	float numBlockers = blockResult.y;
	if (numBlockers == 0)
		return 1.0f;
	
	float avgBlockerDepthWorld = NdcToViewOZ(avgBlockerDepth, subFrustumNear, subFrustumFar);
	avgBlockerDepthWorld -= halfNearGap;
	
	float penumbraRadius = CalPenumbraRadius(punumbraScale, vPosZ, avgBlockerDepthWorld);
	float2 filterRadiusUV = penumbraRadius * sampleSize;

	return CsmPCFPoisson(curInfo, filterRadiusUV, rotationTrig, shadowMapIndex);
}
float CsmPCSS(PcssCsmPixelInfo curInfo, PcssCsmPixelInfo nextInfo, float vPosZ, int csmDataIndex, int shadowMapIndex, int lightIndex)
{
	float random = shadowMaps[bluseNoiseTextureIndex].Sample(samPointClamp, curInfo.uv.xy * invBluseNoiseTextureSize * camInvRenderTargetSize).a;
	random = mad(random, 2.0, -1.0);
	
	float rotationAngle = random * PI;
	float2 rotationTrig = float2(cos(rotationAngle), sin(rotationAngle));
	
	int csmIndex = curInfo.uv.z;
	float subFrustumNear = csmData[csmDataIndex].frustumNear[csmIndex];
	float subFrustumFar = csmData[csmDataIndex].frustumFar[csmIndex];
	float penumbraScale = directionalLight[lightIndex].punumbraScale;
	float penumNearPlane = directionalLight[lightIndex].penumbraNearPlane;
	 
	float halfNearGap = (subFrustumNear - penumNearPlane) * 0.5f;
	vPosZ -= halfNearGap;
	
	uint width, height, count, numMips;
	shadowArray[shadowMapIndex].GetDimensions(0, width, height, count, numMips);
	
	float2 texelSize = float2(1.0f / (float) width, 1.0f / (float) height);
	float sampleSize = texelSize * PCSS_SAMPLE_COUNT;
	float2 searchSize = CalBlockerSearchUv(sampleSize, penumbraScale, penumNearPlane, vPosZ);
	float2 blockResult = CsmFindBlocker(curInfo, nextInfo, searchSize, rotationTrig, shadowMapIndex);

	float avgBlockerDepth = blockResult.x / blockResult.y;
	float numBlockers = blockResult.y;
	if (numBlockers == 0)
		return 1.0f;
	//else if (numBlockers == PCSS_BLOCKER_SAMPLE)
	//	return 0.0f;
	
	float avgBlockerDepthWorld = NdcToViewOZ(avgBlockerDepth, subFrustumNear, subFrustumFar);
	avgBlockerDepthWorld -= halfNearGap;
	
	float penumbraRadius = CalPenumbraRadius(penumbraScale, vPosZ, avgBlockerDepthWorld);
	float2 filterRadiusUV = penumbraRadius * sampleSize;

	return CsmPCFPoisson(curInfo, nextInfo, filterRadiusUV, rotationTrig, shadowMapIndex);
}
float GetDirectionalCsm(float4 posV, int shadowMapIndex, int csmDataIndex)
{
	float4 textureCoord = 0.0f;
	int csmIndex = CalCsmIndex(posV, csmDataIndex, textureCoord);
	float2 uv = textureCoord.xy / textureCoord.w;
	float depth = textureCoord.z / textureCoord.w;
	
	return shadowArray[shadowMapIndex].SampleCmpLevelZero(samCmpLinearPointShadow, float3(uv, csmIndex), depth).r;
}
float CalDirectionalCsmUsePCF(float4 posV, int shadowMapIndex, int csmDataIndex)
{
	float4 textureCoord = 0.0f;
	int csmIndex = CalCsmIndex(posV, csmDataIndex, textureCoord);

	float2 uv = textureCoord.xy / textureCoord.w;
	float depth = textureCoord.z / textureCoord.w;

	float percentLit = CsmPCFSampling(uv, depth, csmIndex, shadowMapIndex);
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
				float nPercentLit = CsmPCFSampling(uv, depth, nextCsmIndex, shadowMapIndex);
				percentLit = lerp(percentLit, nPercentLit, blendAmount);
			}
		}
	}
	return percentLit;
}
float CalDirectionalCsmUsePCSS(float4 posV, int lightIndex, int csmLocalIndex)
{
	int csmDataIndex = directionalLight[lightIndex].csmDataIndex + csmLocalIndex;
	int shadowMapIndex = directionalLight[lightIndex].shadowMapIndex + csmLocalIndex;

	float4 textureCoord = 0.0f;
	int csmIndex = CalCsmIndex(posV, csmDataIndex, textureCoord);

	PcssCsmPixelInfo curInfo;
	curInfo.uv = float3(textureCoord.xy / textureCoord.w, csmIndex);
	curInfo.depth = textureCoord.z / textureCoord.w;
	curInfo.dz = DepthGradient(curInfo.uv.xy, curInfo.depth);
	
	int nextCsmIndex = min(CSM_MAX_COUNT - 1, csmIndex + 1);
	if (csmIndex != nextCsmIndex)
	{
		textureCoord = posV * csmData[csmDataIndex].scale[nextCsmIndex];
		textureCoord += csmData[csmDataIndex].posOffset[nextCsmIndex];
		
		PcssCsmPixelInfo nextInfo;
		nextInfo.uv = float3(textureCoord.xy / textureCoord.w, nextCsmIndex);
		nextInfo.depth = textureCoord.z / textureCoord.w;
		nextInfo.dz = DepthGradient(nextInfo.uv.xy, nextInfo.depth);
		float percentLit = CsmPCSS(curInfo, nextInfo, posV.z, csmDataIndex, shadowMapIndex, lightIndex);
		/*
		if (USE_CSM_LEVEL_BLEND)
		{
			float blendLocation = 1.0f;
			float blendAmount = 1.0f;

			CalBlendAmount(textureCoord, csmData[csmDataIndex].levelBlendRate, blendLocation, blendAmount);
			if (blendLocation < csmData[csmDataIndex].levelBlendRate)
			{
				float nPercentLit = CsmPCSS(nextInfo, posV.z, csmDataIndex, shadowMapIndex, lightIndex);
				percentLit = lerp(percentLit, nPercentLit, blendAmount);
			}
		} 
		*/
		return percentLit;
	}
	else
		return CsmPCSS(curInfo, posV.z, csmDataIndex, shadowMapIndex, lightIndex);
}
float4 CalCascadeDebugFactor(float4 posV, int shadowMapIndex, int csmDataIndex)
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

/**
* Point Light
*/
#if !defined(POINT_LIGHT_NEAR)
#define POINT_LIGHT_NEAR 1.0f
#endif
 
float GetCubeShadow(float3 posW, int lightIndex, int shadowMapIndex)
{
	float3 lightToPos = posW - pointLight[lightIndex].position;
	   
	float litDepth = NonLinearDepth((length(lightToPos) - POINT_LIGHT_NEAR) / (pointLight[lightIndex].range - POINT_LIGHT_NEAR), POINT_LIGHT_NEAR, pointLight[lightIndex].range);
	float smDepth = shadowCubeMap[shadowMapIndex].Sample(samCubeShadow, lightToPos).r;
		
	return smDepth + SHADOW_BIAS < litDepth ? 0.0f : 1.0f;
}
float CalCubeMapShadowUsePCM(float3 posW, int lightIndex, int shadowMapIndex)
{
	float3 lightToPos = posW - pointLight[lightIndex].position;
	uint width, height, numMips;
	shadowCubeMap[shadowMapIndex].GetDimensions(0, width, height, numMips);

	// Texel size.
	//float delta = 2.0f / (float) min(width, height);
	float percentLit = 0.0f;

	float3 sampleOffsetDirections[20] =
	{
		float3(1, 1, 1), float3(1, -1, 1), float3(-1, -1, 1), float3(-1, 1, 1),
	   float3(1, 1, -1), float3(1, -1, -1), float3(-1, -1, -1), float3(-1, 1, -1),
	   float3(1, 1, 0), float3(1, -1, 0), float3(-1, -1, 0), float3(-1, 1, 0),
	   float3(1, 0, 1), float3(-1, 0, 1), float3(1, 0, -1), float3(-1, 0, -1),
	   float3(0, 1, 1), float3(0, -1, 1), float3(0, -1, -1), float3(0, 1, -1)
	};
	 
	[unroll]
	for (int i = 0; i < 20; ++i)
	{
		float3 litVec = lightToPos + sampleOffsetDirections[i] * 0.05f;
		float litDepth = NonLinearDepth((length(litVec) - POINT_LIGHT_NEAR) / (pointLight[lightIndex].range - POINT_LIGHT_NEAR), POINT_LIGHT_NEAR, pointLight[lightIndex].range);
		float smDepth = shadowCubeMap[shadowMapIndex].Sample(samCubeShadow, litVec).r;
		
		percentLit += smDepth + SHADOW_BIAS < litDepth ? 0.0f : 1.0f;
	}
	return percentLit / (float)20.0f;
}

/**
* Spot Light
*/
#if !defined(SPOT_PCF_STEP)
#define SPOT_PCF_STEP 1.0f
#endif

#if !defined(SPOT_PCF_STEP_COUNT)
#define SPOT_PCF_STEP_COUNT (((SPOT_PCF_STEP * 2) + 1) * ((SPOT_PCF_STEP * 2) + 1) * ((SPOT_PCF_STEP * 2) + 1))
#endif
float CalSpotLightSSMUsePCM(float4 shadowPosH, int shadowMapIndex)
{  
	return PCFNeighborAverage(shadowPosH, shadowMapIndex);
}
float GetSpotLightSSM(float4 shadowPosH, int shadowMapIndex)
{
	// Complete projection by doing division by w.
	shadowPosH.xyz /= shadowPosH.w;

	// Depth in NDC space.
	float depth = shadowPosH.z;
	return shadowMaps[shadowMapIndex].SampleCmpLevelZero(samCmpLinearPointShadow, shadowPosH.xy, depth).r;
}

float4 CalculateLight(Material mat,float3 posW,float3 normal,float3 toEye)
{
	float3 directLight = float3(0, 0, 0);
#ifdef SHADOW
	for (int i = directionalLitSt; i < directionalLitEd; ++i)
	{
		float shadowFactor = 1;
		/**
		* DIRECTONAL_LIGHT_HAS_NORMAL_SHADOW_MAP) shadowPosH = posW * view * proj * textureSpace
		* DIRECTONAL_LIGHT_HAS_CSM) shadowPosH = posW * view	... csm index에 따라 CalDirectionalCsmShadowFactorEx 함수 내부에서
		* textureSpace 변환
		*/
		float4 shadowPosH = mul(float4(posW, 1.0f), directionalLight[i].shadowMapTransform);
		if (directionalLight[i].shadowMapType == DIRECTONAL_LIGHT_HAS_NORMAL_SHADOW_MAP)
		{
#ifdef USE_DIRECTIONAL_LIGHT_PCSS
			shadowFactor = CalDirectionalSSMUsePCSS(shadowPosH, posW, i);
#elif USE_DIRECTIONAL_LIGHT_PCM
			shadowFactor = CalDirectionalSSMUsePCF(shadowPosH, directionalLight[i].shadowMapIndex);
#else
			shadowFactor = GetDirectionalSSM(shadowPosH, directionalLight[i].shadowMapIndex);
#endif 
			directLight += ComputeDirectionalLight(directionalLight[i], mat, normal, toEye) * shadowFactor; 
		}
		else if (directionalLight[i].shadowMapType == DIRECTONAL_LIGHT_HAS_CSM)
		{				
#ifdef USE_DIRECTIONAL_LIGHT_PCSS
			shadowFactor = CalDirectionalCsmUsePCSS(shadowPosH, i, csmLocalIndex);
#elif USE_DIRECTIONAL_LIGHT_PCM
			shadowFactor = CalDirectionalCsmUsePCF(shadowPosH, directionalLight[i].shadowMapIndex + csmLocalIndex, directionalLight[i].csmDataIndex + csmLocalIndex);
#else
			shadowFactor =GetDirectionalCsm(shadowPosH, directionalLight[i].shadowMapIndex + csmLocalIndex, directionalLight[i].csmDataIndex + csmLocalIndex);
#endif 
			directLight += ComputeDirectionalLight(directionalLight[i], mat, normal, toEye) * shadowFactor;	
		} 
		else
			directLight += ComputeDirectionalLight(directionalLight[i], mat, normal, toEye);
	}
	for (int i = pointLitSt; i < pointLitEd; ++i)
	{
		if (pointLight[i].hasShadowMap)
		{
			float shadowFactor = 1.0f;
#ifdef USE_POINT_LIGHT_PCM
			shadowFactor = CalCubeMapShadowUsePCM(posW, i, pointLight[i].shadowMapIndex);
#else
			shadowFactor = GetCubeShadow(posW, i, pointLight[i].shadowMapIndex);
#endif 
			directLight += ComputePointLight(pointLight[i], mat, posW, normal, toEye) * shadowFactor;
		}
		else
			directLight += ComputePointLight(pointLight[i], mat, posW, normal, toEye);
	}
	for (int i = spotLitSt; i < spotLitEd; ++i)
	{
		if (spotLight[i].hasShadowMap)
		{
			float4 shadowPosH = mul(float4(posW, 1.0f), spotLight[i].shadowMapTransform);
			float shadowFactor = 1.0f;
#ifdef USE_SPOT_LIGHT_PCM
			shadowFactor = CalSpotLightSSMUsePCM(shadowPosH, spotLight[i].shadowMapIndex);
#else
			shadowFactor = GetSpotLightSSM(shadowPosH, spotLight[i].shadowMapIndex);
#endif
			directLight += ComputeSpotLight(spotLight[i], mat, posW, normal, toEye) * shadowFactor;
		}
		else
			directLight += ComputeSpotLight(spotLight[i], mat, posW, normal, toEye);
	}
#else 
	for (int i = directionalLitSt; i < directionalLitEd; ++i)
		directLight += ComputeDirectionalLight(directionalLight[i], mat, normal, toEye);
	for (int i = pointLitSt; i < pointLitEd; ++i)
		directLight += ComputePointLight(pointLight[i], mat, posW, normal, toEye);
	for (int i = spotLitSt; i < spotLitEd; ++i)
		directLight += ComputeSpotLight(spotLight[i], mat, posW, normal, toEye);
#endif
	return float4(directLight, 0.0f);
}