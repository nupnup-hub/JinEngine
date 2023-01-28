#pragma once

namespace JinEngine
{
	namespace Core
	{
		class JFSMdiagram;  		 
		__interface JFSMconditionStorageUserAccess;

		class JFSMdiagramOwnerInterface
		{
		private:
			friend class JFSMdiagram;
		protected:
			virtual ~JFSMdiagramOwnerInterface() = default; 
		private: 
			virtual JFSMconditionStorageUserAccess* GetConditionStorageUser()noexcept = 0;
			virtual bool AddDiagram(JFSMdiagram* diagram)noexcept = 0;
			virtual bool RemoveDiagram(JFSMdiagram* diagram)noexcept = 0;
		};
	}
}