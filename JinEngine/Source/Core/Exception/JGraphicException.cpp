#include"JGraphicException.h"
#include"../../Graphic/JGraphic.h"
#include"../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Core
	{
		JGraphicException::JGraphicException(int line, const std::string file, HRESULT hr)
			:JException(line, JCUtil::StrToWstr(file)), hr(hr)
		{
			std::wstring errorMsg;
			JGraphic::Instance().GetLastDeviceErrorInfo(hexerrorcode, errorMsg);
			//MessageBox(0, JWindow::Instance().HrException::TranslateErrorCode(gfxHr).c_str(), (L"\n[Error Code] 0x" + JCUtil::StrToWstr(hexerrorcode)).c_str(), 0);
			whatBuffer = errorMsg +  + L"\n" + TranslateErrorCode(hr);
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