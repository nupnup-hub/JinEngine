#include"JHresultException.h"
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Core
	{
		JHresultException::JHresultException(int line, const std::string file, HRESULT hr)
			:JException(line, JCommonUtility::StrToWstr(file)), hr(hr)
		{}
		const std::wstring JHresultException::what() const
		{
			std::wstring oss;
			oss = GetType() + L"\n[Error Code] 0x" + std::to_wstring(GetErrorCode())
				+ L"\n[Description]" + GetErrorDescription() + L"\n" + GetOriginString();

			whatBuffer = oss;
			return whatBuffer;
		}
		const std::wstring JHresultException::GetType() const
		{
			return L"Hr Window Exception";
		}
		HRESULT JHresultException::GetErrorCode() const
		{
			return hr;
		}
		std::wstring JHresultException::GetErrorDescription() const
		{
			return TranslateErrorCode(hr);
		}
	}
}