#include"JFSMconditionStorage.h" 
#include"JFSMcondition.h"  
#include"JFSMconditionStorageAccess.h"
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Core
	{
		uint JFSMconditionStorage::GetConditionCount()const noexcept
		{
			return (uint)conditionVec.size();
		}
		uint JFSMconditionStorage::GetConditionMaxCount()const noexcept
		{
			return maxNumberOffCondition;
		}
		JFSMcondition* JFSMconditionStorage::GetCondition(const uint index)noexcept
		{
			const uint conditionCount = (uint)conditionVec.size();
			if (index >= conditionCount)
				return nullptr;
			else
				return conditionVec[index].get();
		}
		JFSMcondition* JFSMconditionStorage::GetCondition(const std::string& name)noexcept
		{
			const size_t guid = JCommonUtility::CalculateGuid(name);
			auto data = conditionCashMap.find(guid);
			if (data != conditionCashMap.end())
				return data->second;
			else
				return nullptr;
		}
		void JFSMconditionStorage::SetConditionName(const std::string& oldName, const std::string& newName)noexcept
		{
			JFSMcondition* tarCondition = GetCondition(oldName);
			if (tarCondition == nullptr)
				return;

			conditionCashMap.erase(tarCondition->GetId());
			const size_t newGuid = JCommonUtility::CalculateGuid(newName);
			tarCondition->SetName(newName, newGuid);
			conditionCashMap.emplace(newGuid, tarCondition);
		}
		void JFSMconditionStorage::SetConditionValueType(const std::string& conditionName, const J_FSMCONDITION_VALUE_TYPE valueType)noexcept
		{
			JFSMcondition* tarCondition = GetCondition(conditionName);
			if (tarCondition == nullptr)
				return;

			tarCondition->SetValueType(valueType);
		}
		JFSMcondition* JFSMconditionStorage::AddConditionValue()noexcept
		{
			uint conditionVecSize = (uint)conditionVec.size();
			if (conditionVecSize >= maxNumberOffCondition)
				return nullptr;

			std::string newName = "NewCondition";
			bool isOk = false;
			int sameCount = 0;
			while (!isOk)
			{
				bool hasSameName = false;
				for (uint i = 0; i < conditionVecSize; ++i)
				{
					if (conditionVec[i]->GetName() == newName)
					{
						hasSameName = true;
						break;
					}
				}

				if (hasSameName)
				{
					JCommonUtility::ModifyOverlappedName(newName, newName.length(), sameCount);
					++sameCount;
				}
				else
					isOk = true;
			}
			const size_t conditionId = JCommonUtility::CalculateGuid(newName);
			conditionVec.emplace_back(std::make_unique<JFSMcondition>(newName, conditionId, J_FSMCONDITION_VALUE_TYPE::BOOL));
			conditionCashMap.emplace(conditionId, conditionVec[conditionVecSize].get());
			return  conditionVec[conditionVecSize].get();
		}
		bool JFSMconditionStorage::EraseCondition(const std::string& conditionName)noexcept
		{
			JFSMcondition* tarCondition = GetCondition(conditionName);
			if (tarCondition == nullptr)
				return false;

			const uint userCount = (uint)strorageUser.size();
			for (uint i = 0; i < userCount; ++i)
				strorageUser[i]->NotifyEraseCondition(tarCondition);
			return true;
		}
	}
}