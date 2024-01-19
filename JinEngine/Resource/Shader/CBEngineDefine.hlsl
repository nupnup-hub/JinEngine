#pragma once
#ifndef CB_ENGINE_REIGSTER
#define CB_ENGINE_REIGSTER b0
#endif

//48
cbuffer cbEnginePass : register(CB_ENGINE_REIGSTER)
{
	float appTotalTime;
	float aapDeltaTime;
	int missingTextureIndex;
	int bluseNoiseTextureIndex;
	float2 bluseNoiseTextureSize;
	float2 invBluseNoiseTextureSize;
	int ltcMatTextureIndex;
	int ltcAmpTextureIndex;
	float clusterMinDepth;
	int paddPad00;
};