#include"JResourceObjectUserInterface.h"
#include"JResourceObject.h"
#include"JClearableInterface.h"

namespace JinEngine
{
	void JResourceObjectUserInterface::CallOnResourceReference(JReferenceInterface* jRobj)
	{
		if(jRobj != nullptr)
			jRobj->OnReference();
	}
	void JResourceObjectUserInterface::CallOffResourceReference(JReferenceInterface* jRobj)
	{
		if (jRobj != nullptr)
			jRobj->OffReference();
	}
	int JResourceObjectUserInterface::CallGetResourceReferenceCount(JReferenceInterface* jRobj)
	{
		return jRobj->GetReferenceCount();
	}
}