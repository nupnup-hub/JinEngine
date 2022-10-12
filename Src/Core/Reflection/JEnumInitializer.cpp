#include"JEnumInitializer.h"
#include"../JDataType.h"
#include"../../Utility/JCommonUtility.h"
#include<algorithm>

namespace JinEngine
{
	namespace Core
	{
		void DecomposeEnumStr(std::string& ori, std::string& enumName, int& value)
		{
			int commaIndex = ori.find_first_of('@');
			if (commaIndex != -1)
			{
				enumName = ori.substr(0, commaIndex);
				int equalIndex = (int)enumName.find_first_of('=');
				if (equalIndex == -1)
					++value;
				else
				{
					int shiftIndex = (int)enumName.find_first_of("<<");
					if (shiftIndex != -1)
					{
						std::string left = JCUtil::EraseChar(enumName.substr(equalIndex + 1, (shiftIndex - equalIndex - 1)), ' ');
						std::string right = JCUtil::EraseChar(enumName.substr(shiftIndex + 2), ' ');
						value = JCUtil::StringToInt(left) << JCUtil::StringToInt(right);
					}
					else
						value = JCUtil::StringToInt(JCUtil::EraseSideChar(enumName.substr(equalIndex + 1), ' '));
					enumName = enumName.substr(0, equalIndex);
				}
				ori = ori.substr(commaIndex + 1);
			}
			else
			{
				int equalIndex = (int)ori.find_first_of('=');
				if (equalIndex == -1)
					enumName = ori;
				else
				{
					value = JCUtil::StringToInt(JCUtil::EraseSideChar(ori.substr(equalIndex + 1), ' '));
					enumName = ori.substr(0, equalIndex);
				}
			}
		}
		JEnumInitializer::EnumElementMap JEnumInitializer::CreateEnumMap()noexcept
		{
			EnumElementMap map;
			std::string elementCopy = element;

			std::string enumName;
			int value = 0;

			for (size_t i = 0; i < enumSize; ++i)
			{
				DecomposeEnumStr(elementCopy, enumName, value); 
				map.emplace(value, name);
			}
			return map;
		}

		JEnumInitializer::EnumElementVec JEnumInitializer::CreateEnumVec()noexcept
		{
			EnumElementVec vec;
			std::string elementCopy = element;

			std::string enumName;
			int value = 0;
			for (size_t i = 0; i < enumSize; ++i)
			{
				DecomposeEnumStr(elementCopy, enumName, value);
				vec.push_back(value);
			}
			return vec;
		}
	}
}