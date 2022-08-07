#include"JResourceUserInterface.h"
#include"JResourceObject.h"
#include"JClearableInterface.h"

namespace JinEngine
{
	void JResourceUserInterface::OnResourceReference(JReferenceInterface& jRobj)
	{
		jRobj.OnReference();
	}
	void JResourceUserInterface::OffResourceReference(JReferenceInterface& jRobj)
	{
		jRobj.OffReference();
	}
	int JResourceUserInterface::GetResourceReferenceCount(JReferenceInterface& jRobj)
	{
		return jRobj.GetReferenceCount();
	}
	bool JResourceUserInterface::IsVaildResource(JValidInterface& jRobj)
	{
		return jRobj.IsValidResource();
	}
}