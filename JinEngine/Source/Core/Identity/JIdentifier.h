#pragma once 
#include"../JCoreEssential.h"
#include"../Reflection/JTypeBase.h" 
#include"../DI/JDIDataBase.h"
#include<string> 

namespace JinEngine
{
	namespace Core
	{   
		class JIdentifierPrivate;
		class JIdentifier : public JTypeBase
		{
			REGISTER_CLASS_IDENTIFIER_LINE(JIdentifier)
		public:
			struct InitData : public JDITypeDataBase	// it is same as metaData
			{
				REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
			public:
				const JTypeInfo& initTypeInfo;
			public:
				std::wstring name;
				size_t guid;
			public:
				InitData(const JTypeInfo& initTypeInfo);
				InitData(const JTypeInfo& initTypeInfo, const size_t guid);
				InitData(const JTypeInfo& initTypeInfo, const std::wstring& name, const size_t guid);
			public:
				const JTypeInfo& InitDataTypeInfo()const noexcept;
			public:
				bool IsValidData()const noexcept override;
			};
		private:
			friend class JIdentifierPrivate;
			class JIdentifierImpl;
		private:
			std::unique_ptr<JIdentifierImpl> impl;
		public:
			std::wstring GetName() const noexcept;
			std::wstring GetNameWithType()const noexcept; 
			static std::wstring GetDefaultName(const JTypeInfo& info)noexcept; 
			static JIdentifierPrivate* PrivateInterface(const size_t typeGuid)noexcept;
			virtual JIdentifierPrivate& PrivateInterface()const noexcept = 0;
		public:
			virtual void SetName(const std::wstring& newName)noexcept;
		public:  
			static bool BeginCopy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to);
			static bool BeginDestroy(Core::JIdentifier* ptr);
		protected:
			static bool BeginForcedDestroy(Core::JIdentifier* ptr);
			static void RegisterPrivateInterface(const JTypeInfo& info, JIdentifierPrivate& p);	//can't register abstract class 
		public:
			template<typename T, std::enable_if_t<std::is_base_of_v<Core::JIdentifier, T>, int> = 0>
			static std::wstring GetDefaultName()noexcept
			{
				return GetDefaultName(T::StaticTypeInfo());
			}
		protected:
			JIdentifier(const InitData& initData);
			~JIdentifier();
		};
	} 
}