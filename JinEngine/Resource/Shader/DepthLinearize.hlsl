#define USE_HBAO
#include"SsaoCommon.hlsl"
#include"DepthFunc.hlsl"

Texture2D<float> depthMap : register(t0); 
float DepthLinearize(VertexOut pin) :SV_Target
{
	AddViewportOrigin(pin, viewPortTopLeft, camInvRtSize);
	return NdcToViewPZ(depthMap.Load(int3(pin.posH.xy, 0)), camNearFar.x, camNearFar.y);
}