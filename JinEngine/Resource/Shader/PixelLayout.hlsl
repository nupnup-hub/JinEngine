#include"VertexLayout.hlsl"
#if defined(FULL_SCREEN_QUAD)
struct PixelIn
{
	float4 posH    : SV_POSITION; 
	float3 dir : DIRECTION;
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
 