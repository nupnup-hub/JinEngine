#pragma once
#include"JFSMInterface.h"
#include"JFSMparameter.h"

namespace JinEngine
{
	namespace Core
	{
		class JFSMtransition;
		///class JFSMparameter;
		class JFSMconditionOwnerInterface;

		class JFSMcondition : public JFSMInterface
		{
			REGISTER_CLASS(JFSMcondition)
		private:
			friend class JFSMtransition;
		public:
			struct JFSMconditionInitData : public JFSMIdentifierInitData
			{
			public: 
				JUserPtr<JFSMtransition> ownerTransition; 
			public:
				JFSMconditionInitData(const std::wstring& name, const size_t guid, JUserPtr<JFSMtransition> ownerTransition);
				JFSMconditionInitData(JUserPtr<JFSMtransition> ownerTransition);
			public:
				bool IsValid() noexcept;
				J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final;
			};
			using InitData = JFSMconditionInitData;
		private: 
			JFSMconditionOwnerInterface* ownerInterface;  
			REGISTER_PROPERTY_EX(parameter, GetParameter, SetParameter, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::NONE, false, GetSroageParameter))
			JFSMparameter* parameter = nullptr;
			REGISTER_PROPERTY_EX(onValue, GetOnValue, SetOnValue,
					GUI_CHECKBOX(GUI_ENUM_CONDITION_REF_USER(ParameterType, parameter, J_FSM_PARAMETER_VALUE_TYPE::BOOL)),
					GUI_FIXED_INPUT(false, Core::J_PARAMETER_TYPE::Int, GUI_ENUM_CONDITION_REF_USER(ParameterType, parameter, J_FSM_PARAMETER_VALUE_TYPE::INT)),
					GUI_FIXED_INPUT(false, Core::J_PARAMETER_TYPE::Float, GUI_ENUM_CONDITION_REF_USER(ParameterType, parameter, J_FSM_PARAMETER_VALUE_TYPE::FLOAT)))
			float onValue = 0; 
		public: 
			J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final;
			JFSMparameter* GetParameter()const noexcept;
			float GetOnValue()const noexcept;
			void SetParameter(JFSMparameter* newParam)noexcept;
			void SetOnValue(float newValue)noexcept;
		public:
			bool HasParameter()const noexcept;
			bool HasSameParameter(const size_t guid)const noexcept;
			bool IsSatisfied()const noexcept;
			bool PassDefectInspection()const noexcept;
		private:
			//For editor gui
			static std::vector<JIdentifier*> GetSroageParameter(JIdentifier* iden)noexcept;
		private:
			bool IsValidParameter(JFSMparameter* newParam)const noexcept;
			void Clear()override;
		private:
			bool RegisterCashData()noexcept final;
			bool DeRegisterCashData()noexcept final;
		private: 
			static void RegisterCallOnce();
		protected:
			JFSMcondition(const JFSMconditionInitData& initData);
			~JFSMcondition();
			JFSMcondition(JFSMcondition&& rhs) = default;
			JFSMcondition& operator=(JFSMcondition&& rhs) = default;
		};

		/*
				JFSMparameterWrap::JFSMparameterWrap(JFSMparameter* condition)
			:condition(condition), onValue(0)
		{}
		JFSMparameter* JFSMparameterWrap::GetParameter()noexcept
		{
			return condition;
		}
		float JFSMparameterWrap::GetOnValue()const noexcept
		{
			J_FSM_PARAMETER_VALUE_TYPE valueType = condition->GetParamType();
			if (valueType == J_FSM_PARAMETER_VALUE_TYPE::BOOL)
				return static_cast<bool>(onValue);
			else if (valueType == J_FSM_PARAMETER_VALUE_TYPE::INT)
				return static_cast<int>(onValue);
			else
				return onValue;
		}
		void JFSMparameterWrap::SetCondition(JFSMparameter* newCondition)noexcept
		{
			condition = newCondition;
			SetOnValue(onValue);
		}
		void JFSMparameterWrap::SetOnValue(float newValue)noexcept
		{
			onValue = newValue;
		}

		bool JFSMparameterWrap::IsSatisfied()const noexcept
		{
			return (PassDefectInspection()) && (condition->GetValue() == onValue);
		}
		bool JFSMparameterWrap::PassDefectInspection()const noexcept
		{
			return condition != nullptr;
		}

		*/
	}
}