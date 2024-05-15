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
#include "LightShapeDrawCommon.hlsl" 
 
struct PsIn
{
	float4 posH : SV_POSITION; 
#ifdef USE_VERTEX_PACK
	nointerpolation uint4 vertexPos : VERTEX_POSITION;
	nointerpolation uint veretex2PosZ : VERTEX_POSITION_Z;
#else
	nointerpolation float3 vertexPosV[3] : VERTEX_POSITION;
#endif
	nointerpolation uint rtIndex : SV_RenderTargetArrayIndex;
};

bool linesegment_vs_plane(float3 p0, float3 p1, float3 pn, out float lerp_val)
{
	float3 u = p1 - p0;

	float D = dot(pn, u);
	float N = -dot(pn, p0);

    //IEEE 754-2008 in HLSL tricks
	lerp_val = N / D; //Division by zero returns -/+ INF (except for 0/0 which returns NaN)
	return !(lerp_val != saturate(lerp_val)); //saturate NaN returns 0 and saturate -/+INF returns correct value //The comparison NE, when either or both operands is NaN returns TRUE.
}

bool is_in_xslice(float3 top_plane, float3 bottom_plane, float3 vert_point)
{
	return (top_plane.y * vert_point.y + top_plane.z * vert_point.z >= 0.0f && bottom_plane.y * vert_point.y + bottom_plane.z * vert_point.z >= 0.0f);
}

bool is_in_yslice(float3 left_plane, float3 right_plane, float3 vert_point)
{
	return (left_plane.x * vert_point.x + left_plane.z * vert_point.z >= 0.0f && right_plane.x * vert_point.x + right_plane.z * vert_point.z >= 0.0f);
}

bool ray_vs_triangle(float3 ray_dir, float3 vert0, float3 vert1, float3 vert2, out float z_pos)
{
	float3 e1 = vert1 - vert0;
	float3 e2 = vert2 - vert0;
	float3 q = cross(ray_dir, e2);
	float a = dot(e1, q);
	
	if (a > -0.000001f && a < 0.000001f)
		return false;
	
	float f = 1.0f / a;
	float u = f * dot(-vert0, q);

	if (u != saturate(u)) 
		return false;
	
	float3 r = cross(-vert0, e1);
	float v = f * dot(ray_dir, r);

	if (v < 0.0f || (u + v) > 1.0f) 
		return false;
	
	z_pos = f * dot(e2, r) * ray_dir.z;

	return true;
}

float2 min_max(float2 depth_min_max, in float depth)
{
	depth_min_max.x = min(depth_min_max.x, depth);
	depth_min_max.y = max(depth_min_max.y, depth);
	
	return depth_min_max;
} 

float2 PS(PsIn pin, bool isFrontFace : SV_IsFrontFace) : SV_TARGET
{
#ifdef USE_VERTEX_PACK
	float3 vert0 = f16tof32(uint3((pin.vertexPos.x >> 16), (pin.vertexPos.x & 0xFFFF), (pin.vertexPos.y >> 16)));
	float3 vert1 = f16tof32(uint3((pin.vertexPos.y & 0xFFFF), (pin.vertexPos.z >> 16), (pin.vertexPos.z & 0xFFFF)));
	float3 vert2 = f16tof32(uint3((pin.vertexPos.w >> 16), (pin.vertexPos.w & 0xFFFF), (pin.veretex2PosZ & 0xFFFF)));
#else
	float3 vert0 = pin.vertexPosV[0];
	float3 vert1 = pin.vertexPosV[1];
	float3 vert2 = pin.vertexPosV[2];
#endif
	  
	float farZ = cbCam.farZ;
	float2 depthMinMax = float2(farZ, 0.0f);

	const float ey = tan(PI / 4.0f * 0.5f);
    const float ex = ey * cbCam.renderTargetSize.x / cbCam.renderTargetSize.y;

	const float3 left_plane		= normalize(float3(1.0f, 0.0f, (1.0f - 2.0f * floor(pin.posH.x) / CLUSTER_DIM_X) * ex));
	const float3 right_plane	= -normalize(float3(1.0f, 0.0f, (1.0f - 2.0f * floor(pin.posH.x + 1.0f) / CLUSTER_DIM_X) * ex));
	const float3 top_plane		= normalize(float3(0.0f, -1.0f, (1.0f - 2.0f * floor(pin.posH.y) / CLUSTER_DIM_Y) * ey));
	const float3 bottom_plane	= -normalize(float3(0.0f, -1.0f, (1.0f - 2.0f * floor(pin.posH.y + 1.0f) / CLUSTER_DIM_Y) * ey));
	
	////////////////////////////////////////////////////////////////////////
	//Case where the min/max depth is one of the corners of the tile
	//cross product of plane normals return the ray direction through the corner. All planes and rays go through (0,0,0)
	float z_pos;
	depthMinMax = ray_vs_triangle(cross(top_plane, left_plane), vert0, vert1, vert2, z_pos) ? min_max(depthMinMax, z_pos) : depthMinMax;
	depthMinMax = ray_vs_triangle(cross(top_plane, right_plane), vert0, vert1, vert2, z_pos) ? min_max(depthMinMax, z_pos) : depthMinMax;
	depthMinMax = ray_vs_triangle(cross(right_plane, bottom_plane), vert0, vert1, vert2, z_pos) ? min_max(depthMinMax, z_pos) : depthMinMax;
	depthMinMax = ray_vs_triangle(cross(bottom_plane, left_plane), vert0, vert1, vert2, z_pos) ? min_max(depthMinMax, z_pos) : depthMinMax;

	///////////////////////////////////////////////////////////////////////
	//Case where a vertex is the min/max depth of the tile
	//Check if vertex is inside all four planes
	depthMinMax = (is_in_xslice(top_plane, bottom_plane, vert0) && is_in_yslice(left_plane, right_plane, vert0)) ? min_max(depthMinMax, vert0.z) : depthMinMax;
	depthMinMax = (is_in_xslice(top_plane, bottom_plane, vert1) && is_in_yslice(left_plane, right_plane, vert1)) ? min_max(depthMinMax, vert1.z) : depthMinMax;
	depthMinMax = (is_in_xslice(top_plane, bottom_plane, vert2) && is_in_yslice(left_plane, right_plane, vert2)) ? min_max(depthMinMax, vert2.z) : depthMinMax;

	////////////////////////////////////////////////////////////////////////
	//Case where a vertex edge intersects a tile side is the min/max depth
	float lerp_val;
	
	///Left side
	depthMinMax = is_in_xslice(top_plane, bottom_plane, linesegment_vs_plane(vert0, vert1, left_plane, lerp_val) ? lerp(vert0, vert1, lerp_val) : float3(0, 0, -1)) ? min_max(depthMinMax, lerp(vert0.z, vert1.z, lerp_val)) : depthMinMax;
	depthMinMax = is_in_xslice(top_plane, bottom_plane, linesegment_vs_plane(vert1, vert2, left_plane, lerp_val) ? lerp(vert1, vert2, lerp_val) : float3(0, 0, -1)) ? min_max(depthMinMax, lerp(vert1.z, vert2.z, lerp_val)) : depthMinMax;
	depthMinMax = is_in_xslice(top_plane, bottom_plane, linesegment_vs_plane(vert2, vert0, left_plane, lerp_val) ? lerp(vert2, vert0, lerp_val) : float3(0, 0, -1)) ? min_max(depthMinMax, lerp(vert2.z, vert0.z, lerp_val)) : depthMinMax;

	//Right side
	depthMinMax = is_in_xslice(top_plane, bottom_plane, linesegment_vs_plane(vert0, vert1, right_plane, lerp_val) ? lerp(vert0, vert1, lerp_val) : float3(0, 0, -1)) ? min_max(depthMinMax, lerp(vert0.z, vert1.z, lerp_val)) : depthMinMax;
	depthMinMax = is_in_xslice(top_plane, bottom_plane, linesegment_vs_plane(vert1, vert2, right_plane, lerp_val) ? lerp(vert1, vert2, lerp_val) : float3(0, 0, -1)) ? min_max(depthMinMax, lerp(vert1.z, vert2.z, lerp_val)) : depthMinMax;
	depthMinMax = is_in_xslice(top_plane, bottom_plane, linesegment_vs_plane(vert2, vert0, right_plane, lerp_val) ? lerp(vert2, vert0, lerp_val) : float3(0, 0, -1)) ? min_max(depthMinMax, lerp(vert2.z, vert0.z, lerp_val)) : depthMinMax;

	//Bottom side
	depthMinMax = is_in_yslice(left_plane, right_plane, linesegment_vs_plane(vert0, vert1, bottom_plane, lerp_val) ? lerp(vert0, vert1, lerp_val) : float3(0, 0, -1)) ? min_max(depthMinMax, lerp(vert0.z, vert1.z, lerp_val)) : depthMinMax;
	depthMinMax = is_in_yslice(left_plane, right_plane, linesegment_vs_plane(vert1, vert2, bottom_plane, lerp_val) ? lerp(vert1, vert2, lerp_val) : float3(0, 0, -1)) ? min_max(depthMinMax, lerp(vert1.z, vert2.z, lerp_val)) : depthMinMax;
	depthMinMax = is_in_yslice(left_plane, right_plane, linesegment_vs_plane(vert2, vert0, bottom_plane, lerp_val) ? lerp(vert2, vert0, lerp_val) : float3(0, 0, -1)) ? min_max(depthMinMax, lerp(vert2.z, vert0.z, lerp_val)) : depthMinMax;

	//Top side
	depthMinMax = is_in_yslice(left_plane, right_plane, linesegment_vs_plane(vert0, vert1, top_plane, lerp_val) ? lerp(vert0, vert1, lerp_val) : float3(0, 0, -1)) ? min_max(depthMinMax, lerp(vert0.z, vert1.z, lerp_val)) : depthMinMax;
	depthMinMax = is_in_yslice(left_plane, right_plane, linesegment_vs_plane(vert1, vert2, top_plane, lerp_val) ? lerp(vert1, vert2, lerp_val) : float3(0, 0, -1)) ? min_max(depthMinMax, lerp(vert1.z, vert2.z, lerp_val)) : depthMinMax;
	depthMinMax = is_in_yslice(left_plane, right_plane, linesegment_vs_plane(vert2, vert0, top_plane, lerp_val) ? lerp(vert2, vert0, lerp_val) : float3(0, 0, -1)) ? min_max(depthMinMax, lerp(vert2.z, vert0.z, lerp_val)) : depthMinMax;

#ifdef LINEAR_DEPTH_DIST
	return isFrontFace ? float2(1.0f, (CLUSTER_DIM_Z - 1.0f) / 255.0f - floor(max(CLUSTER_DIM_Z * (depthMinMax.y / farZ), 0.0f)) / 255.0f)
	: float2(floor(max(CLUSTER_DIM_Z * (depthMinMax.x / farZ), 0.0f)) / 255.0f, 1.0f);
#else
	// Look up the light list for the cluster
	const float min_depth = log2(NEAR_CLUST);
	const float max_depth = log2(farZ);
	 
	const float scale = 1.0f / (max_depth - min_depth) * (CLUSTER_DIM_Z - 1.0f);
	const float bias = 1.0f - min_depth * scale;
	 
	return !isFrontFace ?
	float2(1.0f, (CLUSTER_DIM_Z - 1.0f) / CLUSTER_MAX_DEPTH - floor(max(log2(depthMinMax.x) * scale + bias, 0.0f)) / CLUSTER_MAX_DEPTH) :
	float2(floor(max(log2(depthMinMax.y) * scale + bias, 0.0f)) / CLUSTER_MAX_DEPTH, 1.0f);
	
	/*
	return !isFrontFace ?
	float2(1.0f, (CLUSTER_DIM_Z - 1.0f) / CLUSTER_MAX_DEPTH - floor(max(log2(depthMinMax.x) * scale + bias, 0.0f)) / CLUSTER_MAX_DEPTH) :
	float2(floor(max(log2(depthMinMax.y) * scale + bias, 0.0f)) / CLUSTER_MAX_DEPTH, 1.0f);
	*/
	 
	//Wrong winding order on triangles, should be the other way around.
	//return isFrontFace ? 
	//float2(1.0f, (CLUSTER_DIM_Z - 1.0f) / CLUSTER_MAX_DEPTH - floor(max(log2(depthMinMax.y) * scale + bias, 0.0f)) / CLUSTER_MAX_DEPTH) :
	//float2(floor(max(log2(depthMinMax.x) * scale + bias, 0.0f)) / CLUSTER_MAX_DEPTH, 1.0f);
#endif
}
