#pragma once

namespace JinEngine
{
	namespace Core
	{
		class JFSMdiagram; 
		class JFSMcondition;

		class IJFSMconditionOwner;
		__interface IJFSMconditionStorageUser;

		class IJFSMdiagramOwner
		{
		private:
			friend class JFSMdiagram;
		protected:
			virtual ~IJFSMdiagramOwner() = default;
		public:
			virtual std::wstring GetUniqueDiagramName(const std::wstring& name)noexcept = 0;
		private: 
			virtual IJFSMconditionStorageUser* GetConditionStorageUser()noexcept = 0;
			virtual bool AddDiagram(JFSMdiagram* diagram)noexcept = 0;
			virtual bool RemoveDiagram(JFSMdiagram* diagram)noexcept = 0;
		};

		class IJFSMconditionStorageOwner
		{
		private:
			friend class JFSMcondition;
		private:
			virtual IJFSMconditionOwner* GetConditionOwner()noexcept = 0;
		};
	}
}