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
#include"Packing.hlsl"

#ifndef G_BUFFER_ALBEDO_COLOR
#define G_BUFFER_ALBEDO_COLOR 0
#endif
 
#ifndef G_BUFFER_LIGHT_PROP
#define G_BUFFER_LIGHT_PROP 1	            //x = specular color, y = metallic, z = roughness, w = ambientFactor
#endif

#ifndef G_BUFFER_NORMAL_AND_TANGENT
#define G_BUFFER_NORMAL_AND_TANGENT 2		//xyw = normal, z = tangent
#endif
  
#ifndef G_BUFFER_LAYER_COUNT
#define G_BUFFER_LAYER_COUNT 3
#endif

float4 InitialLightPropLayer()
{
    return float4(0, 0, 0, 0);
}
float4 InitialNormalTangentLayer()
{
    return float4(0, 0, 0, 0);
}
bool IsInvalidLightPropLayer(float4 v)
{
    return all(v == 0);
}
bool IsInvalidNormalLayer(float4 v)
{
    return all(v == 0);
}

float4 PackLightPropLayer(in float specularFactor, in float metalic, in float roughness, in float aoFactor)
{ 
    return float4(specularFactor, metalic, roughness, aoFactor);
}
void UnpackLightPropLayer(in float4 pack, out float specularFactor, out float metalic, out float roughness, out float aoFactor)
{
    specularFactor = pack.x;
    metalic = pack.y;
    roughness = pack.z;
    aoFactor = pack.w;
}
void UnpackLightPropLayer(in float4 pack, out float4 specularColor, out float metalic, out float roughness, out float aoFactor)
{ 
    float specularFactor = 0;
    UnpackLightPropLayer(pack, specularFactor, metalic, roughness, aoFactor);
    specularColor = float4(specularFactor, specularFactor, specularFactor, 1.0f);
}
void UnpackLightPropLayer(in float4 pack, out float4 specularColor, out float metalic, out float roughness)
{
    float specularFactor = 0;
    float aoFactor = 0;
    UnpackLightPropLayer(pack, specularFactor, metalic, roughness, aoFactor);
    specularColor = float4(specularFactor, specularFactor, specularFactor, 1.0f);
}
float UnpackMetalic(in float4 pack)
{
    float4 specularColor;
    float metalic;
    float roughness;
    UnpackLightPropLayer(pack, specularColor, metalic, roughness);
    return metalic;
}
float UnpackRoughness(in float4 pack)
{
    float4 specularColor;
    float metalic;
    float roughness;
    UnpackLightPropLayer(pack, specularColor, metalic, roughness);
    return roughness;
}

float4 PackNormalAndTangentLayer(in float3 normal, in float3 tangent)
{
    float3 encodeNormal = SignedOctEncode(normal);
	//원본 normal과 encde->decode된 normal에 차이는 미세하며 이는 그 normal을 이용해 encode->decode한 tangent도 마찬가지 이므로 
	//수정없이 그대로 사용한다.
    float encodeTangent = EncodeTangent(normal, normalize(tangent));
    return float4(encodeNormal.xy, encodeTangent, encodeNormal.z);
}
void UnpackNormalAndTangentLayer(in float4 pack, out float3 normal, out float3 tangent)
{
    normal = SignedOctDecode(pack.xyw);
    tangent = DecodeTangent(normal, pack.z);
} 
float3 UnpackNormal(in float4 pack)
{ 
    return SignedOctDecode(pack.xyw);
}

float3 RestructionPosition(float3 viewRay, float linearDepth)
{
    return viewRay * linearDepth;
}
float3 UVToViewSpace(float2 uv, float viewZ, float2 uvToViewA, float2 uvToViewB)
{
    uv = uvToViewA * uv + uvToViewB;
    return float3(uv * viewZ, viewZ);
}
float3 GetViewPos(float2 uv, float viewZ, float2 uvToViewA, float2 uvToViewB)
{
    return UVToViewSpace(uv, viewZ, uvToViewA, uvToViewB);
} 