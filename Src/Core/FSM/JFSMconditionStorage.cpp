#include"JFSMconditionStorage.h" 
#include"JFSMcondition.h"  
#include"JFSMconditionStorageAccess.h"
#include"JFSMLoadGuidMap.h"
#include"../../Utility/JCommonUtility.h"
#include"../Guid/GuidCreator.h"
#include<fstream>

namespace JinEngine
{
	namespace Core
	{
		std::wstring JFSMconditionStorage::GetConditionUniqueName(const std::wstring& initName)const noexcept
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
		JFSMcondition* JFSMconditionStorage::GetConditionByIndex(const uint index)noexcept
		{
			if (conditionVec.size() <= index)
				return nullptr;
			return conditionVec[index].get();
		}
		JFSMcondition* JFSMconditionStorage::AddCondition(const std::wstring& name)noexcept
		{
			uint conditionVecSize = (uint)conditionVec.size();
			if (conditionVecSize >= maxNumberOffCondition)
				return nullptr;
			 		 
			conditionVec.emplace_back(std::make_unique<JFSMcondition>(GetConditionUniqueName(name), Core::MakeGuid(), J_FSMCONDITION_VALUE_TYPE::BOOL));
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
		JFSMcondition* JFSMconditionStorage::AddCondition(const std::wstring& name, const size_t guid)
		{
			conditionVec.emplace_back(std::make_unique<JFSMcondition>(GetConditionUniqueName(name), Core::MakeGuid(), J_FSMCONDITION_VALUE_TYPE::BOOL));
			conditionCashMap.emplace(guid, conditionVec[conditionVec.size() - 1].get());
			return conditionVec[conditionVec.size() - 1].get();
		}
		J_FILE_IO_RESULT JFSMconditionStorage::StoreIdentifierData(std::wofstream& stream)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			const uint conditionCount = (uint)conditionVec.size();
			stream << conditionCount << '\n'; 
			  
			for (uint i = 0; i < conditionCount; ++i)
				JFSMIdentifier::StoreIdentifierData(stream, *conditionVec[i]); 
			
			return J_FILE_IO_RESULT::SUCCESS;
		}
		J_FILE_IO_RESULT JFSMconditionStorage::StoreContentsData(std::wofstream& stream)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			const uint conditionCount = (uint)conditionVec.size();
			stream << conditionCount << '\n';

			for (uint i = 0; i < conditionCount; ++i)
			{
				stream << (uint)conditionVec[i]->GetValueType() << '\n';
				stream << conditionVec[i]->GetValue() << '\n';
			}
			return J_FILE_IO_RESULT::SUCCESS;
		}
		J_FILE_IO_RESULT JFSMconditionStorage::LoadIdentifierData(std::wifstream& stream, JFSMLoadGuidMap& guidMap)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			uint conditionCount = 0;
			stream >> conditionCount;
			for (uint i = 0; i < conditionCount; ++i)
			{
				JFSMIdentifier::JFSMIdentifierData data;
				JFSMIdentifier::LoadIdentifierData(stream, data);

				if (guidMap.isNewGuid)
					guidMap.condition.emplace(data.guid, AddCondition(data.name)->GetGuid());
				else
					AddCondition(data.name, data.guid);
			}

			return J_FILE_IO_RESULT::SUCCESS;
		}
		J_FILE_IO_RESULT JFSMconditionStorage::LoadContentsData(std::wifstream& stream, JFSMLoadGuidMap& guidMap)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			uint conditionCount = 0;
			stream >> conditionCount;

			if(conditionCount != conditionVec.size())
				return J_FILE_IO_RESULT::FAIL_CORRUPTED_DATA;

			for (uint i = 0; i < conditionCount; ++i)
			{
				uint valueType = 0;
				float value = 0;

				stream >> valueType;
				stream >> value;

				conditionVec[i]->SetValueType((J_FSMCONDITION_VALUE_TYPE)valueType);
				conditionVec[i]->SetValue(value);
			}
			return J_FILE_IO_RESULT::SUCCESS;
		}
	}
}