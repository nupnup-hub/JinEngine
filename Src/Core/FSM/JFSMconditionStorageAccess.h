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
		struct JFSMLoadGuidMap;
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
			std::wstring GetConditionUniqueName(const std::wstring& initName)const noexcept;
			uint GetConditionCount()const noexcept;
			uint GetConditionMaxCount()const noexcept; 	
			JFSMcondition* GetCondition(const size_t guid)noexcept;
			JFSMcondition* GetConditionByIndex(const uint index)noexcept;
		};
		 
		__interface IJFSMconditionStorageManager : public IJFSMconditionStorageUser
		{
		public: 
			JFSMcondition* AddCondition(const std::string& name)noexcept;
			bool RemoveCondition(const size_t guid)noexcept; 
			J_FILE_IO_RESULT StoreIdentifierData(std::wofstream& stream);
			J_FILE_IO_RESULT StoreContentsData(std::wofstream& stream); 
			J_FILE_IO_RESULT LoadIdentifierData(std::wifstream& stream, JFSMLoadGuidMap& guidMap);
		};
	}
}