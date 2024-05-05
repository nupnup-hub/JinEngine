#pragma once 
#include"../Core/JCoreEssential.h"

namespace JinEngine
{
	struct JModifiedObjectInfo
	{
	public:
		const size_t objectGuid;
		const size_t typeGuid;
		const std::string typeName;;
		bool isModified = false;
		bool canStore = true;
	public:
		JModifiedObjectInfo(const size_t objectGuid, const size_t typeGuid, const std::string& typeName)
			:objectGuid(objectGuid), typeGuid(typeGuid), typeName(typeName)
		{}
	};
}