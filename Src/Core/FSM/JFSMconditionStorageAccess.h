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
			virtual void NotifyRemoveCondition(JFSMcondition* condition)noexcept = 0;
		};
		 
		__interface IJFSMconditionStorageUser
		{
		public:
			std::string GetConditionUniqueName(const std::string& initName)const noexcept;
			uint GetConditionCount()const noexcept;
			uint GetConditionMaxCount()const noexcept; 	
			JFSMcondition* GetCondition(const size_t guid)noexcept;
		};
		 
		__interface IJFSMconditionStorageManager : public IJFSMconditionStorageUser
		{
		public:
			void SetConditionName(const size_t guid, const std::string& newName)noexcept;
			void SetConditionValueType(const size_t guid, const J_FSMCONDITION_VALUE_TYPE valueType)noexcept;
			JFSMcondition* AddCondition(const std::string& name, const size_t guid)noexcept;
			bool RemoveCondition(const size_t guid)noexcept;
		};
	}
}