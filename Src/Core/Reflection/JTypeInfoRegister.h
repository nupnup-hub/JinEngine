#pragma once
#include"JTypeInfo.h"
#include"JTypeInfoInitializer.h"

namespace JinEngine
{
	namespace Core
	{
		template<typename Type>
		class JTypeInfoRegister
		{
		public:
			JTypeInfoRegister(const std::string& name)
			{
				static JTypeInfo typeInfo{ JTypeInfoInitializer<Type>(name)};
			}
		};
	}
}