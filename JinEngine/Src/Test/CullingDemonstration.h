#pragma once
#include"../Core/JDataType.h"
#include"../Object/Resource/Mesh/JDefaultShapeType.h"
#include<vector>

namespace JinEngine
{
	class JGameObject; 
	class CullingDemonstration
	{
	private:
		static bool isActivated;
		static std::vector<JGameObject*> gameObjectVec;
	public:
		static void OnCullingDemonstration(JGameObject* parent, const uint x, const uint y,const uint z,  const J_DEFAULT_SHAPE DefaultShapeType);
		static void OffCullingDemonstration();
	};
}