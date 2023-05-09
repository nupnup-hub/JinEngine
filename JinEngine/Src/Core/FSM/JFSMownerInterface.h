#pragma once 
#include"../Pointer/JOwnerPtr.h"
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
			virtual bool RegisterDiagram(JUserPtr<JFSMdiagram> diagram)noexcept = 0;
			virtual bool DeRegisterDiagram(JUserPtr<JFSMdiagram> diagram)noexcept = 0;
		protected:
			static void SetOwnerPointer(const JUserPtr<JFSMdiagram>& diagram, JFSMdiagramOwnerInterface* ownerPtr)noexcept;
		};
	}
}