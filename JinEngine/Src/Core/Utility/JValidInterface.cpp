#include"JValidInterface.h"

namespace JinEngine
{
	namespace Core
	{
		bool JValidInterface::IsValid()const noexcept
		{
			return isValid;
		}
		void JValidInterface::SetValid(bool value)
		{
			isValid = value;
		}
	} 
}