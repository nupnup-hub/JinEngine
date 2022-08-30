#pragma once
#include<string>

namespace JinEngine
{
	class JMeshGeometry;
	class JMaterial;

	struct JModelPart
	{
	public:
		std::wstring name;
		int parentIndex;
		JMeshGeometry* mesh = nullptr;
		JMaterial* mat = nullptr;
	public: 
		JModelPart(std::wstring name, int parentIndex, JMeshGeometry* mesh, JMaterial* mat)
			:name(name), parentIndex(parentIndex), mesh(mesh), mat(mat)
		{}
	};
}