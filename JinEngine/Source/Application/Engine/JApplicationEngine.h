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


#pragma once 
#include"../JApplicationType.h" 
#include"../../Core/JCoreEssential.h"
#include"../../Core/Platform/JLanguageType.h"     

namespace JinEngine
{
	namespace Application
	{
		class JApplicationEngine
		{
		public:
			static std::wstring ExePath()noexcept;
			static std::wstring SolutionPath()noexcept; 
			static std::wstring ProjectPath()noexcept;
			static std::wstring BinaryPath()noexcept;
			static std::wstring DotNetBinaryPath()noexcept; 
			static std::wstring ResourcePath();
			static std::wstring DoucmentPath()noexcept;
			static std::wstring ProjectDocumentPath()noexcept;
			static std::wstring LogPath()noexcept;
			static std::wstring ConfigPath()noexcept;
		public:
			static std::wstring DefaultResourcePath()noexcept;
			static std::wstring ProjectLastRsPath()noexcept;
			static std::wstring ProjectListFilePath()noexcept;
			static std::wstring ShaderPath()noexcept;
		public:
			static std::wstring SouceCodePath()noexcept;
		public:
			static std::wstring ThirdPartyPath()noexcept;
		public:
			static std::string GetLatestVersion()noexcept;
			static int GetSubverionDigitRange()noexcept;
			static std::vector<std::string> GetAppVersion()noexcept;
		public:
			static J_APPLICATION_STATE GetApplicationState()noexcept;
			static J_APPLICATION_SUB_STATE GetApplicationSubState()noexcept;
			static Core::J_LANGUAGE_TYPE GetLanguageType()noexcept;
			static std::vector<std::wstring> GetDefaultFolderPath()noexcept;
		public:
			static std::string GetDriveAlpha()noexcept;
		public:
			static bool IsDefaultFolder(const std::wstring& path)noexcept;
		};	 
	}
	using JApplicationEngine = Application::JApplicationEngine;
}