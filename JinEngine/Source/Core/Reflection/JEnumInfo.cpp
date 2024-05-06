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


#include"JEnumInfo.h"
#include"JReflectionInfo.h" 
#include"JReflectionInfoPrivate.h"  

namespace JinEngine
{
	namespace Core
	{
		std::string JEnumInfo::Name()const noexcept
		{
			return name;
		}
		std::string JEnumInfo::FullName()const noexcept
		{
			return fullName;
		}
		std::string JEnumInfo::ElementName(const int value)const noexcept
		{
			auto data = enumElementMap.find(value);
			return data != enumElementMap.end() ? data->second : "";
		}
		size_t JEnumInfo::EnumGuid()const noexcept
		{
			return std::hash<std::string>{}(fullName);
		}
		int JEnumInfo::EnumValue(const int index)const noexcept
		{
			return enumElementVec[index];
		}
		uint JEnumInfo::GetEnumCount()const noexcept
		{
			return (uint)enumElementVec.size();
		}
		EnumNameMap JEnumInfo::GetEnumNameMap()const noexcept
		{
			return enumElementMap;
		}
		EnumNameVec JEnumInfo::GetEnumNameVec()const noexcept
		{ 
			const uint enumCount = (uint)enumElementVec.size();
			EnumNameVec enumNameVec(enumCount);
			for (uint i = 0; i < enumCount; ++i)
				enumNameVec[i] = enumElementMap.find(enumElementVec[i])->second;
			return enumNameVec;
		}
		EnumElementVec JEnumInfo::GetEnumElementVec()const noexcept
		{
			return enumElementVec;
		}
		int JEnumInfo::GetEnumIndex(const int value)const noexcept
		{
			const uint enumCount = (uint)enumElementVec.size();
			for (uint i = 0; i < enumCount; ++i)
			{
				if (enumElementVec[i] == value)
					return i;
			}
			return -1;
		}
		bool JEnumInfo::IsEnumClass()const noexcept
		{
			return isEnumClass;
		}
		bool JEnumInfo::IsTwoSqureEnum()const noexcept
		{
			return isTwoSqureEnum;
		}
		JEnumInfo::JEnumInfo(const JEnumInitializer& jEnumInitializer)
			:name(jEnumInitializer.name),
			fullName(jEnumInitializer.fullName),
			enumElementMap(jEnumInitializer.enumElementMap),
			enumElementVec(jEnumInitializer.enumElementVec),
			isEnumClass(jEnumInitializer.isEnumClass),
			isTwoSqureEnum(jEnumInitializer.isTwoSqureEnum)
		{
			JReflectionInfoPrivate::EnumInterface::AddEnum(this);
		}
	}
}