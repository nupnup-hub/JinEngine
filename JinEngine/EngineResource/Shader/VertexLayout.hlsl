
#if defined(DEBUG)
struct VertexIn
{
	float3 PosL    : POSITION; 
};
struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float3 PosW    : POSITION; 
};

#elif defined(SKY)
struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 TexC    : TEXCOORD;
};
struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 PosL : POSITION;
};

#elif defined(ALBEDO_MAP_ONLY)
struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 TexC    : TEXCOORD;
};
struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float3 PosW    : POSITION;
	float2 TexC    : TEXCOORD;
};

#elif defined(WRITE_SHADOW_MAP)
#if defined(STATIC)
struct VertexIn
{
	float3 PosL    : POSITION; 
};
struct VertexOut
{
	float4 PosH    : SV_POSITION; 
};
#elif defined(SKINNED)
struct VertexIn
{
	float3 PosL    : POSITION;
	float3 BoneWeights : WEIGHTS;
	uint4 BoneIndices  : BONEINDICES;
};
struct VertexOut
{
	float4 PosH    : SV_POSITION;
};
#endif

#elif defined(BOUNDING_OBJECT_DEPTH_TEST)
#if defined(STATIC)
struct VertexIn
{
	float3 PosL    : POSITION;
};
struct VertexOut
{
	float4 PosH    : SV_POSITION;
};
#elif defined(SKINNED)
struct VertexIn
{
	float3 PosL    : POSITION;
	float3 BoneWeights : WEIGHTS;
	uint4 BoneIndices  : BONEINDICES;
};
struct VertexOut
{
	float4 PosH    : SV_POSITION; 
};
#endif

#elif defined(STATIC)
struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 TexC    : TEXCOORD;
	float3 TangentL : TANGENT;
};
struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float3 PosW    : POSITION;
	float3 NormalW : NORMAL;
	float2 TexC    : TEXCOORD;
	float3 TangentW : TANGENT;
};

#elif defined(SKINNED)
struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 TexC    : TEXCOORD;
	float3 TangentL : TANGENT;
	float3 BoneWeights : WEIGHTS;
	uint4 BoneIndices  : BONEINDICES;
};
struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float3 PosW    : POSITION;
	float3 NormalW : NORMAL;
	float2 TexC    : TEXCOORD;
	float3 TangentW : TANGENT;
};
#else

#endif
 