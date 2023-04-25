#pragma once
#include<string> 
#include<memory>
#include<vector>
#include"JApplicationState.h" 
#include"../Core/JDataType.h" 
#include"../Core/Platform/JLanguageType.h"     
#include"../Core/Time/JRealTime.h"
#include"../Utility/JMacroUtility.h"

namespace JinEngine
{
	namespace Application
	{
		class JApplicationProjectInfo
		{
		private:
			using JTime = Core::JRealTime::JTime;
		private:
			const size_t guid;
		private:
			std::wstring name;
			std::wstring path;
			std::wstring version;
			JTime createdTime;
			JTime lastUpdateTime;
		public:
			JApplicationProjectInfo(const size_t guid,
				const std::wstring& name,
				const std::wstring& path,
				const std::wstring& version,
				const JTime& createdTime,
				const JTime& lastUpdateTime);
		public:
			J_SIMPLE_GET(size_t, guid, Guid)
			J_SIMPLE_GET(std::wstring, name, Name)
			J_SIMPLE_GET(std::wstring, path, Path)
			J_SIMPLE_GET(std::wstring, version, Version)
			J_SIMPLE_GET(JTime, lastUpdateTime, LastUpdateTime)
			J_SIMPLE_GET(JTime, createdTime, CreatedTime)
			std::unique_ptr<JApplicationProjectInfo> GetUnique()noexcept;
			std::wstring lastRsPath()noexcept;
		};

		class JApplicationProjectPrivate;
		class JApplicationProject final
		{
		public:
			static std::wstring Name()noexcept;
			static std::wstring RootPath()noexcept;
			static std::wstring ContentsPath()noexcept;
			static std::wstring SettingPath()noexcept;
			static std::wstring LogPath()noexcept;
			static std::wstring ProjectResourcePath()noexcept;
		public:
			//Contents
			static std::wstring ContentScenePath()noexcept;
			static std::wstring ContentScriptPath()noexcept;
			static std::wstring ContentResourcePath()noexcept;
		public:
			//Setting
			static std::wstring EditorSettingPath()noexcept;
		public:
			//Setting
			static std::wstring ProjectLogPath()noexcept;
			static std::wstring ProjectVersionFilePath()noexcept;
		public:
			//ProjectResourcePath
			static std::wstring DefaultResourcePath()noexcept;	//ProjectResourcePath() \\ DefulatResource
			static std::wstring ShaderMetafilePath()noexcept;
			static std::wstring ModResourceCachePath()noexcept;
		public:
			static uint GetProjectInfoCount()noexcept;
			static JApplicationProjectInfo* GetOpenProjectInfo()noexcept;
			static JApplicationProjectInfo* GetProjectInfo(uint index)noexcept;
			static JApplicationProjectInfo* GetProjectInfo(const std::wstring& projectPath)noexcept;
			static std::vector<std::wstring> GetDefaultFolderPath()noexcept;
		public:
			static bool IsValidVersion(const std::string& pVersion)noexcept;
			static bool IsValidPath(const std::wstring& projectPath)noexcept; 
			static bool IsDefaultFolder(const std::wstring& path)noexcept;
		}; 
	}
	using JApplicationProject = Application::JApplicationProject;
	using JApplicationProjectInfo = Application::JApplicationProjectInfo;
}