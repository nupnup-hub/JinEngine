#pragma once 

namespace JinEngine
{
	namespace Core
	{
		class JFSMdiagram;  		 
		__interface JFSMparameterStorageUserAccess;

		class JFSMdiagramOwnerInterface
		{
		private:
			friend class JFSMdiagram;
		protected:
			virtual ~JFSMdiagramOwnerInterface() = default; 
		private: 
			virtual JFSMparameterStorageUserAccess* GetParameterStorageUser()noexcept = 0; 
			virtual bool RegisterDiagram(JFSMdiagram* diagram) = 0;
			virtual bool DeRegisterDiagram(JFSMdiagram* diagram) = 0;
		};
	}
}