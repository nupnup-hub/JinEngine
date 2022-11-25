
Texture2D depthMap	: register(t0);
RWTexture2D<float4> result	: register(u0);
SamplerState samLinearWrap	: register(s0);

// return (2.0 * n) / (f + n - z * (f - n));	
[numthreads(256, 1, 1)]
void CS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	float n = 1.0; // camera z near
	float f = 100.0; // camera z far
	float z = depthMap[int2(dispatchThreadID.x, dispatchThreadID.y)].r;
	float factor = 1 - ((2.0 * n) / (f + n - z * (f - n)));
	result[int2(dispatchThreadID.x, dispatchThreadID.y)] = float4(factor, factor, factor, 1);
} 