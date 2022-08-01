#pragma once 
#include"JMeshStruct.h"
#include<DirectXCollision.h>

namespace JinEngine
{
	class JDefaultGeometryGenerator
	{
	public:
		JStaticMeshData CreateCube(float width, float height, float depth, uint numSubdivisions);

		///<summary>
		/// Creates a sphere centered at the origin with the given radius.  The
		/// slices and stacks parameters control the degree of tessellation.
		///</summary>
		JStaticMeshData CreateSphere(float radius, uint slicecount, uint stackcount);

		///<summary>
		/// Creates a geosphere centered at the origin with the given radius.  The
		/// depth controls the level of tessellation.
		///</summary>
		JStaticMeshData CreateGeosphere(float radius, uint numSubdivisions);

		///<summary>
		/// Creates a cylinder parallel to the y-axis, and centered about the origin.  
		/// The bottom and top radius can vary to form various cone shapes rather than true
		// cylinders.  The slices and stacks parameters control the degree of tessellation.
		///</summary>
		JStaticMeshData CreateCylinder(float bottomRadius, float topRadius, float height, uint slicecount, uint stackcount);

		///<summary>
		/// Creates an mxn grid in the xz-plane with m rows and n columns, centered
		/// at the origin with the specified width and depth.
		///</summary>
		JStaticMeshData CreateGrid(float width, float depth, uint m, uint n);

		///<summary>
		/// Creates a quad aligned with the screen.  This is useful for postprocessing and screen effects.
		///</summary>
		JStaticMeshData CreateQuad(float x, float y, float w, float h, float depth);

		JStaticMeshData CreateBoundingBox();

		JStaticMeshData CreateBoundingFrustum();
	private:
		void Subdivide(JStaticMeshData& meshData);
		JStaticMeshVertex MidPoint(const JStaticMeshVertex& v0, const JStaticMeshVertex& v1);
		void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint slicecount, uint stackcount, JStaticMeshData& meshData);
		void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint slicecount, uint stackcount, JStaticMeshData& meshData);
	};
}
