#pragma once
#include"JEngineToolType.h"
#include<string>
#include<vector>

namespace JinEngine
{
	namespace Core
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
			static void ExecuteTool(const J_ENGINE_TOOL_TYPE type, const std::string& args);
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