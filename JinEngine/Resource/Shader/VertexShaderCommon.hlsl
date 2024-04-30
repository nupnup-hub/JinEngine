#pragma once
#include"CommonConstantsStructureDefine.hlsl"
#include"MaterialData.hlsl"

//cbuffer && StructuredBuffer aligned 16byte
//dx12 cbuffer aligned 256byte

//constants buffer���� array���� 
//float4���� ���� ������ Ÿ���� float4�� ��ŷ�ȴ�.
//ex) float -> float4(valid, empty, empty, empty) ... cpu���� buffer size���� ��������.
 
ConstantBuffer<SceneData> cbScene : register(b0);
ConstantBuffer<CameraData> cbCam : register(b1);
#ifndef DEFERRED_SHADING  
ConstantBuffer<ObjectData> cbObject : register(b2);
#ifdef SKINNED
ConstantBuffer<SkinnedData> cbSkinned  : register(b3);
#endif
StructuredBuffer<MaterialData> materialData : register(t1);
#endif
 
 
 