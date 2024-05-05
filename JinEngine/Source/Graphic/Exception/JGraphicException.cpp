#include"JGraphicException.h"
#include"../JGraphic.h"
#include"../../Core/Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Graphic
	{
		JGraphicException::JGraphicException(int line, const std::string file, HRESULT hr)
			:JException(line, JCUtil::StrToWstr(file)), hr(hr)
		{
			std::wstring errorMsg;
			_JGraphic::Instance().GetLastDeviceErrorInfo(hexerrorcode, errorMsg); 
			whatBuffer = errorMsg + +L"\n" + TranslateErrorCode(hr);
		}
		const std::wstring JGraphicException::what() const
		{
			return GetType() + L"\n[Error Code] 0x" + hexerrorcode +
				L"\n[Description]" + whatBuffer + L"\n" + GetOriginString();
		}
		const std::wstring JGraphicException::GetType() const
		{
			return L"Graphics Exception";
		}
	}
}