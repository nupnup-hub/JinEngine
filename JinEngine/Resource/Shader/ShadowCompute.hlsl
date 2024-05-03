#pragma once
#include"LightCompute.hlsl"  
#include"PixelShaderCommon.hlsl"
/**
* PCCS referenced by NVIDIAGameWorks-GraphicsSamples-master-softshadow 
*/
//Erase VSM function  --2023-11-01-- 

/**
* Common
*/

 //bias = DepthBias / (2^24)
#if !defined(SHADOW_SLOPE_SCALE)
#define SHADOW_SLOPE_SCALE 1.2f				 
#endif

 //0.0015f		//bias = DepthBias / (2^24)
#if !defined(SHADOW_BIAS)
#define SHADOW_BIAS  0.03f 
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
#define USE_POISSON16_F2
#endif

#define USE_POISSON32_F2
#include"Poisson.hlsl"
 
float2 CalRotationTrig(const float2 uv)
{
    float random = textureMaps[cbScene.bluseNoiseTextureIndex].Sample(samPointWrap, uv * cbScene.invBluseNoiseTextureSize * cbCam.invRenderTargetSize).a;
    return RandomRotationTrig(random);
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
		float2 offset = Rotate(poissonDiskF2_16[i], rotationTrig) * pcfData.sampleSize;
#elif USE_PCF_32_SAMPLE
		float2 offset = Rotate(poissonDiskF2_32[i], rotationTrig) * pcfData.sampleSize;
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
        float2 offset = Rotate(poissonDiskF2_32[bStep], rotationTrig) * searchSize;
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
        float2 offset = Rotate(poissonDiskF2_32[pStep], rotationTrig) * filterRadiusUV;
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
		float2 offset = Rotate(poissonDiskF2_16[i], rotationTrig) * pcfData.sampleSize;
#elif USE_PCF_32_SAMPLE
		float2 offset = Rotate(poissonDiskF2_32[i], rotationTrig) * pcfData.sampleSize;
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
        float2 offset = Rotate(poissonDiskF2_32[bStep], rotationTrig) * searchSize;
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
        float2 offset = Rotate(poissonDiskF2_32[pStep], rotationTrig) * filterRadiusUV;
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
#if USE_CSM_LEVEL_BLEND
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
#endif
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
        float2 offset = poissonDiskF2_32[blockStep] * searchSize;
        float3 samplePos = (smData.nLitVec + smData.sideLitVec * offset.x + smData.upLitVec * offset.y) * searchSize;
        float shadowMapDepth = shadowM.SampleLevel(samPcssBloker, samplePos, 0).r;
        float diskLength = length(poissonDiskF2_32[blockStep]);
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
        float2 offset = poissonDiskF2_32[filterStep] * filterRadius;
        float3 samplePos = (smData.nLitVec + smData.sideLitVec * offset.x + smData.upLitVec * offset.y);
        float diskLength = length(poissonDiskF2_32[filterStep]);
        percentLit += shadowM.SampleCmpLevelZero(samCmpLinearPointShadow, samplePos, smData.compareDistance + smData.shadowDepthBias * diskLength).r;
		//* length(poissonDiskF2_32[filterStep])
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
		float3 samplePos = smData.nLitVec + smData.sideLitVec * poissonDiskF2_16[i].x + smData.upLitVec * poissonDiskF2_16[i].y;
		float diskLength = length(poissonDiskF2_16[i]);
#elif USE_PCF_32_SAMPLE
		float3 samplePos = smData.nLitVec + smData.sideLitVec * poissonDiskF2_32[i].x + smData.upLitVec * poissonDiskF2_32[i].y;
		float diskLength = length(poissonDiskF2_32[i]);
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
    DirectionalLight dLit;
    dLit.direction = directionalLight[index].direction;
    dLit.color = directionalLight[index].color;
    dLit.power = directionalLight[index].power;
	
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
		int csmDataIndex = directionalLight[index].csmDataIndex + cbCam.csmLocalIndex;
		int shadowMapIndex = directionalLight[index].shadowMapIndex + cbCam.csmLocalIndex;
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
    return ComputeDirectionalLight(dLit, mat, normalW, tangentW, toEye) * shadowFactor;
}
float3 ComputePointLight(Material mat, float3 posW, float3 normalW, float3 tangentW, float3 toEye, int index)
{
    PointLight pLit;
    pLit.midPosition = pointLight[index].midPosition;
    pLit.sidePosition[0] = pointLight[index].sidePosition[0];
    pLit.sidePosition[1] = pointLight[index].sidePosition[1];
    pLit.color = pointLight[index].color;
    pLit.power = pointLight[index].power;
    pLit.range = pointLight[index].frustumFar;
		
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
    return ComputePointLight(pLit, mat, posW, normalW, tangentW, toEye) * shadowFactor;
}
float3 ComputeSpotLight(Material mat, float3 posW, float3 normalW, float3 tangentW, float3 toEye, int index)
{
    SpotLight sLit;
    sLit.position = spotLight[index].position;
    sLit.direction = spotLight[index].direction;
    sLit.color = spotLight[index].color;
    sLit.power = spotLight[index].power;
    sLit.range = spotLight[index].frustumFar;
    sLit.innerConeCosAngle = spotLight[index].innerConeCosAngle;
    sLit.outerConeCosAngle = spotLight[index].outerConeCosAngle;
	
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
    return ComputeSpotLight(sLit, mat, posW, normalW, tangentW, toEye) * shadowFactor;
}
float3 ComputeRectLight(Material mat, float3 posW, float3 normalW, float3 tangentW, float3 toEye, int index)
{
    RectLight rLit = ComputeRect(rectLight[index], posW);
    rLit.ltcMat = textureMaps[rLit.ltcMatTextureIndex];
    rLit.ltcAmp = textureMaps[rLit.ltcAmpTextureIndex];
    rLit.source = textureMaps[rLit.sourceTextureIndex];
    rLit.samLTC = samLTC;
    rLit.samLTCSample = samLTCSample;
    return ComputeRectLight(rLit, mat, posW, normalW, tangentW, toEye);
}
 
#ifdef LIGHT_CLUSTER
float3 ComputeLight(Material mat, float3 posW, float3 normalW, float3 tangentW, float3 toEye, int2 screenPos, float nonLinearDepth)
{  
	float3 lightColor = float3(0, 0, 0);
	for (uint dLitIndex = cbScene.directionalLitSt; dLitIndex < cbScene.directionalLitEd; ++dLitIndex)
		lightColor += ComputeDirectionalLight(mat, posW, normalW, tangentW, toEye, dLitIndex);
  
	const uint rangeX = cbCam.renderTargetSize.x / CLUSTER_DIM_X;
	const uint rangeY = cbCam.renderTargetSize.y / CLUSTER_DIM_Y; 
	const float minDepth = cbScene.clusterMinDepth; //log2(NEAR_CLUST);
	const float maxDepth = log2(cbCam.FarZ);

	const float scale = 1.0f / (maxDepth - minDepth) * (CLUSTER_DIM_Z - 1.0f);
	const float bias = 1.0f - minDepth * scale;  
	const float depth = NdcToViewPZ(nonLinearDepth, cbCam.nearZ, cbCam.FarZ);
	if (depth == cbCam.FarZ)
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
    for (uint dLitIndex = cbScene.directionalLitSt; dLitIndex < cbScene.directionalLitEd; ++dLitIndex)
        lightColor += ComputeDirectionalLight(mat, posW, normalW, tangentW, toEye, dLitIndex);
    for (uint pLitIndex = cbScene.pointLitSt; pLitIndex < cbScene.pointLitEd; ++pLitIndex) 
        lightColor += ComputePointLight(mat, posW, normalW, tangentW, toEye, pLitIndex);
    for (uint sLitIndex = cbScene.spotLitSt; sLitIndex < cbScene.spotLitEd; ++sLitIndex)
        lightColor += ComputeSpotLight(mat, posW, normalW, tangentW, toEye, sLitIndex);
    for (uint rLitIndex = cbScene.rectLitSt; rLitIndex < cbScene.rectLitEd; ++rLitIndex)
        lightColor += ComputeRectLight(mat, posW, normalW, tangentW, toEye, rLitIndex);
    return lightColor;
}
 
 
