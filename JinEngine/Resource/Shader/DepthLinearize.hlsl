#define USE_HBAO
#include"SsaoCommon.hlsl"
#include"DepthFunc.hlsl"

Texture2D depthMap : register(t0); 
float DepthLinearize(VertexOut pin) :SV_Target
{
    AddViewportOrigin(pin, cbPass.viewPortTopLeft, cbPass.camInvRtSize);
    return NdcToViewPZ(depthMap.Load(int3(pin.posH.xy, 0)).x, cbPass.camNearFar.x, cbPass.camNearFar.y);
}