#pragma once 
#include"JFSMparameterStorageAccess.h"
#include<vector>
#include<memory>
#include<unordered_map>

namespace JinEngine
{
	namespace Core
	{
		class JFSMparameter;
		class JFSMdiagram; 

		class JFSMparameterStorageInterface
		{
		private:
			friend class JFSMparameter;
		public:
			virtual ~JFSMparameterStorageInterface() = default;
		private:
			virtual bool AddParameter(JFSMparameter* fsmCondition)noexcept = 0;
			virtual bool RemoveParameter(JFSMparameter* fsmCondition)noexcept = 0;
		};

		class JFSMparameterStorage : public JFSMparameterStorageManagerAccess, 
			public JFSMparameterStorageInterface
		{
		public:
			struct StorageUser
			{
			public:
				JFSMparameterStorageUserInterface* ptr;
				const size_t guid;
			public:
				StorageUser(JFSMparameterStorageUserInterface* ptr, const size_t guid);
			public:
				size_t GetUserGuid()const noexcept;
			};
		public:
			static constexpr uint maxNumberOfParameter = 250;
		private:
			std::vector<JFSMparameter*> parameterVec;
			std::unordered_map<size_t, JFSMparameter*> parameterCashMap;
			std::vector<std::unique_ptr<StorageUser>>storageUser;
			const size_t guid;
		public:
			JFSMparameterStorage();
			~JFSMparameterStorage();
		public:
			size_t GetStorageGuid()const noexcept;
			std::wstring GetParameterUniqueName(const std::wstring& initName)const noexcept;
			uint GetParameterCount()const noexcept;
			uint GetParameterMaxCount()const noexcept;
			JFSMparameter* GetParameter(const size_t guid)const noexcept;
			JFSMparameter* GetParameterByIndex(const uint index)const noexcept;
			std::vector< JFSMparameter*> GetParameterVec()const noexcept;
		public:
			bool AddUser(JFSMparameterStorageUserInterface* newUser, const size_t guid)noexcept;
			bool RemoveUser(JFSMparameterStorageUserInterface* newUser, const size_t guid)noexcept;
		private:
			bool AddParameter(JFSMparameter* fsmParameter)noexcept final;
			bool RemoveParameter(JFSMparameter* fsmParameter)noexcept final;
		public:
			void Clear();
		public:
			J_FILE_IO_RESULT StoreData(std::wofstream& stream);
			J_FILE_IO_RESULT LoadData(std::wifstream& stream);
		};
	}
}