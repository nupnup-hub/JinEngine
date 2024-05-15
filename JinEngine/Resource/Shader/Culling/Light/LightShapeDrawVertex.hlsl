/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/

#pragma once
#include"LightShapeDrawCommon.hlsl"

struct VertexIn
{
#ifdef USE_VERTEX_PACK
	float4 posL : POSITION;
#else
	float3 posL : POSITION;
#endif
	uint id : SV_INSTANCEID;
};
struct VertexOut
{
	float3 posV : VIEW_POSITION;
	uint id : LIGHT_ID;
};

#ifdef POINT_LIGHT
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	uint index = vin.id ;
	float range = light[index].frustumFar * LIGHT_RANGE_OFFSET;
	 
	float4 posW = float4(vin.posL.xyz * range + light[index].midPosition, 1);
	vout.posV = mul(posW, cbCam.view);
	vout.id = vin.id;   
	return vout;
}
#elif SPOT_LIGHT
VertexOut VS(VertexIn vin)
{ 
	uint index = vin.id ;
	float range = light[index].frustumFar * LIGHT_RANGE_OFFSET;
 
	float angle = light[index].outerConeAngle;
	float XZscale = tan(angle) * range;  
	float3 posL = vin.posL.xyz;
	posL.xz *= XZscale; 
	posL.y *= range;

	if (length(posL) > range)
		posL = normalize(posL) * range;

	 //rotate
	float3 dir = light[index].direction;
	float3 initDir = float3(0, -1, 0);
	float rotAngle = acos(dot(initDir, dir));
	float rotCosAngle = cos(rotAngle); 
	if(abs(rotCosAngle) == 1.0f)		
		initDir.x = 0.000001f;
 
	float3 k = normalize(cross(initDir, dir)); 
	posL = posL * rotCosAngle +  cross(k, posL) * sin(rotAngle) + k * dot(k, posL) * (1.0f - rotCosAngle);

	VertexOut vout;
	float4 posW = float4(posL + light[index].position, 1);
	vout.posV = mul(posW, cbCam.view).xyz;
	vout.id = vin.id;

	return vout;
}
#elif RECT_LIGHT
VertexOut VS(VertexIn vin)
{
#ifdef HEMISPHERE
	uint index = vin.id ;
	float range = light[index].frustumFar * LIGHT_RANGE_OFFSET;
	
	float xScale = light[index].extents.x + range;
	float yScale = light[index].extents.y + range;
	
	float3 posL = vin.posL.xyz;
	//rotate x90
	posL = float3(posL.x, -posL.z, posL.y);
	posL.x *= xScale;
	posL.y *= yScale;
	posL.z *= range;
	posL = mul(posL, float3x3(light[index].axis[0], light[index].axis[1], light[index].axis[2]));
 
	VertexOut vout;
	float4 posW = float4(posL + light[index].origin, 1);
	vout.posV = mul(posW, cbCam.view).xyz;
	vout.id = vin.id;
	return vout;
#else
	VertexOut vout;
	uint index = vin.id ;
	float range = light[index].frustumFar * LIGHT_RANGE_OFFSET; 

	float4 posW = float4(vin.posL.xyz * range + light[index].origin, 1);
	vout.posV = mul(posW, cbCam.view);
	vout.id = vin.id;   
	return vout;
#endif
}
#endif