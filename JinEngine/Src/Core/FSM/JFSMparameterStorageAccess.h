#pragma once 
#include"JFSMparameterValueType.h"
#include"../File/JFileIOResult.h"
#include"../JDataType.h"
#include<string>
#include<vector>

namespace JinEngine
{
	namespace Core
	{ 
		class JFSMparameter;
		class JFSMparameterStorage;
		class JFSMparameterStorageUserInterface
		{
			friend class JFSMparameterStorage;
		public:
			virtual ~JFSMparameterStorageUserInterface() = default;
		private:
			virtual void NotifyRemoveParameter(const size_t guid)noexcept = 0;
		};

		__interface JFSMparameterStorageUserAccess
		{ 
		public:
			size_t GetStorageGuid()const noexcept;
			std::wstring GetParameterUniqueName(const std::wstring& initName)const noexcept;
			uint GetParameterCount()const noexcept;
			uint GetParameterMaxCount()const noexcept; 	
			JFSMparameter* GetParameter(const size_t guid)const noexcept;
			JFSMparameter* GetParameterByIndex(const uint index)const noexcept;
			std::vector< JFSMparameter*> GetParameterVec()const noexcept;
		public:
			bool AddUser(JFSMparameterStorageUserInterface* user, const size_t guid)noexcept;
			bool RemoveUser(JFSMparameterStorageUserInterface* user, const size_t guid)noexcept;
		};
		 
		__interface JFSMparameterStorageManagerAccess : public JFSMparameterStorageUserAccess
		{
		public:
			void Clear();
		public:
			J_FILE_IO_RESULT StoreData(std::wofstream& stream);
			J_FILE_IO_RESULT LoadData(std::wifstream& stream);
		};
	}
}