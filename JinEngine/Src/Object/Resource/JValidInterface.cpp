#include"JValidInterface.h"

namespace JinEngine
{
	void JValidInterface::SetValid(bool value)
	{
		isValid = value;
	}
	bool JValidInterface::IsValid()const noexcept
	{
		return isValid;
	}
}