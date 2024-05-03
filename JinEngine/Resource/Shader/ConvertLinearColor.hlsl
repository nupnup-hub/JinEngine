#include "PostProcessingCommon.hlsl"
Texture2D<float4> src : register(t0); 
RWTexture2D<float3> dst : register(u0);

cbuffer cbToLinearColorPass : register(b0)
{
	float2 textureSize;
	float2 invTextureSize;
};

#ifndef DIMX
#define DIMX 8
#endif

#ifndef DIMY
#define DIMY 8
#endif

[numthreads(DIMX, DIMY, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	if (textureSize.x <= dispatchThreadID.x || textureSize.y <= dispatchThreadID.y)
		return; 
	
#ifdef HDR_DISPLAY_MAPPING
	dst[dispatchThreadID.xy].xyz = RemoveREC709Curve(src[dispatchThreadID.xy].xyz);
	//dst[dispatchThreadID.xy].xyz = REC2020toREC709(RemoveREC2084Curve(src[dispatchThreadID.xy].xyz));
#else
	dst[dispatchThreadID.xy].xyz = RemoveSRGBCurve(src[dispatchThreadID.xy].xyz);
#endif
}
