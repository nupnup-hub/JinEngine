#include"DepthFunc.hlsl"

Texture2D<uint2> dsMap : register(t0); 

cbuffer cbPass : register(b0)
{ 
	float4 colors[7];
	uint offset;
	uint width;
	uint height;
	uint outlinePad00;
};

struct VertexIn
{
	float3 PosL    : POSITION;
	float2 TexC    : TEXCOORD;
};
struct VertexOut
{
	float4 PosH    : SV_POSITION; 
	float2 TexC    : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;  
	vout.PosH = float4(vin.PosL, 1.0f);
	vout.TexC = vin.TexC;
	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{ 
	//uint2 value = dsMap.Sample(sam, pin.TexC);
	uint2 value = dsMap.Load(int3(pin.TexC.x * width, pin.TexC.y * height, 0));
 
	return float4(value.y, value.y, value.y, 1);
	/*
		float4 value = dsMap.Sample(sam, pin.TexC);
	//float4 value = dsMap.Load(int3(pin.TexC.x * width, pin.TexC.y * height, 0));
	float z = 1 - ToLinearZValue(value.r, 1, 1000);
	return float4(z, z, z, 1.0f);
	*/
}