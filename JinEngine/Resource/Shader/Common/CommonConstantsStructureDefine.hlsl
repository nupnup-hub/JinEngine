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