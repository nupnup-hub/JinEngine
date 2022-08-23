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
			std::string GetConditionUniqueName(const std::string& initName)const noexcept;
			uint GetConditionCount()const noexcept;
			uint GetConditionMaxCount()const noexcept;
			JFSMcondition* GetCondition(const size_t guid)noexcept;
			JFSMcondition* GetConditionVec()noexcept;
			void SetConditionName(const size_t guid, const std::string& newName)noexcept;
			void SetConditionValueType(const size_t guid, const J_FSMCONDITION_VALUE_TYPE valueType)noexcept;

			JFSMcondition* AddCondition(const std::string& name, const size_t guid)noexcept;
			bool RemoveCondition(const size_t guid)noexcept;
		};
	}
}