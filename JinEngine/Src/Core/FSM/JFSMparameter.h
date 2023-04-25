#pragma once
#include"JFSMinterface.h"
#include"JFSMparameterValueType.h"

namespace JinEngine
{
	namespace Core
	{ 
		class JFSMparameterPrivate;
		class JFSMparameterStorageInterface;
		class JFSMparameter final: public JFSMinterface
		{
			REGISTER_CLASS_IDENTIFIER_LINE(JFSMparameter)
		public: 
			class InitData final : public JFSMinterface::InitData
			{
				REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
			public:
				J_FSM_PARAMETER_VALUE_TYPE paramType;
				JFSMparameterStorageInterface* paramStorage = nullptr;
			public:
				InitData(JFSMparameterStorageInterface* paramStorage, const J_FSM_PARAMETER_VALUE_TYPE paramType = J_FSM_PARAMETER_VALUE_TYPE::BOOL);
				InitData(const std::wstring& name, const size_t guid, JFSMparameterStorageInterface* paramStorage, const J_FSM_PARAMETER_VALUE_TYPE paramType = J_FSM_PARAMETER_VALUE_TYPE::BOOL);
			public:
				bool IsValidData()const noexcept override;
			};
		private:
			friend class JFSMparameterPrivate;
			class JFSMparameterImpl;
		private:
			std::unique_ptr<JFSMparameterImpl> impl;
		public:
			Core::JIdentifierPrivate& GetPrivateInterface()const noexcept final;
			J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final;
			float GetValue()const noexcept;
			J_FSM_PARAMETER_VALUE_TYPE GetParamType()const noexcept;
		public:
			void SetValue(const float value)noexcept;
			void SetValue(const int value)noexcept;
			void SetValue(const bool value)noexcept;
			void SetParamType(const J_FSM_PARAMETER_VALUE_TYPE paramType)noexcept;
		private:
			JFSMparameter(const InitData& initData);
			~JFSMparameter();
			JFSMparameter(const JFSMparameter& rhs) = delete;
			JFSMparameter& operator=(const JFSMparameter& rhs) = delete;
		};
	}
}

/*
#pragma once
#include"JFSMinterface.h"
#include"JFSMparameterValueType.h"
#include<string>
#include<algorithm>
namespace JinEngine
{
	namespace Core
	{
		class JFSMparameterStorage;
		class JFSMparameterStorageInterface;

		class JFSMparameter : public JFSMinterface
		{
			REGISTER_CLASS(JFSMparameter)
			REGISTER_GUI_ENUM_CONDITION(ParameterType, J_FSM_PARAMETER_VALUE_TYPE, paramType)
		private:
			friend class JFSMparameterStorage;
		public:
			struct JFSMparameterInitData : public JFSMIdentifierInitData
			{
			public:
				J_FSM_PARAMETER_VALUE_TYPE paramType;
				JFSMparameterStorageInterface* paramStorage = nullptr;
			public:
				JFSMparameterInitData(const std::wstring& name, const size_t guid, const J_FSM_PARAMETER_VALUE_TYPE paramType, JFSMparameterStorageInterface* ownerStorage);
				JFSMparameterInitData(const size_t guid, JFSMparameterStorageInterface* paramStorage);
			public:
				bool IsValid() noexcept;
				J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final;
			};
			using InitData = JFSMparameterInitData;
		private:
			REGISTER_PROPERTY_EX(paramType, GetParamType, SetParamType)
			J_FSM_PARAMETER_VALUE_TYPE paramType = J_FSM_PARAMETER_VALUE_TYPE::BOOL;
			float value = 0;
			JFSMparameterStorageInterface* paramStorage = nullptr;
		public:
			void Initialize()noexcept;
		public:
			J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final;
			float GetValue()const noexcept;
			J_FSM_PARAMETER_VALUE_TYPE GetParamType()const noexcept;
			void SetParamType(const J_FSM_PARAMETER_VALUE_TYPE paramType)noexcept;
			//void SetValue(float value)noexcept;
		//	void SetValue(int value)noexcept;
			//void SetValue(bool value)noexcept;
			template<typename T>
			void SetValue(T value)noexcept
			{
				JFSMparameter::value = TypeValue(paramType, value);
			}
		private:
			void Clear()override;
		private:
			bool RegisterInstance()noexcept final;
			bool DeRegisterInstance()noexcept final;
		private:
			static void RegisterCallOnce();
		private:
			JFSMparameter(const JFSMparameterInitData& initData);
			~JFSMparameter();
			JFSMparameter(const JFSMparameter& rhs) = delete;
			JFSMparameter& operator=(const JFSMparameter& rhs) = delete;
		};
	}
}
*/