#pragma once
#include"JEnumInitializer.h"

namespace JinEngine
{
	namespace Core
	{
		template<typename enumType>class JEnumRegister;
		class JEnumInfo
		{
		private:
			template<typename enumType> friend class  JEnumRegister;
		private:
			using JEnumInitializer = JinEngine::Core::JEnumInitializer;
			using EnumElementMap = typename JEnumInitializer::EnumElementMap;
		public:
			std::string Name()const noexcept;
			std::string FullName()const noexcept;
			std::string ElementName(int value)const noexcept;
		private:
			const std::string name;
			const std::string fullName;
			const EnumElementMap enumElementMap;
		private:
			JEnumInfo(const JEnumInitializer& jEnumInitializer);
			~JEnumInfo() = default;
		};
	}
}