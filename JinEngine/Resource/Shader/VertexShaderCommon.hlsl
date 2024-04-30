#pragma once
#include"CommonConstantsStructureDefine.hlsl"
#include"MaterialData.hlsl"

//cbuffer && StructuredBuffer aligned 16byte
//dx12 cbuffer aligned 256byte

//constants buffer에서 array사용시 
//float4보다 작은 데이터 타입은 float4로 패킹된다.
//ex) float -> float4(valid, empty, empty, empty) ... cpu에서 buffer size계산시 주의하자.
 
ConstantBuffer<SceneData> cbScene : register(b0);
ConstantBuffer<CameraData> cbCam : register(b1);
#ifndef DEFERRED_SHADING  
ConstantBuffer<ObjectData> cbObject : register(b2);
#ifdef SKINNED
ConstantBuffer<SkinnedData> cbSkinned  : register(b3);
#endif
StructuredBuffer<MaterialData> materialData : register(t1);
#endif
 
 
 