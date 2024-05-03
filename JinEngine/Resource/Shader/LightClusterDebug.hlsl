#include"LightClusterCommon.hlsl"
#include"FullScreenTriangleVs.hlsl"
#include"DepthFunc.hlsl"

Texture2D depthMap : register(t0); 
//RWByteAddressBuffer startOffsetBuffer : register(u0);
ByteAddressBuffer startOffsetBuffer : register(t1);
StructuredBuffer<LinkedLightID> linkedLightList : register(t2);

cbuffer cbPass : register(b0)
{ 
	float4 passPack;		//x: near, y: far, z rt width, w rtHeight
};

static const float4 kRadarColors[14] =
{
	{ 0, 0.9255, 0.9255, 1 }, // cyan
	{ 0, 0.62745, 0.9647, 1 }, // light blue
	{ 0, 0, 0.9647, 1 }, // blue
	{ 0, 1, 0, 1 }, // bright green
	{ 0, 0.7843, 0, 1 }, // green
	{ 0, 0.5647, 0, 1 }, // dark green
	{ 1, 1, 0, 1 }, // yellow
	{ 0.90588, 0.75294, 0, 1 }, // yellow-orange
	{ 1, 0.5647, 0, 1 }, // orange
	{ 1, 0, 0, 1 }, // bright red
	{ 0.8392, 0, 0, 1 }, // red
	{ 0.75294, 0, 0, 1 }, // dark red
	{ 1, 0, 1, 1 }, // magenta
	{ 0.6, 0.3333, 0.7882, 1 }, // purple
};

float4 DebugDrawNumLightsPerTileRadarColors(uint nNumLightsInThisTile)
{ 
	const uint g_uMaxNumLightsPerTile = 352;
	 
    // black for no lights
	if (nNumLightsInThisTile == 0)
		return float4(0.0f, 0.0f, 0.0f, 1.0f);
    // light purple for reaching the max
	else if (nNumLightsInThisTile == g_uMaxNumLightsPerTile)
		return float4(0.847f, 0.745f, 0.921f, 1.0f);
    // white for going over the max
	else if (nNumLightsInThisTile > g_uMaxNumLightsPerTile)
		return float4(1.0f, 1.0f, 1.0f, 1.0f);
    // else use weather radar colors
	else
	{ 
        // use a log scale to provide more detail when the number of lights is smaller

        // want to find the base b such that the logb of g_uMaxNumLightsPerTile is 14
        // (because we have 14 radar colors)
		float fLogBase = exp2(0.07142857f * log2((float) g_uMaxNumLightsPerTile));

        // change of base
        // logb(x) = log2(x) / log2(b)
		uint nColorIndex = floor(log2((float) nNumLightsInThisTile) / log2(fLogBase));
		return kRadarColors[nColorIndex];
	}
} 
float4 PS(VertexOut pIn) : SV_TARGET
{  
	int2 screenPos = pIn.posH.xy;
	// Look up the light list for the cluster
	const float Near = passPack.x;
	const float Far = passPack.y;
	const float rtWidth = passPack.z;
	const float rtHeight = passPack.w;
	//const uint log2Tile = (uint) passPack.z; 
	const uint rangeX = rtWidth / CLUSTER_DIM_X;
	const uint rangeY = rtHeight / CLUSTER_DIM_Y;
	 
#ifdef LINEAR_DEPTH_DIST
	const float depth = LinearDepthOne(depthMap.Load(int3(screenPos, 0)).r, Near, Far);
	if(depth == 1.0f)
		return float4(1,1,1,1);
	int dep = int(depth * CLUSTER_DIM_Z);
#else
	const float min_depth = log2(NEAR_CLUST);
	const float max_depth = log2(Far);

	const float scale = 1.0f / (max_depth - min_depth) * (CLUSTER_DIM_Z - 1.0f);
	const float bias = 1.0f - min_depth * scale;
	//const float distnace = Far - Near;
	const float depth = NdcToViewPZ(depthMap.Load(int3(screenPos, 0)).r, Near, Far); 
	if (depth == Far)
		return float4(1, 1, 1, 1);

	int dep = int(max(log2(depth) * scale + bias, 0.0f));
#endif
		  
	uint numLights = 0;
	int2 clusterPos = int2(screenPos.x / rangeX, screenPos.y / rangeY);
	uint lightIndex = (startOffsetBuffer.Load((clusterPos.x + CLUSTER_DIM_X * clusterPos.y + CLUSTER_DIM_X * CLUSTER_DIM_Y * dep) * LINKED_LIST_INDEX_PER_BYTE) & CLUSTER_LIGHT_INVALID_ID);
	 
	//uint shade_bit = 1;
	//uint setindex = (shade_bit << 30) | (lightIndex & 0x3FFFFFFF);
	//uint temp;
	///startOffsetBuffer.InterlockedExchange((clusterPos.x + CLUSTER_DIM_X * clusterPos.y + CLUSTER_DIM_X * CLUSTER_DIM_Y * dep) * 4, setindex, temp);
		 
	LinkedLightID linkedLight;
	if (lightIndex != CLUSTER_LIGHT_INVALID_ID)
	{
		linkedLight = linkedLightList[lightIndex];
		//Rect light
		while ((linkedLight.lightID >> CLUSTER_LIGHT_ID_PER_BIT) == RECT_LIGHT_TYPE_VALUE)
		{
			++numLights;
			uint lightID = (linkedLight.lightID & CLUSTER_LIGHT_ID_RANGE);
			lightIndex = linkedLight.link;				
			if (lightIndex == CLUSTER_LIGHT_INVALID_ID)
				break;
			linkedLight = linkedLightList[lightIndex];
		}

		//Spot light
		while ((linkedLight.lightID >> CLUSTER_LIGHT_ID_PER_BIT) == SPOT_LIGHT_TYPE_VALUE)
		{
			++numLights;
			uint lightID = (linkedLight.lightID & CLUSTER_LIGHT_ID_RANGE);
			lightIndex = linkedLight.link;				
			if (lightIndex == CLUSTER_LIGHT_INVALID_ID)
				break;
			linkedLight = linkedLightList[lightIndex];
		}
			 
		//Point light
		while ((linkedLight.lightID >> CLUSTER_LIGHT_ID_PER_BIT) == POINT_LIGHT_TYPE_VALUE)
		{
			++numLights;
			uint lightID = (linkedLight.lightID & CLUSTER_LIGHT_ID_RANGE);
			lightIndex = linkedLight.link;
			if (lightIndex == CLUSTER_LIGHT_INVALID_ID)
				break;
			linkedLight = linkedLightList[lightIndex];
		}
	} 
	return DebugDrawNumLightsPerTileRadarColors(numLights);
} 