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

bool IsValidUv(float2 uv)
{
    return all(uv >= 0.0f) && all(uv <= 1.0f);
}
float ComputeGaussian(const float x, const float sharpness)
{
    return exp(-(x * x) / (2.0f * sharpness * sharpness)) / sqrt((2.0f * PI * sharpness * sharpness));
}
float ComputeGaussianCheap(const float x)
{
   //return exp(-x * x);
    return exp(-0.66f * x * x);
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
            float2 offset;      //targetPosition - samplePosition
            
            void Initialize(float3 _targetViewZ, float2 _targetGradient, float _sigma)
            {
                targetViewZ = _targetViewZ;
                sampleViewZ = _targetViewZ;
                targetGradient = _targetGradient;
                sigma = _sigma;
                offset = float2(0, 0);
            }
            void Initialize(float3 _targetViewZ, float2 _targetGradient)
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
            void Initialize(float3 _targetLuminance, float _sqrtGausLuminanceVariance)
            {
                targetLuminance = _targetLuminance; 
                sampleLuminance = _targetLuminance;
                sqrtGausLuminanceVariance = _sqrtGausLuminanceVariance;
            } 
            /**
            *@ brief svgf color history component w에 variance값을 저장한다.
            */
            void Initialize(float3 _targetLuminance, float2 uv, float2 invTextureSize, Texture2D colorMap, SamplerState samLinear, float sigma)
            { 
                float variance = 0.0f;
                const float kernel[2][2] =
                {
                    { 1.0 / 4.0, 1.0 / 8.0 },
                    { 1.0 / 8.0, 1.0 / 16.0 },
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
            void Initialize(float3 _targetLuminance, float2 uv, float2 invTextureSize, Texture2D colorMap, SamplerState samLinear)
            {
                Initialize(_targetLuminance, uv, invTextureSize, colorMap, samLinear, 4.0f);
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
