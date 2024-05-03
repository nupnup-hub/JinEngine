#define SSAO_SAMPLE_COUNT 32
#define USE_SSAO 1
#define USE_COMPUTE_SHADER 1
#define CB_RAYTRACING_DEBUG_REGISTER u1
#define DEBUG_SAMPLE_RAY_COUNT SSAO_SAMPLE_COUNT
#include"Packing.hlsl"
#include"SsaoCommon.hlsl"
#include"DepthFunc.hlsl"

#ifdef USE_DEBUG
#include"RaytracingDebugCommon.hlsl"
#endif

//unuse
//추후에 rtao 구현시 폐기
RaytracingAccelerationStructure g_scene : register(t0);
Texture2D depthMap : register(t1);
Texture2D nomralMap : register(t2);
Texture2D randomMap : register(t3);
RWTexture2D<float> aoMap : register(u0);

sampler samPointClamp : register(s0);
sampler samPointWrap : register(s1);

cbuffer cbRtSsaoPass : register(b2)
{
	float4x4 camInvView;
}; 

struct ShadowRayPayload
{
	float isVisible;
	uint index;
};
   
float ShadowwRay(float3 ori, float3 dir, float minT, float maxT, uint index)
{
	//RAY_FLAG_SKIP_CLOSEST_HIT_SHADER
	RayDesc ray;
	ray.Origin = ori;
	ray.Direction = dir;
	ray.TMin = minT;
	ray.TMax = maxT;
	
	ShadowRayPayload payload;
	payload.isVisible = 0.0f;
	payload.index = index;
	
	TraceRay(g_scene, RAY_FLAG_CULL_NON_OPAQUE, 0xff, 0, 1, 0, ray, payload);
#ifdef USE_DEBUG
	if (payload.isVisible == 1.0f)
	{
		uint2 pixel = DispatchRaysIndex().xy;
		if(pixel.x >= 960 && pixel.x < 992 && pixel.y == 500)
			debugInfo[pixel.x - 960].intersectPosition[index].w = 0.0f;
	}
#endif
	return payload.isVisible;
}
[shader("raygeneration")]
void RayGenShader()
{
	uint2 pixel = DispatchRaysIndex().xy;
	float2 uv = pixel * camInvRtSize;
	float depth = depthMap.SampleLevel(samPointClamp, uv, 0).r;
	if (depth == 0.0f)
		return;
	
    float3 pos = UVToViewSpace(uv, NdcToViewPZ(depth, camNearMulFar, camNearFar), uvToViewA, uvToViewB);
	pos = mul(float4(pos, 1.0f), camInvView).xyz;
    float3 normalW = UnpackNormal(nomralMap.SampleLevel(samPointClamp, uv, 0));
	   
	float2 noiseScale = aoRtSize / SSAO_RANDOM_MAP_SIZE;
	float3 random = randomMap.SampleLevel(samPointWrap, uv * noiseScale, 0).xyz * 2.0 - 1.0;
	float3x3 TBN = CalTBN(normalW, random);
	float ao = 0.0f;
	 
#ifdef USE_DEBUG
	if (pixel.x >= 960 && pixel.x < 992 && pixel.y == 500)
		debugInfo[pixel.x - 960].index = pixel;
#endif
	
	for (uint i = 0; i < SSAO_SAMPLE_COUNT; ++i)
	{
		float3 dir = mul(sample[i].xyz, TBN);
		ao += ShadowwRay(pos, normalize(dir), 1e-4, 32, i);
#ifdef USE_DEBUG
		if(pixel.x >= 960 && pixel.x < 992 && pixel.y == 500)
		{		 
			debugInfo[pixel.x - 960].rayOriginAndMinT[i].xyz = pos;
			debugInfo[pixel.x - 960].rayOriginAndMinT[i].w = 1e-4;
			debugInfo[pixel.x - 960].rayDirAndMaxT[i].xyz =normalize(sample[i]).xyz;
			debugInfo[pixel.x - 960].rayDirAndMaxT[i].w = 32;
		}
#endif
	}
	const float access = ao / (float) SSAO_SAMPLE_COUNT;
	aoMap[pixel] = access;
}

[shader("closesthit")]
void ClosestHitShader(inout ShadowRayPayload rayPayload, in BuiltInTriangleIntersectionAttributes attr)
{
	float currentT = RayTCurrent();
	if (currentT >= 16)
		rayPayload.isVisible = lerp(0.0f, 1.0f, (currentT - 16.0f) / 16.0f);
	else
		rayPayload.isVisible = 0.0f;
	 
#ifdef USE_DEBUG
	uint2 pixel = DispatchRaysIndex().xy; 
	if(pixel.x >= 960 && pixel.x < 992 && pixel.y == 500)
	{	
		debugInfo[pixel.x - 960].intersectPosition[rayPayload.index].xyz = HitWorldPosition();
		debugInfo[pixel.x - 960].intersectPosition[rayPayload.index].w = 1.0f;
	}
#endif
}
 
[shader("miss")]
void MissShader(inout ShadowRayPayload rayPayload)
{ 
	rayPayload.isVisible = 1.0f;
}

[shader("anyhit")]
 void AnyHitShader(inout ShadowRayPayload rayPayload, in BuiltInTriangleIntersectionAttributes attr)
{
	float currentT = RayTCurrent();
	if (currentT >= 24)
		rayPayload.isVisible = lerp(0.0f, 1.0f, currentT / 32.0f);
	else
		rayPayload.isVisible = 0.0f;
	//rayPayload.isVisible = 1.0f;
}