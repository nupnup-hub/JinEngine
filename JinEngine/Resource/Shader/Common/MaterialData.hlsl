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
 
#ifndef MISSING_TEXTURE_INDEX
#define MISSING_TEXTURE_INDEX 1
#endif
struct MaterialData
{
    float4 albedoColor;
    float4x4 matTransform;
    float metallic;
    float roughness;
    float specularFactor;
    uint albedoMapIndex;
    uint normalMapIndex;
    uint heightMapIndex;
    uint metallicMapIndex;
    uint roughnessMapIndex;
    uint ambientMapIndex;
    uint specularMapIndex;
    uint materialPad00;
    uint materialPad01;
};

float ComputeDefaultSpecularFactor(float3 albedoColor, float metallic)
{ 
    //to ruminance
    return dot(albedoColor.xyz * metallic, float3(0.212671, 0.715160, 0.072169));  
}
