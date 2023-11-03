#include"JFSMparameterStorage.h" 
#include"JFSMparameter.h"  
#include"JFSMparameterStorageAccess.h"  
#include"../File/JFileIOHelper.h"
#include"../Guid/JGuidCreator.h"
#include"../Identity/JIdenCreator.h"
#include"../Utility/JCommonUtility.h"
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
			auto data = parameterMap.find(guid);
			return data != parameterMap.end() ? data->second : nullptr; 
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
		J_FILE_IO_RESULT JFSMparameterStorage::LoadData(JFileIOTool& tool, const JUserPtr<JFSMparameterStorage>& storage)
		{
			if (!tool.CanLoad())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			uint conditionCount;
			JFileIOHelper::LoadAtomicData(tool, conditionCount, "ParameterCount:");

			tool.PushExistStack("ParameterData");
			for (uint i = 0; i < conditionCount; ++i)
			{
				std::wstring name;
				size_t guid;
				J_FSM_OBJECT_TYPE fType;
				J_FSM_PARAMETER_VALUE_TYPE valueType;
				float value;

				tool.PushExistStack();
				JFileIOHelper::LoadFsmIden(tool, name, guid, fType);
				JFileIOHelper::LoadEnumData(tool, valueType, "ValueType:");
				JFileIOHelper::LoadAtomicData(tool, value, "Value");
				tool.PopStack();

				JICI::Create<JFSMparameter>(name, guid, storage, valueType);
			}
			tool.PopStack(); 
			return J_FILE_IO_RESULT::SUCCESS;
		}
		J_FILE_IO_RESULT JFSMparameterStorage::StoreData(JFileIOTool& tool, const JUserPtr<JFSMparameterStorage>& storage)
		{
			if(!tool.CanStore())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			const uint conditionCount = (uint)storage->parameterVec.size();
			JFileIOHelper::StoreAtomicData(tool, conditionCount, "ParameterCount:");

			tool.PushArrayOwner("ParameterData");
			for (uint i = 0; i < conditionCount; ++i)
			{
				tool.PushArrayMember();
				JFileIOHelper::StoreFsmIden(tool, storage->parameterVec[i].Get());
				JFileIOHelper::StoreEnumData(tool, storage->parameterVec[i]->GetParamType(), "ValueType:");
				JFileIOHelper::StoreAtomicData(tool, storage->parameterVec[i]->GetValue(), "Value");
				tool.PopStack();
			}
			tool.PopStack(); 
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
			strage->parameterMap.emplace(fsmParameter->GetGuid(), fsmParameter);
			return true;
		}
		bool OwnTypeInterface::RemoveParameter(const JUserPtr<JFSMparameterStoragePublicAccess>& storagePA, const JUserPtr<JFSMparameter>& fsmParameter)noexcept
		{
			if (fsmParameter == nullptr || !fsmParameter->IsStorageParameter(storagePA->GetGuid()))
				return false;

			const size_t paramGuid = fsmParameter->GetGuid();
			if (storagePA->GetParameter(paramGuid) == nullptr)
				return false;

			auto strage = static_cast<JFSMparameterStorage*>(storagePA.Get());
			const uint userCount = (uint)strage->storageUser.size();
			for (uint i = 0; i < userCount; ++i)
				strage->storageUser[i]->ptr->NotifyRemoveParameter(paramGuid);
			 
			strage->parameterMap.erase(paramGuid);
			strage->parameterVec.erase(strage->parameterVec.begin() + JCUtil::GetTypeIndex(strage->parameterVec, paramGuid));
			return true;
		}
	}
}