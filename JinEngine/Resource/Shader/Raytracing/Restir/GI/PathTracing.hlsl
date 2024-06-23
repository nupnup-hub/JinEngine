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
#include"Common.hlsl"  
#include"../../Common.hlsl" 
#include"../../../Common/RandomNumberGenerator.hlsl" 
#include"../../../Common/MaterialData.hlsl" 

struct HemishpereSample
{
    float3 value;
    uint padding;
};
struct MeshVertex
{
    float3 pos;
    float3 normal;
    float3 tangent;
};
struct MeshMaterial
{
    float3 albedoColor;
    float specularFactor;
    float metallic;
    float roughness;
};
struct RayPayload
{
    MeshVertex vertex; //out
    MeshMaterial material; //out    
    uint hitType; //out  
};
struct ShadowPayload
{
    float isVisible;
};
struct SelectLightInfo
{
    int type;
    uint dataIndex;
    uint isDeltaLight;
    float distance;
    float3 direction; //normalized
    float pdf;
    float3 irradiance;
    
    void Initialize()
    {
        type = 0;
        dataIndex = 0;
        isDeltaLight = false;
        distance = 0;
        direction = float3(0, 0, 0);
        pdf = 0;
        irradiance = float3(0, 0, 0);
    }
};
struct EstimateDataSet
{
    float3 toRayOrigin;
    float3 throughput;
    float2 lightUniformSample;
    float2 bxdfUniformSample;
    float bxdfPdf;
    uint pathLength;
    uint hitType; //0 object, 1 light, 2 miss
};

#ifndef TEXTURE_2D_COUNT
#define TEXTURE_2D_COUNT 1
#endif 

#define ALLOW_CUBE_MAP 1

#define INDIRECT_HIT_SHADER_GROUP 0
#define INDIRECT_MISS_SHADER 0
 
#define DIRECT_LIGHT_HIT_SHADER_GROUP 1 
#define DIRECT_LIGHT_MISS_SHADER 1  

#ifndef T_MIN
#define T_MIN 1e-2
#endif
#ifndef MIN_DEPTH
#define MIN_DEPTH 1
#endif
#ifndef MAX_DEPTH
#define MAX_DEPTH 4
#endif 
#ifndef SAMPLE_COUNT
#define SAMPLE_COUNT 1
#endif 
#ifndef RRP_MIN
#define RRP_MIN 0.6f
#endif  
 
#define INAVALID_INSTANCE_ID 1 << 25        //instance id is 24bit
#define INAVALID_LIGHT_TYPE -1    
#define DIRECTIONAL_LIGHT_TYPE 0  
#define POINT_LIGHT_TYPE 1   
#define SPOT_LIGHT_TYPE 2   
#define RECT_LIGHT_TYPE 3   
 
#define OBJECT_HIT 0
#define LIGHT_HIT 1
#define MISS 2
 
//directionalLight[0] is current scene directionalLightData
StructuredBuffer<DirectionalLightData> directionalLight : register(t2, space0);
StructuredBuffer<PointLightData> pointLight : register(t2, space1);
StructuredBuffer<SpotLightData> spotLight : register(t2, space2);
StructuredBuffer<RectLightData> rectLight : register(t2, space3);
StructuredBuffer<MaterialData> materialData : register(t3);

Texture2D textureMaps[TEXTURE_2D_COUNT] : register(t4, space5);
TextureCube skyCubeMap : register(t4, space6);
Texture2D depthMap : register(t4, space7);
Texture2D screenAlbedoMap : register(t4, space8); //gbuffer layer 
Texture2D screenLightProp : register(t4, space9); //gbuffer layer  
Texture2D screenNormalMap : register(t4, space10); //gbuffer layer 

RWStructuredBuffer<RestirSamplePack> initialSample : register(u0);
  
SamplerState samLinearClamp : register(s0);
//SamplerState samAnisotropicWrap : register(s0);
SamplerState samLTC : register(s1);
SamplerState samLTCSample : register(s2);
 
void SampleBxdf(float3 normal, float3 tangent, MeshMaterial material, float3 toRayOrigin, out float3 toLight, inout float2 u, inout float3 bsdf, inout float bsdfPdf)
{
    BxDF bxdf; 
    if (u.x < 0.5f)
    { 
        //SampleDirectionHemisphere
        //variance Hemisphere < Coshine weighted Hemisphere 
        bxdf.DiffuseSamplingInitialize(material.albedoColor, material.specularFactor, normal, tangent, toRayOrigin, material.roughness, material.metallic, u);
        bxdf.SampleDiffuse(bsdf, bsdfPdf);     
    }
    else
    { 
        bxdf.SpecularSamplingInitialize(material.albedoColor, material.specularFactor, normal, toRayOrigin, material.roughness, material.metallic, u);
        bxdf.SampleSpecular(bsdf, bsdfPdf);
    }
    toLight = bxdf.lightVec;
    
    const bool enableDiffuse = material.metallic < 1.0f;
    if (enableDiffuse)
        bsdfPdf *= 0.5f;    
}
void EvaluateBxdf(float3 normal, MeshMaterial material, float3 toRayOrigin, float3 toLight, inout float2 u, inout float3 bsdf, inout float bsdfPdf)
{
    BxDF bxdf;
    bxdf.Initialize(material.albedoColor, material.specularFactor, normal, toLight, toRayOrigin, material.roughness, material.metallic);
    if (u.x < 0.5f)
        bxdf.SampleDiffuse(bsdf, bsdfPdf);
    else
        bxdf.SampleSpecular(bsdf, bsdfPdf); 
    
    const bool enableDiffuse = material.metallic < 1.0f;
    if (enableDiffuse)
        bsdfPdf *= 0.5f;
}
void EvaluateRectLight(uint dataIndex, float3 pos, float3 normal, float3 toRayOrigin, float roughness, float3 lightVec, out float3 irradiance, out float3 pdf)
{
    RectLight rLit = ComputeRect(rectLight[dataIndex], pos);
    rLit.ltcMat = textureMaps[rLit.ltcMatTextureIndex];
    rLit.ltcAmp = textureMaps[rLit.ltcAmpTextureIndex];
    rLit.source = textureMaps[rLit.sourceTextureIndex];
    rLit.samLTC = samLTC;
    rLit.samLTCSample = samLTCSample;
    
    irradiance = EvaluateRectLight(rLit, pos, normal, toRayOrigin, roughness, lightVec);
    pdf = SampleDirectionRectangularLight_PDF(rLit.area, -toRayOrigin, lightVec, length(rLit.origin - pos));
}
float3 SampleLightSourceImportance(in MeshVertex hitSurface, in MeshMaterial material, inout EstimateDataSet set, in SelectLightInfo lightInfo)
{
    RayDesc rayDesc = CreateRayDesc(hitSurface.pos, lightInfo.direction, hitSurface.normal, lightInfo.distance, T_MIN);
    
    ShadowPayload shadowPayload;
    shadowPayload.isVisible = false;
    
    uint rayFlag = RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH;
    TraceRay(sceneAs, rayFlag, OBJECT_MASK, DIRECT_LIGHT_HIT_SHADER_GROUP, 0, DIRECT_LIGHT_MISS_SHADER, rayDesc, shadowPayload);
    if (!shadowPayload.isVisible)
        return float3(0, 0, 0);
    
    float3 bsdf = float3(0, 0, 0);
    float bsdfPdf = 1.0f;
    EvaluateBxdf(hitSurface.normal, material, set.toRayOrigin, lightInfo.direction, set.lightUniformSample, bsdf, bsdfPdf);
    
    float3 radiacne = float3(0.0f, 0.0f, 0.0f);
    float3 irradiance = lightInfo.irradiance;
    float lightPdf = lightInfo.pdf;
    float dotNL = max(dot(hitSurface.normal, lightInfo.direction), EPSILON);
     
    if (lightInfo.isDeltaLight) 
        radiacne = (set.throughput * irradiance * bsdf * dotNL) / (lightPdf * cb.invTotalLightCount);
    else
    {
        float misWeight = PowerHeuristic(1, lightPdf, 1, bsdfPdf);
        radiacne = (set.throughput * irradiance * bsdf * misWeight * dotNL) / (lightPdf * cb.invTotalLightCount);
    }
    return radiacne;
}
float3 SampleBxdfImportance(inout MeshVertex hitSurface, inout MeshMaterial material, inout EstimateDataSet set, in SelectLightInfo lightInfo)
{
    float3 radiance = float3(0, 0, 0);
    float3 toLight = float3(0, 0, 0);
    float3 bsdf = float3(0, 0, 0);
    float bsdfPdf = 1.0f;
 
    //out toLight = next ray dir
    SampleBxdf(hitSurface.normal, hitSurface.tangent, material, set.toRayOrigin, toLight, set.bxdfUniformSample, bsdf, bsdfPdf);
    set.bxdfPdf = bsdfPdf;
    
    RayDesc rayDesc = CreateRayDesc(hitSurface.pos, toLight, hitSurface.normal, cb.tMax, T_MIN);

    RayPayload rayPayload; 
    rayPayload.hitType = MISS;
    
    uint rayFlag = RAY_FLAG_NONE;
    TraceRay(sceneAs, rayFlag, 0xff, INDIRECT_HIT_SHADER_GROUP, 0, INDIRECT_MISS_SHADER, rayDesc, rayPayload);
    if (rayPayload.hitType == MISS)
        return radiance;
     
    float3 irradiance = float3(0, 0, 0);
    float3 lightVec = lightInfo.direction;
    float lightPdf = 0.0f;
    
    set.hitType = rayPayload.hitType;
    if (!lightInfo.isDeltaLight)
    {
        //irradiance = hit point emission
        float dotNL = max(dot(hitSurface.normal, normalize(lightVec)), EPSILON);
        if (set.hitType == OBJECT_HIT)
        {
            //float dotNL = max(dot(rayPayload.vertex.normal, normalize(hitSurface.pos - rayPayload.vertex.pos)), 0.0001f);
            irradiance = material.albedoColor;
            lightPdf = SampleDirectionHemisphere_PDF();
            //SampleDirectionHemisphere_PDF
        }
       // else if (lightInfo.type == RECT_LIGHT_TYPE)
       //    EvaluateRectLight(rayPayload.dataIndex, hitSurface.pos, hitSurface.normal, set.toRayOrigin, material.roughness, lightVec, irradiance, lightPdf);
 
        if (lightPdf > 0)
        {
            float misWeight = PowerHeuristic(1, bsdfPdf, 1, lightPdf);
            float dotNL = max(dot(hitSurface.normal, normalize(lightVec)), EPSILON);
            radiance = (set.throughput * irradiance * bsdf * dotNL * misWeight) / (bsdfPdf * cb.invTotalLightCount);
        }
    }
     
    hitSurface = rayPayload.vertex;
    material = rayPayload.material;
    set.toRayOrigin = -toLight;
    set.throughput += bsdf / bsdfPdf;
    return radiance;
}
bool ComputeDirectLightIrradiance(in MeshVertex hitSurface, in MeshMaterial material, in EstimateDataSet set, out SelectLightInfo lightInfo)
{
    lightInfo.Initialize();
    uint lightSelector = (uint) (cb.totalLightCount * clamp(set.lightUniformSample.x, 0.0f, 0.999999f));
 
    if (lightSelector < cb.directionalLightRange)
    {
        lightInfo.type = DIRECTIONAL_LIGHT_TYPE;
        lightInfo.dataIndex = lightSelector + cb.directionalLightOffset;
        lightInfo.isDeltaLight = true;
        
        lightInfo.distance = directionalLight[lightInfo.dataIndex].frustumFar;
        lightInfo.direction = normalize(-directionalLight[lightInfo.dataIndex].direction);
         
        float3 color = directionalLight[lightInfo.dataIndex].color;
        float power = directionalLight[lightInfo.dataIndex].power;
 
        lightInfo.irradiance = SampleDirectionalLight(color, power);
        lightInfo.pdf = 1.0;
        return true;
    }
    else if (lightSelector < cb.pointLightRange)
    {
        lightInfo.type = POINT_LIGHT_TYPE;
        lightInfo.dataIndex = lightSelector - cb.directionalLightRange + cb.pointLightOffset;
        lightInfo.isDeltaLight = true;
        
        float3 distance = pointLight[lightInfo.dataIndex].midPosition - hitSurface.pos;
        lightInfo.distance = length(distance);
        lightInfo.direction = normalize(distance);
        
        PointLight pLit;
        pLit.midPosition = pointLight[lightInfo.dataIndex].midPosition;
        pLit.sidePosition[0] = pointLight[lightInfo.dataIndex].sidePosition[0];
        pLit.sidePosition[1] = pointLight[lightInfo.dataIndex].sidePosition[1];
        pLit.color = pointLight[lightInfo.dataIndex].color;
        pLit.power = pointLight[lightInfo.dataIndex].power;
        pLit.range = pointLight[lightInfo.dataIndex].frustumFar;
   
        lightInfo.irradiance = SamplePointLight(pLit, hitSurface.pos, hitSurface.normal);
        lightInfo.pdf = 1.0f;
        return true;
    }
    else if (lightSelector < cb.spotLightRange)
    {
        lightInfo.type = SPOT_LIGHT_TYPE;
        lightInfo.dataIndex = lightSelector - cb.pointLightRange + cb.spotLightOffset;
        lightInfo.isDeltaLight = true;
        
        float3 distance = spotLight[lightInfo.dataIndex].position - hitSurface.pos;
        lightInfo.distance = length(distance);
        lightInfo.direction = normalize(distance);
        
        SpotLight sLit;
        sLit.position = spotLight[lightInfo.dataIndex].position;
        sLit.direction = spotLight[lightInfo.dataIndex].direction;
        sLit.color = spotLight[lightInfo.dataIndex].color;
        sLit.power = spotLight[lightInfo.dataIndex].power;
        sLit.range = spotLight[lightInfo.dataIndex].frustumFar;
        sLit.innerConeCosAngle = spotLight[lightInfo.dataIndex].innerConeCosAngle;
        sLit.outerConeCosAngle = spotLight[lightInfo.dataIndex].outerConeCosAngle;
     
        lightInfo.irradiance = SampleSpotLight(sLit, hitSurface.pos, hitSurface.normal);
        lightInfo.pdf = 1.0f;
        return true;
    }
    else if (lightSelector < cb.rectLightRange)
    {
        lightInfo.type = RECT_LIGHT_TYPE;
        lightInfo.dataIndex = lightSelector - cb.spotLightRange + cb.rectLightOffset;
        lightInfo.isDeltaLight = false;
   
        RectLight rLit = ComputeRect(rectLight[lightInfo.dataIndex], hitSurface.pos);
        rLit.ltcMat = textureMaps[rLit.ltcMatTextureIndex];
        rLit.ltcAmp = textureMaps[rLit.ltcAmpTextureIndex];
        rLit.source = textureMaps[rLit.sourceTextureIndex];
        rLit.samLTC = samLTC;
        rLit.samLTCSample = samLTCSample;
        
        float3 lightVec = SampleDirectionRectangularLight(set.lightUniformSample.x, set.lightUniformSample.y, hitSurface.pos, rLit.area, rLit.origin);
        lightInfo.distance = length(rLit.origin - hitSurface.pos);
        lightInfo.direction = normalize(lightVec);
  
        lightInfo.irradiance = SampleRectLight(rLit, hitSurface.pos, hitSurface.normal, set.toRayOrigin, material.roughness);
        lightInfo.pdf = SampleDirectionRectangularLight_PDF(rLit.area, -set.toRayOrigin, normalize(lightVec), length(rLit.origin - hitSurface.pos));
        return true;
    }
    else
        return false;
}
float3 PathTracing(in MeshVertex hitSurface, in MeshMaterial material, in EstimateDataSet set, uint pixelIndex, inout uint sampleSetIndex)
{
    float rrProb = 1.0f;
    float3 radiance = float3(0, 0, 0);
    
    for (uint i = 0; i < MAX_DEPTH; ++i)
    {
        SelectLightInfo lightInfo;
        if (!ComputeDirectLightIrradiance(hitSurface, material, set, lightInfo))
            return float3(0, 0, 0);
         
        if (lightInfo.pdf > 0)
            radiance += SampleLightSourceImportance(hitSurface, material, set, lightInfo);
        radiance += SampleBxdfImportance(hitSurface, material, set, lightInfo);
        if (set.hitType == LIGHT_HIT || set.hitType == MISS)
            break;
        
        if (i >= MIN_DEPTH)
        {
            rrProb = min(RRP_MIN, max(set.throughput.r, max(set.throughput.g, set.throughput.b)));
            float rrSample = Sample1DPoint(pixelIndex, sampleSetIndex);
            if (rrProb > rrSample)
                break;
            
            set.throughput /= (1.0f - rrProb);
        }
        set.lightUniformSample = Sample2DPoint(pixelIndex, sampleSetIndex);
        set.bxdfUniformSample = Sample2DPoint(pixelIndex, sampleSetIndex);
        ++set.pathLength;
    }
    initialSample[pixelIndex].samplePos = hitSurface.pos;
    initialSample[pixelIndex].random = set.bxdfPdf;
    initialSample[pixelIndex].PackSampleNormal(hitSurface.normal);
    return radiance;
}

[shader("raygeneration")]
void RayGenShader()
{
    const uint2 pixelCoord = DispatchRaysIndex().xy;
    const uint pixelIndex = pixelCoord.y * DispatchRaysDimensions().x + pixelCoord.x;
    uint sampleSetIndex = 0;
    
    initialSample[pixelIndex].Initialize();
    
    float2 uv = (pixelCoord + float2(0.5f, 0.5f)) * cb.halfInvRtSize;
    float depth = depthMap.SampleLevel(samLinearClamp, uv, 0).x;
    if (depth == 1.0f)
        return;
    
    float3 albedo;
    float specularFactor; 
    UnPackAlbedoColorLayer(screenAlbedoMap.SampleLevel(samLinearClamp, uv, 0), albedo, specularFactor);
 
    MeshVertex visibleVertex;
    visibleVertex.pos = GetWorldPos(uv, depth); 
    UnpackNormalAndTangentLayer(screenNormalMap.SampleLevel(samLinearClamp, uv, 0), visibleVertex.normal, visibleVertex.tangent);
     
    MeshMaterial material; 
    material.albedoColor = albedo;
    material.specularFactor = specularFactor;  
    UnpackLightPropLayer(screenLightProp.SampleLevel(samLinearClamp, uv, 0), material.metallic, material.roughness);
 
    float3 radiance = float3(0, 0, 0);
	//[unroll]
    //for (uint i = 0; i < SAMPLE_COUNT; ++i)
    {
        EstimateDataSet set;
        set.toRayOrigin = normalize(cb.camPosW - visibleVertex.pos);
        set.throughput = float3(1, 1, 1);
        set.lightUniformSample = Sample2DPoint(pixelIndex, sampleSetIndex);
        set.bxdfUniformSample = Sample2DPoint(pixelIndex, sampleSetIndex);
        set.bxdfPdf = 0;
        set.hitType = OBJECT_HIT;
        set.pathLength = 0;
        radiance += PathTracing(visibleVertex, material, set, pixelIndex, sampleSetIndex);
    }
    //radiance /= SAMPLE_COUNT;
    initialSample[pixelIndex].visiblePos = visibleVertex.pos;
    initialSample[pixelIndex].PackVisibleNormal(visibleVertex.normal);
    initialSample[pixelIndex].PackRadiance(radiance);
}
[shader("closesthit")]
void HemisphereHitShader(inout RayPayload rayPayload, in BuiltInTriangleIntersectionAttributes attr)
{
    //opaque object and instance ID = instanceInfo index
    //instanceInfo[instanceID]      
    uint instanceID = InstanceID();   
    MaterialData meshMatData = materialData[instanceInfo[instanceID].materialIndex];
 
    StaticVertex vertex = GetStaticMeshVertex(instanceID, PrimitiveIndex(), attr);
    rayPayload.vertex.pos = HitWorldPosition();
    //rayPayload.vertex.pos = mul(ObjectToWorld3x4(), float4(rayPayload.vertex.pos, 1.0f));
    
    float3x3 toWorldM = (float3x3) ObjectToWorld3x4();
    rayPayload.vertex.normal = mul(vertex.normal, toWorldM);
    rayPayload.vertex.tangent = mul(vertex.tangent, toWorldM);
 
    float2 texC = mul(float4(vertex.texC, 0.0f, 1.0f), meshMatData.matTransform).xy;
    
    rayPayload.material.metallic = meshMatData.metallic;
    rayPayload.material.roughness = meshMatData.roughness;
    
    if (meshMatData.metallicMapIndex != MISSING_TEXTURE_INDEX)
        rayPayload.material.metallic = textureMaps[meshMatData.metallicMapIndex].SampleLevel(samLinearClamp, texC, 0).x;
 
    if (meshMatData.roughnessMapIndex != MISSING_TEXTURE_INDEX)
        rayPayload.material.roughness = textureMaps[meshMatData.roughnessMapIndex].SampleLevel(samLinearClamp, texC, 0).x;
        
    rayPayload.material.albedoColor = meshMatData.albedoColor.xyz;
    //if (meshMatData.albedoMapIndex != MISSING_TEXTURE_INDEX)
    //  rayPayload.material.albedoColor *= textureMaps[meshMatData.albedoMapIndex].SampleLevel(samLinearClamp, texC, 0);
 
    float specularFactor = ComputeDefaultSpecularFactor(rayPayload.material.albedoColor, rayPayload.material.metallic);
    rayPayload.material.specularFactor = specularFactor;
   // if (meshMatData.specularMapIndex != MISSING_TEXTURE_INDEX)
   //     rayPayload.material.specularColor *= textureMaps[meshMatData.specularMapIndex].SampleLevel(samLinearClamp, texC, 0);
    
    if (meshMatData.normalMapIndex != MISSING_TEXTURE_INDEX)
    {
        float4 normalMapSample = textureMaps[meshMatData.normalMapIndex].SampleLevel(samLinearClamp, texC, 0);
        float3 normalT = 2.0f * normalMapSample.rgb - 1.0f;
        rayPayload.vertex.normal = normalize(mul(normalT, CalTBN(rayPayload.vertex.normal, rayPayload.vertex.tangent)));
    }  
    rayPayload.hitType = OBJECT_HIT;
}
[shader("miss")]
void HemispherMissShader(inout RayPayload rayPayload)
{
    rayPayload.hitType = MISS;
    //rayPayload.isHitLight = rayPayload.selectedLightType == DIRECTIONAL_LIGHT_TYPE;
    //rayPayload.dataIndex = 0; //direcitonal light exist per scene
    
    //rayPayload.material.albedoColor = float4(1, 0, 0, 1);
    /*
          if (length(lightColor) > 0)
        {
            lightPdf = 1;
            isHitLight = true;
        }
    */
}
 
[shader("closesthit")]
void LightSourceHitShader(inout ShadowPayload s, in BuiltInTriangleIntersectionAttributes attr)
{
   // DirectionalLight dLit;
   // dLit.direction = directionalLight[cb.directionalLightOffset].direction;
   // dLit.color = directionalLight[cb.directionalLightOffset].color;
   // dLit.power = directionalLight[cb.directionalLightOffset].power;
 
    //r.irradiance = SampleDirectionalLight(dLit, r.normalW);
    //r.pdf = 1.0; 
    s.isVisible = false;
}
[shader("miss")]
void LightSourceMissShader(inout ShadowPayload s)
{
    s.isVisible = true;
}

//any HitKind 투명 material 및 light shape를 다룰시 추가필요
/*
[shader("anyhit")]
void HemispherAnyHitShader(inout RayPayload rayPayload, in BuiltInTriangleIntersectionAttributes attr)
{
    uint instanceID = InstanceID();
    if (RECT_LIGHT_INSTANCE_ID_OFFSET <= instanceID && instanceID < LIGHT_INSTANCE_ID_END)
    {
        uint verticiesIndex = cb.rectLightVerticesIndex;
        uint indiciesIndex = cb.rectLightIndiciesIndex;
        rayPayload.vertex = GetStaticMeshVertex(verticiesIndex, indiciesIndex, PrimitiveIndex(), attr);
        rayPayload.vertex.pos = HitWorldPosition();
        //rayPayload.vertex.pos = mul(ObjectToWorld3x4(), float4(rayPayload.vertex.pos, 1.0f));
        rayPayload.vertex.normal = mul(rayPayload.vertex.normal, (float3x3) ObjectToWorld3x4());
        rayPayload.vertex.tangent = mul(rayPayload.vertex.tangent, (float3x3) ObjectToWorld3x4());
        rayPayload.hitType = LIGHT_HIT;
        //hit light
        IgnoreHit();
    }
}
*/

 