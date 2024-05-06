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
#include"JEngineToolType.h" 
#include"../../Core/JCoreEssential.h"

namespace JinEngine
{
	namespace Application
	{
		class JEngineTool
		{
		public:
			//first is file path second is include path 
		public:
			struct SolFileInfo
			{
			public:
				std::string filePath;
				std::string includePath;
			public:
				SolFileInfo(const std::string& filePath, const std::string& includePath);
			};
			struct SolAddFileConfig
			{
			public:
				bool allowBuild = false;
			};
		public:
			static constexpr char invalidArgPerChar = ' ';
		public:
			static void ExecuteTool(const J_ENGINE_TOOL_TYPE type, std::string args);
			/**
			* @param[in] argPerChar input char end of args[0... n - 2] if char is not invalidArgPerChar
			*/
			static void ExecuteTool(const J_ENGINE_TOOL_TYPE type, const std::vector<std::string>& args, const char argPerChar = invalidArgPerChar);
		public:		
			/**
			* @brief solutionManager utility
			*/
			static void CreateNewProject(const std::string& name, 
				const std::string& path,
				const std::vector<std::string>& config = std::vector<std::string>{});
			/**
			* @brief solutionManager utility
			* @param[in] dirPath it is path from project to dir
			*/
			static void CreateProjectVirtualDir(const std::string& slnPath, const std::string& projName, const std::string& dirPath);
			/**
			* @brief solutionManager utility
			* @param[in] dirPath it is path from project to dir
			*/
			static void AddFile(const std::string& slnPath,
				const std::string& projName,
				const SolFileInfo& fileInfo,
				SolAddFileConfig config = SolAddFileConfig());
			/**
			* @brief solutionManager utility 
			*/
			static void AddMultiFile(const std::string& slnPath,
				const std::string& projName,
				const std::vector<SolFileInfo>& fileInfo,
				SolAddFileConfig config = SolAddFileConfig());
			/**
			* @brief solutionManager utility
			* @detail Adding files and folders recursively to a project filter.
			* @param[in] dirPath it is path from project to dir
			*/
			static void AddDirectory(const std::string& slnPath, const std::string& projName, const std::string& dirPath, const std::string& virtualDirPath);
			/**
			* @brief solutionManager utility 
			*/
			static void RemoveProjectItem(const std::string& includePath,
				const std::string& slnPath,
				const std::string& projName);
			/**
			* @brief solutionManager utility
			*/
			static void RemoveAllProjectItem(const std::string& slnPath, const std::string& projName);
			/**
			* @brief solutionManager utility
			*/
			static void SetProjectConfig(const std::string& slnPath, 
				const std::string& projPath, 
				const std::vector<std::string>& config);
		};
	}
}