/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


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
		std::wstring JException::TranslateErrorCode(HRESULT hr)
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