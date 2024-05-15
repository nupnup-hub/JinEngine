#pragma once
 
float2 CalculateCenterUv(int2 pixelCoord, float2 invSize)
{
    return (pixelCoord + 0.5f) * invSize;
}
bool IsValidCoord(int2 coord, float2 rtSize)
{
    return all(coord > int2(0, 0)) && all(coord < rtSize);
}
bool IsValidUv(float2 uv)
{
    return all(uv >= 0.0f) && all(uv <= 1.0f);
}