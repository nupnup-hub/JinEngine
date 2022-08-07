#include"JReferenceInterface.h"

namespace JinEngine
{
	int JReferenceInterface::GetReferenceCount()const noexcept
	{
		return referenceCount;
	}
	void JReferenceInterface::OnReference()noexcept
	{
		++referenceCount;
	}
	void JReferenceInterface::OffReference()noexcept
	{
		--referenceCount;
		if (referenceCount < 0)
			referenceCount = 0;
	}
}