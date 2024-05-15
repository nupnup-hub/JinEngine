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

struct GsIn
{
	float3 posV : VIEW_POSITION;
	uint id : LIGHT_ID;
};
struct GsOut
{
	float4 posH : SV_POSITION;
#ifdef USE_VERTEX_PACK
	nointerpolation uint4 vertexPos : VERTEX_POSITION;
	nointerpolation uint veretex2PosZ : VERTEX_POSITION_Z;
#else
	nointerpolation float3 vertexPosV[3] : VERTEX_POSITION;
#endif
	nointerpolation uint rtIndex : SV_RenderTargetArrayIndex; 
};
 
[maxvertexcount(3)]
void GS(triangle GsIn input[3], inout TriangleStream<GsOut> stream)
{
	GsOut gsOut; 
	gsOut.rtIndex = input[0].id;
#ifdef USE_VERTEX_PACK
	uint3 pack_value0 = f32tof16(float3(input[0].posV.xy, -input[0].posV.z));
	uint3 pack_value1 = f32tof16(float3(input[1].posV.xy, -input[1].posV.z));
	uint3 pack_value2 = f32tof16(float3(input[2].posV.xy, -input[2].posV.z));

	gsOut.vertexPos.x = (pack_value0.x << 16) | (pack_value0.y);
	gsOut.vertexPos.y = (pack_value0.z << 16) | (pack_value1.x);
	gsOut.vertexPos.z = (pack_value1.y << 16) | (pack_value1.z);
	gsOut.vertexPos.w = (pack_value2.x << 16) | (pack_value2.y);
	gsOut.veretex2PosZ = (pack_value2.z);
#else	
	gsOut.vertexPosV[0] = input[0].posV;
	gsOut.vertexPosV[1] = input[1].posV;
	gsOut.vertexPosV[2] = input[2].posV;
#endif  
	gsOut.posH = mul(float4(input[0].posV, 1.0f), cbCam.proj);
	stream.Append(gsOut);
    gsOut.posH = mul(float4(input[1].posV, 1.0f), cbCam.proj);
	stream.Append(gsOut);
    gsOut.posH = mul(float4(input[2].posV, 1.0f), cbCam.proj);
	stream.Append(gsOut);
}

