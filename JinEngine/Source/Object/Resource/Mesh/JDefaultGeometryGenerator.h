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
#include"../../../Core/Geometry/Mesh/JMeshStruct.h"
#include<DirectXCollision.h>

namespace JinEngine
{
	class JDefaultGeometryGenerator
	{
	public:
		std::unique_ptr<Core::JStaticMeshData> CreateCube(const float width, const float height, const float depth, uint numSubdivisions);
		///<summary>
		/// Creates a sphere centered at the origin with the given radius.  The
		/// slices and stacks parameters control the degree of tessellation.
		///</summary>
		std::unique_ptr<Core::JStaticMeshData> CreateSphere(float radius, uint slicecount, uint stackcount);
		std::unique_ptr<Core::JStaticMeshData> CreateHemiSphere(float radius, uint slicecount, uint stackcount);
		///<summary>
		/// Creates a geosphere centered at the origin with the given radius.  The
		/// depth controls the level of tessellation.
		///</summary>
		std::unique_ptr<Core::JStaticMeshData> CreateGeosphere(const float radius, uint numSubdivisions);
		///<summary>
		/// Creates a cylinder parallel to the y-axis, and centered about the origin.  
		/// The bottom and top radius can vary to form various cone shapes rather than true
		// cylinders.  The slices and stacks parameters control the degree of tessellation.
		///</summary>
		std::unique_ptr<Core::JStaticMeshData> CreateCylinder(const float bottomRadius,
			float topRadius,
			const float height, 
			const uint slicecount,
			const uint stackcount, 
			const float ySt = 0);
		///<summary>
		/// Creates an mxn grid in the xz-plane with m rows and n columns, centered
		/// at the origin with the specified width and depth.
		///</summary>
		std::unique_ptr<Core::JStaticMeshData> CreateGrid(const float width,
			const float depth,
			const uint m,
			const uint n);
		///<summary>
		/// Creates a quad aligned with the screen.  This is useful for postprocessing and screen effects.
		///</summary>
		std::unique_ptr<Core::JStaticMeshData> CreateQuad(const float x,
			const float y,
			const float w,
			const float h,
			const float depth);
		std::unique_ptr<Core::JStaticMeshData> CreateLineBoundingBox();
		std::unique_ptr<Core::JStaticMeshData> CreateTriangleBoundingBox();
		std::unique_ptr<Core::JStaticMeshData> CreateBoundingFrustum();
		std::unique_ptr<Core::JStaticMeshData> CreateCircle(float outRadius, float innerRadius);
		std::unique_ptr<Core::JStaticMeshData> CreateLine(uint thickness);
		std::unique_ptr<Core::JStaticMeshData> CreateCone(float height, float outRadius, uint bottomStack);
		std::unique_ptr<Core::JStaticMeshData> CreateBoundingCone();
	private:
		void Subdivide(std::unique_ptr<Core::JStaticMeshData>& meshData);
		Core::JStaticMeshVertex MidPoint(const Core::JStaticMeshVertex& v0, const Core::JStaticMeshVertex& v1);
		void BuildCylinderTopCap(const float bottomRadius,
			const float topRadius,
			const float height,
			const uint slicecount,
			const uint stackcount,
			Core::JStaticMeshData* meshData,
			const float ySt = 0);
		void BuildCylinderBottomCap(const float bottomRadius,
			const float topRadius,
			const float height,
			const uint slicecount,
			const uint stackcount,
			Core::JStaticMeshData* meshData,
			const float ySt = 0);
	};
}
