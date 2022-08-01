#include"JException.h"

namespace JinEngine
{
	namespace Core
	{
		JException::JException(int line, const std::wstring& file)
			:
			line(line),
			file(file)
		{}
		JException::~JException() {}
		const std::wstring JException::what() const
		{
			return GetType() + GetOriginString();
		}

		const std::wstring JException::GetType() const
		{
			return L"JException";
		}

		int JException::GetLine() const
		{
			return line;
		}

		const std::wstring JException::GetFile() const
		{
			return file;
		}

		const std::wstring JException::GetOriginString() const
		{
			return L"[JFile] " + file + L"\n" + L"[Line]" + std::to_wstring(line);
		}
		std::wstring JException::TranslateErrorCode(HRESULT hr)const
		{
			LPVOID  pMsgBuf; //reinterpret_cast<LPTSTR>(&pMsgBuf)
			// windows will allocate memory for err wstring and make our pointer point to it
			const DWORD nMsgLen = FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(wchar_t*)&pMsgBuf, 0, nullptr
			);
			// 0 wstring length returned indicates a failure
			if (nMsgLen == 0)
			{
				return L"Unidentified error code";
			}
			// copy error wstring from windows-allocated buffer to std::wstring

			std::wstring msg((wchar_t*)pMsgBuf, (wchar_t*)pMsgBuf + nMsgLen);

			// free windows buffer
			LocalFree(pMsgBuf);
			return msg;
		}
	}
}