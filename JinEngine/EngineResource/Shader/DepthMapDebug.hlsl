
Texture2D depthMap	: register(t0);
RWTexture2D<float4> result	: register(u0);
SamplerState samLinearWrap	: register(s0);

cbuffer cbSettings : register(b0)
{
	uint width;
	uint height; 
};

// return (2.0 * n) / (f + n - z * (f - n));	
/*
dim info
group 1, 512, 1
thread 512, 1, 1
*/
[numthreads(512, 1, 1)]
void CS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	float textureXFactor = dispatchThreadID.x;
	float textureYFactor = dispatchThreadID.y;
	uint maxPixelCount = width * height;

	if (textureXFactor >= width || textureYFactor >= height)
		return; 

	while (maxPixelCount > (textureXFactor + (textureYFactor * width)))
	{
		static const float n = 1.0; // camera z near
		static const float f = 1000; // camera z far

		float z = depthMap.Load(int3(textureXFactor, textureYFactor, 0));
		//float z = depthMap.SampleLevel(samLinearWrap, float2(textureXFactor / width, textureYFactor / height), 0).r;
		float factor = 1 - ((2.0f * n) / (f + n - z * (f - n))); 
		result[int2(textureXFactor, textureYFactor)] = float4(factor, factor, factor, 1);
		textureYFactor += height;
	}
} 