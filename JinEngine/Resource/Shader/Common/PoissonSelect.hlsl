#pragma once

#if defined(POISSON_8) 
#define USE_POISSON8_F3 
#define POISSON_SAMPLE_COUNT 8
#define HALF_SAMPLE_COUNT 4
#include"Poisson.hlsl" 
float3 GetPoisson(const uint index)
{
    return poissonDiskF3_8[index];
}
#elif defined(POISSON_16)
#define USE_POISSON16_F3 
#define POISSON_SAMPLE_COUNT 16
#define HALF_SAMPLE_COUNT 8
#include"Poisson.hlsl" 
float3 GetPoisson(const uint index)
{
    return poissonDiskF3_16[index];
}
#elif defined(POISSON_32)
#define USE_POISSON32_F3 
#define POISSON_SAMPLE_COUNT 32
#define HALF_SAMPLE_COUNT 16
#include"Poisson.hlsl" 
float3 GetPoisson(const uint index)
{
    return poissonDiskF3_32[index];
}
#endif
