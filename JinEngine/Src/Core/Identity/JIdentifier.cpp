#include"JIdentifier.h"
#include"../../Utility/JCommonUtility.h"
#include"../../Object/GameObject/JGameObject.h"
#include"../../Object/Resource/Texture/JTexture.h"

namespace JinEngine
{
	namespace Core
	{
		std::wstring JIdentifier::GetName() const noexcept
		{
			return name;
		} 
		std::wstring JIdentifier::GetNameWithType()const noexcept
		{
			return L"Type: " + JCUtil::U8StrToWstr(GetTypeInfo().NameWithOutPrefix()) + L"\nName: " + GetName();
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
		bool JIdentifier::BeginDestroy(JIdentifier* iden)
		{
			if (iden == nullptr)
				return false;
			return iden->DoBeginDestroy();
		}
		JIdentifier::JIdentifier(const std::wstring& name, const size_t guid)
			:name(name),guid(guid)
		{}
		JIdentifier::~JIdentifier(){}
	}
}