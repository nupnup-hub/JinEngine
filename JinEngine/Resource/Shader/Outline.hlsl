#include"DepthFunc.hlsl"

Texture2D<float> depthMap : register(t0);
Texture2D<uint2> stencilMap : register(t1);

sampler sam : register(s0);

cbuffer cbPass : register(b0)
{
	//0 is invalid color {0,0,0,}
	//1.. 7 is valid color 
	float4 colors[8];
	float threshold;
	float thickness;
	uint offset;
	uint width;
	uint height;
	uint passPad00;
	uint passPad01;
	uint passPad02;
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
	//has to -1 ~ 1 ndc range for transform screen pos
	vout.PosH = float4(vin.PosL, 1.0f);
	vout.TexC = vin.TexC;
	return vout;
}

static const float xFilter[9] = { -1,0,1,-2,0,2,-1,0,1 };
static const float yFilter[9] = { 1,2,1,0,0,0,-1,-2,-1 };

float4 PS(VertexOut pin) : SV_Target
{
	const int3 baseIndex = int3(pin.TexC.x * width, pin.TexC.y * height, 0);
	int colorIndex[9] =
	{
		stencilMap.Load(baseIndex + int3(-thickness, thickness, 0)).y,
		stencilMap.Load(baseIndex + int3(0, thickness, 0)).y,
		stencilMap.Load(baseIndex + int3(thickness, thickness, 0)).y,
		stencilMap.Load(baseIndex + int3(-thickness, 0, 0)).y,
		stencilMap.Load(baseIndex).y,
		stencilMap.Load(baseIndex + int3(thickness, 0, 0)).y,
		stencilMap.Load(baseIndex + int3(-thickness, -thickness, 0)).y,
		stencilMap.Load(baseIndex + int3(0, -thickness, 0)).y,
		stencilMap.Load(baseIndex + int3(thickness, -thickness, 0)).y
	};

	int maxCount = -1;
	int finalColorIndex = 0;

	int colorIndexCount[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	float gx = 0;
	float gy = 0;

	for (uint i = 0; i < 9; ++i)
	{
		const int index = colorIndex[i] - offset;
		float3 color = float3(0, 0, 0);
		if (index > 0)
		{
			++colorIndexCount[index];
			if (maxCount < colorIndexCount[index])
			{
				finalColorIndex = index;
				maxCount = colorIndexCount[index];
			}
			color = colors[index].xyz;
		}

		const float rate = length(dot(color, float3(0.299f, 0.587f, 0.114f)));
		gx += rate * xFilter[i];
		gy += rate * yFilter[i];
	}
	 
	const float colorDist = sqrt(gx * gx + gy * gy);
	if (colorDist >= threshold)
		return colors[finalColorIndex];
	else
		return float4(0, 0, 0, 0);
}


//normal edge
/*
	//don't make texture
	//21 load
	//9 cal normal
	//9 loop

	//make texture
	//--make texture part
	//4 load
	//4 cal normal
	//	pass data delay
	//--cal outline part
	//9 load
	//9 loop
	float dsV[21] =
	{
		ToLinearZValue(depthMap.Load(baseIndex + int3(2, 1, 0)), 1, 1000),		//0
		ToLinearZValue(depthMap.Load(baseIndex + int3(2, 0, 0)), 1, 1000),		//1
		ToLinearZValue(depthMap.Load(baseIndex + int3(2, -1, 0)), 1, 1000),		//2
		ToLinearZValue(depthMap.Load(baseIndex + int3(1, 2, 0)), 1, 1000),		//3
		ToLinearZValue(depthMap.Load(baseIndex + int3(1, 1, 0)), 1, 1000),		//4
		ToLinearZValue(depthMap.Load(baseIndex + int3(1, 0, 0)), 1, 1000),		//5
		ToLinearZValue(depthMap.Load(baseIndex + int3(1, -1, 0)), 1, 1000),		//6
		ToLinearZValue(depthMap.Load(baseIndex + int3(1, -2, 0)), 1, 1000),		//7
		ToLinearZValue(depthMap.Load(baseIndex + int3(0, 2, 0)), 1, 1000),		//8
		ToLinearZValue(depthMap.Load(baseIndex + int3(0, 1, 0)), 1, 1000),		//9
		ToLinearZValue(depthMap.Load(baseIndex), 1, 1000),						//10
		ToLinearZValue(depthMap.Load(baseIndex + int3(0, -1, 0)), 1, 1000),		//11
		ToLinearZValue(depthMap.Load(baseIndex + int3(0, -2, 0)), 1, 1000),		//12
		ToLinearZValue(depthMap.Load(baseIndex + int3(-1, 2, 0)), 1, 1000),		//13
		ToLinearZValue(depthMap.Load(baseIndex + int3(-1, 1, 0)), 1, 1000),		//14
		ToLinearZValue(depthMap.Load(baseIndex + int3(-1, 0, 0)), 1, 1000),		//15
		ToLinearZValue(depthMap.Load(baseIndex + int3(-1, -1, 0)), 1, 1000),	//16
		ToLinearZValue(depthMap.Load(baseIndex + int3(-1, -2, 0)), 1, 1000),	//17
		ToLinearZValue(depthMap.Load(baseIndex + int3(-2, 1, 0)), 1, 1000),		//18
		ToLinearZValue(depthMap.Load(baseIndex + int3(-2, 0, 0)), 1, 1000),		//19
		ToLinearZValue(depthMap.Load(baseIndex + int3(-2, -1, 0)), 1, 1000)		//20
	};

	// x[1] - x[0], y[1] - y[0]
	const uint colorfactor = 1000;
	const float3 normal[9] =
	{
		normalize(float3(-((dsV[9] - dsV[18]) * colorfactor), -((dsV[13] - dsV[15]) * colorfactor), 1.0f)),		//left up
		normalize(float3(-((dsV[10] - dsV[19]) * colorfactor), -((dsV[14] - dsV[16]) * colorfactor), 1.0f)),	//left mid
		normalize(float3(-((dsV[11] - dsV[20]) * colorfactor), -((dsV[15] - dsV[17]) * colorfactor), 1.0f)),	//left down
		normalize(float3(-((dsV[4] - dsV[14]) * colorfactor), -((dsV[8] - dsV[10]) * colorfactor), 1.0f)),		//up
		normalize(float3(-((dsV[5] - dsV[15]) * colorfactor), -((dsV[9] - dsV[11]) * colorfactor), 1.0f)),		//mid
		normalize(float3(-((dsV[6] - dsV[16]) * colorfactor), -((dsV[10] - dsV[12]) * colorfactor), 1.0f)),		//down
		normalize(float3(-((dsV[0] - dsV[9]) * colorfactor), -((dsV[3] - dsV[5]) * colorfactor), 1.0f)),		//right up
		normalize(float3(-((dsV[1] - dsV[10]) * colorfactor), -((dsV[4] - dsV[6]) * colorfactor), 1.0f)),		//right mid
		normalize(float3(-((dsV[2] - dsV[11]) * colorfactor), -((dsV[5] - dsV[7]) * colorfactor), 1.0f))		//right down
	};

	gx = 0;
	gy = 0;
	for (uint i = 0; i < 9; ++i)
	{
		const float rate = length(dot(normal[i], normal[4]));
		gx += rate * xFilter[i];
		gy += rate * yFilter[i];
	}
	const float normalDist = sqrt(gx * gx + gy * gy);
	if (colorDist > threshold || normalDist > threshold)
		return colors[finalColorIndex];
	else
		return float4(0, 0, 0, 0);
 */