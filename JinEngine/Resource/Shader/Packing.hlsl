#pragma once
#include"Math.hlsl"  

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
//xy = 10bit, z = 2bit
float3 SignedOctEncode(float3 n)
{
    float3 OutN;

    n /= (abs(n.x) + abs(n.y) + abs(n.z));

    OutN.y = n.y * 0.5 + 0.5;
    OutN.x = n.x * 0.5 + OutN.y;
    OutN.y = n.x * -0.5 + OutN.y;
	 
    OutN.z = saturate(n.z * FLT_MAX);
    return OutN;
}
float3 SignedOctDecode(float3 n)
{
    float3 OutN;

    OutN.x = (n.x - n.y);
    OutN.y = (n.x + n.y) - 1.0;
    OutN.z = n.z * 2.0 - 1.0;
    OutN.z = OutN.z * (1.0 - abs(OutN.x) - abs(OutN.y));
  
    return normalize(OutN);
}
uint2 PackSignedOct(float3 rgb)
{
    //10, 10, 2 -> uint2(32(full), 32(1bit))
    uint r = f32tof16(rgb.x);
    uint g = f32tof16(rgb.y);
	//z is 0 or 1 1(byte)
    return uint2(r | (g << 16), ((uint) rgb.z) & 0x1);
}
float3 UnpackSignedOct(uint2 rgb)
{
    float r = f16tof32(rgb.x & 0xFFFF);
    float g = f16tof32(rgb.x >> 16); 
    return float3(r, g, (float) (rgb.y & 0x1));
}

//https://www.jeremyong.com/graphics/2023/01/09/tangent-spaces-and-diamond-encoding/
float EncodeDiamond(float2 p)
{
    // Project to the unit diamond, then to the x-axis.
    float x = p.x / (abs(p.x) + abs(p.y));

    // Contract the x coordinate by a factor of 4 to represent all 4 quadrants in
    // the unit range and remap
    float py_sign = sign(p.y);
    return -py_sign * 0.25f * x + 0.5f + py_sign * 0.25f;
}
float2 DecodeDiamond(float p)
{
    float2 v;

    // Remap p to the appropriate segment on the diamond
    float p_sign = sign(p - 0.5f);
    v.x = -p_sign * 4.f * p + 1.f + p_sign * 2.f;
    v.y = p_sign * (1.f - abs(v.x));

    // Normalization extends the point on the diamond back to the unit circle
    return normalize(v);
}

// Given a normal and tangent vector, encode the tangent as a single float that can be
// subsequently quantized.
float EncodeTangent(float3 normal, float3 tangent)
{
    // First, find a canonical direction in the tangent plane
    float3 t1;
    if (abs(normal.y) > abs(normal.z))
    {
        // Pick a canonical direction orthogonal to n with z = 0
        t1 = float3(normal.y, -normal.x, 0.f);
    }
    else
    {
        // Pick a canonical direction orthogonal to n with y = 0
        t1 = float3(normal.z, 0.f, -normal.x);
    }
    t1 = normalize(t1);

    // Construct t2 such that t1 and t2 span the plane
    float3 t2 = cross(t1, normal);

    // Decompose the tangent into two coordinates in the canonical basis
    float2 packed_tangent = float2(dot(tangent, t1), dot(tangent, t2));

    // Apply our diamond encoding to our two coordinates
    return EncodeDiamond(packed_tangent);
}

float3 DecodeTangent(float3 normal, float diamond_tangent)
{
    // As in the encode step, find our canonical tangent basis span(t1, t2)
    float3 t1;
    if (abs(normal.y) > abs(normal.z))
    {
        t1 = float3(normal.y, -normal.x, 0.f);
    }
    else
    {
        t1 = float3(normal.z, 0.f, -normal.x);
    }
    t1 = normalize(t1);

    float3 t2 = cross(t1, normal);

    // Recover the coordinates used with t1 and t2
    float2 packed_tangent = DecodeDiamond(diamond_tangent);

    return packed_tangent.x * t1 + packed_tangent.y * t2;
}

uint2 PackRGB16(float3 rgb)
{
    uint3 urgb = f32tof16(rgb);
    return uint2(urgb.x | (urgb.y << 16), urgb.z);
}
float3 UnpackRGB16(uint2 rgb)
{
    return f16tof32(uint3(rgb.x & 0xFFFF, rgb.x >> 16, rgb.y & 0xFFFF));
} 

uint PackUnorm8(float v)
{
    v = isnan(v) ? 0.f : saturate(v);
    return (uint) trunc(v * 255.0f + 0.5f);
}
float UnpackUnorm8(uint packed)
{
    return float(packed & 0xFF) * (1.f / 255.0f);
}
uint PackUnorm10(float v)
{
    v = isnan(v) ? 0.0f : saturate(v);
    return (uint) trunc(v * 1023.0f + 0.5f);
}
float UnpackUnorm10(uint packed)
{
    return float(packed & 0x3FF) * (1.0f / 1023.0f);
} 
uint PackUnorm16(float v)
{
    v = isnan(v) ? 0.0f : saturate(v);
    return (uint) trunc(v * 65535.0f + 0.5f);
}
float UnpackUnorm16(uint packed)
{
    return float(packed & 0xFFFF) * (1.0f / 65535.0f);
} 
int PackSnorm16(float v)
{
    v = isnan(v) ? 0.f : min(max(v, -1.f), 1.f);
    return (int) trunc(v * 32767.f + (v >= 0.f ? 0.5f : -0.5f));
}
float UnpackSnorm16(int packed)
{
    int bits = (int) (packed << 16) >> 16;
    precise float unpacked = max((float) bits / 32767.f, -1.0f);
    return unpacked;
}

// Designed to compress (-256.0, +256.0) with a signed 6e3 float
uint PackVelocityXY(float x)
{
    uint signbit = asuint(x) >> 31;
    x = clamp(abs(x / 32768.0), 0, asfloat(0x3BFFE000));
    return (f32tof16(x) + 8) >> 4 | signbit << 9;
}
float UnpackVelocityXY(uint x)
{
    return f16tof32((x & 0x1FF) << 4 | (x >> 9) << 15) * 32768.0;
}
// Designed to compress (-1.0, 1.0) with a signed 8e3 float
uint PackVelocityZ(float x)
{
    uint signbit = asuint(x) >> 31;
    x = clamp(abs(x / 128.0), 0, asfloat(0x3BFFE000));
    return (f32tof16(x) + 2) >> 2 | signbit << 11;
}
float UnpackVelocityZ(uint x)
{
    return f16tof32((x & 0x7FF) << 2 | (x >> 11) << 15) * 128.0;
}
//ref miniengine velocity packing
// Pack the velocity to write to R10G10B10A2_UNORM
uint PackVelocity(float3 velocity)
{ 
    return PackVelocityXY(velocity.x) | (PackVelocityXY(velocity.y) << 10) | (PackVelocityZ(velocity.z) << 20);
}
// Unpack the velocity from R10G10B10A2_UNORM
float3 UnpackVelocity(uint velocity)
{ 
    return float3(UnpackVelocityXY(velocity & 0x3FF), UnpackVelocityXY((velocity >> 10) & 0x3FF), UnpackVelocityZ(velocity >> 20));
} 