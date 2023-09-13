#include"JFileInitData.h" 
#include"../Resource/JResourceObject.h"

namespace JinEngine
{
	JFileInitData::JFileInitData(JResourceObject* rObj)
		:name(rObj->GetName()),
		rGuid(rObj->GetGuid()),
		rTypeInfo(rObj->GetTypeInfo()),
		resourceType(rObj->GetResourceType()),
		flag(rObj->GetFlag()),
		formatIndex(rObj->GetFormatIndex())
	{}
	JFileInitData::JFileInitData(const JUserPtr<JResourceObject>& rObj)
		:name(rObj->GetName()),
		rGuid(rObj->GetGuid()),
		rTypeInfo(rObj->GetTypeInfo()),
		resourceType(rObj->GetResourceType()),
		flag(rObj->GetFlag()),
		formatIndex(rObj->GetFormatIndex())
	{}
	JFileInitData::JFileInitData(const std::wstring& name,
		const size_t rGuid,
		Core::JTypeInfo& rTypeInfo,
		const J_RESOURCE_TYPE resourceType,
		const J_OBJECT_FLAG flag,
		const uint8 formatIndex)
		:name(name), 
		rGuid(rGuid), 
		rTypeInfo(rTypeInfo), 
		resourceType(resourceType), 
		flag(flag),
		formatIndex(formatIndex)
	{}
}