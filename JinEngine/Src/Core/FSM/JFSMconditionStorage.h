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

		class IJFSMconditionOwner
		{
		private:
			friend class JFSMcondition;
		private:
			virtual bool AddCondition(JFSMcondition* fsmCondition)noexcept = 0;
			virtual bool RemoveCondition(JFSMcondition* fsmCondition)noexcept = 0;
		};

		class JFSMconditionStorage : public IJFSMconditionStorageManager, public IJFSMconditionOwner
		{
		public:
			struct StorageUser
			{
			public:
				IJFSMconditionStorage* ptr;
				const size_t guid;
			public:
				StorageUser(IJFSMconditionStorage* ptr, const size_t guid);
			public:
				size_t GetUserGuid()const noexcept;
			};
		public:
			static constexpr uint maxNumberOffCondition = 100;
		private:
			std::vector<JFSMcondition*> conditionVec;
			std::unordered_map<size_t, JFSMcondition*> conditionCashMap;
			std::vector<std::unique_ptr<StorageUser>>strorageUser;
			const size_t guid;
		public:
			JFSMconditionStorage();
		public:
			size_t GetStorageGuid()const noexcept;
			std::wstring GetConditionUniqueName(const std::wstring& initName)const noexcept;
			uint GetConditionCount()const noexcept;
			uint GetConditionMaxCount()const noexcept;
			JFSMcondition* GetCondition(const size_t guid)noexcept;
			JFSMcondition* GetConditionByIndex(const uint index)noexcept; 
		public:
			bool AddUser(IJFSMconditionStorage* newUser, const size_t guid)noexcept;
			bool RemoveUser(IJFSMconditionStorage* newUser, const size_t guid)noexcept;
		private:
			bool AddCondition(JFSMcondition* fsmCondition)noexcept final;
			bool RemoveCondition(JFSMcondition* fsmCondition)noexcept final;
		public:
			J_FILE_IO_RESULT StoreData(std::wofstream& stream);
			J_FILE_IO_RESULT LoadData(std::wifstream& stream);
		};
	}
}