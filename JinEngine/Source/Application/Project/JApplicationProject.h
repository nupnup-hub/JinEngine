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
#include"../../Core/Time/JRealTime.h"
#include"../../Core/Module/JModuleMacro.h"
#include"../../Core/Utility/JMacroUtility.h" 

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
			std::unique_ptr<JApplicationProjectInfo> CreateReplica()noexcept;
			std::wstring lastRsPath()noexcept;
		public:
			//last rs is stored close app by .dss format
			//and next time engine execute
			//JProjectSelectorHub(Editor) try to get texture(.dss format)
			static std::wstring CombineLastRsPath(const size_t guid)noexcept;
		};

		class JApplicationProjectPrivate;
		class JApplicationProject final
		{
		public:
			static std::wstring Name()noexcept;
			/**
			* @brief Project root folder path
			*/
			static std::wstring RootPath()noexcept;
			static std::wstring ContentsPath()noexcept;
			static std::wstring DocumentPath()noexcept;
			static std::wstring ConfigPath()noexcept;
			static std::wstring ProjectPrivateResourcePath()noexcept; 
			static std::wstring BinaryPath()noexcept;  
			static std::wstring SolutionPath()noexcept;
		public:
			//Contents
			static std::wstring ContentScenePath()noexcept; 
			static std::wstring ContentResourcePath()noexcept;
			static std::wstring DefaultResourcePath()noexcept;
		public:
			//Documtent
			static std::wstring LogPath()noexcept;
			static std::wstring DevelopLogPath()noexcept;
		public:
			//Config
			static std::wstring EditoConfigPath()noexcept;
			static std::wstring ProjectVersionFilePath()noexcept;
		public:   
			static std::wstring ShaderMetafilePath()noexcept;
			static std::wstring ModResourceCachePath()noexcept;
		public:
			//Binrary
			static std::wstring ProjectBinaryPath()noexcept;
		public:
			//Source
			static std::wstring SouceCodePath()noexcept;
		public:
			static std::wstring OutDirPath()noexcept;
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