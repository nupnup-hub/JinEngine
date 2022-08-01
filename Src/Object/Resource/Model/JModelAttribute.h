#pragma once
#include<DirectXCollision.h>
#include"../../../Core/JDataType.h"

namespace JinEngine
{
	struct JModelAttribute
	{
	public:
		DirectX::BoundingBox modelBBox;
		DirectX::BoundingSphere modelBSphere;
		DirectX::BoundingSphere skletonBSphere;
		bool hasSkeleton = true;
		uint totalVertex = 0;
		uint totalIndex = 0;
	public:
		JModelAttribute() = default;
		~JModelAttribute() = default;
		JModelAttribute(const JModelAttribute& data) = default;
		JModelAttribute(JModelAttribute&& data) = default;	 
		JModelAttribute& operator= (JModelAttribute&& data) = default;
	};
}