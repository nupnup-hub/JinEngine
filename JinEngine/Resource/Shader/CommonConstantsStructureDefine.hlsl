#pragma once

 //cbuffer && StructuredBuffer aligned 16byte
//dx12 cbuffer aligned 256byte

//constants buffer에서 array사용시 
//float4보다 작은 데이터 타입은 float4로 패킹된다.
//ex) float -> float4(valid, empty, empty, empty) ... cpu에서 buffer size계산시 주의하자.
 
//48

struct SceneData 
{
    float appTotalTime;
    float appDeltaTime;
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
    int missingTextureIndex;
    int bluseNoiseTextureIndex;
    float2 bluseNoiseTextureSize;
    float2 invBluseNoiseTextureSize;
    float clusterMinDepth;
    int pad00;
};  
  
struct CameraData  
{
	//float4x4 camView;					//light shape drawing
    float4x4 invView; //pixel
	//float4x4 camProj;					//pixel	light shape drawing
	//float4x4 camInvProj;				//vertex
    float4x4 viewProj; //vertex
    float4x4 preViewProj; //pixel
	//float4x4 camInvViewProj;
	//float4x4 camViewProjTex;
    float2 renderTargetSize;
    float2 invRenderTargetSize;
    float2 uvToViewA;
    float2 uvToViewB;
    float3 eyePosW;
    float nearZ;
    float FarZ;
    uint csmLocalIndex; //aligned by registered time
    int hasAoTexture;
    uint pad00;
}; 
 
struct ObjectData 
{
    float4x4 world;
    float4x4 texTransform;
    uint materialIndex;
    uint pad00;
    uint pad01;
    uint pad02;
};

struct SkinnedData 
{
    float4x4 boneTransforms[256];
};