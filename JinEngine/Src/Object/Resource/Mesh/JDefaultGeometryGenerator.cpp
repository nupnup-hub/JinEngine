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

	JStaticMeshData JDefaultGeometryGenerator::CreateCube(const float width,
		const float height,
		const float depth,
		uint numSubdivisions)
	{
		//
		// Create the vertices.
		//
		std::vector< JStaticMeshVertex> v(24);

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
		 		 
		//
		// Create the indices.
		//

		std::vector<uint32> i(36);

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
 
		JStaticMeshData meshData(L"Cube", std::move(i), true, true, std::move(v));
		// Put a cap on the number of subdivisions.
		numSubdivisions = std::min<uint>(numSubdivisions, 6u);
		for (uint i = 0; i < numSubdivisions; ++i)
			Subdivide(meshData);

		return meshData;
	}

	JStaticMeshData JDefaultGeometryGenerator::CreateSphere(const float radius,
		const uint slicecount,
		const uint stackcount)
	{
		//
		// Compute the vertices stating at the top pole and moving down the stacks.
		//
		// Poles: note that there will be texture coordinate distortion as there is
		// not a unique point on the texture map to assign to the pole when mapping
		// a rectangular texture onto a sphere.
		JStaticMeshVertex topVertex(0.0f, radius, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
		JStaticMeshVertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f);

		std::vector<JStaticMeshVertex> vertices;
		vertices.push_back(topVertex);
		 
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

				vertices.push_back(v);
			}
		}
		vertices.push_back(bottomVertex);
		//
		// Compute indices for top stack.  The top stack was written first to the vertex buffer
		// and connects the top pole to the first ring.
		//

		std::vector<uint32> indices;
		for (uint i = 1; i <= slicecount; ++i)
		{
			indices.push_back(0);
			indices.push_back(i + 1);
			indices.push_back(i);
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
				indices.push_back(baseIndex + i * ringVertexcount + j);
				indices.push_back(baseIndex + i * ringVertexcount + j + 1);
				indices.push_back(baseIndex + (i + 1) * ringVertexcount + j);

				indices.push_back(baseIndex + (i + 1) * ringVertexcount + j);
				indices.push_back(baseIndex + i * ringVertexcount + j + 1);
				indices.push_back(baseIndex + (i + 1) * ringVertexcount + j + 1);
			}
		}

		//
		// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
		// and connects the bottom pole to the bottom ring.
		//

		// South pole vertex was added last.
		uint southPoleIndex = (uint)vertices.size() - 1;

		// Offset the indices to the index of the first vertex in the last ring.
		baseIndex = southPoleIndex - ringVertexcount;

		for (uint i = 0; i < slicecount; ++i)
		{
			indices.push_back(southPoleIndex);
			indices.push_back(baseIndex + i);
			indices.push_back(baseIndex + i + 1);
		}

		//JStaticMeshData meshData{L"Cube", std::move(i), true, true, std::move(v)};
		return JStaticMeshData{L"Sphere", std::move(indices), true, true, std::move(vertices) };
	}
	JStaticMeshData JDefaultGeometryGenerator::CreateGeosphere(const float radius, uint numSubdivisions)
	{ 
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

		std::vector<uint32> indices =
		{
			1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,
			1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,
			3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
			10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7
		};
		std::vector<JStaticMeshVertex> vertices(12);

		for (uint i = 0; i < 12; ++i)
			vertices[i].position = pos[i];

		JStaticMeshData meshData{ L"GeoSphere", std::move(indices), true, true, std::move(vertices) };

		for (uint i = 0; i < numSubdivisions; ++i)
			Subdivide(meshData);

		// Project vertices onto sphere and scale.
		const uint vertexCount = meshData.GetVertexCount();
		for (uint i = 0; i < vertexCount; ++i)
		{
			// Project onto unit sphere.
			XMVECTOR n = XMVector3Normalize(meshData.GetPosition(i));

			// Project onto sphere.
			XMVECTOR p = radius * n;

			JStaticMeshVertex v;

			XMStoreFloat3(&v.position, p);
			XMStoreFloat3(&v.normal, n);

			// Derive texture coordinates from spherical coordinates.
			float theta = atan2f(v.position.z, v.position.x);

			// Put in [0, 2pi].
			if (theta < 0.0f)
				theta += XM_2PI;

			float phi = acosf(v.position.y / radius);

			v.texC.x = theta / XM_2PI;
			v.texC.y = phi / XM_PI;

			// Partial derivative of P with respect to theta
			v.tangentU.x = -radius * sinf(phi) * sinf(theta);
			v.tangentU.y = 0.0f;
			v.tangentU.z = +radius * sinf(phi) * cosf(theta);

			XMVECTOR T = XMLoadFloat3(&v.tangentU);
			XMStoreFloat3(&v.tangentU, XMVector3Normalize(T));
			meshData.SetVertex(i, v);
		}

		return meshData;
	}
	JStaticMeshData JDefaultGeometryGenerator::CreateCylinder(const float bottomRadius,
		const float topRadius,
		const float height,
		const uint slicecount,
		const uint stackcount)
	{
		//
		// Build Stacks.
		// 
		std::vector<JStaticMeshVertex> vertices;
		std::vector<uint32> indices32;

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

				vertex.position = XMFLOAT3(r * c , y, r * s);

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

				vertices.push_back(vertex);
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
				indices32.push_back(i * ringVertexcount + j);
				indices32.push_back((i + 1) * ringVertexcount + j);
				indices32.push_back((i + 1) * ringVertexcount + j + 1);

				indices32.push_back(i * ringVertexcount + j);
				indices32.push_back((i + 1) * ringVertexcount + j + 1);
				indices32.push_back(i * ringVertexcount + j + 1);
			}
		}
		JStaticMeshData meshData(L"Cylinder", std::move(indices32), true, true, std::move(vertices));
		BuildCylinderTopCap(bottomRadius, topRadius, height, slicecount, stackcount, meshData);
		BuildCylinderBottomCap(bottomRadius, topRadius, height, slicecount, stackcount, meshData);

		return meshData;
	}
	JStaticMeshData JDefaultGeometryGenerator::CreateGrid(const float width,
		const float depth,
		const uint m,
		const uint n)
	{
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

		std::vector<JStaticMeshVertex> vertices(vertexcount);	 
		for (uint i = 0; i < m; ++i)
		{
			float z = halfDepth - i * dz;
			for (uint j = 0; j < n; ++j)
			{
				float x = -halfWidth + j * dx;

				vertices[i * n + j].position = XMFLOAT3(x, 0.0f, 0.0f);
				vertices[i * n + j].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
				vertices[i * n + j].tangentU = XMFLOAT3(1.0f, 0.0f, 0.0f);

				// Stretch texture over grid.
				vertices[i * n + j].texC.x = j * du;
				vertices[i * n + j].texC.y = i * dv;
			}
		}

		//
		// Create the indices.
		//
		std::vector<uint32> indices32(facecount * 3);
		// Iterate over each quad and compute indices.
		uint k = 0;
		for (uint i = 0; i < m - 1; ++i)
		{
			for (uint j = 0; j < n - 1; ++j)
			{
				indices32[k] = i * n + j;
				indices32[k + 1] = i * n + j + 1;
				indices32[k + 2] = (i + 1) * n + j;

				indices32[k + 3] = (i + 1) * n + j;
				indices32[k + 4] = i * n + j + 1;
				indices32[k + 5] = (i + 1) * n + j + 1;

				k += 6; // next quad
			}
		}

		return JStaticMeshData(L"Grid", std::move(indices32), true, true, std::move(vertices));
	}
	JStaticMeshData JDefaultGeometryGenerator::CreateQuad(const float x,
		const float y,
		const float w,
		const float h,
		const float depth)
	{
		std::vector<JStaticMeshVertex> vertices(4);
		std::vector<uint32> indices32(6); 

		// position coordinates specified in NDC space.
		  
		vertices[0] = JStaticMeshVertex(
			x, y - h, depth,
			0.0f, 0.0f, -1.0f,
			0.0f, 1.0f,
			1.0f, 0.0f, 0.0f);

		vertices[1] = JStaticMeshVertex(
			x, y, depth,
			0.0f, 0.0f, -1.0f,
			0.0f, 0.0f,
			1.0f, 0.0f, 0.0f);

		vertices[2] = JStaticMeshVertex(
			x + w, y, depth,
			0.0f, 0.0f, -1.0f,
			1.0f, 0.0f,
			1.0f, 0.0f, 0.0f);

		vertices[3] = JStaticMeshVertex(
			x + w, y - h, depth,
			0.0f, 0.0f, -1.0f,
			1.0f, 1.0f,
			1.0f, 0.0f, 0.0f);

		indices32[0] = 0;
		indices32[1] = 1;
		indices32[2] = 2;

		indices32[3] = 0;
		indices32[4] = 2;
		indices32[5] = 3;

		return JStaticMeshData(L"Quad", std::move(indices32), true, true, std::move(vertices));
	}
	JStaticMeshData JDefaultGeometryGenerator::CreateLineBoundingBox()
	{
		std::vector<JStaticMeshVertex> vertices(8);
		vertices[0].position = XMFLOAT3(-0.5f, -0.5f, -0.5f);
		vertices[1].position = XMFLOAT3(0.5f, -0.5f, -0.5f);
		vertices[2].position = XMFLOAT3(0.5f, -0.5f, 0.5f);
		vertices[3].position = XMFLOAT3(-0.5f, -0.5f, 0.5f);
		vertices[4].position = XMFLOAT3(-0.5f, 0.5f, -0.5f);
		vertices[5].position = XMFLOAT3(0.5f, 0.5f, -0.5f);
		vertices[6].position = XMFLOAT3(0.5f, 0.5f, 0.5f);
		vertices[7].position = XMFLOAT3(-0.5f, 0.5f, 0.5f);

		std::vector<uint32> indices32
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
		return JStaticMeshData(L"Bounding Box_L", std::move(indices32), false, false, std::move(vertices));
	}
	JStaticMeshData JDefaultGeometryGenerator::CreateTriangleBoundingBox()
	{
		std::vector<JStaticMeshVertex> vertices(8);
		vertices[0].position = XMFLOAT3(-0.5f, 0.5f, -0.5);
		vertices[1].position = XMFLOAT3(0.5f, 0.5f, -0.5f);
		vertices[2].position = XMFLOAT3(0.5f, 0.5f, 0.5f);
		vertices[3].position = XMFLOAT3(-0.5f, 0.5f, 0.5f);
		vertices[4].position = XMFLOAT3(-0.5f, -0.5f, 0.5f);
		vertices[5].position = XMFLOAT3(0.5f, -0.5f, 0.5f);
		vertices[6].position = XMFLOAT3(0.5f, -0.5f, -0.5f);
		vertices[7].position = XMFLOAT3(-0.5f, -0.5f, -0.5f);

		std::vector<uint32> indices32
		{
			0, 1, 2,
			0, 2, 3,

			4, 5, 6,
			4, 6, 7,

			3, 2, 5,
			3, 5, 4,

			2, 1, 6,
			2, 6, 5,

			1, 7, 6,
			1, 0, 7,

			0, 3, 4,
			0, 4, 7
		};
		return JStaticMeshData(L"Bounding Box_T", std::move(indices32), false, false, std::move(vertices));
	}
	JStaticMeshData JDefaultGeometryGenerator::CreateBoundingFrustum()
	{
		std::vector<JStaticMeshVertex> vertices(5); 
		vertices[0].position = XMFLOAT3(0, 0, 0);
		vertices[1].position = XMFLOAT3(-0.5f, -0.5f, 1);
		vertices[2].position = XMFLOAT3(0.5f, -0.5f, 1);
		vertices[3].position = XMFLOAT3(0.5f, 0.5f, 1);
		vertices[4].position = XMFLOAT3(-0.5f, 0.5f, 1);

		std::vector<uint32> indices32
		{
			0, 1,
			1, 2,
			2, 0,
			0, 3,
			3, 4,
			4, 0,
			1, 4,
			2, 3
		};

		return JStaticMeshData(L"Bounding Frustum", std::move(indices32), false, false, std::move(vertices));
	} 
	JStaticMeshData JDefaultGeometryGenerator::CreateCircle(float outRadius, float innerRadius)
	{
		const int segments = 32;
		const int vertexCount = (segments + 1) * 2;
		const int indexCount = (vertexCount - 2) * 3 + 6; 
		std::vector<JStaticMeshVertex> vertices(vertexCount);
		std::vector<uint16> indices(indexCount);

		float theta = 2.0f * XM_PI;
		vertices[0].position.x = innerRadius * std::cos(theta);
		vertices[0].position.y = innerRadius * std::sin(theta);

		vertices[1].position.x = outRadius * std::cos(theta);
		vertices[1].position.y = outRadius * std::sin(theta);

		int indexOffset = 0;
		int vertexOffset = 0;

		int loopCount = segments + 1;
		float piRate = 1.0f / segments;

		for (int i = 1; i < loopCount; i++)
		{ 	 
			vertexOffset += 2;
			theta = 2.0f * XM_PI * ((float)((segments + 1) - i) / segments - piRate);
			vertices[vertexOffset].position.x = innerRadius * std::cos(theta);
			vertices[vertexOffset].position.y = innerRadius * std::sin(theta);
			 
			vertices[vertexOffset + 1].position.x = outRadius * std::cos(theta);
			vertices[vertexOffset + 1].position.y = outRadius * std::sin(theta);

			indices[indexOffset] = vertexOffset - 2;
			indices[indexOffset + 1] = vertexOffset - 1;
			indices[indexOffset + 2] = vertexOffset;
			indices[indexOffset + 3] = vertexOffset - 1;
			indices[indexOffset + 4] = vertexOffset + 1;
			indices[indexOffset + 5] = vertexOffset;

			indexOffset += 6;
		}  
		return JStaticMeshData(L"Circle", std::move(indices), false, false, std::move(vertices));
	}

	void JDefaultGeometryGenerator::Subdivide(JStaticMeshData& meshData)
	{
		// Save a copy of the input geometry.
		JStaticMeshData inputCopy = meshData;

		//       v1
		//       *
		//      / \
		//     /   \
		//  m0*-----*m1
		//   / \   / \
		//  /   \ /   \
		// *-----*-----*
		// v0    m2     v2

		std::vector<JStaticMeshVertex> vertices;
		std::vector<uint32> indices32;

		uint numTris = (uint)inputCopy.GetIndexCount() / 3;
		for (uint i = 0; i < numTris; ++i)
		{
			JStaticMeshVertex v0 = inputCopy.GetVertex(inputCopy.GetU32Index(i * 3 + 0));
			JStaticMeshVertex v1 = inputCopy.GetVertex(inputCopy.GetU32Index(i * 3 + 1));
			JStaticMeshVertex v2 = inputCopy.GetVertex(inputCopy.GetU32Index(i * 3 + 2));
			//
			// Generate the midpoints.
			//
			JStaticMeshVertex m0 = MidPoint(v0, v1);
			JStaticMeshVertex m1 = MidPoint(v1, v2);
			JStaticMeshVertex m2 = MidPoint(v0, v2);
			//
			// Add new geometry.
			//
			vertices.push_back(v0); // 0
			vertices.push_back(v1); // 1
			vertices.push_back(v2); // 2
			vertices.push_back(m0); // 3
			vertices.push_back(m1); // 4
			vertices.push_back(m2); // 5

			indices32.push_back(i * 6 + 0);
			indices32.push_back(i * 6 + 3);
			indices32.push_back(i * 6 + 5);

			indices32.push_back(i * 6 + 3);
			indices32.push_back(i * 6 + 4);
			indices32.push_back(i * 6 + 5);

			indices32.push_back(i * 6 + 5);
			indices32.push_back(i * 6 + 4);
			indices32.push_back(i * 6 + 2);

			indices32.push_back(i * 6 + 3);
			indices32.push_back(i * 6 + 1);
			indices32.push_back(i * 6 + 4);
		}
		meshData = JStaticMeshData(inputCopy.GetName(),
			inputCopy.GetGuid(),
			std::move(indices32),
			inputCopy.HasUV(),
			inputCopy.HasNormal(),
			std::move(vertices));
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
	void JDefaultGeometryGenerator::BuildCylinderTopCap(float bottomRadius, float topRadius, float height,
		uint slicecount, uint stackcount, JStaticMeshData& meshData)
	{
		uint baseIndex = (uint)meshData.GetVertexCount();

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

			meshData.AddVertex(JStaticMeshVertex(x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
		}

		// Cap center vertex.
		meshData.AddVertex(JStaticMeshVertex(0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

		// Index of center vertex.
		uint centerIndex = (uint)meshData.GetVertexCount() - 1;

		for (uint i = 0; i < slicecount; ++i)
		{
			meshData.AddIndex(centerIndex);
			meshData.AddIndex(baseIndex + i + 1);
			meshData.AddIndex(baseIndex + i);
		}
	}
	void JDefaultGeometryGenerator::BuildCylinderBottomCap(float bottomRadius, float topRadius, float height,
		uint slicecount, uint stackcount, JStaticMeshData& meshData)
	{
		// 
		// Build bottom cap.
		//

		uint baseIndex = (uint)meshData.GetVertexCount();
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
			meshData.AddVertex(JStaticMeshVertex(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
		}

		// Cap center vertex.
		meshData.AddVertex(JStaticMeshVertex(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

		// Cache the index of center vertex.
		uint centerIndex = (uint)meshData.GetVertexCount() - 1;

		for (uint i = 0; i < slicecount; ++i)
		{
			meshData.AddIndex(centerIndex);
			meshData.AddIndex(baseIndex + i);
			meshData.AddIndex(baseIndex + i + 1);
		}
	}
}