
struct MaterialData
{
	float4 albedoColor;
	float4x4 matTransform;
	float metallic;
	float roughness;
	uint albedoMapIndex;
	uint normalMapIndex;
	uint heightMapIndex;
	uint roughnessMapIndex;
	uint ambientMapIndex;
	uint materialObjPad;
};

//144
cbuffer cbObject : register(b0)
{
	float4x4 objWorld;
	float4x4 objTexTransform;
	uint objMaterialIndex;
	uint objPad00;
	uint objPad01;
	uint objPad02;
};

//16384
cbuffer cbSkinned : register(b1)
{
	float4x4 objBoneTransforms[256];
};

//432 => 240
cbuffer cbCamera : register(b4)
{
	//float4x4 camView;
	//float4x4 camInvView;
	float4x4 camProj;
	float4x4 camProjTex;
	//float4x4 camInvProj;
	float4x4 camViewProj;
	//float4x4 camViewProjTex;
	//float4x4 camInvViewProj;
	float2 camRenderTargetSize;
	float2 camInvRenderTargetSize;
	float3 camEyePosW;
	float camNearZ;
	float camFarZ;
	uint csmLocalIndex; //aligned by registered time
	int cameraPad00;
	uint cameraPad01;
};

StructuredBuffer<MaterialData> materialData : register(t1);