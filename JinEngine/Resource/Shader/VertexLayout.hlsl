
#if defined(FULL_SCREEN_QUAD)
struct VertexIn
{
	float3 posL    : POSITION;
};
struct VertexOut
{
	float4 posH    : SV_POSITION; 
	float3 dir : DIRECTION;
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

