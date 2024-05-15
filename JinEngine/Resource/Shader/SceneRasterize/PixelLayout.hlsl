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


#include"VertexLayout.hlsl"
#if defined(FULL_SCREEN_QUAD)
struct PixelIn
{
	float4 posH : SV_POSITION; 
	float2 texC : TEXCOORD; 
};
#elif defined(FULL_SCREEN_TRIANGLE)
struct PixelIn
{
	float4 posH : SV_POSITION;
	float2 texC : TEXCOORD; 
};
#elif defined(DEBUG)
struct PixelIn
{
	float4 posH    : SV_POSITION;
	float3 posW    : POSITION;
};
#elif defined(SKY)
struct PixelIn
{
	float4 posH : SV_POSITION;
	float3 posL : POSITION;
};
#elif defined(ALBEDO_MAP_ONLY)
struct PixelIn
{
	float4 posH    : SV_POSITION;
	float3 posW    : POSITION;
	float2 texC    : TEXCOORD;
};
#else
struct PixelIn
{
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float3 normalW : NORMAL;
	float2 texC : TEXCOORD;
	float3 tangentW : TANGENT;
};
#endif
 