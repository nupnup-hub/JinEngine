#include"JObject.h"
#include"JObjectPrivate.h" 
#include"../Core/Identity/JIdentifierImplBase.h"

namespace JinEngine
{
	class JObject::JObjectImpl : public Core::JIdentifierImplBase
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JObjectImpl)
	public:
		const J_OBJECT_FLAG flag;
	public:
		JObjectImpl(const InitData& initData)
			:flag(initData.flag)
		{}
	};

	JObject::InitData::InitData(const JTypeInfo& initTypeInfo)
		:Core::JIdentifier::InitData(initTypeInfo)
	{}
	JObject::InitData::InitData(const JTypeInfo& initTypeInfo, const size_t guid)
		: Core::JIdentifier::InitData(initTypeInfo, guid)
	{}
	JObject::InitData::InitData(const JTypeInfo& initTypeInfo, const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag)
		:Core::JIdentifier::InitData(initTypeInfo, name, guid), flag(flag)
	{}  

	JObject::StoreData::StoreData(JObject* obj)
		:obj(obj)
	{}
	bool JObject::StoreData::IsValidData()const noexcept
	{
		return obj != nullptr && !obj->HasFlag(OBJECT_FLAG_DO_NOT_SAVE);
	}
	bool JObject::StoreData::HasCorrectType(const JTypeInfo& correctType)const noexcept
	{
		return obj != nullptr && obj->GetTypeInfo().IsA(correctType);
	}
	bool JObject::StoreData::HasCorrectChildType(const JTypeInfo& correctType)const noexcept
	{
		return obj != nullptr && obj->GetTypeInfo().IsChildOf(correctType);
	}

	J_OBJECT_FLAG JObject::GetFlag()const noexcept
	{ 
		return impl->flag;
	}
	bool JObject::HasFlag(const J_OBJECT_FLAG flag)const noexcept
	{
		return Core::HasSQValueEnum(GetFlag(), flag);
	}

	JObject::JObject(const InitData& initData)
		:Core::JIdentifier(initData), impl(std::make_unique<JObjectImpl>(initData))
	{}
	JObject::~JObject()
	{
		impl.reset();
	} 

	using CreateInstanceInterface = JObjectPrivate::CreateInstanceInterface;
	using DestroyInstanceInterface = JObjectPrivate::DestroyInstanceInterface; 

	bool CreateInstanceInterface::CanCopy(Core::JIdentifier* from, Core::JIdentifier* to)noexcept
	{
		return Core::JIdentifierPrivate::CreateInstanceInterface::CanCopy(from, to) &&
			!static_cast<JObject*>(from)->HasFlag(OBJECT_FLAG_UNCOPYABLE);
	}
	bool DestroyInstanceInterface::CanDestroyInstancce(Core::JIdentifier* ptr, const bool isForced)const noexcept
	{
		const bool isValidPtr =  Core::JIdentifierPrivate::DestroyInstanceInterface::CanDestroyInstancce(ptr, isForced);
		return isValidPtr && (!static_cast<JObject*>(ptr)->HasFlag(J_OBJECT_FLAG::OBJECT_FLAG_UNDESTROYABLE) || isForced);
	}
}