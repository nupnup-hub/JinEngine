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
	float3 posV = mul(vout.posH, camInvProj).xyz;
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
