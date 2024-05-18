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
#define PI 3.1415926 
#define PI2 6.2831852 
#define FLT_MAX 3.402823466e+38F 
#define UINT_MAX 0xffffffff

#define JINENGINE_MATH 1
#define EPSILON 1e-06

float DegToRad(float deg)
{
    return deg * (PI / 180.0f);
}
float RadToDeg(float rad)
{
    return rad * (180.0f / PI);
}
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
// http://marc-b-reynolds.github.io/quaternions/2016/07/06/Orthonormal.html
float3x3 CalBasis(float3 N)
{
    float sz = sign(N.z);
    float a = 1.0 / (sz + N.z);
    float ya = N.y * a;
    float b = N.x * ya;
    float c = N.x * sz;

    float3 T = float3(c * N.x * a - 1.0, sz * b, c);
    float3 B = float3(b, N.y * ya - sz, N.y);

    // Note: due to the quaternion formulation, the generated frame is rotated by 180 degrees,
    // s.t. if N = (0, 0, 1), then T = (-1, 0, 0) and B = (0, -1, 0).
    return float3x3(T, B, N);
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
float2 ApproxAcos(float2 x)
{
    return _ApproxAcos(x);
}
float3 ApproxAcos(float3 x)
{
    return _ApproxAcos(x);
}
float4 ApproxAcos(float4 x)
{
    return _ApproxAcos(x);
}

// REQUIREMENT: a < b
#define _LinearStep( a, b, x ) saturate( ( x - a ) / ( b - a ) )

float LinearStep(float a, float b, float x)
{
    return _LinearStep(a, b, x);
}
float2 LinearStep(float2 a, float2 b, float2 x)
{
    return _LinearStep(a, b, x);
}
float3 LinearStep(float3 a, float3 b, float3 x)
{
    return _LinearStep(a, b, x);
}
float4 LinearStep(float4 a, float4 b, float4 x)
{
    return _LinearStep(a, b, x);
}

 