#pragma once
#include<string>   
#include"../Resource/JResourceObjectType.h" 

namespace JinEngine
{ 
	namespace Core
	{
		class JTypeInfo;
	}
	class JResourceObject;
	struct JFileInitData
	{
	public:
		const std::wstring name;
		const size_t rGuid;
		Core::JTypeInfo& rTypeInfo;
		const J_RESOURCE_TYPE resourceType;
		const uint8 formatIndex; 
	public:
		JFileInitData(JResourceObject* rObj);
		JFileInitData(const std::wstring& name,
			const size_t rGuid,
			Core::JTypeInfo& rTypeInfo,
			const J_RESOURCE_TYPE resourceType,
			const uint8 formatIndex);
	};
}