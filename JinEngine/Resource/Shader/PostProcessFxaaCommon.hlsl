cbuffer cbFxaaPass : register(b0)
{
	float2 rcpTextureSize;
	float contrastThreshold; // default = 0.2, lower is more expensive
	float subpixelRemoval; // default = 0.75, lower blurs less
	uint lastQueueIndex;
	uint2 startPixel; 
}