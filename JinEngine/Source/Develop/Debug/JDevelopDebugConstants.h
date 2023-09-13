#pragma once
#include<string>
#include<vector>
namespace JinEngine
{
	namespace Develop
	{
		namespace Constants
		{
			static std::string defualtLogHandlerName = "DefaultDevelopLog";
			static std::string graphicLogHandlerName = "GraphicDevalopLog";
			static std::vector<std::string> allLogHandlerName =
			{
				graphicLogHandlerName
			};
		}
	}
}