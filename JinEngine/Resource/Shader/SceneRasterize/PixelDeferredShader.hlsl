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


#include"PixelLayout.hlsl" 
#include"ShadowCompute.hlsl"
  
typedef float4 PixelOut;
PixelOut PS(PixelIn pin) : SV_Target
{
    int2 samplePos = int2(pin.posH.xy);
    int3 mapLocation = int3(samplePos, 0);
    float4 albedoColor = gBuffer[G_BUFFER_ALBEDO_COLOR].Load(mapLocation);
    float4 lightProp = gBuffer[G_BUFFER_LIGHT_PROP].Load(mapLocation);
    float4 normalAndTangent = gBuffer[G_BUFFER_NORMAL_AND_TANGENT].Load(mapLocation);
     
    if (IsInvalidLightPropLayer(lightProp) && IsInvalidNormalLayer(normalAndTangent))
        return albedoColor;
	
    float depth = depthMap.Load(mapLocation).r;
	//early return albedo only case
    if (length(normalAndTangent) == 0)
        return albedoColor;
	   
    float3 normalW;
    float3 tangentW;
    UnpackNormalAndTangentLayer(normalAndTangent, normalW, tangentW);
    
    //float linearDepth = camProj[3][2] / (depth - camProj[2][2]);
	//float3 posW = RestructionPosition(pin.dir, linearDepth); 
    //	return ((far * near) / (far - v * (far - near)) - near) / (far - near);
    float viewZ = NdcToViewPZ(depth); 
    float3 posV = UVToViewSpace(pin.texC, viewZ, cbCam.uvToViewA, cbCam.uvToViewB);
    float3 posW = mul(float4(posV, 1.0f), cbCam.invView).xyz;
    float3 toEyeW = normalize(cbCam.eyePosW - posW);
       
    float4 specularColor;
    float metalic;
    float roughness;
    float aoFactor;   
    UnpackLightPropLayer(lightProp, specularColor, metalic, roughness, aoFactor);
     
    Material mat = { albedoColor, specularColor, metalic, roughness, 0.0f };
#ifdef LIGHT_CLUSTER
	float3 directLight = ComputeLight(mat, posW, normalW, tangentW, toEyeW, samplePos, depth);
#else
    float3 directLight = ComputeLight(mat, posW, normalW, tangentW, toEyeW);
#endif  
    if (cbCam.hasAoTexture)
        aoFactor = ambientOcclusionMap.Sample(samLinearWrap, pin.texC);
     
#ifdef GLOBAL_ILLUMINATION
     directLight = CombineGlobalLight(directLight, mat.albedoColor.xyz, giMap.Load(mapLocation).xyz, aoFactor);
#else 
     directLight = CombineApproxGlobalLight(directLight, mat.albedoColor.xyz, aoFactor);
#endif
		
    return float4(directLight, albedoColor.a);
	//return float4(normalW, 1.0f);
}