#pragma once
#include"JFSMInterface.h"
#include"JFSMconditionValueType.h"
#include<string>
#include<algorithm>
namespace JinEngine 
{
	namespace Core
	{
		class JFSMconditionStorage; 
		class IJFSMconditionOwnerInterface;
		 
		class JFSMcondition : public JFSMInterface
		{
			REGISTER_CLASS(JFSMcondition)
		private:
			friend class JFSMconditionStorage;
		public:
			struct JFSMconditionInitData : public JFSMIdentifierInitData
			{
			public:
				J_FSMCONDITION_VALUE_TYPE valueType;
				IJFSMconditionOwnerInterface* conditionOwner = nullptr; 
			public:
				JFSMconditionInitData(const std::wstring& name, const size_t guid, const J_FSMCONDITION_VALUE_TYPE valueType, IJFSMconditionOwnerInterface* ownerStorage);
				JFSMconditionInitData(const size_t guid, IJFSMconditionOwnerInterface* conditionOwner);
			public:
				bool IsValid() noexcept;
				J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final;
			};
			using InitData = JFSMconditionInitData;
		private: 
			float value = 0;
			J_FSMCONDITION_VALUE_TYPE valueType = J_FSMCONDITION_VALUE_TYPE::BOOL;
			IJFSMconditionOwnerInterface* conditionOwner = nullptr;
		public:
			void Initialize()noexcept;  
		public:
			J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final;
			float GetValue()const noexcept;
			J_FSMCONDITION_VALUE_TYPE GetValueType()const noexcept;  
			void SetValueType(const J_FSMCONDITION_VALUE_TYPE valueType)noexcept;
			template<typename T>
			void SetValue(T value)noexcept
			{
				if constexpr (std::is_same_v<T, bool>)
				{
					if (valueType == J_FSMCONDITION_VALUE_TYPE::BOOL)
						JFSMcondition::value = value;
				}
				else if constexpr (std::is_same_v<T, int>)
				{
					if (valueType == J_FSMCONDITION_VALUE_TYPE::INT)
						JFSMcondition::value = value;
				}
				else if constexpr (std::is_same_v<T, float>)
				{
					if (valueType == J_FSMCONDITION_VALUE_TYPE::FLOAT)
						JFSMcondition::value = value;
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
			JFSMcondition(const JFSMconditionInitData& initData);
			~JFSMcondition();
			JFSMcondition(const JFSMcondition& rhs) = delete;
			JFSMcondition& operator=(const JFSMcondition& rhs) = delete;
		};
	}
}