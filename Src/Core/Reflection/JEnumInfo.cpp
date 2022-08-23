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
		std::string JEnumInfo::ElementName(int value)const noexcept
		{
			auto data = enumElementMap.find(value);
			return data != enumElementMap.end() ? data->second : "";
		}
		JEnumInfo::JEnumInfo(const JEnumInitializer& jEnumInitializer)
			:name(jEnumInitializer.name), fullName(jEnumInitializer.fullName), enumElementMap(jEnumInitializer.enumElementMap)
		{
			JReflectionInfo::Instance().AddEnum(this);
		}
	}
}