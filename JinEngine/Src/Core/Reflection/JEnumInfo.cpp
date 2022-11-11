#include"JEnumInfo.h"
#include"JReflectionInfo.h" 

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
			JReflectionInfo::Instance().AddEnum(this);
		}
	}
}