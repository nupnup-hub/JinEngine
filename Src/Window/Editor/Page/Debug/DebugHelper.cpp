#include"DebugHelper.h"

namespace JinEngine
{
	std::vector<std::string> DebugHelper::strBuffer;
	int DebugHelper::st = 0;
	int DebugHelper::ed = 0;
	int DebugHelper::maxCount = 0;
	void DebugHelper::Initialize()
	{
		strBuffer.resize(1000);
		st = 0;
		ed = 0;
		maxCount = 1000;
	}
	void DebugHelper::Log(const std::string& message)
	{
		strBuffer[ed++] = message;
	}
	void DebugHelper::PrintMessage()
	{
		if (st == ed)
			return;

		for (int i = st; i <= ed; ++i)
		{
			ImGui::Text(strBuffer[i].c_str());
			ImGui::SameLine();
		}
		st = 0;
		ed = 0;
		strBuffer.clear();
	}
}
