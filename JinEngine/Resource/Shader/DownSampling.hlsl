Texture2D srcTexture : register(t0);
RWTexture2D<float4> destTexture : register(u0);
 
cbuffer cbDownSamplePass : register(b0)
{
	uint2 srcSize;					//8
	uint2 destSize;					//16
	uint srcMipLevel;  
};
 
SamplerState samLinearClamp : register(s0);
    
[numthreads(DIMX, DIMY, 1)]
void DownSample(int3 dispatchThreadID : SV_DispatchThreadID)
{
	if (destSize.x <= dispatchThreadID.x || destSize.y <= dispatchThreadID.y)
		return;
	 
	float4 sampleColor = float4(0, 0, 0, 0);
	const float2 texelSize = 1.0f / destSize;
#ifdef EXIST 
	const float2 uv = (dispatchThreadID.xy + 0.5f) * texelSize;
	sampleColor = srcTexture.SampleLevel(samLinearClamp, uv, srcMipLevel);
#elif WIDTH_DOWN
	const float2 uv = (dispatchThreadID.xy + uint2(0.25f, 0.5f)) * texelSize;
	const float2 offset = float2(0.5f, 0.0f) * texelSize;
	sampleColor = (srcTexture.SampleLevel(samLinearClamp, uv, srcMipLevel) +
	srcTexture.SampleLevel(samLinearClamp, uv + offset, srcMipLevel)) * 0.5f;
#elif HEIGHT_DOWN
	const float2 uv = (dispatchThreadID.xy + uint2(0.5f, 0.25f)) * texelSize;
	const float2 offset = float2(0.0f, 0.5f) * texelSize;
	sampleColor = (srcTexture.SampleLevel(samLinearClamp, uv, srcMipLevel) +
	srcTexture.SampleLevel(samLinearClamp, uv + offset, srcMipLevel)) * 0.5f;
#elif WIDTH_HEIGHT_DOWN
	const float2 uv = (dispatchThreadID.xy + uint2(0.25f, 0.25f)) * texelSize;
	const float2 offset = texelSize * 0.5f;
	sampleColor = (srcTexture.SampleLevel(samLinearClamp, uv, srcMipLevel) +
	srcTexture.SampleLevel(samLinearClamp, uv + float2(offset.x, 0), srcMipLevel) +
	srcTexture.SampleLevel(samLinearClamp, uv + float2(0, offset.y), srcMipLevel) +
	srcTexture.SampleLevel(samLinearClamp, uv + offset, srcMipLevel)) * 0.25f;
#endif 
	destTexture[dispatchThreadID.xy] = sampleColor;
}
  
