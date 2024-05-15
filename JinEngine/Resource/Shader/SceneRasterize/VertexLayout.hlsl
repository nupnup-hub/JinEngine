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


#if defined(FULL_SCREEN_QUAD)
struct VertexIn
{
	float3 posL    : POSITION;
	float2 texC : TEXCOORD;
};
struct VertexOut
{
	float4 posH    : SV_POSITION; 
	float2 texC : TEXCOORD; 
};
#elif defined(FULL_SCREEN_TRIANGLE)
struct VertexIn
{
	float3 posL    : POSITION;
};
struct VertexOut
{
	float4 posH : SV_POSITION;
	float2 texC : TEXCOORD; 
};
#elif defined(DEBUG)
struct VertexIn
{
	float3 posL    : POSITION;
};
struct VertexOut
{
	float4 posH    : SV_POSITION;
	float3 posW    : POSITION;
};
#elif defined(SKY)
struct VertexIn
{
	float3 posL    : POSITION;
	float3 normalL : NORMAL;
	float2 texC    : TEXCOORD;
};
struct VertexOut
{
	float4 posH : SV_POSITION;
	float3 posL : POSITION;
};
#elif defined(ALBEDO_MAP_ONLY)
struct VertexIn
{
	float3 posL    : POSITION;
	float3 normalL : NORMAL;
	float2 texC    : TEXCOORD;
};
struct VertexOut
{
	float4 posH    : SV_POSITION;
	float3 posW    : POSITION;
	float2 texC    : TEXCOORD;
};
#elif defined(STATIC)
struct VertexIn
{
	float3 posL    : POSITION;
	float3 normalL : NORMAL;
	float2 texC    : TEXCOORD;
	float3 tangentL : TANGENT;
};
struct VertexOut
{
	float4 posH    : SV_POSITION;
	float3 posW    : POSITION;
	float3 normalW : NORMAL;
	float2 texC    : TEXCOORD;
	float3 tangentW : TANGENT;
};
#elif defined(SKINNED)
struct VertexIn
{
	float3 posL    : POSITION;
	float3 normalL : NORMAL;
	float2 texC    : TEXCOORD;
	float3 tangentL : TANGENT;
	float3 boneWeights : WEIGHTS;
	uint4 boneIndices  : BONEINDICES;
};
struct VertexOut
{
	float4 posH    : SV_POSITION;
	float3 posW    : POSITION;
	float3 normalW : NORMAL;
	float2 texC    : TEXCOORD;
	float3 tangentW : TANGENT;
}; 
#else
#endif

