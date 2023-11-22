#include"DepthFunc.hlsl"
#include"Math.hlsl"

#define SSAO_SAMPLE_COUNT 14
#define X_LOOP_COUNT 1
#define Y_LOOP_COUNT 1

Texture2D depthMap : register(t0);
Texture2D nomralMap : register(t1);		//16byte r-g
Texture2D randomMap : register(t2);  
RWTexture2D<float> occlusionMap : register(u0);

//same as cbCamera in VertexCommon
cbuffer cbSsaoInfo : register(b0)
{
	float4 desc;				//x = radius, y = fadeStart, z = fadeEnd, w = surfaceEpsilon
	float3 nearPlaneVertex[2];	//0 = left-bottm, 2 = right-top
	float2 nearFar;				//0 = near, 1 = far
	float2 invTextureSize;
	int2 resolution;
};
cbuffer cbTransform : register(b1)
{
	float4x4 projM;
	float4x4 projTexM;
}; 
SamplerState samPointClamp : register(s0);
SamplerState samLinearBorder : register(s1);

static const float3 offsetVector[SSAO_SAMPLE_COUNT] =
{
	float3(-1.0f, -1.0f, -1.0f),
	float3(1.0f, -1.0f, -1.0f),
	float3(-1.0f, 1.0f, -1.0f),
	float3(1.0f, 1.0f, -1.0f),
	float3(-1.0f, -1.0f, 1.0f),
	float3(1.0f, -1.0f, 1.0f),
	float3(-1.0f, 1.0f, 1.0f),
	float3(1.0f, 1.0f, 1.0f),
	
	float3(-1.0f, 0.0f, 0.0f),
	float3(1.0f, 0.0f, 0.0f),
	float3(0.0f, -1.0f, 0.0f),
	float3(0.0f, 1.0f, 0.0f),
	float3(0.0f, 0.0f, -1.0f),
	float3(0.0f, 0.0f, 1.0f)
};

// Determines how much the sample point q occludes the point p as a function
// of distZ.
float OcclusionFunction(float distZ)
{
	float occlusion = 0.0f;
	if (distZ > desc.w)
	{
		float fadeLength = desc.z - desc.y;
		
		// Linearly decrease occlusion from 1 to 0 as distZ goes 
		// from gOcclusionFadeStart to gOcclusionFadeEnd.	
		occlusion = saturate((desc.z - distZ) / fadeLength);
	}
	
	return occlusion;
}
 
#if defined (DIMX) && defined (DIMY)
#ifdef USE_SSAO
[numthreads(DIMX, DIMY, 1)]
void SSAO(int3 dispatchThreadID : SV_DispatchThreadID)
{
	if (resolution.x <= dispatchThreadID.x || resolution.y <= dispatchThreadID.y)
		return;
	 
	float2 invSize = 1.0f / (float2)resolution;  
	[unroll]
	for (int i = 0; i < Y_LOOP_COUNT; ++i)
	{
		[unroll]
		for (int j = 0; j < X_LOOP_COUNT; ++j)
		{
			int2 index = int2(dispatchThreadID.x + j, dispatchThreadID.y + i);
			float2 uv = index / (float2) resolution;
			float3 vertexRate = float3(index / resolution, 1.0f);
			float3 posV = nearPlaneVertex[0] + (nearPlaneVertex[1] - nearPlaneVertex[0]) * vertexRate;
			float2 normalRG = nomralMap.SampleLevel(samPointClamp, uv, 0).xy;
			float3 normal = float3(normalRG, 1.0f - normalRG.x - normalRG.y);
			 
			float pz = LinearDepth(depthMap.SampleLevel(samLinearBorder, uv, 0).r, nearFar.x, nearFar.y);
			float3 p = (pz / posV.z) * posV;
			
			float random = randomMap.SampleLevel(samPointClamp, uv * invTextureSize * invSize, 0).a;
			random = mad(random, 2.0, -1.0);
			float rotationAngle = random * PI;
			
			float3 randVec = float3(cos(rotationAngle), sin(rotationAngle), random);
			float occlusionSum = 0.0f;
			[unroll]
			for (int k = 0; k < SSAO_SAMPLE_COUNT; ++k)
			{
				float3 offset = reflect(offsetVector[i].xyz, randVec);
				float flip = sign(dot(offset, normal));
				float3 q = p + flip * desc.x * offset;
				
				float4 projQ = mul(float4(q, 1.0f), projTexM);
				projQ /= projQ.w;
				
				float rz = depthMap.SampleLevel(samLinearBorder, projQ.xy, 0.0f).r;
				rz = LinearDepth(rz, nearFar.x, nearFar.y);
				
				float3 r = (rz / q.z) * q;
				
				float distZ = p.z - r.z;
				float dp = max(dot(normal, normalize(r - p)), 0.0f);

				float occlusion = dp * OcclusionFunction(distZ);

				occlusionSum += occlusion;
			}
			
			occlusionSum /= SSAO_SAMPLE_COUNT;
			float access = 1.0f - occlusionSum;
			occlusionMap[index] = pow(access, 6.0f);
		}
	}
	
}
#elif USE_HBAO
[numthreads(DIMX, DIMY, 1)]
void HBAO(int3 dispatchThreadID : SV_DispatchThreadID)
{
	
}
#endif
#endif