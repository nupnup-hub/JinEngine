//=================================================================================================
//
//  MJP's DX12 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code licensed under the MIT license
//
//=================================================================================================
#pragma once
#include"Math.hlsl"

// Maps a value inside the square [0,1]x[0,1] to a value in a disk of radius 1 using concentric squares.
// This mapping preserves area, bi continuity, and minimizes deformation.
// Based off the algorithm "A Low Distortion Map Between Disk and Square" by Peter Shirley and
// Kenneth Chiu. Also includes polygon morphing modification from "CryEngine3 Graphics Gems"
// by Tiago Sousa
float2 SquareToConcentricDiskMapping(float x, float y, float numSides, float polygonAmount)
{
    float phi, r;

    float a = 2.0f * x - 1.0f;
    float b = 2.0f * y - 1.0f;

    if (a > -b)                      // region 1 or 2
    {
        if (a > b)                   // region 1, also |a| > |b|
        {
            r = a;
            phi = (PI / 4.0f) * (b / a);
        }
        else // region 2, also |b| > |a|
        {
            r = b;
            phi = (PI / 4.0f) * (2.0f - (a / b));
        }
    }
    else // region 3 or 4
    {
        if (a < b)                   // region 3, also |a| >= |b|, a != 0
        {
            r = -a;
            phi = (PI / 4.0f) * (4.0f + (b / a));
        }
        else // region 4, |b| >= |a|, but a==0 and b==0 could occur.
        {
            r = -b;
            if (b != 0)
                phi = (PI / 4.0f) * (6.0f - (a / b));
            else
                phi = 0;
        }
    }

    const float N = numSides;
    float polyModifier = cos(PI / N) / cos(phi - (PI2 / N) * floor((N * phi + PI) / PI2));
    r *= lerp(1.0f, polyModifier, polygonAmount);

    float2 result;
    result.x = r * cos(phi);
    result.y = r * sin(phi);

    return result;
}

// Maps a value inside the square [0,1]x[0,1] to a value in a disk of radius 1 using concentric squares.
// This mapping preserves area, bi continuity, and minimizes deformation.
// Based off the algorithm "A Low Distortion Map Between Disk and Square" by Peter Shirley and
// Kenneth Chiu.
float2 SquareToConcentricDiskMapping(float x, float y)
{
    float phi = 0.0f;
    float r = 0.0f;

    float a = 2.0f * x - 1.0f;
    float b = 2.0f * y - 1.0f;

    if (a > -b)                      // region 1 or 2
    {
        if (a > b)                   // region 1, also |a| > |b|
        {
            r = a;
            phi = (PI / 4.0f) * (b / a);
        }
        else // region 2, also |b| > |a|
        {
            r = b;
            phi = (PI / 4.0f) * (2.0f - (a / b));
        }
    }
    else // region 3 or 4
    {
        if (a < b)                   // region 3, also |a| >= |b|, a != 0
        {
            r = -a;
            phi = (PI / 4.0f) * (4.0f + (b / a));
        }
        else // region 4, |b| >= |a|, but a==0 and b==0 could occur.
        {
            r = -b;
            if (b != 0)
                phi = (PI / 4.0f) * (6.0f - (a / b));
            else
                phi = 0;
        }
    }

    float2 result;
    result.x = r * cos(phi);
    result.y = r * sin(phi);
    return result;
}

// Returns a random direction for sampling a GGX distribution (in tangent space)
float3 SampleDirectionGGX(float3 v, float3 n, float roughness, float u1, float u2)
{
    float theta = atan2(roughness * sqrt(u1), sqrt(1 - u1));
    float phi = 2 * PI * u2;

    float3 h;
    h.x = sin(theta) * cos(phi);
    h.y = sin(theta) * sin(phi);
    h.z = cos(theta);

    float3 sampleDir = 2.0f * dot(h, v) * h - v;
    return normalize(sampleDir);
}

float3 SampleGGXVisibleNormal(float3 wo, float ax, float ay, float u1, float u2)
{
    // Stretch the view vector so we are sampling as though
    // roughness==1
    float3 v = normalize(float3(wo.x * ax, wo.y * ay, wo.z));

    // Build an orthonormal basis with v, t1, and t2
    float3 t1 = (v.z < 0.999f) ? normalize(cross(v, float3(0, 0, 1))) : float3(1, 0, 0);
    float3 t2 = cross(t1, v);

    // Choose a point on a disk with each half of the disk weighted
    // proportionally to its projection onto direction v
    float a = 1.0f / (1.0f + v.z);
    float r = sqrt(u1);
    float phi = (u2 < a) ? (u2 / a) * PI : PI + (u2 - a) / (1.0f - a) * PI;
    float p1 = r * cos(phi);
    float p2 = r * sin(phi) * ((u2 < a) ? 1.0f : v.z);

    // Calculate the normal in this stretched tangent space
    float3 n = p1 * t1 + p2 * t2 + sqrt(max(0.0f, 1.0f - p1 * p1 - p2 * p2)) * v;

    // Unstretch and normalize the normal
    return normalize(float3(ax * n.x, ay * n.y, max(0.0f, n.z)));
}

// Returns a random direction on the unit sphere
float3 SampleDirectionSphere(float u1, float u2)
{
    float z = u1 * 2.0f - 1.0f;
    float r = sqrt(max(0.0f, 1.0f - z * z));
    float phi = 2 * PI * u2;
    float x = r * cos(phi);
    float y = r * sin(phi);

    return float3(x, y, z);
}

// Returns a random direction on the hemisphere around z = 1
float3 SampleDirectionHemisphere(float u1, float u2)
{
    float z = u1;
    float r = sqrt(max(0.0f, 1.0f - z * z));
    float phi = 2 * PI * u2;
    float x = r * cos(phi);
    float y = r * sin(phi);

    return float3(x, y, z);
}

// Returns a random cosine-weighted direction on the hemisphere around z = 1
float3 SampleDirectionCosineHemisphere(float u1, float u2)
{
    float2 uv = SquareToConcentricDiskMapping(u1, u2);
    float u = uv.x;
    float v = uv.y;

    // Project samples on the disk to the hemisphere to get a
    // cosine weighted distribution
    float3 dir;
    float r = u * u + v * v;
    dir.x = u;
    dir.y = v;
    dir.z = sqrt(max(0.0f, 1.0f - r));

    return dir;
}

// Returns a random direction from within a cone with angle == theta
float3 SampleDirectionCone(float u1, float u2, float cosThetaMax)
{
    float cosTheta = (1.0f - u1) + u1 * cosThetaMax;
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
    float phi = u2 * 2.0f * PI;
    return float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}
 
// Returns a direction that samples a rectangular area light
float3 SampleDirectionRectangularLight(float u1, float u2, float3 sourcePosW, float2 lightSizeW, float3 lightPosW)
{
    float x = u1 - 0.5f;
    float y = u2 - 0.5f;
     
    // Pick random sample point
    float3 samplePosW = lightPosW + x * lightSizeW.x + y * lightSizeW.y;
    float3 sampleDir = samplePosW - sourcePosW;
    return sampleDir;
}

// Returns the PDF for a particular GGX sample
float SampleDirectionGGX_PDF(float3 n, float3 h, float3 v, float roughness)
{
    float nDotH = saturate(dot(n, h));
    float hDotV = saturate(dot(h, v));
    float m2 = roughness * roughness;
    float x = nDotH * nDotH * (m2 - 1) + 1;
    float d = m2 / (PI * x * x);
    float pM = d * nDotH;
    return pM / (4 * hDotV);
}

// Returns the (constant) PDF of sampling uniform directions on the unit sphere
float SampleDirectionSphere_PDF()
{
    return 1.0f / (PI * 4.0f);
}

// Returns the (constant) PDF of sampling uniform directions on a unit hemisphere
float SampleDirectionHemisphere_PDF()
{
    return 1.0f / (PI * 2.0f);
}

// Returns the PDF of of a single sample on a cosine-weighted hemisphere
float SampleDirectionCosineHemisphere_PDF(float cosTheta)
{
    return cosTheta / PI;
}

// Returns the PDF of of a single sample on a cosine-weighted hemisphere
float SampleDirectionCosineHemisphere_PDF(float3 normal, float3 sampleDir)
{
    return saturate(dot(normal, sampleDir)) / PI;
}

// Returns the PDF of of a single uniform sample within a cone
float SampleDirectionCone_PDF(float cosThetaMax)
{
    return 1.0f / (2.0f * PI * (1.0f - cosThetaMax));
}

// Returns the PDF of of a single sample on a rectangular area light
float SampleDirectionRectangularLight_PDF(float lightArea, float3 sampleDir, float3 lightDir, float distanceToLight)
{
    float areaNDotL = saturate(dot(-sampleDir, lightDir));
    return (distanceToLight * distanceToLight) / (areaNDotL * lightArea);
}

uint CMJPermute(uint i, uint l, uint p)
{
    uint w = l - 1;
    w |= w >> 1;
    w |= w >> 2;
    w |= w >> 4;
    w |= w >> 8;
    w |= w >> 16;
    do
    {
        i ^= p;
        i *= 0xe170893d;
        i ^= p >> 16;
        i ^= (i & w) >> 4;
        i ^= p >> 8;
        i *= 0x0929eb3f;
        i ^= p >> 23;
        i ^= (i & w) >> 1;
        i *= 1 | p >> 27;
        i *= 0x6935fa69;
        i ^= (i & w) >> 11;
        i *= 0x74dcb303;
        i ^= (i & w) >> 2;
        i *= 0x9e501cc3;
        i ^= (i & w) >> 2;
        i *= 0xc860a3df;
        i &= w;
        i ^= i >> 5;
    }
    while (i >= l);
    return (i + p) % l;
}

float CMJRandFloat(uint i, uint p)
{
    i ^= p;
    i ^= i >> 17;
    i ^= i >> 10;
    i *= 0xb36534e5;
    i ^= i >> 12;
    i ^= i >> 21;
    i *= 0x93fc4795;
    i ^= 0xdf6e307f;
    i ^= i >> 17;
    i *= 1 | p >> 18;
    return i * (1.0f / 4294967808.0f);
}
 
// Returns a 2D sample from a particular pattern using correlated multi-jittered sampling [Kensler 2013]
float SampleCMJ1D(uint sampleSetIndex, uint sampleSetCount, uint pattern)
{
    uint sx = CMJPermute(sampleSetIndex, sampleSetCount, pattern * 0x68bc21eb);
    float jx = CMJRandFloat(sampleSetIndex, pattern * 0x967a889b);
    return (sx + jx) / sampleSetCount;
}

 // Returns a 2D sample from a particular pattern using correlated multi-jittered sampling [Kensler 2013]
float2 SampleCMJ2D(uint sampleSetIndex, uint sampleSetX, uint sampleSetY, uint pattern)
{
    uint N = sampleSetX * sampleSetY;
    sampleSetIndex = CMJPermute(sampleSetIndex, N, pattern * 0x51633e2d);
    uint sx = CMJPermute(sampleSetIndex % sampleSetX, sampleSetX, pattern * 0x68bc21eb);
    uint sy = CMJPermute(sampleSetIndex / sampleSetX, sampleSetY, pattern * 0x02e5be93);
    float jx = CMJRandFloat(sampleSetIndex, pattern * 0x967a889b);
    float jy = CMJRandFloat(sampleSetIndex, pattern * 0x368cc8b7);
    return float2((sx + (sy + jx) / sampleSetY) / sampleSetX, (sampleSetIndex + jy) / N);
}

float PowerHeuristic(int nf, float pdfF, int ng, float pdfG)
{
    float f = nf * pdfF;
    float g = ng * pdfG;
    float f2 = f * f;
    return f2 / (f2 + g * g);
}
 
namespace Catmul
{ 
    struct Parameter
    {
        float2 origin;
        float2 weight[4]; 
        float2 weight12;
        float2 offset12;
        
        float2 samplePos0;
        float2 samplePos3;
        float2 samplePos12;
        
        void Initialize(float2 uv, float2 texSize, float2 invTexSize)
        {  
            // We're going to sample a a 4x4 grid of texels surrounding the target UV coordinate. We'll do this by rounding
            // down the sample location to get the exact center of our "starting" texel. The starting texel will be at
            // location [1, 1] in the grid, where [0, 0] is the top left corner.
            float2 samplePos = uv * texSize;
            origin = floor(samplePos - 0.5f) + 0.5f;
            //float2 texPos1 = samplePos;
        
            // Compute the fractional offset from our starting texel to our original sample location, which we'll
            // feed into the Catmull-Rom spline function to get our filter weights.
            float2 f = samplePos - origin;
            float2 f2 = f * f;
            float2 f3 = f2 * f;
            
            // Compute the Catmull-Rom weights using the fractional offset that we calculated earlier.
            // These equations are pre-expanded based on our knowledge of where the texels will be located,
            // which lets us avoid having to evaluate a piece-wise function.
            weight[0] = f * (-0.5f + f * (1.0f - 0.5f * f));
            weight[1] = 1.0f + f2 * (-2.5f + 1.5f * f);
            weight[2] = f * (0.5f + f * (2.0f - 1.5f * f));
            weight[3] = f2 * (-0.5f + 0.5f * f);
            
             // Work out weighting factors and sampling offsets that will let us use bilinear filtering to
            // simultaneously evaluate the middle 2 samples from the 4x4 grid.
            weight12 = weight[1] + weight[2];
            offset12 = weight[2] / (weight[1] + weight[2]);

            // Compute the final UV coordinates we'll use for sampling the texture
            samplePos0 = (origin - 1) * invTexSize;
            samplePos3 = (origin + 2) * invTexSize;
            samplePos12 = (origin + offset12) * invTexSize;
        }
    };
    float2 Compute(in Texture2D<float2> tex, in SamplerState linearSampler, Parameter param)
    {
        float2 result = 0.0f;
        result += tex.SampleLevel(linearSampler, float2(param.samplePos0.x, param.samplePos0.y), 0.0f) * param.weight[0].x * param.weight[0].y;
        result += tex.SampleLevel(linearSampler, float2(param.samplePos12.x, param.samplePos0.y), 0.0f) * param.weight12.x * param.weight[0].y;
        result += tex.SampleLevel(linearSampler, float2(param.samplePos3.x, param.samplePos0.y), 0.0f) * param.weight[3].x * param.weight[0].y;

        result += tex.SampleLevel(linearSampler, float2(param.samplePos0.x, param.samplePos12.y), 0.0f) * param.weight[0].x * param.weight12.y;
        result += tex.SampleLevel(linearSampler, float2(param.samplePos12.x, param.samplePos12.y), 0.0f) * param.weight12.x * param.weight12.y;
        result += tex.SampleLevel(linearSampler, float2(param.samplePos3.x, param.samplePos12.y), 0.0f) * param.weight[3].x * param.weight12.y;

        result += tex.SampleLevel(linearSampler, float2(param.samplePos0.x, param.samplePos3.y), 0.0f) * param.weight[0].x * param.weight[3].y;
        result += tex.SampleLevel(linearSampler, float2(param.samplePos12.x, param.samplePos3.y), 0.0f) * param.weight12.x * param.weight[3].y;
        result += tex.SampleLevel(linearSampler, float2(param.samplePos3.x, param.samplePos3.y), 0.0f) * param.weight[3].x * param.weight[3].y;

        return result;
    }
    float4 Compute(in Texture2D<float4> tex, in SamplerState linearSampler, Parameter param)
    {
        float4 result = 0.0f;
        result += tex.SampleLevel(linearSampler, float2(param.samplePos0.x, param.samplePos0.y), 0.0f) * param.weight[0].x * param.weight[0].y;
        result += tex.SampleLevel(linearSampler, float2(param.samplePos12.x, param.samplePos0.y), 0.0f) * param.weight12.x * param.weight[0].y;
        result += tex.SampleLevel(linearSampler, float2(param.samplePos3.x, param.samplePos0.y), 0.0f) * param.weight[3].x * param.weight[0].y;

        result += tex.SampleLevel(linearSampler, float2(param.samplePos0.x, param.samplePos12.y), 0.0f) * param.weight[0].x * param.weight12.y;
        result += tex.SampleLevel(linearSampler, float2(param.samplePos12.x, param.samplePos12.y), 0.0f) * param.weight12.x * param.weight12.y;
        result += tex.SampleLevel(linearSampler, float2(param.samplePos3.x, param.samplePos12.y), 0.0f) * param.weight[3].x * param.weight12.y;

        result += tex.SampleLevel(linearSampler, float2(param.samplePos0.x, param.samplePos3.y), 0.0f) * param.weight[0].x * param.weight[3].y;
        result += tex.SampleLevel(linearSampler, float2(param.samplePos12.x, param.samplePos3.y), 0.0f) * param.weight12.x * param.weight[3].y;
        result += tex.SampleLevel(linearSampler, float2(param.samplePos3.x, param.samplePos3.y), 0.0f) * param.weight[3].x * param.weight[3].y;

        return result;
    }
}

namespace TextureSampling
{
    float2 Bilinear(Texture2D<float2> tex, SamplerState linearSampler, float2 sampleCoord, float2 invTexutreSize)
    {
        float x = frac(sampleCoord.x);
        float y = frac(sampleCoord.y);

        int2 offset[4] =
        {
            uint2(0, 0), uint2(0, 1), uint2(1, 0), uint2(1, 1)
        };
        // bilinear weights
        const float w[4] = { (1 - x) * (1 - y), x * (1 - y), (1 - x) * y, x * y };

        float2 colorSum = float2(0, 0);
        float weightSum = 0;
        // perform the actual bilinear interpolation
        [unroll]
        for (int sampleIdx = 0; sampleIdx < 4; sampleIdx++)
        {
            float2 sampleUv = (sampleCoord + offset[sampleIdx]) * invTexutreSize;
            colorSum += w[sampleIdx] * tex.SampleLevel(linearSampler, sampleUv, 0);
            weightSum += w[sampleIdx];
        }
        return colorSum / weightSum;
    }
    float4 Bilinear(Texture2D<float4> tex, SamplerState linearSampler, float2 sampleCoord, float2 invTexutreSize)
    {
        float x = frac(sampleCoord.x);
        float y = frac(sampleCoord.y);

        int2 offset[4] =
        {
            uint2(0, 0), uint2(0, 1), uint2(1, 0), uint2(1, 1)
        };
        // bilinear weights
        const float w[4] = { (1 - x) * (1 - y), x * (1 - y), (1 - x) * y, x * y };

        float4 colorSum = float4(0, 0, 0, 0);
        float weightSum = 0;
        // perform the actual bilinear interpolation
        [unroll]
        for (int sampleIdx = 0; sampleIdx < 4; sampleIdx++)
        {
            float2 sampleUv = (sampleCoord + offset[sampleIdx]) * invTexutreSize;
            colorSum += w[sampleIdx] * tex.SampleLevel(linearSampler, sampleUv, 0);
            weightSum += w[sampleIdx];
        }
        return colorSum / weightSum;
    }
}
namespace CustomSampling
{
    struct BilinearParameter
    {
        float2 origin;
        float2 weights;
    };
    
    BilinearParameter GetBilinearFilter(float2 uv, float2 texSize)
    {
        BilinearParameter result;
        result.origin = floor(uv * texSize - 0.5);
        result.weights = frac(uv * texSize - 0.5);
        return result;
    }
    
    float4 GetBilinearCustomWeights(BilinearParameter param, float4 customWeights)
    {
        float4 weights;
        weights.x = (1.0 - param.weights.x) * (1.0 - param.weights.y);
        weights.y = param.weights.x * (1.0 - param.weights.y);
        weights.z = (1.0 - param.weights.x) * param.weights.y;
        weights.w = param.weights.x * param.weights.y;
        return weights * customWeights;
    }
     
    float4 ApplyBilinearCustomWeights(float4 s00, float4 s10, float4 s01, float4 s11, float4 w, bool normalize = true)
    {
        float4 r = s00 * w.x + s10 * w.y + s01 * w.z + s11 * w.w;
        return r * (normalize ? rcp(dot(w, 1.0)) : 1.0);
    }
   
    float4 ComputeBilinear(Texture2D<float4> tex, SamplerState linearSampler, BilinearParameter param, float2 invTexutreSize, float4 w)
    {
        int2 offset[4] =
        {
            uint2(0, 0), uint2(1, 0), uint2(0, 1), uint2(1, 1)
        };
        
        float4 s00 = tex.SampleLevel(linearSampler, (param.origin + offset[0]) * invTexutreSize, 0);
        float4 s10 = tex.SampleLevel(linearSampler, (param.origin + offset[1]) * invTexutreSize, 0);
        float4 s01 = tex.SampleLevel(linearSampler, (param.origin + offset[2]) * invTexutreSize, 0);
        float4 s11 = tex.SampleLevel(linearSampler, (param.origin + offset[3]) * invTexutreSize, 0);
        
        return ApplyBilinearCustomWeights(s00, s10, s01, s11, w);
    }
}