#include"JWindowException.h"
#include"../../Utility/JCommonUtility.h"
#include"../../Window/JWindows.h"

namespace JinEngine
{
	namespace Core
	{
		JWindowException::JWindowException(int line, const std::string file)
			:JException(line, JCommonUtility::StrToWstr(file)), lastError(GetLastError())
		{ 
			DWORD dwError = GetDlgItemInt(JWindow::Instance().HandleInterface()->GetHandle(), 1000, NULL, FALSE);
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
			std::wstring oss;
			oss = GetType() + L"\n[Description]" + whatBuffer + L"\n" + GetOriginString();
			return oss;
		}
		const std::wstring JWindowException::GetType() const
		{
			return L"Winuser Exception";
		}
	}
}