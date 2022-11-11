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
		class IJFSMconditionStorage
		{
			friend class JFSMconditionStorage;
		private:
			virtual void NotifyRemoveCondition(JFSMcondition* condition)noexcept = 0;
		};

		__interface IJFSMconditionStorageUser
		{
		public:
			size_t GetStorageGuid()const noexcept;
			std::wstring GetConditionUniqueName(const std::wstring& initName)const noexcept;
			uint GetConditionCount()const noexcept;
			uint GetConditionMaxCount()const noexcept; 	
			JFSMcondition* GetCondition(const size_t guid)noexcept;
			JFSMcondition* GetConditionByIndex(const uint index)noexcept;
		public:
			bool AddUser(IJFSMconditionStorage* user, const size_t guid)noexcept;
			bool RemoveUser(IJFSMconditionStorage* user, const size_t guid)noexcept;
		};
		 
		__interface IJFSMconditionStorageManager : public IJFSMconditionStorageUser
		{ 
		public:
			J_FILE_IO_RESULT StoreData(std::wofstream& stream);
			J_FILE_IO_RESULT LoadData(std::wifstream& stream);
		};
	}
}