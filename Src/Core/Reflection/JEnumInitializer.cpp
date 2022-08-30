#include"JEnumInitializer.h"
#include"../JDataType.h"
#include"../../Utility/JFuncUtility.h"
#include<algorithm>

namespace JinEngine
{
	namespace Core
	{
		JEnumInitializer::EnumElementMap JEnumInitializer::CreateEnumMap()noexcept
		{
			EnumElementMap map;

			int eleValue = 0;
			std::string elementCopy = element;
			for (size_t i = 0; i < enumSize; ++i)
			{
				int commaIndex = elementCopy.find_first_of('@');
				if (commaIndex != -1)
				{
					std::string elementPart = elementCopy.substr(0, commaIndex);
					int equalIndex = (int)elementPart.find_first_of('=');
					if (equalIndex == -1)
					{
						map.emplace(eleValue, elementPart);
						++eleValue;
					}
					else
					{
						int value = FuncUtil::StrToInt(FuncUtil::EraseSpace(elementPart.substr(equalIndex + 1)));
						map.emplace(value, elementPart.substr(0, equalIndex));
						eleValue += value;
					}
					elementCopy = elementCopy.substr(commaIndex + 1);
				}
				else
				{
					int equalIndex = (int)elementCopy.find_first_of('=');
					if (equalIndex == -1)
						map.emplace(eleValue, elementCopy);
					else
					{
						int value = FuncUtil::StrToInt(FuncUtil::EraseSpace(elementCopy.substr(equalIndex + 1)));
						map.emplace(value, elementCopy.substr(0, equalIndex));
					}
					break;
				}
			}
			return map;
		}
	}
}