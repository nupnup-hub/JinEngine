#pragma once
#include<string>

namespace JinEngine
{
	struct JModifiedObjectInfo
	{
	public:
		const size_t guid;
		const std::string typeName;
		bool isModified = false;
		bool isStore = true;
	public:
		JModifiedObjectInfo(const size_t guid, const std::string& typeName)
			:guid(guid), typeName(typeName)
		{}
	};
}