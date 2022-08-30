#pragma once
#include"JFSMIdentifier.h"
#include"JFSMconditionValueType.h"
#include<string>
#include<algorithm>
namespace JinEngine 
{
	namespace Core
	{
		class JFSMcondition : public JFSMIdentifier
		{
			friend class JFSMconditionStorage;
		private: 
			float value = 0;
			J_FSMCONDITION_VALUE_TYPE valueType;
		public:
			JFSMcondition(const std::wstring& name, const size_t guid, const J_FSMCONDITION_VALUE_TYPE valueType);
			~JFSMcondition();
			JFSMcondition(const JFSMcondition& rhs) = delete;
			JFSMcondition& operator=(const JFSMcondition& rhs) = delete;
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
		};
	}
}