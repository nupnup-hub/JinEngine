#include"DepthFunc.hlsl"
#include"SsaoCommon.hlsl"
#include"Math.hlsl"

Texture2D aoMap : register(t0); 
Texture2D depthMap : register(t1); 
 
SamplerState samPointClamp : register(s0);
SamplerState samLinearClamp : register(s1); 

#ifdef BLUR_X 
typedef float2 Result;		//ao, depth
#else	
typedef float Result;		//depth
#endif
 
float CrossBilateralWeight(float r, float z, float z0)
{
	const float blurSigma = (KERNEL_RADIUS + 1.0f) * 0.5f;
	const float blurFalloff = 1.0f / (2.0f * blurSigma * blurSigma);

	float dz = z0 - z;
	return exp2(-r * r * blurFalloff - dz * dz);
}

void ProcessRadius(float r0, float centerDepth, float2 centerUv, float2 deltaUv, inout float totalAO, inout float totalWeight)
{
#if 1
	float r = r0;

    [unroll]
    for (; r <= KERNEL_RADIUS/2; r += 1)
    {
		float2 uv = r * deltaUv + centerUv;
		float2 aoz = aoMap.SampleLevel(samPointClamp, uv, 0).xy;
		float ao = aoz.x;
		float z = aoz.y;
		float w = CrossBilateralWeight(r, z, centerDepth);
		 
		totalAO += ao * w;
		totalWeight += w;
	}
	 
    //[unroll(KERNEL_RADIUS)]
	for (; r <= KERNEL_RADIUS; r += 2)
	{
		float2 uv = (r + 0.5f) * deltaUv + centerUv;
		float2 aoz = aoMap.SampleLevel(samLinearClamp, uv, 0).xy;
		float ao = aoz.x;
		float z = aoz.y;
		float w = CrossBilateralWeight(r, z, centerDepth);
		 
		totalAO += ao * w;
		totalWeight += w;
	}
#else
    [unroll]
	for (float r = r0; r <= KERNEL_RADIUS; r += 1)
	{
		float2 uv = r * deltaUv + centerUv;
		float ao = aoMap.SampleLevel(samPointClamp, uv, 0);
		float z = LinearDepth(depthMap.SampleLevel(samPointClamp, uv, 0).r);
		float w = CrossBilateralWeight(r, z, centerDepth);
		 
		totalAO += ao * w;
		totalWeight += w;
	}
#endif 

}
#ifdef BLUR_X
Result BilateralBlurX(VertexOut pin) : SV_Target
{
	float2 aoz = aoMap.SampleLevel(samPointClamp, pin.texC, 0).xy;
	float centerDepth = aoz.y;

	float w = 1.0;
	float totalAo = aoz.x * w;
	float totalWeight = w;
	float i = 1.0;
	
	ProcessRadius(i, centerDepth, pin.texC, float2(cbPass.camInvRtSize.x, 0), totalAo, totalWeight);
	ProcessRadius(i, centerDepth, pin.texC, float2(-cbPass.camInvRtSize.x, 0), totalAo, totalWeight); 
	return totalAo / totalWeight;
}
#else
Result BilateralBlurY(VertexOut pin) : SV_Target
{
    SubtractViewportOrigin(pin, cbPass.viewPortTopLeft, cbPass.camInvRtSize);
	float2 aoz = aoMap.SampleLevel(samPointClamp, pin.texC, 0).xy;
	float centerDepth = aoz.y;

	float w = 1.0;
	float totalAo = aoz.x * w;
	float totalWeight = w;
	float i = 1.0; 
	
	ProcessRadius(i, centerDepth, pin.texC, float2(0, cbPass.camInvRtSize.y), totalAo, totalWeight);
	ProcessRadius(i, centerDepth, pin.texC, float2(0, -cbPass.camInvRtSize.y), totalAo, totalWeight);
    return pow(saturate(totalAo / totalWeight), cbPass.sharpness);
}
#endif