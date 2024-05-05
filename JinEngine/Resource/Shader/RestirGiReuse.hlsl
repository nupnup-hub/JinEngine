#pragma once
#include"RaytracingCommon.hlsl"
#include"RestirGiCommon.hlsl"   
#include"ColorSpaceUtility.hlsl"

#define USE_BRDF_LAMBERTIAN_DIFFUSE 1
#define USE_BRDF_GGX_MICROFACET 1
#define USE_BRDF_ISOTROPY_NDF 1
#include"LightCompute.hlsl"

#define LARGE_FLOAT 1e20f

#ifndef DIMX
#define DIMX 16
#endif
#ifndef DIMY
#define DIMY 16
#endif

#ifndef TEMPORAL_SAMPLE_MAX
#define TEMPORAL_SAMPLE_MAX 32
#endif
#ifndef SPATIAL_SAMPLE_MAX
#define SPATIAL_SAMPLE_MAX 128
#endif
#ifndef RESERVOIR_COUNT
#define RESERVOIR_COUNT 1
#endif
#ifndef SAMPLE_MAX_AGE
#define SAMPLE_MAX_AGE 128         ///< Maximum frames that a sample can survive.
#endif  
#ifndef USE_RESTIRN
#define USE_RESTIRN 0
#endif
#ifndef DEPTH_THRESHOLD
#define DEPTH_THRESHOLD 0.98f
#endif
#ifndef VELOCITY_THRESHOLD
#define VELOCITY_THRESHOLD 0.1f
#endif
#ifndef NORMAL_THRESHOLD
#define NORMAL_THRESHOLD 0.8f
#endif  
#ifndef RAND_THRESHOLD
#define RAND_THRESHOLD 0.15f
#endif
#ifndef POS_THRESHOLD
#define POS_THRESHOLD 2.0f
#endif
#ifndef JCOB_MAX
#define JCOB_MAX LARGE_FLOAT
#endif  
#ifndef NEIBOR_NORMAL_THRESHOLD
#define NEIBOR_NORMAL_THRESHOLD  0.906f
#endif 
#ifndef NEIGHBOR_WEIGHT_CLAMP_THRESHOLD
#define NEIGHBOR_WEIGHT_CLAMP_THRESHOLD 128.0f
#endif
#ifndef SPATIAL_WEIGHT_CLAMP_THRESHOLD
#define SPATIAL_WEIGHT_CLAMP_THRESHOLD 10.0f
#endif
#ifndef NEIGHBOR_LOOP_COUNT
#define NEIGHBOR_LOOP_COUNT 10
#endif
#ifndef T_MIN
#define T_MIN 1e-2
#endif
#define RESTIR_TEMPORAL_JCOB 1
#define RESTIR_UNBIASED 1
#define RESTIR_ZERO_WEIGHT 1

StructuredBuffer<RestirSamplePack> initialSample : register(t1);
StructuredBuffer<RestirReserviorPack> preTemporal : register(t2);
StructuredBuffer<RestirReserviorPack> preSpatial : register(t3);
Texture2D albedoMap : register(t4); 
Texture2D lightPropMap : register(t5); 
Texture2D depthMap : register(t6);
Texture2D preNormalMap : register(t7);
Texture2D preDepthMap : register(t8);
RWStructuredBuffer<RestirReserviorPack> temporal : register(u0);
RWStructuredBuffer<RestirReserviorPack> spatial : register(u1);
SamplerState samLinearBorder : register(s0);

RestirReserviorData LoadReservior(uint baseIndex, uint sampleIndex, uint elementCount)
{
    return UnpackRestirGiSample(initialSample[baseIndex + sampleIndex * elementCount]);
}
RestirReserviorData LoadReservior(StructuredBuffer<RestirReserviorPack> buffer, uint baseIndex, uint sampleIndex, uint elementCount)
{
    return UnpackRestirGiReservior(buffer[baseIndex + sampleIndex * elementCount]);
}
void StoreReservior(RWStructuredBuffer<RestirReserviorPack> buffer, RestirReserviorData reservior, uint baseIndex, uint sampleIndex, uint elementCount)
{
    buffer[baseIndex + sampleIndex * elementCount] = PackRestirGiReservior(reservior);
}
   
float EvaluateTargetFunction(float3 radiance, float3 posW, float3 normalW, float3 lightPosW, Material material)
{ 
#if 1
    float3 lightVec = normalize(lightPosW - posW);
    float3 viewVec = normalize(cb.camPosW - posW);
    float3 fCos = max(float3(0.1f, 0.1f, 0.1f), CalBxDF(normalW, lightVec, viewVec, material) * saturate(dot(normalW, lightVec)));
    float pdf = RGBToLuminance(radiance * fCos);
    //float pdf = length(radiance * fCos);
    return max(pdf, 0.0f);
#else
    return RGBToLuminance(radiance);
#endif
}

struct ReuseDataSet
{
    Material material;
    float depth;
    float3 posW;
    float3 normalW;
     
    int2 pixelCoord;
    int2 prePixelCoord;
    uint pixelIndex;
    uint prePixelIndex;
    int3 gBufferLocation;
    int indexOffset;
    
    float viewZThresHold;
    RandomNumberGenerator rng;
};
void TemporalReuse(in ReuseDataSet set, inout bool isPreValid)
{
    RestirReserviorData initialReservior = LoadReservior(set.pixelIndex, set.indexOffset, cb.totalNumPixels);
        
    // Temporal Reuse.
    // Read temporal reservoir.
    RestirReserviorData reservior = LoadReservior(preTemporal, set.prePixelIndex, set.indexOffset, cb.totalNumPixels);
    if (length(reservior.visiblePos - set.posW) > POS_THRESHOLD)
        isPreValid = false;
    
    reservior.M = clamp(reservior.M, 0, TEMPORAL_SAMPLE_MAX);
    if (!isPreValid || reservior.age > SAMPLE_MAX_AGE)
        reservior.M = 0;
    
    float reusePdf = EvaluateTargetFunction(reservior.radiance, set.posW, set.normalW, reservior.samplePos, set.material);
#ifdef RESTIR_TEMPORAL_JCOB
    {
        float3 offsetB = reservior.samplePos - reservior.visiblePos;
        float3 offsetA = reservior.samplePos - set.posW;

        float RB2 = dot(offsetB, offsetB);
        float RA2 = dot(offsetA, offsetA);
        offsetB = normalize(offsetB);
        offsetA = normalize(offsetA);
        float cosA = dot(set.normalW, offsetA);
        float cosB = dot(reservior.visibleNormal, offsetB);
        float cosPhiA = -dot(offsetA, reservior.sampleNormal);
        float cosPhiB = -dot(offsetB, reservior.sampleNormal);

        if (cosA <= 0.f || cosPhiA <= 0.f || RA2 <= 0.f || RB2 <= 0.f || cosB <= 0.f || cosPhiB <= 0.f)
            reusePdf = 0.f;

        // assuming visible
        // Calculate Jacobian determinant and weight. 
        float jacobian = RA2 * cosPhiB <= 0.f ? 0.f : clamp(RB2 * cosPhiA / (RA2 * cosPhiB), 0.f, LARGE_FLOAT);
        reusePdf *= jacobian;
    }
#endif
    float weightSum = max(0.0f, reservior.W) * reservior.M * reusePdf;
    
    float targetPdf = EvaluateTargetFunction(initialReservior.radiance, set.posW, set.normalW, initialReservior.samplePos, set.material);
    float weight = initialReservior.W > 0 ? targetPdf * initialReservior.W : 0;
  
    bool isUpdated = reservior.Update(weight, initialReservior, set.rng, weightSum);
    
    float weightAverage = weightSum / reservior.M;
    float resampledPdf = EvaluateTargetFunction(reservior.radiance, reservior.visiblePos, reservior.visibleNormal, reservior.samplePos, set.material);
    reservior.W = weightAverage / resampledPdf;
    reservior.M = clamp(reservior.M, 0, TEMPORAL_SAMPLE_MAX);
    reservior.age++;
    reservior.visiblePos = set.posW;
    reservior.visibleNormal = set.normalW;
    
    StoreReservior(temporal, reservior, set.pixelIndex, set.indexOffset, cb.totalNumPixels);
}
void SpatialReuse(in ReuseDataSet set, bool isPreValid)
{
    RestirReserviorData reservior = LoadReservior(preSpatial, set.prePixelIndex, set.indexOffset, cb.totalNumPixels);
    reservior.M = clamp(reservior.M, 0, SPATIAL_SAMPLE_MAX);
    if (!isPreValid || reservior.age > SAMPLE_MAX_AGE)
        reservior.M = 0;
     
    float weightSum = max(0.0f, reservior.W) * reservior.M * EvaluateTargetFunction(reservior.radiance, reservior.visiblePos, reservior.visibleNormal, reservior.samplePos, set.material);

    const float searchRadiusRatio = 0.1f;
    float searchRadius = cb.rtSize.x * searchRadiusRatio;
        
    // Initialize reuse history.
#if RESTIR_UNBIASED 
    float3 positionList[NEIGHBOR_LOOP_COUNT + 1];
    float3 normalList[NEIGHBOR_LOOP_COUNT + 1];
    int MList[NEIGHBOR_LOOP_COUNT + 1];
    int nReuse = 0;
    int reuseID = 0;
    positionList[nReuse] = set.posW;
    normalList[nReuse] = set.normalW;
    MList[nReuse] = reservior.M;
    ++nReuse;
#endif
    reservior.visiblePos = set.posW;
    reservior.visibleNormal = set.normalW;
    
    const float fastReuseRatio = 0.5f;
    const float fastReuseThreshold = SPATIAL_SAMPLE_MAX * fastReuseRatio;
    const int normalIteration = 3;
    const int fastReuseIteration = NEIGHBOR_LOOP_COUNT;

    int maxIteration = reservior.M > fastReuseThreshold ? normalIteration : fastReuseIteration;
    
    RestirReserviorData neighborReservoir;
    //[unroll]
    for (int j = 0; j < maxIteration; j++)
    {
        // Get search radius.
        const float radiusShrinkRatio = 0.5f;
        const float minSearchRadius = 10.0f;
        searchRadius = max(searchRadius * radiusShrinkRatio, minSearchRadius);
             
        float2 randOffset = float2(set.rng.Random01(), set.rng.Random01());
        int2 neighborID = set.prePixelCoord + searchRadius * (randOffset * 2.0f - 1.0f);
        
        uint2 boundary = cb.rtSize - 1;
        neighborID.x = neighborID.x < 0 ? 0 : (neighborID.x > boundary.x ? 2 * boundary.x - neighborID.x : neighborID.x);
        neighborID.y = neighborID.y < 0 ? 0 : (neighborID.y > boundary.y ? 2 * boundary.y - neighborID.y : neighborID.y);

        //float2 minCoord = max(set.pixelCoord - float2(searchRadius, searchRadius), float2(0, 0));
        //float2 maxCoord = min(set.pixelCoord + float2(searchRadius, searchRadius), cb.rtSize - float2(1, 1));
       
        // Randomly sample a neighbor. 
        //float2 randOffset = Sample2DPoint(set.pixelIndex, currSampleSetIndex);   
    
        float3 neighborNormal = SignedOctDecode(preNormalMap.Load(int3(neighborID, 0)).xyw);
        float neighborDepth = NdcToViewPZ(preDepthMap.Load(int3(neighborID, 0)).x, cb.camNearMulFar, cb.camNearFar);
             
        // Check geometric similarity. 
        if (!IsValidNeighbor(set.normalW, neighborNormal, set.depth, neighborDepth, NEIBOR_NORMAL_THRESHOLD, set.viewZThresHold))
            continue;

        // Read neighbor's spatial reservoir.
        uint neighborIndex = neighborID.x + neighborID.y * cb.rtSize.x;
#if RESTIR_UNBIASED                       
        bool bReuseSpatialSample = (j % 2);
        if (bReuseSpatialSample)
            neighborReservoir = LoadReservior(preSpatial, neighborIndex, set.indexOffset, cb.totalNumPixels);
        else
            neighborReservoir = LoadReservior(preTemporal, neighborIndex, set.indexOffset, cb.totalNumPixels);
#else           
            neighborReservoir = LoadReservior(preTemporal, neighborIndex, set.indexOffset, cb.totalNumPixels);
#endif            
         // Discard black samples.
        if (neighborReservoir.M <= 0)
            continue;
        
        float targetPdf = EvaluateTargetFunction(neighborReservoir.radiance, set.posW, set.normalW, neighborReservoir.samplePos, set.material);
        float3 offsetB = neighborReservoir.samplePos - neighborReservoir.visiblePos;
        float3 offsetA = neighborReservoir.samplePos - set.posW;
      
        float RB2 = dot(offsetB, offsetB);
        float RA2 = dot(offsetA, offsetA);
        offsetB = normalize(offsetB);
        offsetA = normalize(offsetA);
        float cosA = dot(set.normalW, offsetA);
        float cosB = dot(neighborReservoir.visibleNormal, offsetB);
        float cosPhiA = -dot(offsetA, neighborReservoir.sampleNormal);
        float cosPhiB = -dot(offsetB, neighborReservoir.sampleNormal);
        if (cosB <= 0.f || cosPhiB <= 0.f)
            continue;
        if (cosA <= 0.f || cosPhiA <= 0.f || RA2 <= 0.f || RB2 <= 0.f)
            targetPdf = 0.0f;
   
        bool isVisible = EvaluateSegmentVisibility(ComputeRayOrigin(set.posW, set.normalW), neighborReservoir.samplePos, T_MIN);
        if (!isVisible)
            targetPdf = 0.0f;
        
        // Calculate Jacobian determinant and tWeight. 
        float jacobian = RA2 * cosPhiB <= 0.f ? 0.f : clamp(RB2 * cosPhiA / (RA2 * cosPhiB), 0.0f, JCOB_MAX);
        float nWeight = clamp(neighborReservoir.W * neighborReservoir.M * targetPdf * jacobian, 0.0f, NEIGHBOR_WEIGHT_CLAMP_THRESHOLD);
     
        bool isUpdated = reservior.Update(nWeight, neighborReservoir, set.rng, weightSum);
#if RESTIR_UNBIASED  
        // Conditionally update spatial reservoir.       
        if (isUpdated)
            reuseID = nReuse;

        // Update reuse history.
        positionList[nReuse] = neighborReservoir.visiblePos;
        normalList[nReuse] = neighborReservoir.visibleNormal;
        MList[nReuse] = neighborReservoir.M;
        ++nReuse;
#endif
        // Expand search radius.
        const float radiusExpandRatio = 3.0f;
        searchRadius *= radiusExpandRatio;
    }
        
    // Calculate tWeight of spatial reuse.
    float m = 0;
#if RESTIR_UNBIASED  
    // Trace extra rays if unbiased spatial reuse is enabled.
    float totalWeight = 0.0f;
    float chosenWeight = 0.0f;
    int nValid = 0;
    int Z = 0;
    for (int k = 0; k < nReuse; k++)
    {
        bool isVisible = true;
        bool shouldTest = true;
        float3 directionVec = reservior.samplePos - positionList[k];
        if (dot(directionVec, normalList[k]) < 0.0f)
        {
            shouldTest = false;
            isVisible = false;
        }
        if (shouldTest)
            isVisible = EvaluateSegmentVisibility(ComputeRayOrigin(positionList[k], normalList[k]), reservior.samplePos, T_MIN);
            
        // Discard new sample if it is occluded.
        if (isVisible)
        {
#if RESTIR_ZERO_WEIGHT
            totalWeight += MList[k];
#else
            float misWeight = saturate(dot(normalList[k], normalize(directionVec))) * RGBToLuminance(reservior.radiance);
            totalWeight += misWeight * MList[k];
            if (reuseID == k)
                chosenWeight = misWeight;  
#endif
            ++nValid;
        }
        else if (k == 0)
            break;
    }
        
#if RESTIR_ZERO_WEIGHT
    m = totalWeight <= 0.f ? 0.f : 1.0f / totalWeight;
#else    
    m = totalWeight <= 0.f ? 0.f : chosenWeight / totalWeight;
#endif   
#else 
    m = reservior.M <= 0.0f ? 0.0f : 1.0f / float(reservior.M);
#endif          
    float resampledPdf = EvaluateTargetFunction(reservior.radiance, set.posW, set.normalW, reservior.samplePos, set.material);
    float mWeight = resampledPdf <= 0.f ? 0.0f : 1.0f / resampledPdf * m;
    reservior.M = clamp(reservior.M, 0, SPATIAL_SAMPLE_MAX);
    reservior.W = clamp(weightSum * mWeight, 0.0f, SPATIAL_WEIGHT_CLAMP_THRESHOLD);
    reservior.age++;
     
    //Write spatial reservoir.
    StoreReservior(spatial, reservior, set.pixelIndex, set.indexOffset, cb.totalNumPixels);
}
//reference Screen-Space-ReSTIR-GI\Rendering\ReSTIRGI\GIResampling.cs.slang
[numthreads(DIMX, DIMY, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= cb.rtSize.x || dispatchThreadID.y >= cb.rtSize.y)
        return;
     
    ReuseDataSet set;
    set.pixelCoord = dispatchThreadID.xy;
    set.pixelIndex = set.pixelCoord.y * cb.rtSize.x + set.pixelCoord.x;
    set.gBufferLocation = int3(set.pixelCoord, 0);
 
    float2 currUv = set.pixelCoord * cb.invRtSize;
    //set.prePixelCoord = set.pixelCoord;
    //set.prePixelIndex = set.pixelIndex;
    
    //float2 preUv = uv + velocity;
    //uint2 prevID = clamp(preUv * cb.rtSize, 0, cb.rtSize - 1);
    //uint preReserviorIndex = prevID.y * cb.rtSize.x + prevID.x;
 
    set.depth = NdcToViewPZ(depthMap.Load(set.gBufferLocation).x, cb.camNearMulFar, cb.camNearFar);
    set.normalW = initialSample[set.pixelIndex].UnpackVisibleNormal();
    set.posW = ComputeRayOrigin(initialSample[set.pixelIndex].visiblePos, set.normalW);
    //set.posW = initialSample[set.pixelIndex].visiblePos;
    set.material.albedoColor = albedoMap.Load(set.gBufferLocation);
    set.viewZThresHold = cb.camNearFar.y - cb.camNearFar.x * 0.02f;
    
    float4 lightProp = lightPropMap.Load(set.gBufferLocation);
    UnpackLightPropLayer(lightProp, set.material.specularColor, set.material.metallic, set.material.roughness);
 
    float4 prePosH = mul(float4(set.posW, 1.0f), cb.camPreViewProj);
    float3 prePosNdc = prePosH.xyz / prePosH.w;
    float2 preUv = prePosNdc.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
 
    set.prePixelCoord = clamp(preUv * cb.rtSize, 0, cb.rtSize - 1); 
    set.prePixelIndex = set.prePixelCoord.y * cb.rtSize.x + set.prePixelCoord.x;
    
    set.rng.Initialize(set.pixelCoord, cb.currSampleSetIndex);
    bool isPreValid = cb.updateCount > 0 && !cb.forceClearReservoirs && all(preUv > 0.0f) && all(preUv < 1.0f);
    if (isPreValid)
    {
        float3 curNormal = set.normalW;
        float3 preNormal = SignedOctDecode(preNormalMap.Load(int3(set.prePixelCoord, 0)).xyw); 
        isPreValid &= length(preUv - currUv) < VELOCITY_THRESHOLD && dot(curNormal, preNormal) > NORMAL_THRESHOLD;
        
        float rand = set.rng.Random01();
        if ((length(set.posW - cb.camPosW) / length(set.posW - cb.camPrePosW)) < DEPTH_THRESHOLD && rand < RAND_THRESHOLD)
            isPreValid = false;
    }

    //[unroll]
    //for (uint i = 0; i < RESERVOIR_COUNT; ++i)
    {
        //set.indexOffset = USE_RESTIRN ? i : 0;
        set.indexOffset = 0;
        TemporalReuse(set, isPreValid);
        SpatialReuse(set, isPreValid);
    }
}