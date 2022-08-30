#pragma once
#include<string>
#include"../Core/JDataType.h"

namespace JinEngine
{
	namespace Core
	{ 
		class FuncUtil
		{
		public:
			static std::string EraseSpace(const std::string& str)noexcept;
			static int StrToInt(const std::string& str)noexcept;
		};
	}
}