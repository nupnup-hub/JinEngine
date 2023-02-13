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
		private:
			friend class JFSMparameterStorage;
		public:
			struct JFSMparameterInitData : public JFSMIdentifierInitData
			{
			public:
				J_FSM_PARAMETER_VALUE_TYPE valueType;
				IJFSMparameterStorageInterface* paramStorage = nullptr; 
			public:
				JFSMparameterInitData(const std::wstring& name, const size_t guid, const J_FSM_PARAMETER_VALUE_TYPE valueType, IJFSMparameterStorageInterface* ownerStorage);
				JFSMparameterInitData(const size_t guid, IJFSMparameterStorageInterface* paramStorage);
			public:
				bool IsValid() noexcept;
				J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final;
			};
			using InitData = JFSMparameterInitData;
		private: 
			//REGISTER_GUI_ENUM_CONDITION_GROUP(ParameterType, J_FSM_PARAMETER_VALUE_TYPE, valueType)
			//REGISTER_PROPERTY_EX(valueType, GetValueType, SetValueType, GUI_ENUM_COMBO(J_FSM_PARAMETER_VALUE_TYPE))
			J_FSM_PARAMETER_VALUE_TYPE valueType = J_FSM_PARAMETER_VALUE_TYPE::BOOL; 
			float value = 0;
			IJFSMparameterStorageInterface* paramStorage = nullptr;
		public:
			void Initialize()noexcept;  
		public:
			J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final;
			float GetValue()const noexcept;
			J_FSM_PARAMETER_VALUE_TYPE GetValueType()const noexcept;  
			void SetValueType(const J_FSM_PARAMETER_VALUE_TYPE valueType)noexcept;
			template<typename T>
			void SetValue(T value)noexcept
			{ 
				if constexpr (std::is_same_v<T, bool>)
				{
					if (valueType == J_FSM_PARAMETER_VALUE_TYPE::BOOL)
						JFSMparameter::value = value;
				}
				else if constexpr (std::is_same_v<T, int>)
				{
					if (valueType == J_FSM_PARAMETER_VALUE_TYPE::INT)
						JFSMparameter::value = value;
				}
				else if constexpr (std::is_same_v<T, float>)
				{
					if (valueType == J_FSM_PARAMETER_VALUE_TYPE::FLOAT)
						JFSMparameter::value = value;
				}
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