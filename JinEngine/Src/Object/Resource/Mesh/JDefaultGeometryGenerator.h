#pragma once 
#include"JMeshStruct.h"
#include<DirectXCollision.h>

namespace JinEngine
{
	class JDefaultGeometryGenerator
	{
	public:
		JStaticMeshData CreateCube(const float width, const float height, const float depth, uint numSubdivisions);
		///<summary>
		/// Creates a sphere centered at the origin with the given radius.  The
		/// slices and stacks parameters control the degree of tessellation.
		///</summary>
		JStaticMeshData CreateSphere(const float radius, const uint slicecount, const uint stackcount);

		///<summary>
		/// Creates a geosphere centered at the origin with the given radius.  The
		/// depth controls the level of tessellation.
		///</summary>
		JStaticMeshData CreateGeosphere(const float radius, uint numSubdivisions);

		///<summary>
		/// Creates a cylinder parallel to the y-axis, and centered about the origin.  
		/// The bottom and top radius can vary to form various cone shapes rather than true
		// cylinders.  The slices and stacks parameters control the degree of tessellation.
		///</summary>
		JStaticMeshData CreateCylinder(const float bottomRadius,
			const float topRadius,
			const float height, 
			const uint slicecount,
			const uint stackcount);

		///<summary>
		/// Creates an mxn grid in the xz-plane with m rows and n columns, centered
		/// at the origin with the specified width and depth.
		///</summary>
		JStaticMeshData CreateGrid(const float width,
			const float depth,
			const uint m,
			const uint n);

		///<summary>
		/// Creates a quad aligned with the screen.  This is useful for postprocessing and screen effects.
		///</summary>
		JStaticMeshData CreateQuad(const float x,
			const float y,
			const float w,
			const float h,
			const float depth);

		JStaticMeshData CreateLineBoundingBox();
		JStaticMeshData CreateTriangleBoundingBox();
		JStaticMeshData CreateBoundingFrustum(); 
		JStaticMeshData CreateCircle(float outRadius, float innerRadius);
		JStaticMeshData CreateLine(const uint thickness);
	private:
		void Subdivide(JStaticMeshData& meshData);
		JStaticMeshVertex MidPoint(const JStaticMeshVertex& v0, const JStaticMeshVertex& v1);
		void BuildCylinderTopCap(const float bottomRadius,
			const float topRadius,
			const float height,
			const uint slicecount,
			const uint stackcount,
			JStaticMeshData& meshData);
		void BuildCylinderBottomCap(const float bottomRadius,
			const float topRadius,
			const float height,
			const uint slicecount,
			const uint stackcount,
			JStaticMeshData& meshData);
	};
}
