#pragma once 
#include"JFSMobjectType.h" 
#include"../Identity/JIdentifier.h"

namespace JinEngine
{
	namespace Core
	{
		class JFSMinterface : public JIdentifier
		{
			REGISTER_CLASS_IDENTIFIER_LINE(JFSMinterface)
		public:
			class InitData : public Core::JIdentifier::InitData
			{
				REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData) 
			public:
				InitData(const JTypeInfo& initTypeInfo);
				InitData(const JTypeInfo& initTypeInfo, const std::wstring& name, const size_t guid);
			};
		protected: 
			class StoreData : public Core::JDITypeDataBase
			{
				REGISTER_CLASS_ONLY_USE_TYPEINFO(StoreData)
			public:
				JFSMinterface* fsmObj = nullptr;
			public:
				StoreData(JFSMinterface* fsmObj);
			public:
				bool IsValidData()const noexcept override;
				bool HasCorrectType(const JTypeInfo& correctType)const noexcept;
				bool HasCorrectChildType(const JTypeInfo& correctType)const noexcept;
			};
		public:
			virtual J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept = 0;
		protected:
			JFSMinterface(const InitData& initData);
		};
	}
}