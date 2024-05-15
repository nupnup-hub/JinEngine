#pragma once
#include "Common.hlsl" 

Texture2D<float3> src : register(t0);       //linear color
StructuredBuffer<float> exposure : register(t1);
RWTexture2D<uint> outLuma : register(u0);

SamplerState samLinearClamp : register(s0);
cbuffer cbPass : register(b0)
{
	float2 invOutputSize;   
};
 
#ifndef DIMX
#define DIMX 8
#endif

#ifndef DIMY
#define DIMY 8
#endif

[numthreads(DIMX, DIMY, 1)]
void ExtractLuma(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // We need the scale factor and the size of one pixel so that our four samples are right in the middle
    // of the quadrant they are covering.
    float2 uv = dispatchThreadID.xy * invOutputSize;
    float2 offset = invOutputSize * 0.25f;

    // Use 4 bilinear samples to guarantee we don't undersample when downsizing by more than 2x
    float3 color1 = src.SampleLevel(samLinearClamp, uv + float2(-offset.x, -offset.y), 0).xyz;
    float3 color2 = src.SampleLevel(samLinearClamp, uv + float2(offset.x, -offset.y), 0).xyz;
    float3 color3 = src.SampleLevel(samLinearClamp, uv + float2(-offset.x, offset.y), 0).xyz;
    float3 color4 = src.SampleLevel(samLinearClamp, uv + float2(offset.x, offset.y), 0).xyz;

    // Compute average luminance
	float luma = RGBToLuminance(color1 + color2 + color3 + color4) * 0.25f;

    // Prevent log(0) and put only pure black pixels in Histogram[0]
    if (luma == 0.0)
    {
		outLuma[dispatchThreadID.xy] = 0;
	}
    else
    {
        const float minLog = exposure[4];
        const float rcpLogRange = exposure[7];
		float logLuma = saturate((log2(luma) - minLog) * rcpLogRange); // Rescale to [0.0, 1.0]
		outLuma[dispatchThreadID.xy] = logLuma * 254.0 + 1.0; // Rescale to [1, 255]

	}
}
