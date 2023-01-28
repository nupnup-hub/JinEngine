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

		class IJFSMconditionOwnerInterface
		{
		private:
			friend class JFSMcondition;
		public:
			virtual ~IJFSMconditionOwnerInterface() = default;
		private:
			virtual bool AddCondition(JFSMcondition* fsmCondition)noexcept = 0;
			virtual bool RemoveCondition(JFSMcondition* fsmCondition)noexcept = 0;
		};

		class JFSMconditionStorage : public JFSMconditionStorageManagerAccess, 
			public IJFSMconditionOwnerInterface
		{
		public:
			struct StorageUser
			{
			public:
				JFSMconditionStorageUserInterface* ptr;
				const size_t guid;
			public:
				StorageUser(JFSMconditionStorageUserInterface* ptr, const size_t guid);
			public:
				size_t GetUserGuid()const noexcept;
			};
		public:
			static constexpr uint maxNumberOffCondition = 100;
		private:
			std::vector<JFSMcondition*> conditionVec;
			std::unordered_map<size_t, JFSMcondition*> conditionCashMap;
			std::vector<std::unique_ptr<StorageUser>>storageUser;
			const size_t guid;
		public:
			JFSMconditionStorage();
			~JFSMconditionStorage();
		public:
			size_t GetStorageGuid()const noexcept;
			std::wstring GetConditionUniqueName(const std::wstring& initName)const noexcept;
			uint GetConditionCount()const noexcept;
			uint GetConditionMaxCount()const noexcept;
			JFSMcondition* GetCondition(const size_t guid)noexcept;
			JFSMcondition* GetConditionByIndex(const uint index)noexcept; 
		public:
			bool AddUser(JFSMconditionStorageUserInterface* newUser, const size_t guid)noexcept;
			bool RemoveUser(JFSMconditionStorageUserInterface* newUser, const size_t guid)noexcept;
		private:
			bool AddCondition(JFSMcondition* fsmCondition)noexcept final;
			bool RemoveCondition(JFSMcondition* fsmCondition)noexcept final;
		public:
			void Clear();
		public:
			J_FILE_IO_RESULT StoreData(std::wofstream& stream);
			J_FILE_IO_RESULT LoadData(std::wifstream& stream);
		};
	}
}