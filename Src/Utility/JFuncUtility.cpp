#include"JFuncUtility.h"

namespace JinEngine
{
	namespace Core
	{
		std::string FuncUtil::EraseSpace(const std::string& str)noexcept
		{
			std::string res = str;
			while (!res.empty())
			{
				if (res.front() == ' ')
					res = res.substr(1);
				else
					break;
			}

			while (!res.empty())
			{
				if (res.back() == ' ')
					res.pop_back();
				else
					break;
			}
			return res;
		}

		int FuncUtil::StrToInt(const std::string& str)noexcept
		{
			const uint valueSize = (uint)str.size();

			int res = 0;
			int mulFactor = (int)pow(10, valueSize - 1);

			for (uint i = 0; i < valueSize; ++i)
				res += (int)(str[i] - '0') * (mulFactor - i);
			 
			return res;
		}
	}
}