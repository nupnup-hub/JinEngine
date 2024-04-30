#include "PostProcessingCommon.hlsl"
Texture2D<float3> src : register(t0);
Texture2D<float4> ori : register(t1);
RWTexture2D<float4> dst : register(u0);

cbuffer cbToDisplayColorPass : register(b0)
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
	
	float2 texCoord = (dispatchThreadID.xy + 0.5) * invTextureSize;
	float3 color = src[dispatchThreadID.xy];
#ifdef HDR_DISPLAY_MAPPING
	dst[dispatchThreadID.xy].xyz = ApplyREC709Curve(color);  
	//dst[dispatchThreadID.xy].xyz = ApplyREC2084Curve(REC709toREC2020(color));
#else
	dst[dispatchThreadID.xy].xyz = ApplySRGBCurve(color);
#endif
	dst[dispatchThreadID.xy].w = ori[dispatchThreadID.xy].w;
}
