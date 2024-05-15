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
#include"PixelLayout.hlsl" 
#include"PixelShaderCommon.hlsl"
#include"../Common/ColorSpaceUtility.hlsl"

struct PixelOut
{
	float4 albedoColor : SV_TARGET0;			//8,8,8,8 unorm 
    float4 lightingProp : SV_TARGET1;			//8,8,8,8 unorm, x = specular, y = metallic, z =roughness, w = ambient
	float4 normalAndTangent : SV_TARGET2;		//10,10,10,2 unorm	(nx, ny, tx, nz) 
};
 
#if defined(DEBUG)
PixelOut PS(PixelIn pin) : SV_Target
{
	PixelOut pOut;
	pOut.albedoColor = materialData[cbObject.materialIndex].albedoColor; 
	pOut.lightingProp = InitialLightPropLayer();
	pOut.normalAndTangent = InitialNormalTangentLayer();
	return pOut;
}
#elif defined(SKY)
PixelOut PS(PixelIn pin) : SV_Target
{ 
	PixelOut pOut;
	pOut.albedoColor = materialData[cbObject.materialIndex].albedoColor;
	pOut.albedoColor *=	cubeMap[materialData[cbObject.materialIndex].albedoMapIndex].Sample(samLinearWrap, pin.posL);
	pOut.lightingProp = InitialLightPropLayer();
	pOut.normalAndTangent = InitialNormalTangentLayer();
	return pOut; 
}
#elif defined(ALBEDO_MAP_ONLY)
PixelOut PS(PixelIn pin) : SV_Target
{ 
	PixelOut pOut;
	pOut.albedoColor = materialData[cbObject.materialIndex].albedoColor;
	pOut.albedoColor *= textureMaps[materialData[cbObject.materialIndex].albedoMapIndex].Sample(samAnisotropicWrap, pin.texC); 
	pOut.lightingProp = InitialLightPropLayer();
	pOut.normalAndTangent = InitialNormalTangentLayer();
	return pOut;
}
#else
PixelOut PS(PixelIn pin) : SV_Target
{
	PixelOut pOut; 
	MaterialData matData = materialData[cbObject.materialIndex];
	float4 albedoColor = matData.albedoColor; 
#ifdef ALBEDO_MAP
	albedoColor *= textureMaps[matData.albedoMapIndex].Sample(samAnisotropicWrap, pin.texC);
#endif
 
	float3 normalW = normalize(pin.normalW);
	float3 toEyeW = normalize(cbCam.eyePosW - pin.posW);
	float2 texC = pin.texC;
 	
#ifdef NORMAL_MAP
	float3x3 TBN = CalTBN(normalW, pin.tangentW);
#ifdef HEIGHT_MAP
	float3 tangentSpaceViewDir = normalize(mul(TBN, toEyeW));
	texC = saturate(ApplyParallaxOffset(textureMaps[ matData.normalMapIndex], textureMaps[matData.heightMapIndex], texC, tangentSpaceViewDir)); 
#endif
	float4 normalMapSample = textureMaps[matData.normalMapIndex].Sample(samAnisotropicWrap, texC);
    float3 normalT = 2.0f * normalMapSample.rgb - 1.0f;
	normalW =  normalize(mul(normalT, TBN));
#endif 
		 
#ifdef METALLIC_MAP
	float metallic = textureMaps[matData.metallicMapIndex].Sample(samLinearWrap, texC).x;
#else
	float metallic = matData.metallic;
#endif
#ifdef ROUGHNESS_MAP
	float roughness = textureMaps[matData.roughnessMapIndex].Sample(samLinearWrap, texC).x;
#else
#ifdef NORMAL_MAP
	float roughness = matData.roughness * normalMapSample.a;
#else
	float roughness = matData.roughness;
#endif
#endif
 
	//deferred update후 추가필요.
#ifdef AMBIENT_OCCLUSION_MAP
	float ambientFactor = textureMaps[matData.ambientMapIndex].Sample(samLinearWrap, texC).x;	 
#else
	float ambientFactor = 1.0f;
#endif 
  		
    float specularFactor = 0.0f;
#ifdef SPECULAR_MAP	
	specularFactor = RGBToLuminance(textureMaps[matData.specularMapIndex].Sample(samAnisotropicWrap, texC).xyz * metallic);
#else
    specularFactor = ComputeDefaultSpecularFactor(albedoColor.xyz, metallic);
#endif
	 
	pOut.albedoColor = albedoColor; 
    pOut.lightingProp = PackLightPropLayer(specularFactor, metallic, roughness, ambientFactor);
    pOut.normalAndTangent = PackNormalAndTangentLayer(normalW, normalize(pin.tangentW));
	return pOut;
}
#endif