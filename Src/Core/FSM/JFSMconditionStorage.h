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
		class JFSMconditionStorage : public IJFSMconditionStorageManager
		{
		private:
			std::vector<std::unique_ptr<JFSMcondition>> conditionVec;
			std::unordered_map<size_t, JFSMcondition*> conditionCashMap;
			std::vector<IJFSMconditionStorage*>strorageUser;
			static constexpr uint maxNumberOffCondition = 100;
		public:
			std::wstring GetConditionUniqueName(const std::wstring& initName)const noexcept;
			uint GetConditionCount()const noexcept;
			uint GetConditionMaxCount()const noexcept;
			JFSMcondition* GetCondition(const size_t guid)noexcept;
			JFSMcondition* GetConditionByIndex(const uint index)noexcept; 

			JFSMcondition* AddCondition(const std::wstring& name)noexcept;
			bool RemoveCondition(const size_t guid)noexcept; 
		private:
			JFSMcondition* AddCondition(const std::wstring& name, const size_t guid);
		public:
			J_FILE_IO_RESULT StoreIdentifierData(std::wofstream& stream);
			J_FILE_IO_RESULT StoreContentsData(std::wofstream& stream); 
			J_FILE_IO_RESULT LoadIdentifierData(std::wifstream& stream, JFSMLoadGuidMap& guidMap);
			J_FILE_IO_RESULT LoadContentsData(std::wifstream& stream, JFSMLoadGuidMap& guidMap);
		};
	}
}