#include"JFSMconditionStorage.h" 
#include"JFSMcondition.h"  
#include"JFSMconditionStorageAccess.h"
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Core
	{
		std::string JFSMconditionStorage::GetConditionUniqueName(const std::string& initName)const noexcept
		{
			return JCommonUtility::MakeUniqueName(conditionVec, initName);
		}
		uint JFSMconditionStorage::GetConditionCount()const noexcept
		{
			return (uint)conditionVec.size();
		}
		uint JFSMconditionStorage::GetConditionMaxCount()const noexcept
		{
			return maxNumberOffCondition;
		}
		JFSMcondition* JFSMconditionStorage::GetCondition(const size_t guid)noexcept
		{ 
			auto data = conditionCashMap.find(guid);
			if (data != conditionCashMap.end())
				return data->second;
			else
				return nullptr;
		}
		void JFSMconditionStorage::SetConditionName(const size_t guid, const std::string& newName)noexcept
		{
			JFSMcondition* tarCondition = GetCondition(guid);
			if (tarCondition == nullptr)
				return;
			  
			tarCondition->SetName(newName); 
		}
		void JFSMconditionStorage::SetConditionValueType(const size_t guid, const J_FSMCONDITION_VALUE_TYPE valueType)noexcept
		{
			JFSMcondition* tarCondition = GetCondition(guid);
			if (tarCondition == nullptr)
				return;

			tarCondition->SetValueType(valueType);
		}
		JFSMcondition* JFSMconditionStorage::AddCondition(const std::string& name, const size_t guid)noexcept
		{
			uint conditionVecSize = (uint)conditionVec.size();
			if (conditionVecSize >= maxNumberOffCondition)
				return nullptr;

			std::string newName = name;
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

			conditionVec.emplace_back(std::make_unique<JFSMcondition>(newName, J_FSMCONDITION_VALUE_TYPE::BOOL));
			conditionCashMap.emplace(conditionVec[conditionVecSize]->GetGuid(), conditionVec[conditionVecSize].get());
			return  conditionVec[conditionVecSize].get();
		}
		bool JFSMconditionStorage::RemoveCondition(const size_t guid)noexcept
		{
			JFSMcondition* tarCondition = GetCondition(guid);
			if (tarCondition == nullptr)
				return false;

			const uint userCount = (uint)strorageUser.size();
			for (uint i = 0; i < userCount; ++i)
				strorageUser[i]->NotifyRemoveCondition(tarCondition);
			return true;
		}
	}
}