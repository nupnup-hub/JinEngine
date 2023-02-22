#include"JFSMparameterStorage.h" 
#include"JFSMparameter.h"  
#include"JFSMparameterStorageAccess.h" 
#include"JFSMfactory.h"
#include"../File/JFileIOHelper.h"
#include"../Guid/GuidCreator.h"
#include"../../Utility/JCommonUtility.h"
#include<fstream>

namespace JinEngine
{
	namespace Core
	{
		JFSMparameterStorage::StorageUser::StorageUser(JFSMparameterStorageUserInterface* ptr, const size_t guid)
			:ptr(ptr),guid(guid)
		{}
		size_t JFSMparameterStorage::StorageUser::GetUserGuid()const noexcept
		{
			return guid;
		}

		JFSMparameterStorage::JFSMparameterStorage()
			:guid(MakeGuid())
		{}
		JFSMparameterStorage::~JFSMparameterStorage()
		{
			
		}
		size_t JFSMparameterStorage::GetStorageGuid()const noexcept
		{
			return guid;
		}
		std::wstring JFSMparameterStorage::GetParameterUniqueName(const std::wstring& initName)const noexcept
		{
			return JCUtil::MakeUniqueName(parameterVec, initName);
		}
		uint JFSMparameterStorage::GetParameterCount()const noexcept
		{
			return (uint)parameterVec.size();
		}
		uint JFSMparameterStorage::GetParameterMaxCount()const noexcept
		{
			return maxNumberOfParameter;
		}
		JFSMparameter* JFSMparameterStorage::GetParameter(const size_t guid)const noexcept
		{
			auto data = parameterCashMap.find(guid);
			if (data != parameterCashMap.end())
				return data->second;
			else
				return nullptr;
		}
		JFSMparameter* JFSMparameterStorage::GetParameterByIndex(const uint index)const noexcept
		{
			if (parameterVec.size() <= index)
				return nullptr;
			return parameterVec[index];
		}
		std::vector<JFSMparameter*>JFSMparameterStorage::GetParameterVec()const noexcept
		{
			return parameterVec;
		}
		bool JFSMparameterStorage::AddUser(JFSMparameterStorageUserInterface* newUser, const size_t guid)noexcept
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
		bool JFSMparameterStorage::RemoveUser(JFSMparameterStorageUserInterface* newUser, const size_t guid)noexcept
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
		bool JFSMparameterStorage::AddParameter(JFSMparameter* fsmParameter)noexcept
		{
			if (fsmParameter == nullptr || parameterCashMap.find(fsmParameter->GetGuid()) != parameterCashMap.end())
				return false;

			fsmParameter->SetName(GetParameterUniqueName(fsmParameter->GetName()));
			parameterVec.emplace_back(fsmParameter);
			parameterCashMap.emplace(fsmParameter->GetGuid(), fsmParameter);
			return true;
		}
		bool JFSMparameterStorage::RemoveParameter(JFSMparameter* fsmParameter)noexcept
		{ 
			if (fsmParameter == nullptr)
				return false;

			const uint userCount = (uint)storageUser.size();
			for (uint i = 0; i < userCount; ++i)
				storageUser[i]->ptr->NotifyRemoveParameter(fsmParameter->GetGuid());

			parameterCashMap.erase(fsmParameter->GetGuid());
			parameterVec.erase(parameterVec.begin() + JCUtil::GetJIdenIndex(parameterVec, fsmParameter->GetGuid()));
			return true;
		}
		void JFSMparameterStorage::Clear()
		{
			std::vector<JFSMparameter*> copy = parameterVec; 
			const uint conditionCount = (uint)copy.size();
			for (uint i = 0; i < conditionCount; ++i)
				JFSMInterface::Destroy(copy[i]);

			storageUser.clear();
			parameterCashMap.clear();
			parameterVec.clear();
		}
		J_FILE_IO_RESULT JFSMparameterStorage::StoreData(std::wofstream& stream)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			const uint conditionCount = (uint)parameterVec.size();
			JFileIOHelper::StoreAtomicData(stream, L"ParameterCount:", conditionCount);

			for (uint i = 0; i < conditionCount; ++i)
			{
				JFileIOHelper::StoreFsmObjectIden(stream, parameterVec[i]);
				JFileIOHelper::StoreEnumData(stream, L"ValueType:", parameterVec[i]->GetParamType());
				JFileIOHelper::StoreAtomicData(stream, L"Value:", parameterVec[i]->GetValue());
			}
			return J_FILE_IO_RESULT::SUCCESS;
		}
		J_FILE_IO_RESULT JFSMparameterStorage::LoadData(std::wifstream& stream)
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
				J_FSM_PARAMETER_VALUE_TYPE valueType;
				float value;

				JFileIOHelper::LoadFsmObjectIden(stream, name, guid, fType);
				JFileIOHelper::LoadEnumData(stream, valueType);
				JFileIOHelper::LoadAtomicData(stream, value);

				JFFI<JFSMparameter>::Create(JPtrUtil::MakeOwnerPtr<JFSMparameter::InitData>(name, guid, valueType, this));
			}

			return J_FILE_IO_RESULT::SUCCESS;
		}
	}
}