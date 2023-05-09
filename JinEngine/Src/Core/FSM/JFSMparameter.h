#pragma once
#include"JFSMinterface.h"
#include"JFSMparameterValueType.h"

namespace JinEngine
{
	namespace Core
	{ 
		class JFSMparameterPrivate; 
		__interface JFSMparameterStoragePublicAccess;
		class JFSMparameter final: public JFSMinterface
		{
			REGISTER_CLASS_IDENTIFIER_LINE(JFSMparameter)
		public: 
			class InitData final : public JFSMinterface::InitData
			{
				REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
			public:
				J_FSM_PARAMETER_VALUE_TYPE paramType;
				JUserPtr<JFSMparameterStoragePublicAccess> paramStorage = nullptr;
			public:
				InitData(const JUserPtr<JFSMparameterStoragePublicAccess>& paramStorage, const J_FSM_PARAMETER_VALUE_TYPE paramType = J_FSM_PARAMETER_VALUE_TYPE::BOOL);
				InitData(const std::wstring& name, const size_t guid, const JUserPtr<JFSMparameterStoragePublicAccess>& paramStorage, const J_FSM_PARAMETER_VALUE_TYPE paramType = J_FSM_PARAMETER_VALUE_TYPE::BOOL);
			public:
				bool IsValidData()const noexcept override;
			};
		private:
			friend class JFSMparameterPrivate;
			class JFSMparameterImpl;
		private:
			std::unique_ptr<JFSMparameterImpl> impl;
		public:
			JIdentifierPrivate& GetPrivateInterface()const noexcept final;
			J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final;
			float GetValue()const noexcept;
			J_FSM_PARAMETER_VALUE_TYPE GetParamType()const noexcept; 
		public:
			void SetValue(const float value)noexcept;
			void SetValue(const int value)noexcept;
			void SetValue(const bool value)noexcept;
			void SetParamType(const J_FSM_PARAMETER_VALUE_TYPE paramType)noexcept;
		public:
			bool IsStorageParameter(const size_t storageGuid)const noexcept;
		private:
			JFSMparameter(const InitData& initData);
			~JFSMparameter();
			JFSMparameter(const JFSMparameter& rhs) = delete;
			JFSMparameter& operator=(const JFSMparameter& rhs) = delete;
		};
	}
}