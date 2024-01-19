#pragma once
#define PI 3.1415926 
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
float2 OctWrap(float2 v)
{
	return (1.0f - abs(v.yx)) * float2(v.x >= 0.0f ? 1.0f : -1.0f, v.y >= 0.0f ? 1.0f : -1.0f);
}
float2 EncodeOct(float3 n)
{
	n /= (abs(n.x) + abs(n.y) + abs(n.z));
	n.xy = n.z >= 0.0f ? n.xy : OctWrap(n.xy);
	n.xy = n.xy * 0.5f + 0.5f;
	return n.xy;
}
float3 DecodeOct(float2 f)
{
	f = f * 2.0f - 1.0f;
	float3 n = float3(f.x, f.y, 1.0f - abs(f.x) - abs(f.y));
	float t = saturate(-n.z);
	n.x += n.x >= 0.0f ? -t : t;
	n.y += n.y >= 0.0f ? -t : t;
	return normalize(n);
}
float3 SignedOctEncode(float3 n)
{
	float3 OutN;

	n /= (abs(n.x) + abs(n.y) + abs(n.z));

	OutN.y = n.y * 0.5 + 0.5;
	OutN.x = n.x * 0.5 + OutN.y;
	OutN.y = n.x * -0.5 + OutN.y;

	OutN.z = saturate(n.z * 3.402823466e+38F);
	return OutN;
}
float3 SignedOctDecode(float3 n)
{
	float3 OutN;

	OutN.x = (n.x - n.y);
	OutN.y = (n.x + n.y) - 1.0;
	OutN.z = n.z * 2.0 - 1.0;
	OutN.z = OutN.z * (1.0 - abs(OutN.x) - abs(OutN.y));
 
	OutN = normalize(OutN);
	return OutN;
}
float3 RestructionPosition(float3 viewRay, float linearDepth)
{
	return viewRay * linearDepth;
}
float3x3 CalTBN(float3 unitNormal, float3 tangent)
{
    // Build orthonormal basis.
	float3 N = unitNormal;
	float3 T = normalize(tangent - dot(tangent, N) * N);
	float3 B = cross(N, T);

	return float3x3(T, B, N);
}