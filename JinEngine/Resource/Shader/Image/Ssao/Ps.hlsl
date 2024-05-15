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
#include"../../Common/GBufferCommon.hlsl"
#include"../../Common/DepthFunc.hlsl"

#ifdef USE_SSAO_INTERLEAVE
Texture2DArray<float> depthMap : register(t0);
#else
Texture2D depthMap : register(t0);
#endif
Texture2D normal : register(t1);
Texture2D randomMap : register(t2); 
 
SamplerState samPointClamp : register(s0);
SamplerState samPointWrap : register(s1);
//SamplerState samDepth : register(s2);
 
#ifdef USE_SSAO_INTERLEAVE	
typedef float Result;		//ao
#elif USE_BLUR
typedef float2 Result;		//ao, depth
#else
typedef float Result;		//depth
#endif
 
float3 GetViewPos(float2 uv)
{
#if defined(USE_SSAO_INTERLEAVE) && defined(USE_HBAO)
	return UVToViewSpace(uv, depthMap.SampleLevel(samPointClamp, float3(uv, 0), 0), cbPass.uvToViewA, cbPass.uvToViewB);
#else
    return UVToViewSpace(uv, NdcToViewPZ(depthMap.SampleLevel(samPointClamp, uv, 0).r, cbPass.camNearMulFar, cbPass.camNearFar), cbPass.uvToViewA, cbPass.uvToViewB);
#endif
}
float InvLength(float2 v)
{
	return rsqrt(dot(v, v));
}
float3 MinDiff(float3 p, float3 pR, float3 pL)
{
	float3 v1 = pR - p;
	float3 v2 = p - pL;
	return (dot(v1, v1) < dot(v2, v2)) ? v1 : v2;
} 
float Falloff(float d2)
{
	return d2 * cbPass.negInvR2 + 1.0f;
}
float2 RotateDirections(float2 dir, float2 cosSin)
{
	return float2(dir.x * cosSin.x - dir.y * cosSin.y,
                  dir.x * cosSin.y + dir.y * cosSin.x);
}
#ifdef USE_SSAO
Result SsaoPs(VertexOut pin) : SV_Target
{ 
	float pz = NdcToViewPZ(depthMap.SampleLevel(samPointClamp, pin.texC, 0).r, cbPass.camNearMulFar, cbPass.camNearFar);
	float3 posV = GetViewPos(pin.texC); //RestructionPosition(pin.dir, pz);
 
	float3 normalW = UnpackNormal(normal.SampleLevel(samPointClamp, pin.texC, 0));
	float3 normalV = normalize(mul(normalW, (float3x3)cbPass.camView));
	  
	float2 noiseScale = cbPass.aoRtSize / SSAO_RANDOM_MAP_SIZE;
	float3 random = randomMap.SampleLevel(samPointWrap, pin.texC * noiseScale, 0).xyz * 2.0 - 1.0;
	float3x3 TBN = CalTBN(normalV, random);
	  
	float occlusionSum = 0;
	[unroll]
	for (int i = 0; i < SSAO_SAMPLE_COUNT; ++i)
	{
		float3 samplePos = mul(cbSample.sample[i].xyz, TBN); // from tangent to view-space
		float3 q = posV + samplePos * cbPass.radius;

		float4 projQ = mul(float4(q, 1.0f), cbPass.camProj);
		projQ.xy /= projQ.w;
		projQ.x = projQ.x * 0.5f + 0.5f;
		projQ.y = projQ.y * -0.5f + 0.5f;

		float rz = NdcToViewPZ(depthMap.SampleLevel(samPointClamp, projQ.xy, 0.0f).r, cbPass.camNearMulFar, cbPass.camNearFar);
		float rangeCheck = smoothstep(0.0f, 1.0f, cbPass.radius / abs(posV.z - rz));
		occlusionSum += (rz < (q.z + cbPass.bias) ? 1.0 : 0.0) * rangeCheck;
	}
			 
	const float access = 1.0f - (occlusionSum / SSAO_SAMPLE_COUNT); 
	
#ifdef USE_BLUR 
	return float2(saturate(access), pz);
#else	
	return saturate(pow(access, cbPass.sharpness));
#endif
}
#else
float ComputeAO(float3 p, float3 n, float3 s)
{
	float3 v = s - p;
	float vDotv = dot(v, v);
	float nDotv = dot(n, v) * rsqrt(vDotv);
	return saturate(nDotv - cbPass.tanBias) * saturate(Falloff(vDotv));
} 
 
Result HbaoPs(VertexOut pin) : SV_Target
{	
#ifdef USE_SSAO_INTERLEAVE
	pin.posH.xy = floor(pin.posH.xy) * 4.0f + cbSlice.posOffset;
	pin.texC = pin.posH.xy * (cbPass.aoInvQuaterRtSize / 4.0f);  
#endif
	  
	float3 centerPos = GetViewPos(pin.texC);
	float radiusPixel = cbPass.radiusToScreen / centerPos.z;
	// Calculate the projected size of the hemisphere
	if (radiusPixel < 1.0f)
		return 1.0f;
   
#ifdef USE_SSAO_INTERLEAVE
	float3 random = cbSlice.jitter.xyz;
#else
	float2 noiseScale = cbPass.aoRtSize / SSAO_RANDOM_MAP_SIZE;
	float3 random = randomMap.SampleLevel(samPointWrap, pin.texC * noiseScale, 0).xyz;
#endif
	 	   
#ifdef USE_SSAO_INTERLEAVE
	VertexOut pinC = pin;
	AddViewportOrigin(pinC, cbPass.viewPortTopLeft, cbPass.camInvRtSize); 
	float3 normalW = UnpackNormal(normal.SampleLevel(samPointClamp, pinC.texC, 0));
#else
    float3 normalW = UnpackNormal(normal.SampleLevel(samPointClamp, pin.texC, 0));
#endif 
    float3 normal = normalize(mul(normalW, (float3x3)cbPass.camView));
	
	const float alpha = 2.0 * PI / SSAO_DIR_COUNT;
	float smallScaleAO = 0;
	float largeScaleAO = 0;
#ifdef USE_SSAO_INTERLEAVE
	float stepSizePixels = (radiusPixel / 4.0f) / (SSAO_SAMPLE_COUNT + 1);
#else
	float stepSizePixels = radiusPixel / (SSAO_SAMPLE_COUNT + 1);
#endif
    [unroll]
	for (float directionIndex = 0; directionIndex < SSAO_DIR_COUNT; ++directionIndex)
	{
		float angle = alpha * directionIndex;
        // Compute normalized 2D direction
		float2 dir = RotateDirections(float2(cos(angle), sin(angle)), random.xy);
        // Jitter starting point within the first step
		float rayPixels = (random.z * stepSizePixels + 1.0);
		 
#ifdef USE_SSAO_INTERLEAVE	
		float2 snappedUV = round(rayPixels * dir) * cbPass.aoInvQuaterRtSize + pin.texC;
#else
		float2 snappedUV = round(rayPixels * dir) * cbPass.aoInvRtSize + pin.texC; 
#endif
		float3 samplePos = GetViewPos(snappedUV);	
		smallScaleAO += ComputeAO(centerPos, normal, samplePos);
		
        [unroll]
		for (float stepIndex = 1; stepIndex < SSAO_SAMPLE_COUNT; ++stepIndex)
		{
#ifdef USE_SSAO_INTERLEAVE	
			snappedUV = round(rayPixels * dir) * cbPass.aoInvQuaterRtSize + pin.texC;
#else
			snappedUV = round(rayPixels * dir) * cbPass.aoInvRtSize + pin.texC;
#endif 
			samplePos = GetViewPos(snappedUV);
			rayPixels += stepSizePixels;

			largeScaleAO += ComputeAO(centerPos, normal, samplePos);
		}
	}

	float ao = (smallScaleAO * cbPass.smallScaleAOAmount) + (largeScaleAO * cbPass.largeScaleAOAmount);
	ao /= (SSAO_DIR_COUNT * SSAO_SAMPLE_COUNT);
	ao = saturate(1.0 - ao * 2.0);

#ifdef USE_SSAO_INTERLEAVE	
	return ao;
#elif USE_BLUR
    return float2(ao, centerPos.z);
#else
	return pow(saturate(ao), cbPass.sharpness);
#endif 
 
} 
#endif

 