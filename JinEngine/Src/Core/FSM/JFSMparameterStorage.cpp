#include"JFSMparameterStorage.h" 
#include"JFSMparameter.h"  
#include"JFSMparameterStorageAccess.h"  
#include"../File/JFileIOHelper.h"
#include"../Guid/GuidCreator.h"
#include"../Identity/JIdenCreator.h"
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

		JFSMparameterStorage::JFSMparameterStorage(const JUserPtr<JIdentifier>& owner)
			:owner(owner), guid(MakeGuid())
		{}
		JFSMparameterStorage::~JFSMparameterStorage()
		{}
		JUserPtr<JIdentifier> JFSMparameterStorage::GetOwner()const noexcept
		{
			return owner;
		}
		size_t JFSMparameterStorage::GetGuid()const noexcept
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
		JUserPtr<JFSMparameter> JFSMparameterStorage::GetParameter(const size_t guid)const noexcept
		{
			auto paramUser = JFSMparameter::StaticTypeInfo().GetInstanceUserPtr<JFSMparameter>(guid);
			return paramUser.IsValid() && paramUser->IsStorageParameter(guid) ? paramUser : nullptr;
		}
		JUserPtr<JFSMparameter> JFSMparameterStorage::GetParameterByIndex(const uint index)const noexcept
		{
			if (parameterVec.size() <= index)
				return nullptr;
			return parameterVec[index];
		}
		std::vector<JUserPtr<JFSMparameter>>JFSMparameterStorage::GetParameterVec()const noexcept
		{
			return parameterVec;
		}
		bool JFSMparameterStorage::AddUser(JFSMparameterStorageUserInterface* newUser, const size_t guid)noexcept
		{
			if (newUser != nullptr)
			{
				int index = JCUtil::GetTypeIndex(storageUser, guid, &StorageUser::GetUserGuid);
				if (index == -1)
				{
					storageUser.push_back(std::make_unique<StorageUser>(newUser, guid));
					return true;
				}
			}
			return false;
		}
		bool JFSMparameterStorage::RemoveUser(JFSMparameterStorageUserInterface* newUser, const size_t guid)noexcept
		{
			if (newUser != nullptr)
			{
				int index = JCUtil::GetTypeIndex(storageUser, guid, &StorageUser::GetUserGuid);
				if (index != -1)
				{
					storageUser.erase(storageUser.begin() + index);
					return true;
				}
			}
			return false;
		}
		void JFSMparameterStorage::Clear()
		{
			std::vector<JUserPtr<JFSMparameter>> copy = parameterVec;
			const uint conditionCount = (uint)copy.size();
			for (uint i = 0; i < conditionCount; ++i)
				JFSMparameter::BeginDestroy(copy[i].Get());

			storageUser.clear(); 
			parameterVec.clear();
		}
		J_FILE_IO_RESULT JFSMparameterStorage::StoreData(std::wofstream& stream, const JUserPtr<JFSMparameterStorage>& storage)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			const uint conditionCount = (uint)storage->parameterVec.size();
			JFileIOHelper::StoreAtomicData(stream, L"ParameterCount:", conditionCount);

			for (uint i = 0; i < conditionCount; ++i)
			{
				JFileIOHelper::StoreFsmObjectIden(stream, storage->parameterVec[i].Get());
				JFileIOHelper::StoreEnumData(stream, L"ValueType:", storage->parameterVec[i]->GetParamType());
				JFileIOHelper::StoreAtomicData(stream, L"Value:", storage->parameterVec[i]->GetValue());
			}
			return J_FILE_IO_RESULT::SUCCESS;
		}
		J_FILE_IO_RESULT JFSMparameterStorage::LoadData(std::wifstream& stream, const JUserPtr<JFSMparameterStorage>& storage)
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

				JICI::Create<JFSMparameter>(name, guid, storage, valueType);
			}

			return J_FILE_IO_RESULT::SUCCESS;
		}

		using OwnTypeInterface = JFSMparameterStoragePrivate::OwnTypeInterface;

		bool OwnTypeInterface::AddParameter(const JUserPtr<JFSMparameterStoragePublicAccess>& storagePA, const JUserPtr<JFSMparameter>& fsmParameter)noexcept
		{
			if (fsmParameter == nullptr || !fsmParameter->IsStorageParameter(storagePA->GetGuid()))
				return false;
			 
			if (storagePA->GetParameter(fsmParameter->GetGuid()) != nullptr)
				return false;

			auto strage = static_cast<JFSMparameterStorage*>(storagePA.Get());
			fsmParameter->SetName(strage->GetParameterUniqueName(fsmParameter->GetName()));
			strage->parameterVec.emplace_back(fsmParameter); 
			return true;
		}
		bool OwnTypeInterface::RemoveParameter(const JUserPtr<JFSMparameterStoragePublicAccess>& storagePA, const JUserPtr<JFSMparameter>& fsmParameter)noexcept
		{
			if (fsmParameter == nullptr || !fsmParameter->IsStorageParameter(storagePA->GetGuid()))
				return false;

			if (storagePA->GetParameter(fsmParameter->GetGuid()) == nullptr)
				return false;

			auto strage = static_cast<JFSMparameterStorage*>(storagePA.Get());
			const uint userCount = (uint)strage->storageUser.size();
			for (uint i = 0; i < userCount; ++i)
				strage->storageUser[i]->ptr->NotifyRemoveParameter(fsmParameter->GetGuid());
			 
			strage->parameterVec.erase(strage->parameterVec.begin() + JCUtil::GetTypeIndex(strage->parameterVec, fsmParameter->GetGuid()));
			return true;
		}
	}
}