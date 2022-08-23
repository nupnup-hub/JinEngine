#pragma once
#include<string>
#include<algorithm>
#include"JFSMconditionValueType.h"

namespace JinEngine 
{
	namespace Core
	{
		class JFSMcondition
		{
			friend class JFSMconditionStorage;
		private:
			std::string name;
			const size_t guid;
			float value = 0;
			J_FSMCONDITION_VALUE_TYPE valueType;
		public:
			JFSMcondition(const std::string& name, const size_t guid, const J_FSMCONDITION_VALUE_TYPE valueType);
			~JFSMcondition();
			JFSMcondition(const JFSMcondition& rhs) = delete;
			JFSMcondition& operator=(const JFSMcondition& rhs) = delete;
		public:
			void Initialize()noexcept;
			std::string GetName()const noexcept;
			size_t GetGuid()const noexcept;
			float GetValue()const noexcept;
			J_FSMCONDITION_VALUE_TYPE GetValueType()const noexcept;
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
			void SetName(const std::string& name)noexcept;
			void SetValueType(const J_FSMCONDITION_VALUE_TYPE valueType)noexcept;
		};
	}
}