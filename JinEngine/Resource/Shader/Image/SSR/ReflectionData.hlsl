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
#include"../../Common/CommonConstantsStructureDefine.hlsl"  
#include"../../Common/DepthFunc.hlsl" 
#include"../../Common/Math.hlsl"
 
#ifndef CONE_TRACING_LOOP
#define CONE_TRACING_LOOP 16
#endif

#ifndef MIP_COUNT
#define MIP_COUNT 6
#endif
  
#ifndef RAY_COUNT
#define RAY_COUNT 4
#endif
  
ConstantBuffer<SSRData> cb : register(b0);
Texture2D srcMap[MIP_COUNT] : register(t0, space1);
TextureCube skyMap : register(t1);
Texture2D<float> viewZMap : register(t2);
Texture2D normalMap : register(t3);
Texture2D lightProperty : register(t4);
RWTexture2D<float4> destMap : register(u0);
SamplerState samPointClamp : register(s0);
SamplerState samLinearClamp : register(s1);
 
namespace SSR
{ 
    class HitData
    {
        float2 uv;
        float viewZ;
        bool isValid;
    };
    HitData CreateHitData()
    {
        HitData hitData;
        hitData.uv = float2(0, 0);
        hitData.viewZ = 0;
        hitData.isValid = false; 
        
        return hitData;
    }
     
    class PixelData
    {
        int2 pixelCoord;
        float2 centerUv;
    
        float3 posV;
        float viewZ;
    
        float3 posW;
        float roughness;
    
        float3 normalW;
        float metallic;
        
        float3 tanW;
        //float coneTriangleTan;
        float coneConeCos;
         
        int curSampleNumber;
        int mipLowLevel;
        int mipHighLevel;
        int mipLowWeight;
        
        float4 reflectColor;
        //float brdf;
        //float pdf;
        
        //float pdf;    
        float sampleCount;
        bool isValid;
         
        void InsertFailColor()
        {
            destMap[pixelCoord] = float4(0, 0, 0, 0);
        }
        void InsertReflectionColor()
        {
            destMap[pixelCoord] = reflectColor / sampleCount;
        }
        float2 SampleHalton()
        {
            const float2 halton[16] =
            {
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
            if (curSampleNumber >= 16)
                curSampleNumber = 0;
            
            const float2 result = halton[curSampleNumber];           
            ++curSampleNumber;
            return result; 
        }   
        float2 Sample2DPoint()
        {
            uint pixelIndex = pixelCoord.y * cb.halfRtSize.x + pixelCoord.x;
            if (curSampleNumber >= cb.sampleMax)
                curSampleNumber = 0;
            
            const uint permutation = curSampleNumber * (cb.halfRtSize.x * cb.halfRtSize.y) + pixelIndex;
            const float2 u = SampleCMJ2D(curSampleNumber, cb.sampleSetSize, cb.sampleSetSize, permutation);
            ++curSampleNumber;
            
            return u;
        }
        void SampleSkyColor(const float3 dir)
        {
            //const float alpha = (1.0f - roughness) * (1.0f - roughness);
            reflectColor += float4(skyMap.SampleLevel(samLinearClamp, dir, 0).xyz, 1.0f); 
            sampleCount += 1.0f;
            //pdf = SampleDirectionCone_PDF(coneConeCos);
        };
        void SampleHitcolor(in HitData hitData)
        {
            if (!hitData.isValid)
                return; 
             
            //fade out
            //0.0f ~ 1.0f 
            //값에 범위에만 관심이 있으므로 y역전은 적용x  
    
            float fadeDistance = cb.fadeDistance;
            float fadeFactor = saturate(max(abs(hitData.uv.x - 0.5f) * 2.0f, abs(hitData.uv.y - 0.5f) * 2.0f));
 
            float alpha = fadeFactor > (1.0f - fadeDistance) ? (1.0f - fadeFactor) * cb.fadeOneRate : 1.0f;
            alpha = alpha * alpha;
            
            reflectColor += srcMap[mipLowLevel].SampleLevel(samLinearClamp, hitData.uv, 0) * mipLowWeight + srcMap[mipHighLevel].SampleLevel(samLinearClamp, hitData.uv, 0) * (1.0f - mipLowWeight);
            reflectColor.w *= alpha; 
            sampleCount += 1.0f;
            //pdf = SampleDirectionCone_PDF(coneConeCos);
        }
    };
    
    /*
    unuse
    float ComputeTriangleTan(const float roughness)
    {
        float factor = (1.0f - (1.0f - roughness) * (1.0f - roughness)) * 45.0f;
        return tan(DegToRad(factor));  
    }
    */
    float ComputeConeCos(const float roughness)
    {  
        return cos(DegToRad((roughness * roughness) * 22.5f));
    }
    PixelData CreatePixelData(const int3 dispatchThreadID)
    {
        PixelData pixelData;
        pixelData.pixelCoord = dispatchThreadID.xy;
        pixelData.reflectColor = float4(0, 0, 0, 0);
        pixelData.sampleCount = 0;
        
        pixelData.centerUv = (pixelData.pixelCoord + float2(0.5f, 0.5f)) * cb.halfInvRtSize;
        pixelData.viewZ = viewZMap.SampleLevel(samPointClamp, pixelData.centerUv, 0).x;
    
        //depth buffer는 1로 초기화되고 View-Z map  생성 과정에서 유효하지 않은 depth는 far값을 가짐. 
        pixelData.isValid = pixelData.viewZ != cb.ta.camNearFar.y;
        if (!pixelData.isValid)
            return pixelData;
     
        pixelData.posV = GetViewPos(pixelData.centerUv, pixelData.viewZ, cb.ta.uvToViewA, cb.ta.uvToViewB);
        pixelData.posW = mul(float4(pixelData.posV, 1.0f), cb.ta.camInvView).xyz;
     
        pixelData.normalW = UnpackNormal(normalMap.SampleLevel(samLinearClamp, pixelData.centerUv, 0));
        UnpackLightPropLayer(lightProperty.SampleLevel(samLinearClamp, pixelData.centerUv, 0), pixelData.metallic, pixelData.roughness);
       
        //pixelData.coneTriangleTan = ComputeTriangleTan(pixelData.roughness);
        pixelData.coneConeCos = ComputeConeCos(pixelData.roughness);
        pixelData.curSampleNumber = cb.sampleNumber; 
        
        const float mipLevelFactor = pixelData.roughness * (MIP_COUNT - 1);
        pixelData.mipLowLevel = mipLevelFactor;
        pixelData.mipHighLevel = min(pixelData.mipLowLevel + 1, MIP_COUNT - 1);
        pixelData.mipLowWeight = 1.0f - (mipLevelFactor - int(mipLevelFactor)); 
        
        return pixelData;
    };  
      
    class ReflectionData
    {
        float3 centerReflection; 
        float3 position;      
        float3x3 basis; 
        
        float3 Compute(in PixelData data)
        {
            if (data.roughness == 0)
                return centerReflection;
             
            //[-1 ~ 1]
            const float2 u = data.Sample2DPoint();
            const float3 dir = SampleDirectionCone(u.x, u.y, data.coneConeCos);
            
            //const float2 scale = (data.SampleHalton() * 2.0f - 1.0f) * data.coneTriangleTan;
            //const float3 offset = mul(float3(scale, 1.0f), basis);
            return normalize(mul(dir, basis));
        }
    };
    ReflectionData CreateWorldReflectionData(in PixelData data)
    {
        ReflectionData wr;
        wr.centerReflection = normalize(reflect(normalize(data.posW - cb.camPosW), data.normalW));
        wr.position = data.posW;
        
        const float3 T = normalize(cross(data.normalW, wr.centerReflection));
        const float3 B = cross(wr.centerReflection, T);
        const float3 N = cross(T, B);
        
        wr.basis = float3x3(T, B, N);
        
        return wr;
    } 
    ReflectionData CreateViewReflection(in PixelData data)
    {
        const float3 normalV = normalize(mul(data.normalW, (float3x3) cb.camView));
        
        ReflectionData vr;
        vr.centerReflection = normalize(reflect(data.posV, normalV));
        vr.position = data.posV;
        
        float3 T = normalize(cross(normalV, vr.centerReflection));
        float3 B = cross(vr.centerReflection, T);
        float3 N = cross(T, B);
        
        vr.basis = float3x3(T, B, N);
        
        //vr.basis = CalBasis(vr.centerReflection);     
        return vr;
    }
}
 