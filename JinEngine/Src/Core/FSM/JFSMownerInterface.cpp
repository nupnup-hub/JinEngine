#include"JFSMownerInterface.h"
#include"JFSMdiagramPrivate.h"

namespace JinEngine
{
	namespace Core
	{
		using OwnerInterface = JFSMdiagramPrivate::OwnerTypeInterface;

		void JFSMdiagramOwnerInterface::SetOwnerPointer(const JUserPtr<JFSMdiagram>& diagram, JFSMdiagramOwnerInterface* ownerPtr)noexcept
		{
			OwnerInterface::SetOwnerPointer(diagram, ownerPtr);
		}
	}
}