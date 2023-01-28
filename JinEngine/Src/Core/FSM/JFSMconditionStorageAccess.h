#pragma once 
#include"JFSMconditionValueType.h"
#include"../File/JFileIOResult.h"
#include"../JDataType.h"
#include<string>
#include<vector>

namespace JinEngine
{
	namespace Core
	{ 
		class JFSMcondition;
		class JFSMconditionStorage;
		class JFSMconditionStorageUserInterface
		{
			friend class JFSMconditionStorage;
		public:
			virtual ~JFSMconditionStorageUserInterface() = default;
		private:
			virtual void NotifyRemoveCondition(JFSMcondition* condition)noexcept = 0;
		};

		__interface JFSMconditionStorageUserAccess
		{ 
		public:
			size_t GetStorageGuid()const noexcept;
			std::wstring GetConditionUniqueName(const std::wstring& initName)const noexcept;
			uint GetConditionCount()const noexcept;
			uint GetConditionMaxCount()const noexcept; 	
			JFSMcondition* GetCondition(const size_t guid)noexcept;
			JFSMcondition* GetConditionByIndex(const uint index)noexcept;
		public:
			bool AddUser(JFSMconditionStorageUserInterface* user, const size_t guid)noexcept;
			bool RemoveUser(JFSMconditionStorageUserInterface* user, const size_t guid)noexcept;
		};
		 
		__interface JFSMconditionStorageManagerAccess : public JFSMconditionStorageUserAccess
		{
		public:
			void Clear();
		public:
			J_FILE_IO_RESULT StoreData(std::wofstream& stream);
			J_FILE_IO_RESULT LoadData(std::wifstream& stream);
		};
	}
}