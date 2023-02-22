#pragma once
#include"JFSMInterface.h"
#include"JFSMparameterValueType.h"
#include<string>
#include<algorithm>
namespace JinEngine
{
	namespace Core
	{
		class JFSMparameterStorage;
		class IJFSMparameterStorageInterface;

		class JFSMparameter : public JFSMInterface
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
				IJFSMparameterStorageInterface* paramStorage = nullptr;
			public:
				JFSMparameterInitData(const std::wstring& name, const size_t guid, const J_FSM_PARAMETER_VALUE_TYPE paramType, IJFSMparameterStorageInterface* ownerStorage);
				JFSMparameterInitData(const size_t guid, IJFSMparameterStorageInterface* paramStorage);
			public:
				bool IsValid() noexcept;
				J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final;
			};
			using InitData = JFSMparameterInitData;
		private:
			REGISTER_PROPERTY_EX(paramType, GetParamType, SetParamType)
			J_FSM_PARAMETER_VALUE_TYPE paramType = J_FSM_PARAMETER_VALUE_TYPE::BOOL;
			float value = 0;
			IJFSMparameterStorageInterface* paramStorage = nullptr;
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
			bool RegisterCashData()noexcept final;
			bool DeRegisterCashData()noexcept final;
		private:
			static void RegisterJFunc();
		private:
			JFSMparameter(const JFSMparameterInitData& initData);
			~JFSMparameter();
			JFSMparameter(const JFSMparameter& rhs) = delete;
			JFSMparameter& operator=(const JFSMparameter& rhs) = delete;
		};
	}
}