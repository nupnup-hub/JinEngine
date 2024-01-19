#include"CBCameraDefine.hlsl"
#include"CBEngineDefine.hlsl"

//cbuffer && StructuredBuffer aligned 16byte
//dx12 cbuffer aligned 256byte

//constants buffer에서 array사용시 
//float4보다 작은 데이터 타입은 float4로 패킹된다.
//ex) float -> float4(valid, empty, empty, empty) ... cpu에서 buffer size계산시 주의하자.
struct MaterialData
{
	float4 albedoColor;
	float4x4 matTransform;
	float metallic;
	float roughness;
	uint albedoMapIndex;
	uint normalMapIndex;
	uint heightMapIndex;
	uint metallicMapIndex;
	uint roughnessMapIndex;
	uint ambientMapIndex; 
};
 
//48
cbuffer cbScenePass : register(b1)
{
	float sceneTotalTime;
	float sceneDeltaTime;
	uint directionalLitSt;
	uint directionalLitEd;
	uint pointLitSt;
	uint pointLitEd;
	uint spotLitSt;
	uint spotLitEd;
	uint rectLitSt;
	uint rectLitEd;
	uint scenePassPad00;
	uint scenePassPad01;
};  

#ifndef DEFERRED_SHADING  
//144
cbuffer cbObject : register(b3)
{
	float4x4 objWorld;
	float4x4 objTexTransform;
	uint objMaterialIndex;
	uint objPad00;
	uint objPad01;
	uint objPad02;
};
//16384
cbuffer cbSkinned : register(b4)
{
	float4x4 objBoneTransforms[256];
};
StructuredBuffer<MaterialData> materialData : register(t1);
#endif