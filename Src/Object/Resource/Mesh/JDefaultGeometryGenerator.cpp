#include<iostream>
#include <algorithm>
#include<fstream>
#include "JDefaultGeometryGenerator.h"

namespace JinEngine
{
	//***************************************************************************************
// JDefaultGeometryGenerator.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************
	using namespace DirectX;

	JStaticMeshData JDefaultGeometryGenerator::CreateCube(float width, float height, float depth, uint numSubdivisions)
	{
		JStaticMeshData meshData;

		//
		// Create the vertices.
		//

		JStaticMeshVertex v[24];

		float w2 = 0.5f * width;
		float h2 = 0.5f * height;
		float d2 = 0.5f * depth;

		// Fill in the front face vertex data.
		v[0] = JStaticMeshVertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f);
		v[1] = JStaticMeshVertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
		v[2] = JStaticMeshVertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f);
		v[3] = JStaticMeshVertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f);

		// Fill in the back face vertex data.
		v[4] = JStaticMeshVertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f);
		v[5] = JStaticMeshVertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f);
		v[6] = JStaticMeshVertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
		v[7] = JStaticMeshVertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f);

		// Fill in the top face vertex data.
		v[8] = JStaticMeshVertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f);
		v[9] = JStaticMeshVertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
		v[10] = JStaticMeshVertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f);
		v[11] = JStaticMeshVertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f);

		// Fill in the bottom face vertex data.
		v[12] = JStaticMeshVertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f);
		v[13] = JStaticMeshVertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f);
		v[14] = JStaticMeshVertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
		v[15] = JStaticMeshVertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f);

		// Fill in the left face vertex data.
		v[16] = JStaticMeshVertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f);
		v[17] = JStaticMeshVertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f);
		v[18] = JStaticMeshVertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f);
		v[19] = JStaticMeshVertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f);

		// Fill in the right face vertex data.
		v[20] = JStaticMeshVertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
		v[21] = JStaticMeshVertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		v[22] = JStaticMeshVertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		v[23] = JStaticMeshVertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);

		meshData.vertices.assign(&v[0], &v[24]);

		//
		// Create the indices.
		//

		uint i[36];

		// Fill in the front face index data
		i[0] = 0; i[1] = 1; i[2] = 2;
		i[3] = 0; i[4] = 2; i[5] = 3;

		// Fill in the back face index data
		i[6] = 4; i[7] = 5; i[8] = 6;
		i[9] = 4; i[10] = 6; i[11] = 7;

		// Fill in the top face index data
		i[12] = 8; i[13] = 9; i[14] = 10;
		i[15] = 8; i[16] = 10; i[17] = 11;

		// Fill in the bottom face index data
		i[18] = 12; i[19] = 13; i[20] = 14;
		i[21] = 12; i[22] = 14; i[23] = 15;

		// Fill in the left face index data
		i[24] = 16; i[25] = 17; i[26] = 18;
		i[27] = 16; i[28] = 18; i[29] = 19;

		// Fill in the right face index data
		i[30] = 20; i[31] = 21; i[32] = 22;
		i[33] = 20; i[34] = 22; i[35] = 23;

		meshData.indices32.assign(&i[0], &i[36]);

		// Put a cap on the number of subdivisions.
		numSubdivisions = std::min<uint>(numSubdivisions, 6u);

		for (uint i = 0; i < numSubdivisions; ++i)
			Subdivide(meshData);

		return meshData;
	}

	JStaticMeshData JDefaultGeometryGenerator::CreateSphere(float radius, uint slicecount, uint stackcount)
	{
		JStaticMeshData meshData;

		//
		// Compute the vertices stating at the top pole and moving down the stacks.
		//

		// Poles: note that there will be texture coordinate distortion as there is
		// not a unique point on the texture map to assign to the pole when mapping
		// a rectangular texture onto a sphere.
		JStaticMeshVertex topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
		JStaticMeshVertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f);

		meshData.vertices.push_back(topVertex);

		float phiStep = XM_PI / stackcount;
		float thetaStep = 2.0f * XM_PI / slicecount;

		// Compute vertices for each stack ring (do not count the poles as rings).
		for (uint i = 1; i <= stackcount - 1; ++i)
		{
			float phi = i * phiStep;

			// vertices of ring.
			for (uint j = 0; j <= slicecount; ++j)
			{
				float theta = j * thetaStep;

				JStaticMeshVertex v;

				// spherical to cartesian
				v.position.x = radius * sinf(phi) * cosf(theta);
				v.position.y = radius * cosf(phi);
				v.position.z = radius * sinf(phi) * sinf(theta);

				// Partial derivative of P with respect to theta
				v.tangentU.x = -radius * sinf(phi) * sinf(theta);
				v.tangentU.y = 0.0f;
				v.tangentU.z = +radius * sinf(phi) * cosf(theta);

				XMVECTOR T = XMLoadFloat3(&v.tangentU);
				XMStoreFloat3(&v.tangentU, XMVector3Normalize(T));

				XMVECTOR p = XMLoadFloat3(&v.position);
				XMStoreFloat3(&v.normal, XMVector3Normalize(p));

				v.texC.x = theta / XM_2PI;
				v.texC.y = phi / XM_PI;

				meshData.vertices.push_back(v);
			}
		}

		meshData.vertices.push_back(bottomVertex);

		//
		// Compute indices for top stack.  The top stack was written first to the vertex buffer
		// and connects the top pole to the first ring.
		//

		for (uint i = 1; i <= slicecount; ++i)
		{
			meshData.indices32.push_back(0);
			meshData.indices32.push_back(i + 1);
			meshData.indices32.push_back(i);
		}

		//
		// Compute indices for inner stacks (not connected to poles).
		//

		// Offset the indices to the index of the first vertex in the first ring.
		// This is just skipping the top pole vertex.
		uint baseIndex = 1;
		uint ringVertexcount = slicecount + 1;
		for (uint i = 0; i < stackcount - 2; ++i)
		{
			for (uint j = 0; j < slicecount; ++j)
			{
				meshData.indices32.push_back(baseIndex + i * ringVertexcount + j);
				meshData.indices32.push_back(baseIndex + i * ringVertexcount + j + 1);
				meshData.indices32.push_back(baseIndex + (i + 1) * ringVertexcount + j);

				meshData.indices32.push_back(baseIndex + (i + 1) * ringVertexcount + j);
				meshData.indices32.push_back(baseIndex + i * ringVertexcount + j + 1);
				meshData.indices32.push_back(baseIndex + (i + 1) * ringVertexcount + j + 1);
			}
		}

		//
		// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
		// and connects the bottom pole to the bottom ring.
		//

		// South pole vertex was added last.
		uint southPoleIndex = (uint)meshData.vertices.size() - 1;

		// Offset the indices to the index of the first vertex in the last ring.
		baseIndex = southPoleIndex - ringVertexcount;

		for (uint i = 0; i < slicecount; ++i)
		{
			meshData.indices32.push_back(southPoleIndex);
			meshData.indices32.push_back(baseIndex + i);
			meshData.indices32.push_back(baseIndex + i + 1);
		}

		return meshData;
	}

	void JDefaultGeometryGenerator::Subdivide(JStaticMeshData& meshData)
	{
		// Save a copy of the input geometry.
		JStaticMeshData inputCopy = meshData;


		meshData.vertices.resize(0);
		meshData.indices32.resize(0);

		//       v1
		//       *
		//      / \
		//     /   \
		//  m0*-----*m1
		//   / \   / \
		//  /   \ /   \
		// *-----*-----*
		// v0    m2     v2

		uint numTris = (uint)inputCopy.indices32.size() / 3;
		for (uint i = 0; i < numTris; ++i)
		{
			JStaticMeshVertex v0 = inputCopy.vertices[inputCopy.indices32[i * 3 + 0]];
			JStaticMeshVertex v1 = inputCopy.vertices[inputCopy.indices32[i * 3 + 1]];
			JStaticMeshVertex v2 = inputCopy.vertices[inputCopy.indices32[i * 3 + 2]];

			//
			// Generate the midpoints.
			//

			JStaticMeshVertex m0 = MidPoint(v0, v1);
			JStaticMeshVertex m1 = MidPoint(v1, v2);
			JStaticMeshVertex m2 = MidPoint(v0, v2);

			//
			// Add new geometry.
			//

			meshData.vertices.push_back(v0); // 0
			meshData.vertices.push_back(v1); // 1
			meshData.vertices.push_back(v2); // 2
			meshData.vertices.push_back(m0); // 3
			meshData.vertices.push_back(m1); // 4
			meshData.vertices.push_back(m2); // 5

			meshData.indices32.push_back(i * 6 + 0);
			meshData.indices32.push_back(i * 6 + 3);
			meshData.indices32.push_back(i * 6 + 5);

			meshData.indices32.push_back(i * 6 + 3);
			meshData.indices32.push_back(i * 6 + 4);
			meshData.indices32.push_back(i * 6 + 5);

			meshData.indices32.push_back(i * 6 + 5);
			meshData.indices32.push_back(i * 6 + 4);
			meshData.indices32.push_back(i * 6 + 2);

			meshData.indices32.push_back(i * 6 + 3);
			meshData.indices32.push_back(i * 6 + 1);
			meshData.indices32.push_back(i * 6 + 4);
		}
	}

	JStaticMeshVertex JDefaultGeometryGenerator::MidPoint(const JStaticMeshVertex& v0, const JStaticMeshVertex& v1)
	{
		XMVECTOR p0 = XMLoadFloat3(&v0.position);
		XMVECTOR p1 = XMLoadFloat3(&v1.position);

		XMVECTOR n0 = XMLoadFloat3(&v0.normal);
		XMVECTOR n1 = XMLoadFloat3(&v1.normal);

		XMVECTOR tan0 = XMLoadFloat3(&v0.tangentU);
		XMVECTOR tan1 = XMLoadFloat3(&v1.tangentU);

		XMVECTOR tex0 = XMLoadFloat2(&v0.texC);
		XMVECTOR tex1 = XMLoadFloat2(&v1.texC);

		// Compute the midpoints of all the attributes.  Vectors need to be normalized
		// since linear interpolating can make them not unit length.  
		XMVECTOR pos = 0.5f * (p0 + p1);
		XMVECTOR normal = XMVector3Normalize(0.5f * (n0 + n1));
		XMVECTOR tangent = XMVector3Normalize(0.5f * (tan0 + tan1));
		XMVECTOR tex = 0.5f * (tex0 + tex1);

		JStaticMeshVertex v;
		XMStoreFloat3(&v.position, pos);
		XMStoreFloat3(&v.normal, normal);
		XMStoreFloat3(&v.tangentU, tangent);
		XMStoreFloat2(&v.texC, tex);

		return v;
	}

	JStaticMeshData JDefaultGeometryGenerator::CreateGeosphere(float radius, uint numSubdivisions)
	{
		JStaticMeshData meshData;

		// Put a cap on the number of subdivisions.
		numSubdivisions = std::min<uint>(numSubdivisions, 6u);

		// Approximate a sphere by tessellating an icosahedron.

		const float X = 0.525731f;
		const float Z = 0.850651f;

		XMFLOAT3 pos[12] =
		{
			XMFLOAT3(-X, 0.0f, Z),  XMFLOAT3(X, 0.0f, Z),
			XMFLOAT3(-X, 0.0f, -Z), XMFLOAT3(X, 0.0f, -Z),
			XMFLOAT3(0.0f, Z, X),   XMFLOAT3(0.0f, Z, -X),
			XMFLOAT3(0.0f, -Z, X),  XMFLOAT3(0.0f, -Z, -X),
			XMFLOAT3(Z, X, 0.0f),   XMFLOAT3(-Z, X, 0.0f),
			XMFLOAT3(Z, -X, 0.0f),  XMFLOAT3(-Z, -X, 0.0f)
		};

		uint k[60] =
		{
			1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,
			1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,
			3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
			10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7
		};

		meshData.vertices.resize(12);
		meshData.indices32.assign(&k[0], &k[60]);

		for (uint i = 0; i < 12; ++i)
			meshData.vertices[i].position = pos[i];

		for (uint i = 0; i < numSubdivisions; ++i)
			Subdivide(meshData);

		// Project vertices onto sphere and scale.
		for (uint i = 0; i < meshData.vertices.size(); ++i)
		{
			// Project onto unit sphere.
			XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&meshData.vertices[i].position));

			// Project onto sphere.
			XMVECTOR p = radius * n;

			XMStoreFloat3(&meshData.vertices[i].position, p);
			XMStoreFloat3(&meshData.vertices[i].normal, n);

			// Derive texture coordinates from spherical coordinates.
			float theta = atan2f(meshData.vertices[i].position.z, meshData.vertices[i].position.x);

			// Put in [0, 2pi].
			if (theta < 0.0f)
				theta += XM_2PI;

			float phi = acosf(meshData.vertices[i].position.y / radius);

			meshData.vertices[i].texC.x = theta / XM_2PI;
			meshData.vertices[i].texC.y = phi / XM_PI;

			// Partial derivative of P with respect to theta
			meshData.vertices[i].tangentU.x = -radius * sinf(phi) * sinf(theta);
			meshData.vertices[i].tangentU.y = 0.0f;
			meshData.vertices[i].tangentU.z = +radius * sinf(phi) * cosf(theta);

			XMVECTOR T = XMLoadFloat3(&meshData.vertices[i].tangentU);
			XMStoreFloat3(&meshData.vertices[i].tangentU, XMVector3Normalize(T));
		}

		return meshData;
	}

	JStaticMeshData JDefaultGeometryGenerator::CreateCylinder(float bottomRadius, float topRadius, float height, uint slicecount, uint stackcount)
	{
		JStaticMeshData meshData;

		//
		// Build Stacks.
		// 

		float stackHeight = height / stackcount;

		// Amount to increment radius as we move up each stack level from bottom to top.
		float radiusStep = (topRadius - bottomRadius) / stackcount;

		uint ringcount = stackcount + 1;

		// Compute vertices for each stack ring starting at the bottom and moving up.
		for (uint i = 0; i < ringcount; ++i)
		{
			float y = -0.5f * height + i * stackHeight;
			float r = bottomRadius + i * radiusStep;

			// vertices of ring
			float dTheta = 2.0f * XM_PI / slicecount;
			for (uint j = 0; j <= slicecount; ++j)
			{
				JStaticMeshVertex vertex;

				float c = cosf(j * dTheta);
				float s = sinf(j * dTheta);

				vertex.position = XMFLOAT3(r * c, y, r * s);

				vertex.texC.x = (float)j / slicecount;
				vertex.texC.y = 1.0f - (float)i / stackcount;

				// Cylinder can be parameterized as follows, where we introduce v
				// parameter that goes in the same direction as the v tex-coord
				// so that the bitangent goes in the same direction as the v tex-coord.
				//   Let r0 be the bottom radius and let r1 be the top radius.
				//   y(v) = h - hv for v in [0,1].
				//   r(v) = r1 + (r0-r1)v
				//
				//   x(t, v) = r(v)*cos(t)
				//   y(t, v) = h - hv
				//   z(t, v) = r(v)*sin(t)
				// 
				//  dx/dt = -r(v)*sin(t)
				//  dy/dt = 0
				//  dz/dt = +r(v)*cos(t)
				//
				//  dx/dv = (r0-r1)*cos(t)
				//  dy/dv = -h
				//  dz/dv = (r0-r1)*sin(t)

				// This is unit length.
				vertex.tangentU = XMFLOAT3(-s, 0.0f, c);

				float dr = bottomRadius - topRadius;
				XMFLOAT3 bitangent(dr * c, -height, dr * s);

				XMVECTOR T = XMLoadFloat3(&vertex.tangentU);
				XMVECTOR B = XMLoadFloat3(&bitangent);
				XMVECTOR N = XMVector3Normalize(XMVector3Cross(T, B));
				XMStoreFloat3(&vertex.normal, N);

				meshData.vertices.push_back(vertex);
			}
		}

		// Add one because we duplicate the first and last vertex per ring
		// since the texture coordinates are different.
		uint ringVertexcount = slicecount + 1;

		// Compute indices for each stack.
		for (uint i = 0; i < stackcount; ++i)
		{
			for (uint j = 0; j < slicecount; ++j)
			{
				meshData.indices32.push_back(i * ringVertexcount + j);
				meshData.indices32.push_back((i + 1) * ringVertexcount + j);
				meshData.indices32.push_back((i + 1) * ringVertexcount + j + 1);

				meshData.indices32.push_back(i * ringVertexcount + j);
				meshData.indices32.push_back((i + 1) * ringVertexcount + j + 1);
				meshData.indices32.push_back(i * ringVertexcount + j + 1);
			}
		}

		BuildCylinderTopCap(bottomRadius, topRadius, height, slicecount, stackcount, meshData);
		BuildCylinderBottomCap(bottomRadius, topRadius, height, slicecount, stackcount, meshData);

		return meshData;
	}

	void JDefaultGeometryGenerator::BuildCylinderTopCap(float bottomRadius, float topRadius, float height,
		uint slicecount, uint stackcount, JStaticMeshData& meshData)
	{
		uint baseIndex = (uint)meshData.vertices.size();

		float y = 0.5f * height;
		float dTheta = 2.0f * XM_PI / slicecount;

		// Duplicate cap ring vertices because the texture coordinates and normals differ.
		for (uint i = 0; i <= slicecount; ++i)
		{
			float x = topRadius * cosf(i * dTheta);
			float z = topRadius * sinf(i * dTheta);

			// Scale down by the height to try and make top cap texture coord area
			// proportional to base.
			float u = x / height + 0.5f;
			float v = z / height + 0.5f;

			meshData.vertices.emplace_back(x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v);
		}

		// Cap center vertex.
		meshData.vertices.emplace_back(0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f);

		// Index of center vertex.
		uint centerIndex = (uint)meshData.vertices.size() - 1;

		for (uint i = 0; i < slicecount; ++i)
		{
			meshData.indices32.push_back(centerIndex);
			meshData.indices32.push_back(baseIndex + i + 1);
			meshData.indices32.push_back(baseIndex + i);
		}
	}

	void JDefaultGeometryGenerator::BuildCylinderBottomCap(float bottomRadius, float topRadius, float height,
		uint slicecount, uint stackcount, JStaticMeshData& meshData)
	{
		// 
		// Build bottom cap.
		//

		uint baseIndex = (uint)meshData.vertices.size();
		float y = -0.5f * height;

		// vertices of ring
		float dTheta = 2.0f * XM_PI / slicecount;
		for (uint i = 0; i <= slicecount; ++i)
		{
			float x = bottomRadius * cosf(i * dTheta);
			float z = bottomRadius * sinf(i * dTheta);

			// Scale down by the height to try and make top cap texture coord area
			// proportional to base.
			float u = x / height + 0.5f;
			float v = z / height + 0.5f;

			//수정필요
			meshData.vertices.emplace_back(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v);
		}

		// Cap center vertex.
		meshData.vertices.emplace_back(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f);

		// Cache the index of center vertex.
		uint centerIndex = (uint)meshData.vertices.size() - 1;

		for (uint i = 0; i < slicecount; ++i)
		{
			meshData.indices32.push_back(centerIndex);
			meshData.indices32.push_back(baseIndex + i);
			meshData.indices32.push_back(baseIndex + i + 1);
		}
	}

	JStaticMeshData JDefaultGeometryGenerator::CreateGrid(float width, float depth, uint m, uint n)
	{
		JStaticMeshData meshData;

		uint vertexcount = m * n;
		uint facecount = (m - 1) * (n - 1) * 2;

		//
		// Create the vertices.
		//

		float halfWidth = 0.5f * width;
		float halfDepth = 0.5f * depth;

		float dx = width / (n - 1);
		float dz = depth / (m - 1);

		float du = 1.0f / (n - 1);
		float dv = 1.0f / (m - 1);

		meshData.vertices.resize(vertexcount);
		for (uint i = 0; i < m; ++i)
		{
			float z = halfDepth - i * dz;
			for (uint j = 0; j < n; ++j)
			{
				float x = -halfWidth + j * dx;

				meshData.vertices[i * n + j].position = XMFLOAT3(x, 0.0f, z);
				meshData.vertices[i * n + j].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
				meshData.vertices[i * n + j].tangentU = XMFLOAT3(1.0f, 0.0f, 0.0f);

				// Stretch texture over grid.
				meshData.vertices[i * n + j].texC.x = j * du;
				meshData.vertices[i * n + j].texC.y = i * dv;
			}
		}

		//
		// Create the indices.
		//

		meshData.indices32.resize(facecount * 3); // 3 indices per face

		// Iterate over each quad and compute indices.
		uint k = 0;
		for (uint i = 0; i < m - 1; ++i)
		{
			for (uint j = 0; j < n - 1; ++j)
			{
				meshData.indices32[k] = i * n + j;
				meshData.indices32[k + 1] = i * n + j + 1;
				meshData.indices32[k + 2] = (i + 1) * n + j;

				meshData.indices32[k + 3] = (i + 1) * n + j;
				meshData.indices32[k + 4] = i * n + j + 1;
				meshData.indices32[k + 5] = (i + 1) * n + j + 1;

				k += 6; // next quad
			}
		}

		return meshData;
	}

	JStaticMeshData JDefaultGeometryGenerator::CreateQuad(float x, float y, float w, float h, float depth)
	{
		JStaticMeshData meshData;

		meshData.vertices.resize(4);
		meshData.indices32.resize(6);

		// position coordinates specified in NDC space.
		meshData.vertices[0] = JStaticMeshVertex(
			x, y - h, depth,
			0.0f, 0.0f, -1.0f,
			0.0f, 1.0f,
			1.0f, 0.0f, 0.0f);

		meshData.vertices[1] = JStaticMeshVertex(
			x, y, depth,
			0.0f, 0.0f, -1.0f,
			0.0f, 0.0f,
			1.0f, 0.0f, 0.0f);

		meshData.vertices[2] = JStaticMeshVertex(
			x + w, y, depth,
			0.0f, 0.0f, -1.0f,
			1.0f, 0.0f,
			1.0f, 0.0f, 0.0f);

		meshData.vertices[3] = JStaticMeshVertex(
			x + w, y - h, depth,
			0.0f, 0.0f, -1.0f,
			1.0f, 1.0f,
			1.0f, 0.0f, 0.0f);

		meshData.indices32[0] = 0;
		meshData.indices32[1] = 1;
		meshData.indices32[2] = 2;

		meshData.indices32[3] = 0;
		meshData.indices32[4] = 2;
		meshData.indices32[5] = 3;

		return meshData;
	}
	JStaticMeshData JDefaultGeometryGenerator::CreateBoundingBox()
	{
		JStaticMeshData meshData;
		meshData.vertices.resize(8);
		meshData.vertices[0].position = XMFLOAT3(-0.5f, -0.5f, -0.5f);
		meshData.vertices[1].position = XMFLOAT3(0.5f, -0.5f, -0.5f);
		meshData.vertices[2].position = XMFLOAT3(0.5f, -0.5f, 0.5f);
		meshData.vertices[3].position = XMFLOAT3(-0.5f, -0.5f, 0.5f);
		meshData.vertices[4].position = XMFLOAT3(-0.5f, 0.5f, -0.5f);
		meshData.vertices[5].position = XMFLOAT3(0.5f, 0.5f, -0.5f);
		meshData.vertices[6].position = XMFLOAT3(0.5f, 0.5f, 0.5f);
		meshData.vertices[7].position = XMFLOAT3(-0.5f, 0.5f, 0.5f);

		meshData.indices32.resize(24);

		const int s_indices[24] =
		{
			0, 1,
			1, 2,
			2, 3,
			3, 0,
			4, 5,
			5, 6,
			6, 7,
			7, 4,
			0, 4,
			1, 5,
			2, 6,
			3, 7
		};
		for (uint i = 0; i < 24; ++i)
			meshData.indices32[i] = s_indices[i];

		return meshData;
	}
	JStaticMeshData JDefaultGeometryGenerator::CreateBoundingFrustum()
	{
		JStaticMeshData meshData;
		meshData.vertices.resize(5);
		meshData.vertices[0].position = XMFLOAT3(0, 0, 0);
		meshData.vertices[1].position = XMFLOAT3(-0.5f, -0.5f, 1);
		meshData.vertices[2].position = XMFLOAT3(0.5f, -0.5f, 1);
		meshData.vertices[3].position = XMFLOAT3(0.5f, 0.5f, 1);
		meshData.vertices[4].position = XMFLOAT3(-0.5f, 0.5f, 1);

		meshData.indices32.resize(16);

		const int s_indices[16] =
		{
			0, 1,
			1, 2,
			2, 0,
			0, 3,
			3, 4,
			4, 0,
			1, 3,
			2, 4
		};
		for (uint i = 0; i < 16; ++i)
			meshData.indices32[i] = s_indices[i];
		return meshData;
	}
}