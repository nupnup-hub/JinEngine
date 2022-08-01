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
			size_t id;
			float value = 0;
			J_FSMCONDITION_VALUE_TYPE valueType;
		public:
			JFSMcondition(const std::string& name, const size_t id, const J_FSMCONDITION_VALUE_TYPE valueType);
			~JFSMcondition();
			JFSMcondition(const JFSMcondition& rhs) = delete;
			JFSMcondition& operator=(const JFSMcondition& rhs) = delete;

			void Initialize()noexcept;
			std::string GetName()const noexcept;
			size_t GetId()const noexcept;
			float GetValue()const noexcept;
			J_FSMCONDITION_VALUE_TYPE GetValueType()const noexcept;
			void SetValue(float value)noexcept;
		private:
			void SetName(const std::string& name, const size_t id)noexcept;
			void SetValueType(const J_FSMCONDITION_VALUE_TYPE valueType)noexcept;
		};
	}
}