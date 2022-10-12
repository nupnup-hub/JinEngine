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
			using EnumElementVec = typename JEnumInitializer::EnumElementVec;
		private:
			const std::string name;
			const std::string fullName;
			const EnumElementMap enumElementMap;
			const EnumElementVec enumElementVec;
		public:
			std::string Name()const noexcept;
			std::string FullName()const noexcept;
			std::string ElementName(int value)const noexcept;
			std::vector<int> GetEnumVec()const noexcept;
		private:
			JEnumInfo(const JEnumInitializer& jEnumInitializer);
			~JEnumInfo() = default;
		};
	}
}