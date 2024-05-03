#pragma once
#define PI 3.1415926 
#define PI2 6.2831852 
#define FLT_MAX 3.402823466e+38F 
#define UINT_MAX 0xffffffff

#define JINENGINE_MATH 1

float2 RandomRotationTrig(float random)
{ 
#if 1
	random *= 2.0f;
	float rotationAngle = random * PI;
	return float2(cos(rotationAngle), sin(rotationAngle));
#else
	random = mad(random, 2.0f, -1.0f);
	float rotationAngle = random * PI;
	return float2(cos(rotationAngle), sin(rotationAngle));
#endif
} 
float2 Rotate(float2 pos, float2 rotationTrig)
{
    return float2(pos.x * rotationTrig.x - pos.y * rotationTrig.y, pos.y * rotationTrig.x + pos.x * rotationTrig.y);
}
float3x3 CalTBN(float3 unitNormal, float3 tangent)
{
    // Build orthonormal basis.
	float3 N = unitNormal;
	float3 T = normalize(tangent - dot(tangent, N) * N);
	float3 B = cross(N, T);

	return float3x3(T, B, N);
}
float3 CalBinormal(float3 unitNormal, float3 tangent)
{
    // Build orthonormal basis.
    float3 N = unitNormal;
    float3 T = normalize(tangent - dot(tangent, N) * N);
    return cross(N, T);	 
} 
float FastAcos(float inX)
{
    float x = abs(inX);
    float res = -0.156583f * x + 1.570796f;
    res *= sqrt(1.0f - x);
    return (inX >= 0) ? res : PI - res;
}
#define _ApproxAcos( x ) ( sqrt( 2.0 ) * sqrt( saturate( 1.0 - x ) ) )
float ApproxAcos(float x)
{
    return _ApproxAcos(x);
}
 