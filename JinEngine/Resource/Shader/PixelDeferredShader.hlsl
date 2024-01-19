#include "PixelLayout.hlsl"
#include "ShadowCompute.hlsl"
 
typedef float4 PixelOut;
PixelOut PS(PixelIn pin) :SV_Target
{   
	int2 samplePos = int2(pin.posH.xy);
	float depth = depthMap.Load(int3(samplePos, 0)).r; 
	
	float4 albedoColor = gBuffer[G_BUFFER_ALBEDO_COLOR].Load(int3(samplePos, 0));
	float4 normalG = gBuffer[G_BUFFER_NORMAL].Load(int3(samplePos, 0));
	 
	//early return albedo only case
	if (length(normalG) == 0)
		return albedoColor;
	
	float3 normalW = DecodeOct(normalG.xy);
	float gBufferFlag = normalG.w;
	 
	if (gBufferFlag == G_BUFFER_ALBEDO_ONLY_TYPE)
		return albedoColor;
	else
	{ 
		float linearDepth = camProj[3][2] / (depth - camProj[2][2]); 
		float3 posW = RestructionPosition(pin.dir, linearDepth);
		posW = mul(float4(posW, 1.0f), camInvView).xyz;
		//mul(float4(posV, 1.0f), camInvView).xyz;
		
		float3 tangentW = DecodeOct(gBuffer[G_BUFFER_TANGENT].Load(int3(samplePos, 0)).xy);
		float4 lightProp = gBuffer[G_BUFFER_LIGHT_PROP].Load(int3(samplePos, 0));

		float3 toEyeW = normalize(camEyePosW - posW);
		Material mat = { albedoColor, lightProp.x, lightProp.y };
#ifdef LIGHT_CLUSTER
		float3 directLight = ComputeLight(mat, posW, normalW, tangentW, toEyeW, samplePos, depth);
#else
		float3 directLight = ComputeLight(mat, posW, normalW, tangentW, toEyeW);
#endif 
		return float4(directLight, albedoColor.a);
		//return float4(normalW, 1.0f);
	}
}