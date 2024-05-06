/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


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