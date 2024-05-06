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


#include"JWindowException.h"
#include"../JWindowPrivate.h"
#include"../../Core/Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Window
	{
		JWindowException::JWindowException(int line, const std::string file)
			:JException(line, JCUtil::StrToWstr(file)), lastError(GetLastError())
		{ 
			DWORD dwError = GetDlgItemInt(Window::JWindowPrivate::HandleInterface::GetHandle(), 1000, NULL, FALSE);
			HLOCAL hlocal = NULL;
			DWORD systemLocale = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
			// Get the error code's textual description
			BOOL fOk = FormatMessage(
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
				FORMAT_MESSAGE_ALLOCATE_BUFFER,
				NULL, dwError, systemLocale,
				(PTSTR)&hlocal, 0, NULL);
			if (!fOk) {
				// Is it a network-related error?
				HMODULE hDll = LoadLibraryEx(TEXT("netmsg.dll"), NULL,
					DONT_RESOLVE_DLL_REFERENCES);

				if (hDll != NULL) {
					fOk = FormatMessage(
						FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS |
						FORMAT_MESSAGE_ALLOCATE_BUFFER,
						hDll, dwError, systemLocale,
						(PTSTR)&hlocal, 0, NULL);
					FreeLibrary(hDll);
				}
			}

			if (fOk && (hlocal != NULL))
			{
				whatBuffer = (PCTSTR)LocalLock(hlocal);
				LocalFree(hlocal);
			}
			else
				whatBuffer = L"No text found for this error number.";
		}
		const std::wstring JWindowException::what() const
		{ 
			return GetType() + L"\n[Description]" + whatBuffer + L"\n" + GetOriginString();
		}
		const std::wstring JWindowException::GetType() const
		{
			return L"Winuser Exception";
		}
	}
}