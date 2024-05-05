#pragma once 
#include"JFSMparameterStorageAccess.h"
#include"../Reflection/JTypeBase.h"
#include"../File/JFileIOHelper.h" 

namespace JinEngine
{
	namespace Core
	{
		class JFSMparameter;
		class JFSMdiagram; 

		class JFSMparameterStoragePrivate
		{
		public:
			class OwnTypeInterface
			{
			private:
				friend class JFSMparameter;
			private:
				static bool AddParameter(const JUserPtr<JFSMparameterStoragePublicAccess>& storagePA, const JUserPtr<JFSMparameter>& fsmParameter)noexcept;
				static bool RemoveParameter(const JUserPtr<JFSMparameterStoragePublicAccess>& storagePA, const JUserPtr<JFSMparameter>& fsmParameter)noexcept;
			};
		};

		class JFSMparameterStorage : public JFSMparameterStorageUserAccess
		{
			REGISTER_CLASS_USE_ALLOCATOR(JFSMparameterStorage)
		private:
			friend class JFSMparameterStoragePrivate;
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
			JUserPtr<Core::JIdentifier> owner;
		private:
			std::vector<JUserPtr<JFSMparameter>> parameterVec;
			std::unordered_map<size_t, JUserPtr<JFSMparameter>> parameterMap;
			std::vector<std::unique_ptr<StorageUser>>storageUser; 
		private:
			const size_t guid;
		public:
			JFSMparameterStorage(const JUserPtr<Core::JIdentifier>& owner);
			~JFSMparameterStorage();
		public:
			JUserPtr<Core::JIdentifier> GetOwner()const noexcept;
			size_t GetGuid()const noexcept;
			std::wstring GetParameterUniqueName(const std::wstring& initName)const noexcept;
			uint GetParameterCount()const noexcept;
			uint GetParameterMaxCount()const noexcept;
			JUserPtr<JFSMparameter> GetParameter(const size_t guid)const noexcept;
			JUserPtr<JFSMparameter> GetParameterByIndex(const uint index)const noexcept;
			std::vector<JUserPtr<JFSMparameter>> GetParameterVec()const noexcept;
		public:
			bool AddUser(JFSMparameterStorageUserInterface* newUser, const size_t guid)noexcept;
			bool RemoveUser(JFSMparameterStorageUserInterface* newUser, const size_t guid)noexcept;
		public:
			void Clear();
		public:
			static J_FILE_IO_RESULT LoadData(JFileIOTool& tool, const JUserPtr< JFSMparameterStorage>& storage);
			static J_FILE_IO_RESULT StoreData(JFileIOTool& tool,  const JUserPtr< JFSMparameterStorage>& storage);
		};
	}
}