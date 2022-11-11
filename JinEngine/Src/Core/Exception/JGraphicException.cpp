#include"JGraphicException.h"
#include"../../Graphic/JGraphic.h"
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Core
	{
		JGraphicException::JGraphicException(int line, const std::string file, HRESULT hr)
			:JException(line, JCUtil::StrToWstr(file)), hr(hr), gfxHr(JGraphic::Instance().DeviceInterface()->GetDevice()->GetDeviceRemovedReason())
		{
			std::ostringstream oss;
			oss << std::hex << gfxHr << std::endl;
			hexerrorcode = oss.str();
			//MessageBox(0, JWindow::Instance().HrException::TranslateErrorCode(gfxHr).c_str(), (L"\n[Error Code] 0x" + JCUtil::StrToWstr(hexerrorcode)).c_str(), 0);
			whatBuffer = TranslateErrorCode(gfxHr);
		}
		const std::wstring JGraphicException::GetType() const
		{
			return L"Graphics Exception";
		}
		const std::wstring JGraphicException::what() const
		{
			std::wstring oss;
			oss = GetType() + L"\n[Error Code] 0x" + JCUtil::StrToWstr(hexerrorcode)
				+ L"\n[Description]" + whatBuffer + L"\n" + GetOriginString();
			return oss;
		}
	}
}