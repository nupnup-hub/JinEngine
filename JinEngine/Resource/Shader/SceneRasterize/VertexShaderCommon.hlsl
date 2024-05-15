/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once
#include"../Common/CommonConstantsStructureDefine.hlsl"
#include"../Common/MaterialData.hlsl"

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
 
 
 