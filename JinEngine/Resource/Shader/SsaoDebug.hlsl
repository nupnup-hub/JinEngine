#include"SsaoCommon.hlsl"

Texture2D aoMap : register(t0);
RWTexture2D<float4> destTexture : register(u0);

[numthreads(DIMX, DIMY, DIMZ)]
void Visualize(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	float width = camDataPack.x;
	float height = camDataPack.y;
	if (width <= dispatchThreadID.x || height <= dispatchThreadID.y)
		return;
	
	uint2 index = uint2(dispatchThreadID.x, dispatchThreadID.y);
	float visibility = aoMap.Load(uint3(index, 0));
	destTexture[index] = float4(visibility, visibility, visibility, 1.0f);
}