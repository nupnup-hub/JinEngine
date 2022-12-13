#include"DepthFunc.hlsl"

Texture2D<uint> depthMap : register(t0);
Texture2D<uint2> stencilMap : register(t1);

sampler sam : register(s0);

cbuffer cbPass : register(b0)
{
	//0 is invalid color {0,0,0,}
	//1.. 7 is valid color
	float4x4 world;
	float4 colors[8];
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
	vout.PosH = mul(float4(vin.PosL, 1.0f), world);
	vout.TexC = vin.TexC;
	return vout;
}

static const float xFilter[9] = { -1,0,1,-2,0,2,-1,0,1 };
static const float yFilter[9] = { 1,2,1,0,0,0,-1,-2,-1 };

float4 PS(VertexOut pin) : SV_Target
{
	const int3 baseIndex = int3(pin.TexC.x * width, pin.TexC.y * height, 0);
	const int thickness = 2;

	int colorIndex[9] =
	{
		stencilMap.Load(baseIndex + int3(-thickness, -thickness, 0)).y,
		stencilMap.Load(baseIndex + int3(0, thickness, 0)).y,
		stencilMap.Load(baseIndex + int3(thickness, -thickness, 0)).y,
		stencilMap.Load(baseIndex + int3(-thickness, 0, 0)).y,
		stencilMap.Load(baseIndex).y,
		stencilMap.Load(baseIndex + int3(thickness, 0, 0)).y,
		stencilMap.Load(baseIndex + int3(-thickness, thickness, 0)).y,
		stencilMap.Load(baseIndex + int3(0, thickness, 0)).y,
		stencilMap.Load(baseIndex + int3(thickness, thickness, 0)).y
	};

	int maxCount = -1;
	int finalColorIndex = 0;

	int colorIndexCount[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	float gx = 0;
	float gy = 0;

	for (uint i = 0; i < 9; ++i)
	{
		const int index = colorIndex[i] - offset;
		++colorIndexCount[index];
		if (maxCount < colorIndexCount[index] && index != 0)
		{
			finalColorIndex = index;
			maxCount = colorIndexCount[index];
		}

		const float rate = length(dot(colors[index].xyz, float3(0.299f, 0.587f, 0.114f)));
		gx += rate * xFilter[i];
		gy += rate * yFilter[i];
	}

	const float dist = pow(sqrt(gx * gx + gy * gy) / 2.0f, 2.);
	return float4(dist, dist, dist, 1.0f);
}