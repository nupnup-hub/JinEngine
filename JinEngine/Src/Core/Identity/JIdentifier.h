#pragma once
#include"../Reflection/JReflection.h"
#include<string> 

namespace JinEngine
{
	namespace Core
	{
		class JIdentifier
		{
			REGISTER_CLASS(JIdentifier)
		private:
			std::wstring name;
			size_t guid;
		public:
			static constexpr uint maxNameOfLength = 100;
		public:
			std::wstring GetName() const noexcept;
			size_t GetGuid()const noexcept;
			virtual void SetName(const std::wstring& newName)noexcept;
		public:
			template<typename T>
			static std::wstring GetDefaultName()noexcept
			{
				return L"New" + std::wstring(T::TypeWName()).substr(1);
			}
		public:
			template<typename T, std::enable_if_t<std::is_base_of_v<JIdentifier, T>, int> = 0>
			static bool AddInstance(Core::JOwnerPtr<T> ptr)noexcept
			{
				if (ptr.IsValid() && static_cast<JIdentifier*>(ptr.Get())->RegisterCashData())
				{ 
					const size_t guid = ptr->GetGuid(); 
					if (T::StaticTypeInfo().AddInstance(guid, std::move(ptr)))
						return true;
					else
					{
						static_cast<JIdentifier*>(ptr.Get())->DeRegisterCashData();
						return false;
					}
				}
				else
					return false;
			}
			bool RemoveInstance()noexcept;
			template<typename T, std::enable_if_t<std::is_base_of_v<JIdentifier, T>, int> = 0>
			static Core::JOwnerPtr<JIdentifier> ReleaseInstance(const size_t guid)noexcept
			{
				auto ownerPtr = T::StaticTypeInfo().ReleaseInstance(guid);
				if (ownerPtr.IsValid())
					ownerPtr->DeRegisterCashData();
				return ownerPtr;
			}
		private:
			virtual bool RegisterCashData()noexcept = 0;
			virtual bool DeRegisterCashData()noexcept = 0;
		protected:
			JIdentifier(const std::wstring& name, const size_t guid);
			virtual ~JIdentifier();
		};
	}
}