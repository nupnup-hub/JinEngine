#include"JNormalException.h"
#include"../../Utility/JCommonUtility.h"
#include"../../Window/JWindow.h"

namespace JinEngine
{
	namespace Core
	{
		JNormalException::JNormalException(int line, const std::string file, const std::string msg)
			:JException(line, JCUtil::StrToWstr(file))
		{
			whatBuffer = JCUtil::StrToWstr(msg);
		}
		const std::wstring JNormalException::what() const
		{ 
			return GetType() + L"\n[Description]" + whatBuffer + L"\n" + GetOriginString();
		}
		const std::wstring JNormalException::GetType() const
		{
			return L"Winuser Exception";
		}
	}
}