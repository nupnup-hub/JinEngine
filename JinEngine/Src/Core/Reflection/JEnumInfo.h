#pragma once
#include"JEnumInitializer.h" 
#include"../JDataType.h"

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
		private:
			const std::string name;
			const std::string fullName;
			const EnumNameMap enumElementMap;
			const EnumElementVec enumElementVec;
			const bool isEnumClass;
			const bool isTwoSqureEnum;
		public:
			std::string Name()const noexcept;
			std::string FullName()const noexcept;
			std::string ElementName(const int value)const noexcept;
			int EnumValue(const int index)const noexcept;
			uint GetEnumCount()const noexcept;
			EnumNameMap GetEnumNameMap()const noexcept;
			EnumNameVec GetEnumNameVec()const noexcept;
			EnumElementVec GetEnumElementVec()const noexcept;
			int GetEnumIndex(const int value)const noexcept;
		public:
			bool IsEnumClass()const noexcept;
			bool IsTwoSqureEnum()const noexcept;
		private:
			JEnumInfo(const JEnumInitializer& jEnumInitializer);
			~JEnumInfo() = default;
		};
	}
}