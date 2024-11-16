#pragma once
#include"../../Common/TemporalAccumulationCommon.hlsl"

#ifndef DIMX
#define DIMX 16
#endif
#ifndef DIMY
#define DIMY 16
#endif   
 
struct SSRData
{
    TACommonPassData ta;
    
    float4x4 camView;
    float4x4 camProj; 
 
    float3 camPosW;
    float rayTMin;
  
    float2 halfRtSize;
    float2 halfInvRtSize;
    
    float startOffset;
    float stepScale;  
    uint maxStepCount;
    float rayDistance;              //0 ~ 10000   
   
    float thickness;                //0 ~ 10000    depth backface를 사용하면 두 번 렌더링 해야하므로 사용자 설정값으로 대채   
    float objectViewZBias;          //0 ~ 10000    
    float fadeDistance;             //0 ~ 1.0f
    float fadeOneRate;              //to [0.0f ~ 1.0f]   
   
    uint sampleNumber;    
    uint sampleSetSize;    
    uint sampleMax; 
    uint pad00;
};
 

struct SSRReuseSamplePack
{
    float4 data; // x, y = color, b = weight
};

struct SSRReuseSampleData
{
    float4 color; //8 byte 
    float3 weight; //12 byte
};
 
  