
#pragma once 
#define USE_HBAO 
#include"Common.hlsl" 
 
Texture2D depthMap : register(t0);
sampler samPointClamp : register(s0);
 
struct PixelOut
{
	float z00 : SV_Target0;
	float z10 : SV_Target1;
	float z20 : SV_Target2;
	float z30 : SV_Target3; 
    float z01 : SV_Target4;
    float z11 : SV_Target5;
    float z21 : SV_Target6;
    float z31 : SV_Target7;
};

PixelOut DepthInterleave(VertexOut pin)
{
	PixelOut pout;

	pin.posH.xy = floor(pin.posH.xy) * 4.0f + (cbSlice.posOffset + 0.5f);
	pin.texC = pin.posH.xy * cbPass.camInvRtSize;

    // Gather sample ordering: (-,+),(+,+),(+,-),(-,-),
	float4 s0 = depthMap.GatherRed(samPointClamp, pin.texC);
	float4 s1 = depthMap.GatherRed(samPointClamp, pin.texC, int2(2, 0));

	pout.z00 = s0.w;
	pout.z10 = s0.z;
	pout.z20 = s1.w;
	pout.z30 = s1.z;

	pout.z01 = s0.x;
	pout.z11 = s0.y;
	pout.z21 = s1.x;
	pout.z31 = s1.y;

	return pout;
}