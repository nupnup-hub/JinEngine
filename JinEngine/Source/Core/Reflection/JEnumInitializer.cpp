#include"JEnumInitializer.h"
#include"../JCoreEssential.h"
#include"../Utility/JCommonUtility.h"
#include<algorithm>

namespace JinEngine
{
	namespace Core
	{
		void DecomposeEnumStr(const EnumNameMap& map,
			std::string& ori, 
			std::string& enumName,
			int& value, 
			int preValue)
		{
			auto findEnumValueLam = [](const EnumNameMap& map, const std::string& str) ->size_t
			{
				for (const auto& data : map)
				{
					if (data.second == str)
						return data.first;
				}
				return 0;
			};

			int commaIndex = ori.find_first_of('@');
			if (commaIndex != -1)
			{
				enumName = ori.substr(0, commaIndex);
				int equalIndex = (int)enumName.find_first_of('=');
				int orIndex = (int)enumName.find_first_of('|');
				orIndex = -1;
				if (orIndex != -1)
				{
					std::string copyEnum = enumName.substr(equalIndex + 1);
					std::vector<int> orIndexVec{ equalIndex };
					orIndex -= equalIndex;
					while (orIndex != -1)
					{
						orIndexVec.push_back(orIndex);
						copyEnum = copyEnum.substr(orIndex + 1);
						orIndex = (int)copyEnum.find_first_of('|'); 
					}

					copyEnum = enumName;
					size_t sum = 0;
					const uint orCount = (uint)orIndexVec.size();
					for (uint i = 0; i < orCount; ++i)
					{				
						if (i == orCount - 1)
						{
							std::string indexValue = JCUtil::EraseChar(copyEnum.substr(orIndexVec[i] + 1), ' ');
							sum += findEnumValueLam(map, indexValue);
						}
						else
						{
							std::string indexValue = JCUtil::EraseChar(copyEnum.substr(orIndexVec[i] + 1, orIndexVec[i + 1] - 1), ' ');
							sum += findEnumValueLam(map, indexValue);
							copyEnum = copyEnum.substr(orIndexVec[i] + 1);						 
						} 
					}
					value = sum;
					enumName = enumName.substr(0, equalIndex);
				}
				else if(equalIndex != -1)
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
				else 
					value = preValue + 1;
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
			enumName = JCUtil::EraseSideChar(enumName, ' ');
		}

		bool IsTwoSqure(const int oriValue)
		{
			if (oriValue < 0)
				return false;

			uint value = (uint)oriValue;
			while (value > 2)
			{
				if (value % 2 == 0)
					value /= 2;
				else
					return false;
			}
			return true;
		}
		EnumNameMap JEnumInitializer::CreateEnumMap()noexcept
		{
			EnumNameMap map;
			std::string elementCopy = element;

			std::string enumName;
			int value = 0;
			int preValue = -1;
			for (size_t i = 0; i < enumSize; ++i)
			{
				DecomposeEnumStr(map, elementCopy, enumName, value, preValue);
				preValue = value;
				map.emplace(value, enumName);
			}
			return map;
		}

		EnumElementVec JEnumInitializer::CreateEnumVec()noexcept
		{
			EnumNameMap map;
			EnumElementVec vec;
			std::string elementCopy = element;

			std::string enumName;
			int value = 0;
			int preValue = -1;
			for (size_t i = 0; i < enumSize; ++i)
			{
				DecomposeEnumStr(map, elementCopy, enumName, value, preValue);
				preValue = value;
				vec.push_back(value);
				map.emplace(value, enumName);
			}
			return vec;
		}

		bool JEnumInitializer::IsTwoSqureEnum()const noexcept
		{ 
			const uint enumCount = (uint)enumElementVec.size();
			for (uint i = 0; i < enumCount; ++i)
			{
				int oriValue = enumElementVec[i];
				if (oriValue < 0)
					return false;

				if (IsTwoSqure(oriValue))
					continue;
 
				for (uint j = 0; j < enumCount; ++j)
				{
					if (i == j)
						continue;
					 
					int tarValue = enumElementVec[j];
					if (tarValue < 0)
						return false;

					if (IsTwoSqure(tarValue))
						oriValue = (oriValue ^ (tarValue & oriValue));
				}
				if (oriValue == 0)
					continue;
				else
					return false;
			}
			return true;
		}
	}
}