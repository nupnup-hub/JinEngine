	#pragma once
#include<string>
#include<vector>
#include"JApplicationState.h" 
#include"../Core/Platform/JLanguageType.h"     

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