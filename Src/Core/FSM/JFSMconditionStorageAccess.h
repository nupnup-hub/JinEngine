#pragma once 
#include"JFSMconditionValueType.h"
#include"../JDataType.h"
#include<string>
#include<vector>

namespace JinEngine
{
	namespace Core
	{
		class JFSMcondition;
		class IJFSMconditionStorage
		{
			friend class JFSMconditionStorage;
		private:
			virtual void NotifyEraseCondition(JFSMcondition* condition)noexcept = 0;
		};
		 
		__interface IJFSMconditionStorageUser
		{
		public:
			uint GetConditionCount()const noexcept;
			uint GetConditionMaxCount()const noexcept;
			JFSMcondition* GetCondition(const uint index)noexcept;
			JFSMcondition* GetCondition(const std::string& name)noexcept;  
		};
		 
		__interface IJFSMconditionStorageManager : public IJFSMconditionStorageUser
		{
		public:
			void SetConditionName(const std::string& oldName, const std::string& newName)noexcept;
			void SetConditionValueType(const std::string& conditionName, const J_FSMCONDITION_VALUE_TYPE valueType)noexcept;
			JFSMcondition* AddConditionValue()noexcept;
			bool EraseCondition(const std::string& conditionName)noexcept;
		};
	}
}