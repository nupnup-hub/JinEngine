#pragma once
#include<string>
#include<vector>
#include"../../ImGuiEx/ImGuiManager.h"

namespace JinEngine
{
	class DebugHelper
	{
	private:
		static std::vector<std::string> strBuffer;
		static int st;
		static int ed;
		static int maxCount;
	public:
		static void Initialize();
		static void Log(const std::string& message);
		static void PrintMessage();
	};
}
