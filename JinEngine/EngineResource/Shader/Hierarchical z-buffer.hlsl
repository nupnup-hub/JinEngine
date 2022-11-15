#define DownSamplingCount 10
#define OcclusionQueryCount 1000

Texture2D depthMap : register(t0);
RWStructuredBuffer<int> queryResult : register(u0);
RWTexture2D<float4> downSamplingDepthMap[DownSamplingCount] : register(u1);

SamplerState samPoint : register(s0);

struct BoundSphere
{
    float3 center;
    float radius;
};

cbuffer cbObject : register(b0)
{
    BoundSphere objWorld[OcclusionQueryCount];
    int validQueryCount;
};

cbuffer cbPass : register(b1)
{
    float4x4 camView;
    float4x4 camInvView;
    float4x4 camProj;
    float4x4 camInvProj;
    float4x4 camViewProj;
    float4x4 camInvViewProj;
    float2 camRenderTargetSize;
    float2 camInvRenderTargetSize;
    float3 camEyePosW;
    uint cameraPad00;
    float camNearZ;
    float camFarZ;
    uint cameraPad01;
    uint cameraPad02;
}; 

[numthreads(16, 16, 1)]
void HZB(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
   
}