#include"JFSMconditionStorage.h" 
#include"JFSMcondition.h"  
#include"JFSMconditionStorageAccess.h" 
#include"JFSMfactory.h"
#include"../File/JFileIOHelper.h"
#include"../Guid/GuidCreator.h"
#include"../../Utility/JCommonUtility.h"
#include<fstream>

namespace JinEngine
{
	namespace Core
	{
		JFSMconditionStorage::StorageUser::StorageUser(JFSMconditionStorageUserInterface* ptr, const size_t guid)
			:ptr(ptr),guid(guid)
		{}
		size_t JFSMconditionStorage::StorageUser::GetUserGuid()const noexcept
		{
			return guid;
		}

		JFSMconditionStorage::JFSMconditionStorage()
			:guid(MakeGuid())
		{}
		JFSMconditionStorage::~JFSMconditionStorage()
		{
			
		}
		size_t JFSMconditionStorage::GetStorageGuid()const noexcept
		{
			return guid;
		}
		std::wstring JFSMconditionStorage::GetConditionUniqueName(const std::wstring& initName)const noexcept
		{
			return JCUtil::MakeUniqueName(conditionVec, initName);
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
			return conditionVec[index];
		}
		bool JFSMconditionStorage::AddUser(JFSMconditionStorageUserInterface* newUser, const size_t guid)noexcept
		{
			if (newUser != nullptr)
			{
				int index = JCUtil::GetJIdenIndex(storageUser, guid, &StorageUser::GetUserGuid);
				if (index == -1)
				{
					storageUser.push_back(std::make_unique< StorageUser>(newUser, guid));
					return true;
				}
			}
			return false;
		}
		bool JFSMconditionStorage::RemoveUser(JFSMconditionStorageUserInterface* newUser, const size_t guid)noexcept
		{
			if (newUser != nullptr)
			{
				int index = JCUtil::GetJIdenIndex(storageUser, guid, &StorageUser::GetUserGuid);
				if (index != -1)
				{
					storageUser.erase(storageUser.begin() + index);
					return true;
				}
			}
			return false;
		}
		bool JFSMconditionStorage::AddCondition(JFSMcondition* fsmCondition)noexcept
		{
			if (fsmCondition == nullptr || conditionCashMap.find(fsmCondition->GetGuid()) != conditionCashMap.end())
				return false;

			fsmCondition->SetName(GetConditionUniqueName(fsmCondition->GetName()));
			conditionVec.emplace_back(fsmCondition);
			conditionCashMap.emplace(fsmCondition->GetGuid(), fsmCondition);
			return true;
		}
		bool JFSMconditionStorage::RemoveCondition(JFSMcondition* fsmCondition)noexcept
		{ 
			if (fsmCondition == nullptr)
				return false;

			const uint userCount = (uint)storageUser.size();
			for (uint i = 0; i < userCount; ++i)
				storageUser[i]->ptr->NotifyRemoveCondition(fsmCondition);

			conditionCashMap.erase(fsmCondition->GetGuid());
			conditionVec.erase(conditionVec.begin() + JCUtil::GetJIdenIndex(conditionVec, fsmCondition->GetGuid()));
			return true;
		}
		void JFSMconditionStorage::Clear()
		{
			std::vector<JFSMcondition*> copy = conditionVec; 
			const uint conditionCount = (uint)copy.size();
			for (uint i = 0; i < conditionCount; ++i)
				JFSMInterface::Destroy(copy[i]);

			storageUser.clear();
			conditionCashMap.clear();
			conditionVec.clear();
		}
		J_FILE_IO_RESULT JFSMconditionStorage::StoreData(std::wofstream& stream)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			const uint conditionCount = (uint)conditionVec.size();
			JFileIOHelper::StoreAtomicData(stream, L"ConditionCount:", conditionCount);

			for (uint i = 0; i < conditionCount; ++i)
			{
				JFileIOHelper::StoreFsmObjectIden(stream, conditionVec[i]);
				JFileIOHelper::StoreEnumData(stream, L"ValueType:", conditionVec[i]->GetValueType());
				JFileIOHelper::StoreAtomicData(stream, L"Value:", conditionVec[i]->GetValue());
			}
			return J_FILE_IO_RESULT::SUCCESS;
		}
		J_FILE_IO_RESULT JFSMconditionStorage::LoadData(std::wifstream& stream)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			uint conditionCount;
			JFileIOHelper::LoadAtomicData(stream, conditionCount);

			for (uint i = 0; i < conditionCount; ++i)
			{ 
				std::wstring name;
				size_t guid;
				J_FSM_OBJECT_TYPE fType;
				J_FSMCONDITION_VALUE_TYPE valueType;
				float value;

				JFileIOHelper::LoadFsmObjectIden(stream, name, guid, fType);
				JFileIOHelper::LoadEnumData(stream, valueType);
				JFileIOHelper::LoadAtomicData(stream, value);

				JFFI<JFSMcondition>::Create(JPtrUtil::MakeOwnerPtr<JFSMcondition::InitData>(name, guid, valueType, this));
			}

			return J_FILE_IO_RESULT::SUCCESS;
		}
	}
}