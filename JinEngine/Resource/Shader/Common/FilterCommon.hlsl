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
#include"Math.hlsl"

#ifdef USE_3x3_KERNEL
#define KERNEL_RADIUS 1
#elif USE_5x5_KERNEL
#define KERNEL_RADIUS 2
#elif USE_7x7_KERNEL
#define KERNEL_RADIUS 3
#elif USE_2x2_KERNEL
#define KERNEL_RADIUS 1
#define KERNEL_COUNT_IS_EVEN_NUMBER 1
#else 
#define KERNEL_RADIUS 1
#endif

#ifdef KERNEL_COUNT_IS_EVEN_NUMBER
#define KERNEL_SIZE KERNEL_RADIUS * 2
#define KERNEL_START -KERNEL_RADIUS + 1
#define KERNEL_END KERNEL_RADIUS
#define KERNEL_OFFSET KERNEL_RADIUS - 1
#else
#define KERNEL_SIZE KERNEL_RADIUS * 2 + 1
#define KERNEL_START -KERNEL_RADIUS
#define KERNEL_END KERNEL_RADIUS
#define KERNEL_OFFSET KERNEL_RADIUS
#endif

#ifndef KERNEL_MAX_SIZE 
#define KERNEL_MAX_SIZE 7
#endif
 
#define FILTER_EPSILON 1e-06
 
float ComputeGaussian(const float x, const float sharpness)
{
    return exp(-(x * x) / (2.0f * sharpness * sharpness)) / sqrt((2.0f * PI * sharpness * sharpness));
}
float ComputeGaussian(const float x, const float y, const float sharpness)
{
    return exp(-(x * x + y * y) / (2.0f * sharpness * sharpness)) / sqrt((2.0f * PI * sharpness * sharpness));
}
//return std::exp(-(x * x + y * y) / (2.0f * sig * sig)) / (2.0f * PI * sig * sig);
float ComputeGaussianCheap(const float x)
{
   //return exp(-x * x);
    return exp(-0.66f * x * x);
}
float ComputeGaussianCheap(const float x, const float y)
{
   //return exp(-x * x);
    return exp(-0.66f * (x * x + y * y));
}

//Edge stopping
//ref svgf paper 4.4 Edge-stopping function
namespace CrossBilateral
{
    namespace Normal
    {
        struct Parameters
        {
            float3 targetNormal;
            float3 sampleNormal;
            float sigma;
            float sigmaExponent;
            
            void Initialize(float3 _targetNormal, float _sigma, float _sigmaExponent)
            {
                targetNormal = _targetNormal;
                sampleNormal = _targetNormal;
                sigma = _sigma;
                sigmaExponent = _sigmaExponent;
            }
            void Initialize(float3 _targetNormal)
            {
                Initialize(_targetNormal, 1.0f, 128.0f);
            }
            void Update(float3 _sampleNormal)
            {
                sampleNormal = _sampleNormal;
            };
        };
        float ComputeWeight(in Parameters params)
        {
            float dotTS = dot(params.targetNormal, params.sampleNormal);

            // Apply adjustment scale to the dot product. 
            // Values greater than 1 increase tolerance scale 
            // for unwanted inflated normal differences,
            // such as due to low-precision normal quantization.
            dotTS *= params.sigma;

            float normalWeights = pow(saturate(dotTS), params.sigmaExponent);
            return normalWeights;
        }
    }
    namespace Depth
    {
        struct Parameters
        {
            float targetViewZ;
            float sampleViewZ;
            float sigma;
            float2 targetGradient;
            float2 offset; //targetPosition - samplePosition
            
            void Initialize(float _targetViewZ, float2 _targetGradient, float _sigma)
            {
                targetViewZ = _targetViewZ;
                sampleViewZ = _targetViewZ;
                targetGradient = _targetGradient;
                sigma = _sigma;
                offset = float2(0, 0);
            }
            void Initialize(float _targetViewZ, float2 _targetGradient)
            {
                Initialize(_targetViewZ, _targetGradient, 1.0f);
            }
            void Update(float _sampleViewZ, float2 _offset)
            {
                sampleViewZ = _sampleViewZ;
                offset = _offset;
            };
        };
        float ComputeExponentialParameter(in Parameters params)
        {
            float distance = abs(params.targetViewZ - params.sampleViewZ);
            float spaceDerivative = dot(abs(params.targetGradient), abs(params.offset)) * params.sigma;
            return -distance / (spaceDerivative + FILTER_EPSILON);
        }
        float ComputeWeight(in Parameters params)
        {
            return exp(ComputeExponentialParameter(params));
        }
    }
    namespace Luminance
    {
        struct Parameters
        {
            float targetLuminance;
            float sampleLuminance;
            float sqrtGausLuminanceVariance; //applied sqrt(gaus3x3(targetLuminance))
             
            float ComputeSqrtVariance(float gausAppliedVariance, float sigma)
            {
                return sigma * sqrt(max(gausAppliedVariance + FILTER_EPSILON, 0));
            }
            void Initialize(float _targetLuminance, float _sqrtGausLuminanceVariance)
            {
                targetLuminance = _targetLuminance;
                sampleLuminance = _targetLuminance;
                sqrtGausLuminanceVariance = _sqrtGausLuminanceVariance;
            }
            /**
            *@ brief svgf color history component w에 variance값을 저장한다.
            */
            void Initialize(float _targetLuminance, float2 uv, float2 invTextureSize, Texture2D colorMap, SamplerState samLinear, float sigma)
            {
                float variance = 0.0f;
                const float kernel[2][2] =
                {
                    { 1.0f / 4.0f, 1.0f / 8.0f },
                    { 1.0f / 8.0f, 1.0f / 16.0f },
                };
                    
                [unroll]
                for (int yy = -1; yy <= 1; yy++)
                {
                    [unroll]
                    for (int xx = -1; xx <= 1; xx++)
                    {
                        const float2 sampleUv = uv + float2(xx, yy) * invTextureSize;
                        const float k = kernel[abs(xx)][abs(yy)];   
                        variance += colorMap.SampleLevel(samLinear, sampleUv, 0).w * k;
                    }
                }  
                float _sqrtGausLuminanceVariance = ComputeSqrtVariance(variance, sigma);
                Initialize(_targetLuminance, _sqrtGausLuminanceVariance);
            }
            void Initialize(float _targetLuminance, float2 uv, float2 invTextureSize, Texture2D colorMap, SamplerState samLinear)
            {
                Initialize(_targetLuminance, uv, invTextureSize, colorMap, samLinear, 128.0f);
            }
            void Update(float _sampleLuminance)
            {
                sampleLuminance = _sampleLuminance;
            };
        };
        float ComputeExponentialParameter(in Parameters params)
        {
            float difference = abs(params.targetLuminance - params.sampleLuminance);
            return -difference / (params.sqrtGausLuminanceVariance);
        }
        float ComputeWeight(in Parameters params)
        {
            return exp(ComputeExponentialParameter(params));
        }
    }
    namespace NormalDepth
    {
        struct Parameters
        {
            Normal::Parameters normal;
            Depth::Parameters depth;
        };
        float ComputeWeight(in Parameters params)
        {
            return Normal::ComputeWeight(params.normal) * Depth::ComputeWeight(params.depth);
        }
    }
    namespace NormalDepthLuminance
    {
        struct Parameters
        {
            Normal::Parameters normal;
            Depth::Parameters depth;
            Luminance::Parameters luminance;
        };
        float ComputeWeight(in Parameters params)
        {
            float exponentialParameter = Depth::ComputeExponentialParameter(params.depth) + Luminance::ComputeExponentialParameter(params.luminance);
            return Normal::ComputeWeight(params.normal) * exp(exponentialParameter);
        }
    }
}

/* 
reference :
- s22699 Fast Denoising with Self Stabilizing Recurrent Blurs
- Raytracing gems2 ch 48 ReBlur
- 2023 ReSTIR_Course_Cyberpunk_2077_Integration
*/ 
namespace ReCurrent
{ 
    float GetGeometryWeight(float3 centerPosition, float3 samplePosition, float3 centerNormal, float planeDistNorm)
    {
        // where planeDistNorm = accumSpeedFactor / ( 1.0 + centerZ ) 
        //It represents { 1 / "max possible allowed distance between a point and the plane"       
        float3 ray = samplePosition - centerPosition;
        float distToPlane = dot(centerNormal, ray);
        return saturate(1.0f - abs(distToPlane) * planeDistNorm);
    }
    float GetNormalWeight(float3 centerNormal, float3 sampleNormal, float sampleRoughness, float accumSpeed)
    {
        float roughness2 = sampleRoughness * sampleRoughness;
        float a0 = 90.0f * roughness2 / (1.0f + roughness2);
          
        a0 = a0 * lerp(0.0f, 1.0f, accumSpeed) + DegToRad(0.5f); // Optional
        float cosa = saturate(dot(centerNormal, sampleNormal));
        float a = ApproxAcos(cosa);
        float w = LinearStep(a0, 0.0f, a);
        return w;
    }
    float GetRoughnessWeight(float centerRoughness, float sampleRoughness)
    {
        float norm = centerRoughness * centerRoughness * 0.99f + 0.01f;
        float w = abs(centerRoughness - sampleRoughness) * rcp(norm);
        return saturate(1.0 - w);
    }
    
    float3 GetSpecularDominantDirection(float3 normal, float3 view, float roughness)
    { 
        float f = (1.0 - roughness) * (sqrt(1.0 - roughness) + roughness);
        float3 R = reflect(-view, normal);
        float3 dir = lerp(normal, R, f);
        return normalize(dir);
    }
    float GetSpecularDominantFactor(float dotNV, float roughness)
    {
        float a = 0.298475 * log(39.4115 - 39.0029 * roughness);
        float f = pow(saturate(1.0 - dotNV), 10.8649) * (1.0 - a) + a;
        return saturate(f);
    }
   
    float2x3 GetDisffuseKernelBasis(float3 normalV, float worldRadius)
    {
        float3x3 basis = CalBasis(normalV);
        return float2x3(basis[0] * worldRadius, basis[1] * worldRadius);
    }
    float2x3 GetSpecularKernelBasis(float3 posV, float3 normalV, float worldRadius, float roughness, float normalizedAccumFrameNum)
    {
        float3 view = -normalize(posV); // Assuming view space
        float3 dir = GetSpecularDominantDirection(normalV, view, roughness);
        
        float3x3 basis = CalBasis(normalV);
        float3 T = basis[0];
        float3 B = basis[1];
        
        float dotDN = abs(dot(dir, normalV));
        if (dotDN < 0.999f)
        {
            float3 reflectDir = reflect(-dir, normalV);
            T = normalize(cross(normalV, reflectDir)); // IMPORTANT: doesn’t handle the case when N = R!
            B = cross(reflectDir, T);

            float angle = saturate(ApproxAcos(abs(normalV.z)) / (PI * 0.5f));
            float skewFactor = lerp(1.0, roughness, angle);
            T *= lerp(1.0f, skewFactor, normalizedAccumFrameNum);
        }
        T *= worldRadius;
        B *= worldRadius;
        return float2x3(T, B);
    }
}
 