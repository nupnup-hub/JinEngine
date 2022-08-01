#pragma once
#include"JEnumInitializer.h"

namespace JinEngine
{
	namespace Core
	{
		class JEnumInfo
		{
		private:
			using JEnumInitializer = JinEngine::Core::JEnumInitializer;
			using EnumElementMap = typename JEnumInitializer::EnumElementMap;
		public:
			JEnumInfo(const JEnumInitializer& jEnumInitializer);
			std::string Name()const noexcept;
			std::string FullName()const noexcept;
			std::string ElementName(int value)const noexcept;
		private:
			const std::string name;
			const std::string fullName;
			const EnumElementMap enumElementMap;
		};
	}
}