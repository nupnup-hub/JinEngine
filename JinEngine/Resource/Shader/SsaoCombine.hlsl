#include"SsaoCommon.hlsl"

Texture2DArray aoMap : register(t0);
Texture2D<float> depthMap : register(t1);
SamplerState samPointClamp : register(s0);
 
#ifdef USE_BLUR 
	typedef float2 PixelOut;
#else	
	typedef float PixelOut;
#endif

PixelOut SsaoCombine(VertexOut pin) : SV_Target
{
#ifndef USE_BLUR 
    SubtractViewportOrigin(pin, cbPass.viewPortTopLeft, cbPass.camInvRtSize);
#endif
	
	int2 fullResPos = int2(pin.posH.xy);
	int2 offset = fullResPos & 3;
	int sliceId = offset.y * 4 + offset.x;
	int2 quarterResPos = fullResPos >> 2;
	float ao = aoMap.Load(int4(quarterResPos, sliceId, 0));
	
#if USE_BLUR 
    float viewDepth = depthMap.Sample(samPointClamp, pin.texC);
    return float2(ao, viewDepth);
#else    
    return pow(saturate(ao), cbPass.sharpness);
#endif 
}
