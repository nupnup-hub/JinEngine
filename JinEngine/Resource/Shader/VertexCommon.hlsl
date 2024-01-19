#include"CBCameraDefine.hlsl"
#include"CBEngineDefine.hlsl"

//cbuffer && StructuredBuffer aligned 16byte
//dx12 cbuffer aligned 256byte

//constants buffer���� array���� 
//float4���� ���� ������ Ÿ���� float4�� ��ŷ�ȴ�.
//ex) float -> float4(valid, empty, empty, empty) ... cpu���� buffer size���� ��������.
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