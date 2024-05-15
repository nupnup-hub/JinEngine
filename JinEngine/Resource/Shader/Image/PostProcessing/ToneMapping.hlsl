//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#pragma once
#include"Common.hlsl"

//
// Reinhard
// 
// The Reinhard tone operator.  Typically, the value of k is 1.0, but you can adjust exposure by 1/k.
// I.e. TM_Reinhard(x, 0.5) == TM_Reinhard(x * 2.0, 1.0)
float3 TM_Reinhard(float3 hdr, float k = 1.0)
{
	return hdr / (hdr + k);
}

// The inverse of Reinhard
float3 ITM_Reinhard(float3 sdr, float k = 1.0)
{
	return k * sdr / (k - sdr);
}

//
// Reinhard-Squared
//

// This has some nice properties that improve on basic Reinhard.  Firstly, it has a "toe"--that nice,
// parabolic upswing that enhances contrast and color saturation in darks.  Secondly, it has a long
// shoulder giving greater detail in highlights and taking longer to desaturate.  It's invertible, scales
// to HDR displays, and is easy to control.
//
// The default constant of 0.25 was chosen for two reasons.  It maps closely to the effect of Reinhard
// with a constant of 1.0.  And with a constant of 0.25, there is an inflection point at 0.25 where the
// curve touches the line y=x and then begins the shoulder.
//
// Note:  If you are currently using ACES and you pre-scale by 0.6, then k=0.30 looks nice as an alternative
// without any other adjustments.

float3 TM_ReinhardSq(float3 hdr, float k = 0.25)
{
	float3 reinhard = hdr / (hdr + k);
	return reinhard * reinhard;
}

float3 ITM_ReinhardSq(float3 sdr, float k = 0.25)
{
	return k * (sdr + sqrt(sdr)) / (1.0 - sdr);
}

//
// Stanard (New)
//

// This is the new tone operator.  It resembles ACES in many ways, but it is simpler to evaluate with ALU.  One
// advantage it has over Reinhard-Squared is that the shoulder goes to white more quickly and gives more overall
// brightness and contrast to the image.

float3 TM_Stanard(float3 hdr)
{
	return TM_Reinhard(hdr * sqrt(hdr), sqrt(4.0 / 27.0));
}

float3 ITM_Stanard(float3 sdr)
{
	return pow(ITM_Reinhard(sdr, sqrt(4.0 / 27.0)), 2.0 / 3.0);
}

//
// Stanard (Old)
//

// This is the old tone operator first used in HemiEngine and then MiniEngine.  It's simplistic, efficient,
// invertible, and gives nice results, but it has no toe, and the shoulder goes to white fairly quickly.
//
// Note that I removed the distinction between tone mapping RGB and tone mapping Luma.  Philosophically, I
// agree with the idea of trying to remap brightness to displayable values while preserving hue.  But you
// run into problems where one or more color channels end up brighter than 1.0 and get clipped.

float3 ToneMap(float3 hdr)
{
	return 1 - exp2(-hdr);
}

float3 InverseToneMap(float3 sdr)
{
	return -log2(max(1e-6, 1 - sdr));
}

float ToneMapLuma(float luma)
{
	return 1 - exp2(-luma);
}

float InverseToneMapLuma(float luma)
{
	return -log2(max(1e-6, 1 - luma));
}

//
// ACES
//

// The next generation of filmic tone operators.
float3 ToneMapACES(float3 hdr)
{
	const float A = 2.51, B = 0.03, C = 2.43, D = 0.59, E = 0.14;
	return saturate((hdr * (A * hdr + B)) / (hdr * (C * hdr + D) + E));
}

float3 InverseToneMapACES(float3 sdr)
{
	const float A = 2.51, B = 0.03, C = 2.43, D = 0.59, E = 0.14;
	return 0.5 * (D * sdr - sqrt(((D * D - 4 * C * E) * sdr + 4 * A * E - 2 * B * D) * sdr + B * B) - B) / (A - C * sdr);
} 
Texture2D<float3> src : register(t0);		//linear Color
RWTexture2D<float3> dst : register(u0);

#ifdef APPLY_EXPOSURE
StructuredBuffer<float> exposure : register(t1);
#endif
#ifdef APPLY_BLOOM
Texture2D<float3> bloom : register(t2);
SamplerState samLinearClamp : register(s0);
#endif
#ifdef OUT_LUMA 
RWTexture2D<float> outLuma : register(u1);
#endif

cbuffer cbTonePass : register(b0)
{
	float2 textureSize;
	float2 invTextureSize; 
	float paperWhiteRatio;			// PaperWhite / MaxBrightness
	float maxBrightness;   
	float bloomStrength;
	float tonePassPad00;
};
 
#ifndef DIMX
#define DIMX 8
#endif

#ifndef DIMY
#define DIMY 8
#endif

[numthreads(DIMX, DIMY, 1)]
void ToneMapping(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	if (textureSize.x <= dispatchThreadID.x || textureSize.y <= dispatchThreadID.y)
		return;
	
	float2 texCoord = (dispatchThreadID.xy + 0.5) * invTextureSize;
	float3 color = src[dispatchThreadID.xy].xyz;
#ifdef APPLY_BLOOM
	color +=  bloomStrength * bloom.SampleLevel(samLinearClamp, texCoord, 0);
#endif
#ifdef APPLY_EXPOSURE
	color *= exposure[0];
#endif  
	
#ifdef HDR_DISPLAY_MAPPING
	color = ToneMapACES(color * paperWhiteRatio) * maxBrightness;
	dst[dispatchThreadID.xy].xyz  = color;
#else	 
    // Tone map to SDR
	color = ToneMapACES(color);
	dst[dispatchThreadID.xy].xyz = color;
#endif
	
#ifdef OUT_LUMA
	outLuma[dispatchThreadID.xy] = RGBToLogLuminance(color);
#endif
}
 



