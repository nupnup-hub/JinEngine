#pragma once
#include<string>

namespace JinEngine
{
	struct JModifiedObjectInfo
	{
	public:
		const size_t objectGuid;
		const size_t typeGuid;
		const std::string typeName;
		std::wstring lastObjName;
		std::wstring lastObjPath;
		std::wstring lastObjMetaPath;
		bool isModified = false;
		bool isStore = true;
		bool isRemoved = false;
	public:
		JModifiedObjectInfo(const size_t objectGuid, const size_t typeGuid, const std::string& typeName)
			:objectGuid(objectGuid), typeGuid(typeGuid), typeName(typeName)
		{}
	};
}