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
struct VertexOut
{
	float4 posH : SV_POSITION;
	float2 texC : TEXCOORD;
#ifdef USE_TRIANGLE_VERTEX_DIR
	float3 dir : DIRECTION;
#endif
};

VertexOut FullScreenTriangleVS(uint VertexId : SV_VertexID)
{
	VertexOut vout = (VertexOut) 0.0f;
	vout.texC = float2((VertexId << 1) & 2, VertexId & 2);
	vout.posH = float4(vout.texC * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
#ifdef USE_TRIANGLE_VERTEX_DIR
	float3 posV = mul(vout.posH, InvProj).xyz;
	vout.dir = float3(posV.xy / posV.z, 1.0f);
#endif
	return vout;
}

//----------------------------------------------------------------------------------
void AddViewportOrigin(inout VertexOut vin, float2 viewPortTopLeft, float2 invRtSize)
{
	vin.posH.xy += viewPortTopLeft;
	vin.texC = vin.posH.xy * invRtSize;
}

//----------------------------------------------------------------------------------
void SubtractViewportOrigin(inout VertexOut vin, float2 viewPortTopLeft, float2 invRtSize)
{
	vin.posH.xy -= viewPortTopLeft;
	vin.texC = vin.posH.xy * invRtSize;
}
