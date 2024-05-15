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
#include"../../../Common/DepthFunc.hlsl"
#include"../../../Common/Sampling.hlsl"
#include"../../../Common/Packing.hlsl"
#include"../../../Common/RandomNumberGenerator.hlsl"
#include"../../../Common/GBufferCommon.hlsl"

struct RestirGiConstants
{
    float4x4 camInvView;
    float4x4 camPreViewProj;
    float2 camNearFar;
    float2 uvToViewA;
    float2 uvToViewB;
    float2 rtSize;
    float2 invRtSize;
    float tMax;
    uint totalNumPixels; //mul rtSize.x * rtSize.y
    
    float3 camPosW;
    float camNearMulFar;
    float3 camPrePosW;
    uint sampleSetSize;
    uint sampleSetMax;
    uint currSampleSetIndex; 
    uint updateCount;
    
    uint directionalLightRange;
    uint pointLightRange;
    uint spotLightRange;
    uint rectLightRange;
    
    uint directionalLightOffset;
    uint pointLightOffset;
    uint spotLightOffset;
    uint rectLightOffset;
    
    uint totalLightCount;
    float invTotalLightCount;
    uint forceClearReservoirs; ///< Clear temporal and spatial reservoirs.
    uint pad00;
    uint pad01;
}; 
ConstantBuffer<RestirGiConstants> cb : register(b0);

float Sample1DPoint(in uint pixelIndex, inout uint sampleSetIndex)
{
    if (sampleSetIndex >= cb.sampleSetMax)
        sampleSetIndex = 0;
    const uint permutation = sampleSetIndex * cb.totalNumPixels + pixelIndex;
    ++sampleSetIndex;
    return SampleCMJ1D(cb.currSampleSetIndex, cb.sampleSetSize, permutation);
}
float2 Sample2DPoint(in uint pixelIndex, inout uint sampleSetIndex)
{
    if (sampleSetIndex >= cb.sampleSetMax)
        sampleSetIndex = 0;
    const uint permutation = sampleSetIndex * cb.totalNumPixels + pixelIndex;
    ++sampleSetIndex;
    return SampleCMJ2D(cb.currSampleSetIndex, cb.sampleSetSize, cb.sampleSetSize, permutation);
}

//Packed
struct RestirSamplePack
{ 
	float3 visiblePos;			//12        
    uint visibleNormal;         //16
    float3 samplePos;           //28   
    uint sampleNormal;          //32    
    uint2 smallDataPack;        //40  x: radiance xy, y: radiance z(0 ~ 15), visibleNormalZ(16), sampleNormalZ(17), ReserviorM(18 ~ 31) 
	float random;				//44 
    
    void Initialize()
    {
        visiblePos = float3(0, 0, 0);
        visibleNormal = 0;
        samplePos = float3(0, 0, 0);
        sampleNormal = 0;
        smallDataPack = uint2(0, 0);
        random = 0;
    }
    void PackVisibleNormal(float3 normal)
    {
        uint2 packNormal = PackSignedOct(SignedOctEncode(normal));
        visibleNormal = packNormal.x;
        smallDataPack.y |= ((packNormal.y << 16) & 0x1FFFF);
    }
    void PackSampleNormal(float3 normal)
    {
        uint2 packNormal = PackSignedOct(SignedOctEncode(normal));
        sampleNormal = packNormal.x;
        smallDataPack.y |= ((packNormal.y << 17) & 0x3FFFF);
    }
    void PackRadiance(float3 radiance)
    {
        uint2 packedRadiance = PackRGB16(radiance);
        smallDataPack.x = packedRadiance.x;
        smallDataPack.y |= (packedRadiance.y & 0xFFFF);
    }
    float3 UnpackVisibleNormal()
    {
        return SignedOctDecode(UnpackSignedOct(uint2(visibleNormal, (smallDataPack.y >> 16) & 0x1)));
    }
    float3 UnpackSampleNormal()
    {
        return SignedOctDecode(UnpackSignedOct(uint2(sampleNormal, (smallDataPack.y >> 17) & 0x1)));
    } 
    float3 UnpackRadiance()
    {
        return UnpackRGB16(smallDataPack);
    }
};

//Packed
struct RestirReserviorPack
{
    RestirSamplePack sample;    //44
    uint age;                  //48 
    float W;                   //52
    
    void Initialize()
    {
        sample.Initialize();
        age = 0;
        W = 0;
    }
    void PackM(uint M)
    {
        //sample.smallDataPack.y |= ((M << 18) & 0x7FFFFFF);
        sample.smallDataPack.y |= ((M << 18) & 0x7FFFFFFF);
    }
    uint UnpackM()
    {
        //return (sample.smallDataPack.y >> 18) & 0x1FF;
        return (sample.smallDataPack.y >> 18) & 0x3FFF;
    }
};

//UnPacked
struct RestirReserviorData
{
    float3 visiblePos;      //12        
    float3 samplePos;       //24   
    float3 radiance;        //36
    float3 visibleNormal;   //48
    float3 sampleNormal;    //60
    float random;           //64  
    uint M;                 //72
    uint age;               //76
    float W;                //80
    
    bool Update(float weight, RestirReserviorData reservoir, RandomNumberGenerator rng, inout float weightSum)
    { 
        weightSum += weight; 
        ++M;
         
        //Conditionally update reservoir.
        float r = rng.Random01();
        //float r = Sample1DPoint(pixelIndex, setIndex); 
        bool isUpdaed = r * weightSum <= weight;
        if (isUpdaed)
        { 
            samplePos = reservoir.samplePos;
            sampleNormal = reservoir.sampleNormal;
            radiance = reservoir.radiance;
            //random = reservoir.random; 
            age = reservoir.age;
            //visibleNormal = reservoir.visibleNormal;
            //visiblePos = reservoir.visiblePos;
            //dstReservoir.age = srcReservoir.age;
        }  
        return isUpdaed;
    }
    bool Merge(float weight, RestirReserviorData reservoir, RandomNumberGenerator rng, inout float weightSum)
    {
        uint tempM = M;
        bool result = Update(weight * reservoir.W * reservoir.M, reservoir, rng, weightSum);
        M = tempM + reservoir.M;
        return result;
    }
};
   
float3 GetWorldPos(float2 uv, float depth)
{ 
    //주의! float3이아닌 float4로 변환해야한다(이동)
    return mul(float4(UVToViewSpace(uv, NdcToViewPZ(depth, cb.camNearMulFar, cb.camNearFar), cb.uvToViewA, cb.uvToViewB), 1.0f), cb.camInvView).xyz;
}
 
RestirReserviorData UnpackRestirGiSample(RestirSamplePack pack)
{
    RestirReserviorData data;
    data.visiblePos = pack.visiblePos;
    data.samplePos = pack.samplePos;
    data.radiance = pack.UnpackRadiance();
    data.visibleNormal = pack.UnpackVisibleNormal();
    data.sampleNormal = pack.UnpackSampleNormal();
    data.random = pack.random;   
    if (dot(data.sampleNormal, data.visiblePos - data.samplePos) < 0)
        data.sampleNormal *= -1;
    if (dot(data.visibleNormal, data.samplePos - data.visiblePos) < 0)
        data.visibleNormal *= -1;
    data.M = 1; 
    data.age = 1;
    data.W = data.random == 0.0f ? 0.0f : (1.0f / data.random);
    return data;
}
RestirReserviorData UnpackRestirGiReservior(RestirReserviorPack pack)
{
    RestirReserviorData data;
    data.visiblePos = pack.sample.visiblePos;
    data.samplePos = pack.sample.samplePos;
    data.radiance = pack.sample.UnpackRadiance();
    data.visibleNormal = pack.sample.UnpackVisibleNormal();
    data.sampleNormal = pack.sample.UnpackSampleNormal();
    data.random = pack.sample.random;
    if (dot(data.sampleNormal, data.visiblePos - data.samplePos) < 0)
        data.sampleNormal *= -1;
    if (dot(data.visibleNormal, data.samplePos - data.visiblePos) < 0)
        data.visibleNormal *= -1;
 
    data.M = pack.UnpackM();
    data.age = pack.age;
    data.W = pack.W; 
    return data;
}
RestirReserviorPack PackRestirGiReservior(RestirReserviorData data)
{
    RestirReserviorPack pack; 
    pack.sample.visiblePos = data.visiblePos;
    pack.sample.samplePos = data.samplePos; 
    pack.sample.random = data.random; 
    pack.age = data.age;
    pack.W = data.W;
       
    pack.sample.smallDataPack = uint2(0, 0);
    pack.sample.PackRadiance(data.radiance);
    pack.sample.PackVisibleNormal(data.visibleNormal);
    pack.sample.PackSampleNormal(data.sampleNormal);
    pack.PackM(data.M);
    return pack;
} 
bool IsValidNeighbor(float3 normal, float3 otherNormal, float depth, float otherDepth, float normalThreshold, float depthThreshold)
{
    return dot(normal, otherNormal) >= normalThreshold && abs((depth - otherDepth)) <= depthThreshold;
}
