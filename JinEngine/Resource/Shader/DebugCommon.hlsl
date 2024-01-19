#pragma once
#include"DepthFunc.hlsl"

#ifndef DIMX
#define DIMX 1
#endif
#ifndef DIMY
#define DIMY 1
#endif
#ifndef DIMZ
#define DIMZ 1
#endif 
 
Texture2D srcMap : register(t0);
RWTexture2D<float4> result : register(u0);
SamplerState samLinearWrap : register(s0);

cbuffer cbSettings : register(b0)
{
	uint width;
	uint height;
	float near;
	float far;
};