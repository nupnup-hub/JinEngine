/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once
#include"ColorSpaceUtility.hlsl"
#include"GBufferCommon.hlsl"
#include"FilterCommon.hlsl"
#include"PixelCommon.hlsl"   
#include"Sampling.hlsl"

#ifndef MAX_FAST_FRAME_ACCMURATION
#define MAX_FAST_FRAME_ACCMURATION 8
#endif

#ifndef MAX_FRAME_ACCMURATION
#define MAX_FRAME_ACCMURATION 64
#endif 

#ifndef FIXED_FRAME_COUNT
#define FIXED_FRAME_COUNT 4
#endif 
 
#define TAA_COLOR_ERROR_ESTIMATE_RADIUS 1
#define TAA_CLAMP_SCALE 1
#define TAA_COLOR_GAUS_BLUR_RADIUS 1
#define TAA_COLOR_GAUS_BLUR_LENGTH TAA_COLOR_GAUS_BLUR_RADIUS * 2 + 1

#ifndef NORMAL_THRESHOLD
#define NORMAL_THRESHOLD 0.5f
#endif
#ifndef DISOCCLUSION_THRES_HOLD
#define DISOCCLUSION_THRES_HOLD 0.01f 
#endif 

float ComputeColorVariance(const float3 color)
{
    float2 moments;
    moments.r = RGBToLuminance(color);
    moments.g = moments.r * moments.r;
    
    return max(0.0f, moments.g - moments.r * moments.r);
}


// Remap partial depth derivatives at z0 from [1,1] pixel offset to a new pixel offset.
float2 RemapDdxy(in float z0, in float2 ddxy, in float2 pixelOffset)
{
    // Perspective correction for non-linear depth interpolation.
    // Ref: https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/visibility-problem-depth-buffer-depth-interpolation
    // Given a linear depth interpolation for finding z at offset q along z0 to z1
    //      z =  1 / (1 / z0 * (1 - q) + 1 / z1 * q)
    // and z1 = z0 + ddxy, where z1 is at a unit pixel offset [1, 1]
    // z can be calculated via ddxy as
    //
    //      z = (z0 + ddxy) / (1 + (1-q) / z0 * ddxy) 
    float2 z = (z0 + ddxy) / (1 + ((1 - pixelOffset) / z0) * ddxy);
    return sign(pixelOffset) * (z - z0);
}

uint SmallestPowerOf2GreaterThan(in uint x)
{
    // Set all the bits behind the most significant non-zero bit in x to 1.
    // Essentially giving us the largest value that is smaller than the
    // next power of 2 we're looking for.
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;

    // Return the next power of two value.
    return x + 1;
}
// Returns float precision for a given float value.
// Values within (value -precision, value + precision) map to the same value. 
// Precision = exponentRange/MaxMantissaValue = (2^e+1 - 2^e) / (2^numMantissaBits)
// Ref: https://blog.demofox.org/2017/11/21/floating-point-precision/
float FloatPrecision(in float x, in uint numMantissaBits)
{
    // Find the exponent range the value is in.
    uint nextPowerOfTwo = SmallestPowerOf2GreaterThan(x);
    float exponentRange = nextPowerOfTwo - (nextPowerOfTwo >> 1);

    float MaxMantissaValue = (float) ((uint) 1 << numMantissaBits);

    return exponentRange / MaxMantissaValue;
}

float AccumSpeed(const uint currHistoryLength)
{
#if 1
    return 1.0f / float(min(currHistoryLength, MAX_FRAME_ACCMURATION));
#else 
    return 1.0f / float(min(currHistoryLength + 1.0f, MAX_FRAME_ACCMURATION));
#endif
}
float AccumSpeed(const uint currHistoryLength, float minSpeed)
{
    return max(1.0f / float(currHistoryLength), minSpeed);
}
float4 AccumSpeed4(const uint4 currHistoryLength)
{
    return float4(AccumSpeed(currHistoryLength.x), AccumSpeed(currHistoryLength.y), AccumSpeed(currHistoryLength.z), AccumSpeed(currHistoryLength.w));
}
float FastAccumSpeed(const uint currHistoryLength)
{
#if 1
    return 1.0f / float(min(currHistoryLength, MAX_FAST_FRAME_ACCMURATION));
#else 
    return 1.0f / float(min(currHistoryLength + 1.0f, MAX_FAST_FRAME_ACCMURATION));
#endif
}
 
float3 HistoryRgbClamping(float3 minRgb, float3 maxRgb, float3 currentPixelRgb, float3 historyRgb)
{
    float3 dir = currentPixelRgb - historyRgb;
    float3 p0 = (minRgb - historyRgb) / dir;
    float3 p1 = (maxRgb - historyRgb) / dir;
    float t = max(max(min(p0.x, p1.x), min(p0.y, p1.y)), min(p0.z, p1.z));
    return lerp(historyRgb, currentPixelRgb, t);
}
    
struct TACommonPassData
{
    float4x4 camInvView;
    float4x4 camPreInvView;
    float4x4 camPreViewProj;
    
    float2 rtSize;
    float2 invRtSize;
    
    float2 uvToViewA;
    float2 uvToViewB;
    
    float2 preUvToViewA;
    float2 preUvToViewB;
    
    float2 camNearFar;
    float camNearMulFar;
    uint haltonSampleNumber;                      
};
namespace TA
{   
    struct PixelData
    {
        uint2 coord;
        uint groupIndex;
        
        float2 centerUv;
        float2 jitteredCenterUv;
    
        float viewZ;
        float3 normalW;
        uint materialID;
 
        float3 posV;
        float3 posW;
    
        double4 prePosH;
        double2 preCenterUv;
        double2 velocity;
         
        void Initialize(const uint2 _pixelCoord,  
            const uint _groupIndex,
            const TACommonPassData cb,
            Texture2D<float> viewZMap,
            Texture2D normalMap, 
            Texture2D lightProp,
            SamplerState samPointClmap,
            SamplerState samLinearClmap)
        { 
            coord = _pixelCoord;
            groupIndex = _groupIndex;
            centerUv = float2(coord + 0.5f) * cb.invRtSize;
    
            viewZ = viewZMap.SampleLevel(samLinearClmap, centerUv, 0);
            normalW = UnpackNormal(normalMap.SampleLevel(samLinearClmap, centerUv, 0));
            materialID = UnpackMaterialID(lightProp.SampleLevel(samPointClmap, centerUv, 0));
 
            posV = UVToViewSpace(centerUv, viewZ, cb.uvToViewA, cb.uvToViewB);
            posW = mul(float4(posV, 1.0f), cb.camInvView).xyz;
            prePosH = mul(float4(posW, 1.0f), cb.camPreViewProj);
            preCenterUv = (prePosH.xy / prePosH.w) * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
            velocity = preCenterUv - centerUv;
        }  
        void SetJitter(const uint sampleNumber, const float2 invRtSize)
        {
            const float2 halton[16] =
            {
                //{0.5f, 0.5f },
                { 0.5f, 0.333333f },
                { 0.25f, 0.666667f },
                { 0.75f, 0.111111f },
                { 0.125f, 0.444444f },
                { 0.625f, 0.777778f },
                { 0.375f, 0.222222f },
                { 0.875f, 0.555556f },
                { 0.0625f, 0.888889f },
                { 0.5625f, 0.037037f },
                { 0.3125f, 0.37037f },
                { 0.8125f, 0.703704f },
                { 0.1875f, 0.148148f },
                { 0.6875f, 0.481481f },
                { 0.4375f, 0.814815f },
                { 0.9375f, 0.259259f },
                { 0.03125f, 0.592593f }
            };
            
            float2 jitter = (halton[sampleNumber] - float2(0.5f, 0.5f)) * 2.0f;
            jitteredCenterUv = float2(coord + float2(0.5f, 0.5f) + jitter) * invRtSize;
        }
        void SkipVelocity(const float threshold)
        {
            //부동소수점 오차 교정
            if (abs(velocity.x) <= threshold)
                velocity.x = 0;
            if (abs(velocity.y) <= threshold)
                velocity.y = 0;
            
            preCenterUv = velocity + centerUv;
            //velocity = abs(velocity);
        }
    };
     
    struct GeometryErrorResult
    {
        Catmul::Parameter bicubicParameter;
        Bilinear::Parameter bilinearParameter;
        float4 customWeight;
        int passCount;
        bool canUseCubic;
        bool canUseBilinear;
    };    
    class GeometryErrorEstimationActor
    {
        float2 preUv;
        float3 centerPosW;
        float3 centerNormal;
        uint centerMaterialID;
        
        float2 rtSize;
        float2 invRtSize;
            
        float invCenterDistToPoint;
        float disocclusionThreshold;
        float normalThresHold;
         
        float3x3 camPreInvView;
        float2 preUvToViewA;
        float2 preUvToViewB;
        
        Texture2D<float> preViewZMap;
        Texture2D preLightPropMap;
        Texture2D preNormalMap;
        SamplerState samPointClamp;
        SamplerState samLinearClmap;
          
        float4 ComputeDisocclusion2x2(float2 sampleCenterUv, float4 prevViewZ)
        {
            float3 sampleCenterNormal = UnpackNormal(preNormalMap.SampleLevel(samLinearClmap, sampleCenterUv, 0));
            if (dot(centerNormal, sampleCenterNormal) < normalThresHold)
                return float4(1, 1, 1, 1);
            
            float sampleCenterViewZ = preViewZMap.SampleLevel(samLinearClmap, sampleCenterUv, 0).x;
            float3 preSampleCenterPosV = GetViewPos(sampleCenterUv, sampleCenterViewZ, preUvToViewA, preUvToViewB);
            float3 preSampleCenterPosW = mul(preSampleCenterPosV, camPreInvView);

            float sampleDistance00 = abs(dot(preSampleCenterPosW, centerNormal));
            float sampleDistance01 = abs(dot(preSampleCenterPosW, sampleCenterNormal));
            
            //Ratio of distance between sample and center
            float centerDistanceRate = max(sampleDistance00, sampleDistance01) * invCenterDistToPoint;
            
            //Ratio of distance between sample center and sample tabs center
            float4 sampleViewZRate = (1.0f / abs(preSampleCenterPosV.z)) * abs(prevViewZ);
            
            float4 relativePlaneDist = centerDistanceRate * sampleViewZRate - centerDistanceRate;
            //float4 relativePlaneDist = (centerDistanceRate / abs(preSampleCenterPosV.z)) * abs(prevViewZ) - centerDistanceRate;
 
            return step(relativePlaneDist, disocclusionThreshold);
        } 
        
        void Initialze(float2 _preUv,
            float3 _centerPosW,
            float3 _centerNormal,
            uint _centerMaterialID,
            float2 _rtSize,
            float2 _invRtSize,
            float _invCenterDistToPoint,
            float _disocclusionThreshold,
            float _normalThresHold,
            float3x3 _camPreInvView,
            float2 _preUvToViewA,
            float2 _preUvToViewB,
            Texture2D<float> _preViewZMap,
            Texture2D _preLightPropMap,
            Texture2D _preNormalMap,
            SamplerState _samPointClamp,
            SamplerState _samLinearClmap)
        {
            preUv = _preUv;
            centerPosW = _centerPosW;
            centerNormal = _centerNormal;
            centerMaterialID = _centerMaterialID;
            
            rtSize = _rtSize;
            invRtSize = _invRtSize;
            invCenterDistToPoint = _invCenterDistToPoint;
            disocclusionThreshold = _disocclusionThreshold;
            normalThresHold = _normalThresHold;
                   
            camPreInvView = _camPreInvView;
            preUvToViewA = _preUvToViewA;
            preUvToViewB = _preUvToViewB;
            
            preViewZMap = _preViewZMap;
            preLightPropMap = _preLightPropMap;
            preNormalMap = _preNormalMap;
            samPointClamp = _samPointClamp;
            samLinearClmap = _samLinearClmap;
        }
        void Compute(inout GeometryErrorResult result)
        {
            // bc - bicubic tap,
            // bl - bicubic & bilinear tap
            //
            // -- bc bc --      Gather 
            // bc bl bl bc      w  z 
            // bc bl bl bc      x  y
            // -- bc bc --
 
            //  CatRom12     => Bilinear
            //    0y 1x
            // 0z 0w 1z 1w       0z 1y
            // 2x 2y 3x 3y       2y 3x
            //    2w 3z
            // Gather sample ordering: (-,+),(+,+),(+,-),(-,-),
             
            result.bicubicParameter.Initialize(preUv, rtSize, invRtSize);
                    
            //float preCenterViewZ = preViewZMap.SampleLevel(samLinearClmap, preUv, 0);
            //float3 preCenterPosV = GetViewPos(preUv, preCenterViewZ, preUvToViewA, preUvToViewB);
            //float3 preCenterPosW = mul(preCenterPosV, camPreInvView);
            //float3 preCenterNormal = UnpackNormal(preNormalMap.SampleLevel(samLinearClmap, preUv, 0));
        
            /*
                Since the catmulParam origin is located at the center,
                it is necessary to subtract 0.5f in order to perform the GatherRed operation in Cubic 
            */
            float2 gatherCenterUv = (result.bicubicParameter.leftUpCenter) * invRtSize;
         
            float4 preViewZ00 = preViewZMap.GatherRed(samLinearClmap, gatherCenterUv).wzxy;
            float4 preViewZ10 = preViewZMap.GatherRed(samLinearClmap, gatherCenterUv + float2(2.0f, 0.0f) * invRtSize).wzxy;
            float4 preViewZ01 = preViewZMap.GatherRed(samLinearClmap, gatherCenterUv + float2(0.0f, 2.0f) * invRtSize).wzxy;
            float4 preViewZ11 = preViewZMap.GatherRed(samLinearClmap, gatherCenterUv + float2(2.0f, 2.0f) * invRtSize).wzxy;
            //float disocclusionThreshold = 1.0f / (camFar - camNear);
            
            float3 disocclusion00 = ComputeDisocclusion2x2(gatherCenterUv + float2(-0.5f, -0.5f) * invRtSize, preViewZ00).yzw;
            float3 disocclusion10 = ComputeDisocclusion2x2(gatherCenterUv + float2(1.5f, -0.5f) * invRtSize, preViewZ10).xzw;
            float3 disocclusion01 = ComputeDisocclusion2x2(gatherCenterUv + float2(-0.5f, 1.5f) * invRtSize, preViewZ01).xyw;
            float3 disocclusion11 = ComputeDisocclusion2x2(gatherCenterUv + float2(1.5f, 1.5f) * invRtSize, preViewZ11).xyz;
             
            float4 lightProp00 = preLightPropMap.GatherAlpha(samPointClamp, gatherCenterUv).wzxy;
            float4 lightProp10 = preLightPropMap.GatherAlpha(samPointClamp, gatherCenterUv + float2(2.0f, 0.0f) * invRtSize).wzxy;
            float4 lightProp01 = preLightPropMap.GatherAlpha(samPointClamp, gatherCenterUv + float2(0.0f, 2.0f) * invRtSize).wzxy;
            float4 lightProp11 = preLightPropMap.GatherAlpha(samPointClamp, gatherCenterUv + float2(2.0f, 2.0f) * invRtSize).wzxy;
      
            uint3 materialID00 = float3(lightProp00.y, lightProp00.z, lightProp00.w) * MATERIAL_ID_RANGE;
            uint3 materialID10 = float3(lightProp10.x, lightProp10.z, lightProp10.w) * MATERIAL_ID_RANGE;
            uint3 materialID01 = float3(lightProp01.x, lightProp01.y, lightProp01.w) * MATERIAL_ID_RANGE;
            uint3 materialID11 = float3(lightProp11.x, lightProp11.y, lightProp11.z) * MATERIAL_ID_RANGE;
            
            disocclusion00 *= all(materialID00 == centerMaterialID);
            disocclusion10 *= all(materialID10 == centerMaterialID);
            disocclusion01 *= all(materialID01 == centerMaterialID);
            disocclusion11 *= all(materialID11 == centerMaterialID);
            float4 bilinearOcclusion = float4(disocclusion00.z, disocclusion10.y, disocclusion01.y, disocclusion11.x);
           
            uint viewTestPassCount = dot(disocclusion00 + disocclusion10 + disocclusion01 + disocclusion11, 1.0f);
            result.passCount += viewTestPassCount;
            result.canUseCubic = viewTestPassCount > 11.0f;
            
            result.bilinearParameter = Bilinear::CreateFilter(preUv, rtSize);
            result.customWeight = Bilinear::ComputeWeights(result.bilinearParameter, bilinearOcclusion);
            result.canUseBilinear = any(bilinearOcclusion > 0.0f); 
            //any(result.customWeight > 0.0f);
            //any(result.customWeight > 0.0f);
            //result.customWeight *= result.customWeight;
        }
    };   
    
    GeometryErrorEstimationActor CreateActor(in PixelData pixel,
        in TACommonPassData common,
        Texture2D<float> preViewZMap,
        Texture2D preLightPropMap,
        Texture2D preNormalMap,
        SamplerState samPointClamp,
        SamplerState samLinearClmap)
    {
        float centerPlaneDist = dot(pixel.posW, pixel.normalW);
        GeometryErrorEstimationActor actor;
        actor.Initialze(float2(pixel.preCenterUv),
            pixel.posW,
            pixel.normalW,
            pixel.materialID,
            common.rtSize,
            common.invRtSize,
            1.0f / centerPlaneDist,
            DISOCCLUSION_THRES_HOLD * (common.camNearFar.y - common.camNearFar.x),
            NORMAL_THRESHOLD,
            (float3x3) common.camPreInvView,
            common.preUvToViewA,
            common.preUvToViewB,
            preViewZMap,
            preLightPropMap,
            preNormalMap,
            samPointClamp,
            samLinearClmap);
        
        return actor;
    }   
    bool ComputeGeometryErrorEstimate(in GeometryErrorEstimationActor actor, out GeometryErrorResult result)
    {
        result.customWeight = float4(0, 0, 0, 0);
        result.passCount = 0;
        result.canUseCubic = false;
        result.canUseBilinear = false;
      
        bool isValidCoord = IsValidUv(actor.preUv);
        if (!isValidCoord)
            return false;
    
        actor.Compute(result);
        return true;
    }

    
#ifdef USE_GROUP_BUFFER_FOR_TAA
#ifndef SHARED_BUFFER_SIZE
#define SHARED_BUFFER_SIZE DIMX * DIMY
#endif
groupshared float4 sharedColor[SHARED_BUFFER_SIZE];
#endif
       
    struct ColorErrorResult
    {
        float3 clampColor; 
        float errorFactor; //outOfRange bbox border 0 ~ 1.0f
        bool isSafe;
    };
    class ColorErrorEstimateActor
    {
        float2 curUv;
        float2 preUv;

        float3 curPixelCenterColor;
        float3 preHistoryCenterColor;
        
        float2 invRtSize;
        
        Texture2D srcColorMap;
        SamplerState samPointClamp;
        SamplerState samLinearClmap;
        
        int groupIndex; //Optional       
        void Initialize(float2 _curUv,
            float2 _preUv,
            float3 _curPixelCenterColor,
            float3 _preHistoryCenterColor,
            float2 _invRtSize,
            Texture2D _srcColorMap,
            SamplerState _samPointClamp,
            SamplerState _samLinearClmap)
        {
            curUv = _curUv;
            preUv = _preUv;
            curPixelCenterColor = _curPixelCenterColor;
            preHistoryCenterColor = _preHistoryCenterColor;
            invRtSize = _invRtSize;
            srcColorMap = _srcColorMap;
            samPointClamp = _samPointClamp;
            samLinearClmap = _samLinearClmap;
        }
        void SetGroupIndex(int _groupIndex)
        {
            groupIndex = _groupIndex;
        }
        void Compute(inout ColorErrorResult result)
        {
            float3 m1 = float3(0, 0, 0);
            float3 m2 = float3(0, 0, 0);
            uint loopCount = 0;
            
            [unroll]
            for (int y = -TAA_COLOR_ERROR_ESTIMATE_RADIUS; y <= TAA_COLOR_ERROR_ESTIMATE_RADIUS; y++)
            {
                [unroll]
                for (int x = -TAA_COLOR_ERROR_ESTIMATE_RADIUS; x <= TAA_COLOR_ERROR_ESTIMATE_RADIUS; x++)
                {
                    float2 sampleUv = curUv + float2(x, y) * invRtSize;
                    if (!IsValidUv(sampleUv))
                        continue;
                    
#ifdef USE_GROUP_BUFFER_FOR_TAA
                    int sampleGroupIndex = groupIndex + x + (y * DIMX);
                    if (sampleGroupIndex < 0 || sampleGroupIndex >= SHARED_BUFFER_SIZE)
                        continue;
                    
                    float3 sampleYCoCg = sharedColor[sampleGroupIndex].xyz;
#else   
                    float3 sampleYCoCg = RGBToYCoCg(srcColorMap.SampleLevel(samPointClamp, sampleUv, 0).xyz);
#endif 
                    m1 += sampleYCoCg;
                    m2 += m1 * m1;
                    ++loopCount;
                }
            }
            m1 /= loopCount;
            m2 /= loopCount;
            float3 sigma = sqrt(m2 - m1 * m1) * TAA_CLAMP_SCALE;
            float3 minYCoCg = m1 - sigma;
            float3 maxYCoCg = m1 + sigma;
            
            float3 curCenterRgb = curPixelCenterColor;
            float3 curCenterYCoCg = RGBToYCoCg(curCenterRgb);
            
            float3 preHistoryCenterRgb = preHistoryCenterColor;
            float3 preHistoryCenterYCoCg = RGBToYCoCg(preHistoryCenterRgb);
            
            float3 dir = curCenterRgb - preHistoryCenterRgb;
            float3 p0 = (YCoCgToRGB(minYCoCg) - preHistoryCenterRgb) / dir;
            float3 p1 = (YCoCgToRGB(maxYCoCg) - preHistoryCenterRgb) / dir;
            float t = saturate(max(max(min(p0.x, p1.x), min(p0.y, p1.y)), min(p0.z, p1.z)));
             
            float3 clampYCoCg = lerp(preHistoryCenterYCoCg, curCenterYCoCg, t);
            result.clampColor = YCoCgToRGB(clampYCoCg);
            result.errorFactor = t;
            result.isSafe = t == 0;
        }
    };
    
    ColorErrorEstimateActor CreateColorActor(in PixelData pixel,
            in TACommonPassData common,
            float3 curPixelCenterColor,
            float3 preHistoryCenterColor,
            Texture2D srcColorMap,
            SamplerState samPointClamp,
            SamplerState samLinearClmap)
    {
        ColorErrorEstimateActor actor;
        actor.Initialize(pixel.centerUv, float2(pixel.preCenterUv), curPixelCenterColor, preHistoryCenterColor, common.invRtSize, srcColorMap, samPointClamp, samLinearClmap);
        actor.SetGroupIndex(pixel.groupIndex);
        
        return actor;
    };    
    ColorErrorEstimateActor CreateColorActorWithJittered(in PixelData pixel,
            in TACommonPassData common,
            float3 curPixelCenterColor,
            float3 preHistoryCenterColor,
            Texture2D srcColorMap,
            SamplerState samPointClamp,
            SamplerState samLinearClmap)
    {
        ColorErrorEstimateActor actor;
        actor.Initialize(pixel.jitteredCenterUv, float2(pixel.preCenterUv), curPixelCenterColor, preHistoryCenterColor, common.invRtSize, srcColorMap, samPointClamp, samLinearClmap);
        actor.SetGroupIndex(pixel.groupIndex);
        
        return actor;
    };    
    bool ComputeColorErrorEstimate(in ColorErrorEstimateActor actor, out ColorErrorResult result)
    { 
        result.clampColor = actor.preHistoryCenterColor;
        result.isSafe = true;
        if (!IsValidUv(actor.preUv))
            return false;
        
        actor.Compute(result);
        return true;
    } 
     
    float3 GaiussianBlur(Texture2D colorMap, SamplerState sam, const float3 centerColor, const float2 centerUv, const float2 invRtSize)
    {
        //Accum speed는 1.0f 이므로 AA효과가 사라진다.
        //따라서 Blur로 AA를 대체.
        
        //sig = 0.7
        float gausFilter[TAA_COLOR_GAUS_BLUR_LENGTH][TAA_COLOR_GAUS_BLUR_LENGTH] =
        {
            { 0.0421996f, 0.117076f, 0.0421996f},
            { 0.117076f, 0.324806f, 0.117076f },
            { 0.0421996f, 0.117076f, 0.0421996f }
        }; 
        float weightSum = 0;
        float3 colorSum = float3(0, 0, 0);
        
        [unroll]
        for (int y = -TAA_COLOR_GAUS_BLUR_RADIUS; y <= TAA_COLOR_GAUS_BLUR_RADIUS; y++)
        {
            [unroll]
            for (int x = -TAA_COLOR_GAUS_BLUR_RADIUS; x <= TAA_COLOR_GAUS_BLUR_RADIUS; x++)
            {  
                float2 sampleUv = centerUv + float2(x, y) * invRtSize;
                if (!IsValidUv(sampleUv))
                    continue;
                    
                int factor = 2 - (abs(y) + abs(x));
                float weight = gausFilter[y + TAA_COLOR_GAUS_BLUR_RADIUS][x + TAA_COLOR_GAUS_BLUR_RADIUS];
                
                float3 sampleColor = colorMap.SampleLevel(sam, sampleUv, 0).xyz;
                colorSum += sampleColor * weight;
                weightSum += weight;
            }
        }
        return colorSum / weightSum;
    }
}