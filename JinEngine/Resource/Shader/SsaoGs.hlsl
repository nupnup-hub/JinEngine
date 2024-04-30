#pragma once  
#include"SsaoCommon.hlsl"

#ifndef USE_HBAO
#define USE_HBAO
#endif 

struct GSOut
{
	float4 posH : SV_POSITION;
	float2 texC : TEXCOORD;
	uint LayerIndex : SV_RenderTargetArrayIndex;
};

[maxvertexcount(3)]
void SsaoGs(triangle VertexOut input[3], inout TriangleStream<GSOut> stream)
{
	GSOut gsOut;
    gsOut.LayerIndex = cbSlice.uSliceIndex;

    [unroll]
	for (int i = 0; i < 3; i++)
	{
		gsOut.texC = input[i].texC;
		gsOut.posH = input[i].posH;
		stream.Append(gsOut);
	}
}