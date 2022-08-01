#pragma once 
#include"JFSMconditionStorageAccess.h"
#include<vector>
#include<memory>
#include<unordered_map>

namespace JinEngine
{
	namespace Core
	{
		class JFSMcondition;
		class JFSMdiagram;
		class IJFSMconditionStorage;
		class JFSMconditionStorage : public IJFSMconditionStorageManager
		{
		private:
			std::vector<std::unique_ptr<JFSMcondition>> conditionVec;
			std::unordered_map<size_t, JFSMcondition*> conditionCashMap;
			std::vector<IJFSMconditionStorage*>strorageUser;
			static constexpr uint maxNumberOffCondition = 100;
		public:
			uint GetConditionCount()const noexcept;
			uint GetConditionMaxCount()const noexcept;
			JFSMcondition* GetCondition(const uint index)noexcept;
			JFSMcondition* GetCondition(const std::string& name)noexcept; 
			void SetConditionName(const std::string& oldName, const std::string& newName)noexcept;
			void SetConditionValueType(const std::string& conditionName, const J_FSMCONDITION_VALUE_TYPE valueType)noexcept;

			JFSMcondition* AddConditionValue()noexcept;
			bool EraseCondition(const std::string& conditionName)noexcept;
		};
	}
}