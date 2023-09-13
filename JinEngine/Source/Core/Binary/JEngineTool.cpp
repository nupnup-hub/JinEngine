#include"JEngineTool.h"
#include"../JCoreEssential.h"
#include"../Platform/JPlatformInfo.h"
#include"../Utility/JCommonUtility.h"
#include"../../Application/JApplicationEngine.h"
#include"../../Application/JApplicationProject.h"

namespace JinEngine::Core
{
	namespace
	{
		static void GetToolExePath(const J_ENGINE_TOOL_TYPE type,
			_Out_ std::string& folderPath,
			_Out_ std::string& name)noexcept
		{
			switch (type)
			{
			case JinEngine::Core::J_ENGINE_TOOL_TYPE::SOLUTION_MANAGER:
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

	void JEngineTool::ExecuteTool(const J_ENGINE_TOOL_TYPE type, const std::string& args)
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
		args += GetSolutionPlatform() + ", ";
		args += "false, ";
 
		for (const auto& data : config)
			args += data + ", ";
		ExecuteTool(J_ENGINE_TOOL_TYPE::SOLUTION_MANAGER, args);
	}
	void JEngineTool::CreateProjectVirtualDir(const std::string& slnPath, const std::string& projName, const std::string& dirPath)
	{
		std::string args = "-d, " + dirPath + ", ";
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
		args += GetSolutionPlatform();
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
		args += GetSolutionPlatform() + ", ";
		for (const auto& data : fileInfo)
		{
			args += data.filePath + ", ";
			args += data.includePath + ", ";
		}
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