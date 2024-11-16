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
#include"Common.hlsl"  
#include"../../Common/FilterCommon.hlsl"  

#ifndef DIMX
#define DIMX 16
#endif
#ifndef DIMY
#define DIMY 16
#endif 

#define EDGE_STOPPING_BLUR_RADIUS 1
#define GAUS_BLUR_RADIUS 1
#define BOX_BLUR_RADIUS 1

#define SHARED_BUFFER_SIZE DIMX * DIMY  
//groupshared float4 sharedColor[SHARED_BUFFER_SIZE];

#ifndef MIP_COUNT 
#define MIP_COUNT 7
#endif

ConstantBuffer<SSRData> cb : register(b0);
Texture2D srcMap[MIP_COUNT] : register(t0, space1);
Texture2D<float> viewZMap : register(t1);
Texture2D normalMap : register(t2);
Texture2D lightProp : register(t3);
Texture2D<float2> depthDerivative : register(t4);
RWTexture2D<float4> destMap : register(u0);
SamplerState samLinearClamp : register(s0);
  
struct FilterData
{
    int groupIndex;
    float2 centerUv;
    
    float4 centerColor;
    float3 centerNormal;
    float centerViewZ;
    float2 ddxy;
    
    float metallic;
    float roughness;
    
    int mipLow;
    int mipHigh;
    float mipLowWeight;
};
float4 ComputeColor(const float2 uv, const int mipLow, const int mipHigh, const float mipLowWeight)
{
    return srcMap[mipLow].SampleLevel(samLinearClamp, uv, 0) * mipLowWeight + srcMap[mipHigh].SampleLevel(samLinearClamp, uv, 0) * (1.0f - mipLowWeight);
}
/*
float4 EdgeStoppingFiltering(in FilterData data)
{
    CrossBilateral::NormalDepth::Parameters param;
    param.normal.Initialize(data.centerNormal);
    param.depth.Initialize(data.centerViewZ, data.ddxy);
  
    float4 colorSum = data.centerColor;
    float weightSum = CrossBilateral::NormalDepth::ComputeWeight(param);
    
    //3x3, 5x5, 7x7
    [unroll]
    for (int y = -EDGE_STOPPING_BLUR_RADIUS; y <= EDGE_STOPPING_BLUR_RADIUS; ++y)
    {
        [unroll]
        for (int x = -EDGE_STOPPING_BLUR_RADIUS; x <= EDGE_STOPPING_BLUR_RADIUS; ++x)
        {
            if (x == 0 && y == 0)
                continue;
                 
            int sampleGroupIndex = data.groupIndex + x + (y * DIMX);
            if (sampleGroupIndex < 0 || sampleGroupIndex >= SHARED_BUFFER_SIZE)
                continue;
            
            float2 offset = float2(x, y);
            float2 uv = data.centerUv + offset * cb.invRtSize;
         
            float4 sampleColor = sharedColor[sampleGroupIndex];
            float sampleViewZ = viewZMap.SampleLevel(samLinearClamp, uv, 0);
            float3 sampleNormalW = UnpackNormal(normalMap.SampleLevel(samLinearClamp, uv, 0));
         
            param.normal.Update(sampleNormalW);
            param.depth.Update(sampleViewZ, -offset);
                
            float weight = IsValidUv(uv);
            weight *= CrossBilateral::NormalDepth::ComputeWeight(param);
            colorSum += sampleColor * weight;
            weightSum += weight;
        }
    }
    weightSum = max(weightSum, EPSILON);
    colorSum /= weightSum;
    return float4(colorSum);
} 
float4 BoxFiltering(in FilterData data)
{ 
    float4 colorSum = float4(0, 0, 0, 0);
    float weightSum = 0;
      
    [unroll]
    for (int y = -BOX_BLUR_RADIUS; y <= BOX_BLUR_RADIUS; ++y)
    {
        [unroll]
        for (int x = -BOX_BLUR_RADIUS; x <= BOX_BLUR_RADIUS; ++x)
        {
            int sampleGroupIndex = data.groupIndex + x + (y * DIMX);
            if (sampleGroupIndex < 0 || sampleGroupIndex >= SHARED_BUFFER_SIZE)
                continue;
            
            float2 offset = float2(x, y) * 4;
            float2 uv = data.centerUv + offset * cb.invRtSize;
         
            float4 sampleColor = srcMap.SampleLevel(samLinearClamp, uv, 0);
            float weight = IsValidUv(uv); 
            colorSum += sampleColor * weight;
            weightSum += weight;
        }
    }
    weightSum = max(weightSum, EPSILON);
    colorSum /= weightSum;
    return float4(colorSum);
}
*/
float4 GaussianFiltering(in FilterData data)
{
    float gaussianFilter[3][3] =
    {
        { 0.05899813f, 0.12489905f, 0.05899813f },
        { 0.12489905f, 0.26441128f, 0.12489905f },
        { 0.05899813f, 0.12489905f, 0.05899813f }
    };
    
    float4 colorSum = float4(0, 0, 0, 0);
    float weightSum = 0;
       
    for (int y = -EDGE_STOPPING_BLUR_RADIUS; y <= EDGE_STOPPING_BLUR_RADIUS; ++y)
    {
        for (int x = -EDGE_STOPPING_BLUR_RADIUS; x <= EDGE_STOPPING_BLUR_RADIUS; ++x)
        { 
            float2 offset = float2(x, y);
            float2 uv = data.centerUv + offset * cb.invRtSize;
         
            float4 sampleColor = ComputeColor(uv, data.mipLow, data.mipHigh, data.mipLowWeight);
            float weight = IsValidUv(uv);
            weight *= gaussianFilter[EDGE_STOPPING_BLUR_RADIUS + y][EDGE_STOPPING_BLUR_RADIUS + x];
            colorSum += sampleColor * weight;
            weightSum += weight;
        }
    }
    weightSum = max(weightSum, EPSILON);
    colorSum /= weightSum;
    return float4(colorSum);
}
float4 BoxFiltering(in FilterData data)
{
    float4 colorSum = float4(0, 0, 0, 0);
    float weightSum = 0;
      
    [unroll]
    for (int y = -BOX_BLUR_RADIUS; y <= BOX_BLUR_RADIUS; ++y)
    {
        [unroll]
        for (int x = -BOX_BLUR_RADIUS; x <= BOX_BLUR_RADIUS; ++x)
        { 
            float2 offset = float2(x, y) + float2(x, y) * 2 * data.roughness;
            float2 uv = data.centerUv + offset * cb.invRtSize;
         
            float4 sampleColor = ComputeColor(uv, data.mipLow, data.mipHigh, data.mipLowWeight);
            float weight = IsValidUv(uv);
            colorSum += sampleColor * weight;
            weightSum += weight;
        }
    }
    weightSum = max(weightSum, EPSILON);
    colorSum /= weightSum;
    return float4(colorSum);
}
[numthreads(DIMX, DIMY, 1)]
void main(int groupIndex : SV_GroupIndex, int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= cb.rtSize.x || dispatchThreadID.y >= cb.rtSize.y)
        return;
    
    int2 pixelCoord = dispatchThreadID.xy;
    float2 uv = (pixelCoord + float2(0.5f, 0.5f)) * cb.invRtSize;
  
    FilterData filterData;
    filterData.groupIndex = groupIndex;
    filterData.centerUv = uv; 
    filterData.centerNormal = UnpackNormal(normalMap.SampleLevel(samLinearClamp, uv, 0));
    filterData.centerViewZ = viewZMap.SampleLevel(samLinearClamp, uv, 0);
    filterData.ddxy = depthDerivative.SampleLevel(samLinearClamp, uv, 0);
 
    UnpackLightPropLayer(lightProp.SampleLevel(samLinearClamp, uv, 0), filterData.metallic, filterData.roughness);
    
    //¿¬±¸Áß.
    //0 ~ MIP_COUNT - 1 
    float mipLevelFactor = filterData.roughness * filterData.roughness * (MIP_COUNT - 1);
    filterData.mipLow = mipLevelFactor;
    filterData.mipHigh = min(filterData.mipLow + 1, MIP_COUNT - 1);
    filterData.mipLowWeight = 1.0f - (mipLevelFactor - int(mipLevelFactor));
    filterData.centerColor = float4(0, 0, 0, 0);
    //filterData.centerColor = ComputeColor(uv, filterData.mipLow, filterData.mipHigh, filterData.mipLowWeight);
    
    //filterData.mipLow = 5;
   /// filterData.mipHigh = 6;
   // filterData.mipLowWeight = 0.5f;
    
    //sharedColor[groupIndex] = filterData.centerColor;
    //GroupMemoryBarrierWithGroupSync();
    
    
    //float4 edgeStopping = EdgeStoppingFiltering(filterData);
   // float4 gaus = GaussianFiltering(filterData);
    //float4 box = BoxFiltering(filterData);
    
    //  GroupMemoryBarrierWithGroupSync();
    destMap[pixelCoord] = BoxFiltering(filterData);
    //destMap[pixelCoord] = ComputeColor(uv, filterData.mipLow, filterData.mipHigh, filterData.mipLowWeight);
    //ComputeColor(uv, filterData.mipLow, filterData.mipHigh, filterData.mipLowWeight);
    //resultColor[pixelCoord].xyz = diffuse;
    
    //destMap[pixelCoord] = srcMap[0].SampleLevel(samLinearClamp, uv, 0);

}