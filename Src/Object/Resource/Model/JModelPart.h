#pragma once
#include<string>

namespace JinEngine
{
	class JMeshGeometry;
	class JMaterial;

	struct JModelPart
	{
	public:
		std::string name;
		int parentIndex;
		JMeshGeometry* mesh = nullptr;
		JMaterial* mat = nullptr;
	public: 
		JModelPart(std::string name, int parentIndex, JMeshGeometry* mesh, JMaterial* mat)
			:name(name), parentIndex(parentIndex), mesh(mesh), mat(mat)
		{}
	};
}