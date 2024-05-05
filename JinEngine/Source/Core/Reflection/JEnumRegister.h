#pragma once 
#include"JEnumInitializer.h"
#include"JEnumInfo.h"

namespace JinEngine
{
	namespace Core
	{
		template<typename enumType>
		class JEnumRegister
		{
		public:
			JEnumRegister(const std::string& name, const std::string element, const size_t enumSize, bool isEnumClass)
			{
				static JEnumInfo enumInfo{ JEnumInitializer(name, typeid(enumType).name(), element, enumSize, isEnumClass) };
			}
		};
	}
}