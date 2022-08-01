#include"JNormalException.h"
#include"../../Utility/JCommonUtility.h"
#include"../../Window/JWindows.h"

namespace JinEngine
{
	namespace Core
	{
		JNormalException::JNormalException(int line, const std::string file, const std::string msg)
			:JException(line, JCommonUtility::StringToWstring(file))
		{
			whatBuffer = JCommonUtility::StringToWstring(msg);
		}
		const std::wstring JNormalException::what() const
		{
			std::wstring oss;
			oss = GetType() + L"\n[Description]" + whatBuffer + L"\n" + GetOriginString();
			return oss;
		}
		const std::wstring JNormalException::GetType() const
		{
			return L"Winuser Exception";
		}
	}
}