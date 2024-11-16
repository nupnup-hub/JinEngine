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
#include"../../Common/LightCompute.hlsl"
#include"../../Common/CommonConstantsStructureDefine.hlsl" 
#include"../../Common/GBufferCommon.hlsl"

ConstantBuffer<SSRData> cb : register(b0);
Texture2D srcMap : register(t0); 
Texture2D ssrMap : register(t1);
Texture2D albedoMap : register(t2); 
Texture2D lightProp : register(t3);
RWTexture2D<float4> destMap : register(u0);
SamplerState samLinearClamp : register(s0);
//SamplerState samPointClamp : register(s1);
   
#define REFLECTION_MULTIPLE_POW_FIX_FACTOR 0.1f

[numthreads(DIMX, DIMY, 1)]
void main(int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= cb.ta.rtSize.x || dispatchThreadID.y >= cb.ta.rtSize.y)
        return;
     
    const int2 pixelCoord = dispatchThreadID.xy;
    const float2 centerCoord = pixelCoord + 0.5f;
    const float2 centerUv = centerCoord * cb.ta.invRtSize;
     
    float3 albedoColor;
    float specularFactor;
    UnPackAlbedoColorLayer(albedoMap.SampleLevel(samLinearClamp, centerUv, 0), albedoColor, specularFactor);
     
    float metallic;
    float roughness;
    float aoFactor;
    uint materialID;
    UnpackLightPropLayer(lightProp.SampleLevel(samLinearClamp, centerUv, 0), metallic, roughness, aoFactor, materialID);

    float4 srcColor = srcMap.SampleLevel(samLinearClamp, centerUv, 0);
    float4 ssrColor = ssrMap.SampleLevel(samLinearClamp, centerUv, 0);   
    
    float reflectStrength = (1.0f - roughness) * specularFactor;
    float reflectionMultiplier = reflectStrength == 0 ? 0 : pow(reflectStrength, 1.0f - metallic + REFLECTION_MULTIPLE_POW_FIX_FACTOR);
    reflectionMultiplier *= ssrColor.a; 
  
    if (ssrColor.a > 0)
        srcColor.xyz = srcColor.xyz * (1.0f - reflectionMultiplier) + reflectionMultiplier * ssrColor.xyz;
    
    destMap[pixelCoord] = srcColor;
}