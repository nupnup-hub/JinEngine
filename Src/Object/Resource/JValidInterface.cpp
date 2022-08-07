#include"JValidInterface.h"

namespace JinEngine
{
	void JValidInterface::SetValid(bool value)
	{
		isValid = value;
	}
	bool JValidInterface::IsValidResource()const noexcept
	{
		return isValid;
	}
}