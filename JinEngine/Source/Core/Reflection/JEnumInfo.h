/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once
#include"JEnumInitializer.h" 
#include"../JCoreEssential.h"

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
			template<typename E, std::enable_if_t<std::is_enum_v<E>, int> = 0>
			std::string ElementName(const E value)const noexcept
			{
				return ElementName((int)value);
			}
			size_t EnumGuid()const noexcept;
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