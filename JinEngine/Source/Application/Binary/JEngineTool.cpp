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


#include"JEngineTool.h" 
#include"../Engine/JApplicationEngine.h" 
#include"../../Core/Platform/JPlatformInfo.h"
#include"../../Core/Utility/JCommonUtility.h"

namespace JinEngine::Application
{
	namespace
	{
		static constexpr bool debuggingMode = true;

		static void GetToolExePath(const J_ENGINE_TOOL_TYPE type,
			_Out_ std::string& folderPath,
			_Out_ std::string& name)noexcept
		{
			switch (type)
			{
			case JinEngine::Application::J_ENGINE_TOOL_TYPE::SOLUTION_MANAGER:
			{
				folderPath = JCUtil::WstrToU8Str(JApplicationEngine::DotNetBinaryPath());
				name = "JSolutionManager.exe";
				break;
			}
			default:
				break;
			}
		}
		static void CommandCall(const std::string& command)
		{   
			system(command.c_str());
		}
		static std::string CombineCommand(const std::string& drive,
			const std::string& exeFolder,
			const std::string& exeName,
			const std::string& args)
		{
			return drive + ": && cd " + exeFolder + " && " + exeName + " " + args;
		}
		static std::string DecomposeArgs(const std::vector<std::string>& args, const char argPerChar)
		{
			std::string res;
			const uint count = (uint)args.size();
			for (uint i = 0; i < count; ++i)
			{
				res += args[i];
				if (i > 0 && i < count - 1)
				{
					if (argPerChar != JEngineTool::invalidArgPerChar)
						res += argPerChar;
					res += " && ";
				}
			}
			return res;
		}
	}

	JEngineTool::SolFileInfo::SolFileInfo(const std::string& filePath, const std::string& includePath)
		:filePath(filePath), includePath(includePath)
	{}

	void JEngineTool::ExecuteTool(const J_ENGINE_TOOL_TYPE type, std::string args)
	{  
		std::string exeFoldePath;
		std::string exeName;
		GetToolExePath(type, exeFoldePath, exeName);
		 
		const std::string drive = JApplicationEngine::GetDriveAlpha();
		CommandCall(CombineCommand(drive, exeFoldePath, exeName, args));
	}
	void JEngineTool::ExecuteTool(const J_ENGINE_TOOL_TYPE type, const std::vector<std::string>& args, const char argPerChar)
	{
		ExecuteTool(type, DecomposeArgs(args, argPerChar));
	}
	void JEngineTool::CreateNewProject(const std::string& name,
		const std::string& path,
		const std::vector<std::string>& config)
	{
		std::string args = "-pd, " + name + ", ";
		args += path + ", ";
		args += "Release, ";
		args += Core::GetSolutionPlatform() + ", ";
		args += "false, ";
 
		for (const auto& data : config)
			args += data + ", ";
		ExecuteTool(J_ENGINE_TOOL_TYPE::SOLUTION_MANAGER, args);
	}
	void JEngineTool::CreateProjectVirtualDir(const std::string& slnPath, const std::string& projName, const std::string& dirPath)
	{
		std::string args = "-vd, " + dirPath + ", ";
		args += slnPath + ", ";
		args += projName;
		ExecuteTool(J_ENGINE_TOOL_TYPE::SOLUTION_MANAGER, args);
	}
	void JEngineTool::AddFile(const std::string& slnPath,
		const std::string& projName,
		const SolFileInfo& fileInfo,
		SolAddFileConfig config)
	{
		std::string commandType = "-f";
		if (config.allowBuild)
			commandType += 'b';

		std::string args = commandType + ", ";
		args += fileInfo.filePath + ", ";
		args += fileInfo.includePath + ", ";
		args += slnPath + ", ";
		args += projName + ", ";
		args += "Release, ";
		args += Core::GetSolutionPlatform();
		ExecuteTool(J_ENGINE_TOOL_TYPE::SOLUTION_MANAGER, args);
	}
	void JEngineTool::AddMultiFile(const std::string& slnPath,
		const std::string& projName,
		const std::vector<SolFileInfo>& fileInfo,
		SolAddFileConfig config)
	{
		std::string commandType = "-mf";
		if (config.allowBuild)
			commandType += 'b';

		std::string args = commandType + ", ";
		args += slnPath + ", ";
		args += projName + ", ";
		args += "Release, ";
		args += Core::GetSolutionPlatform() + ", ";
		for (const auto& data : fileInfo)
		{
			args += data.filePath + ", ";
			args += data.includePath + ", ";
		}
		ExecuteTool(J_ENGINE_TOOL_TYPE::SOLUTION_MANAGER, args);
	}
	void JEngineTool::AddDirectory(const std::string& slnPath, const std::string& projName, const std::string& dirPath, const std::string& virtualDirPath)
	{
		std::string args = "-d, " + dirPath + ", ";
		args += virtualDirPath + ", ";
		args += slnPath + ", ";
		args += projName;
		ExecuteTool(J_ENGINE_TOOL_TYPE::SOLUTION_MANAGER, args);
	}
	void JEngineTool::RemoveProjectItem(const std::string& includePath,
		const std::string& slnPath,
		const std::string& projName)
	{
		std::string commandType = "-r";

		std::string args = commandType + ", ";
		args += includePath + ", ";
		args += slnPath + ", "; 
		args += projName;  
		ExecuteTool(J_ENGINE_TOOL_TYPE::SOLUTION_MANAGER, args);	 
	}
	void JEngineTool::RemoveAllProjectItem(const std::string& slnPath, const std::string& projName)
	{
		std::string commandType = "-ar";

		std::string args = commandType + ", ";  
		args += slnPath + ", ";
		args += projName;
		ExecuteTool(J_ENGINE_TOOL_TYPE::SOLUTION_MANAGER, args);
	}
	void JEngineTool::SetProjectConfig(const std::string& slnPath,
		const std::string& projPath,
		const std::vector<std::string>& config)
	{
		std::string commandType = "-pc";

		std::string args = commandType + ", ";
		args += slnPath + ", ";
		args += projPath + ", ";
		for (const auto& data : config)
			args += data + ", ";
		ExecuteTool(J_ENGINE_TOOL_TYPE::SOLUTION_MANAGER, args);
	}
}