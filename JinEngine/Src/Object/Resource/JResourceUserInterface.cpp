#include"JResourceUserInterface.h"
#include"JResourceObject.h"
#include"JClearableInterface.h"

namespace JinEngine
{
	void JResourceUserInterface::CallOnResourceReference(JReferenceInterface* jRobj)
	{
		if(jRobj != nullptr)
			jRobj->OnReference();
	}
	void JResourceUserInterface::CallOffResourceReference(JReferenceInterface* jRobj)
	{
		if (jRobj != nullptr)
			jRobj->OffReference();
	}
	int JResourceUserInterface::CallGetResourceReferenceCount(JReferenceInterface& jRobj)
	{
		return jRobj.GetReferenceCount();
	}
}