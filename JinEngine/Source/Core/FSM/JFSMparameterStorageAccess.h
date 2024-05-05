#pragma once 
#include"JFSMparameterValueType.h"
#include"../File/JFileIOResult.h"
#include"../JCoreEssential.h" 

namespace JinEngine
{
	namespace Core
	{ 
		class JFSMparameter;
		class JFSMparameterStoragePrivate;
		class JFSMparameterStorageUserInterface
		{
			friend class JFSMparameterStoragePrivate;
		public:
			virtual ~JFSMparameterStorageUserInterface() = default;
		private:
			virtual void NotifyRemoveParameter(const size_t guid)noexcept = 0;
		};

		__interface JFSMparameterStoragePublicAccess
		{
		public:
			size_t GetGuid()const noexcept;
			std::wstring GetParameterUniqueName(const std::wstring& initName)const noexcept;
			uint GetParameterCount()const noexcept;
			uint GetParameterMaxCount()const noexcept;
			JUserPtr<JFSMparameter> GetParameter(const size_t guid)const noexcept;
			JUserPtr<JFSMparameter> GetParameterByIndex(const uint index)const noexcept;
			std::vector<JUserPtr<JFSMparameter>> GetParameterVec()const noexcept;
		};

		__interface JFSMparameterStorageUserAccess : public JFSMparameterStoragePublicAccess
		{ 
		public:
			bool AddUser(JFSMparameterStorageUserInterface* user, const size_t guid)noexcept;
			bool RemoveUser(JFSMparameterStorageUserInterface* user, const size_t guid)noexcept;
		};	 
	}
}