#include"JIdentifier.h"


namespace JinEngine
{
	namespace Core
	{
		std::wstring JIdentifier::GetName() const noexcept
		{
			return name;
		}
		size_t JIdentifier::GetGuid()const noexcept
		{
			return guid;
		}
		void JIdentifier::SetName(const std::wstring& newName)noexcept
		{
			if (!newName.empty())
				name = newName;
		}
		bool JIdentifier::RemoveInstance()noexcept
		{
			DeRegisterCashData();
			return GetTypeInfo().RemoveInstance(GetGuid());
		}
		JIdentifier::JIdentifier(const std::wstring& name, const size_t guid)
			:name(name),guid(guid)
		{}
		JIdentifier::~JIdentifier(){}
	}
}