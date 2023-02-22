#pragma once
#include"../Identity/JIdentifierInterface.h"

namespace JinEngine
{
	namespace Core
	{
		class JFSMdiagram;  		 
		__interface JFSMparameterStorageUserAccess;

		class JFSMdiagramOwnerInterface : public JTypeCashInterface<JFSMdiagram>
		{
		private:
			friend class JFSMdiagram;
		protected:
			virtual ~JFSMdiagramOwnerInterface() = default; 
		private: 
			virtual JFSMparameterStorageUserAccess* GetParameterStorageUser()noexcept = 0; 
		};
	}
}