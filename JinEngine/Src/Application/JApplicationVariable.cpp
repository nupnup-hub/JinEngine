#include"JApplicationVariable.h" 
#include"JApplication.h"
#include"../Core/JDataType.h"
#include"../Core/File/JFileIOHelper.h"
#include"../Utility/JCommonUtility.h"
#include<Windows.h> 
#include <direct.h>	 
#include<fstream>  
#include<io.h>

namespace JinEngine
{
	namespace Application
	{ 
		std::wstring JApplicationVariable::engineExePath;
		std::wstring JApplicationVariable::enginePath; 
		std::wstring JApplicationVariable::engineEngineResourcePath;
		std::wstring JApplicationVariable::engineDefaultResourcePath;
		std::wstring JApplicationVariable::engineInfoPath;
		std::wstring JApplicationVariable::engineProjectListFilePath;

		std::wstring JApplicationVariable::shaderPath;
		std::vector<std::wstring> JApplicationVariable::engineFolderPath;

		std::wstring JApplicationVariable::activatedProjectPath;
		std::wstring JApplicationVariable::activatedProjectName;

		std::wstring JApplicationVariable::contentPath;
		std::wstring JApplicationVariable::projectSettingPath;
		std::wstring JApplicationVariable::libraryPath;
		std::wstring JApplicationVariable::shaderMetafilePath;
		std::wstring JApplicationVariable::projectDefaultResourcePath;
		std::wstring JApplicationVariable::projectEditorResoucePath;
		std::wstring JApplicationVariable::projectVersionFilePath;

		std::wstring JApplicationVariable::sceneFolderPath;
		std::wstring JApplicationVariable::scriptFolderPath;
		std::wstring JApplicationVariable::resourceFolderPath;
		std::vector<std::wstring> JApplicationVariable::projectFolderPath;

		J_APPLICATION_STATE JApplicationVariable::applicationState = J_APPLICATION_STATE::PROJECT_SELECT;
		Core::J_LANGUAGE_TYPE JApplicationVariable::engineLanguage;

		std::wstring JApplicationVariable::engineProjectSymbol = L"JinEngineProject";
		std::deque<std::unique_ptr<JApplicationVariable::AppCommandBind>>  JApplicationVariable::commandQueue;
		std::unique_ptr<JApplicationVariable::StoreProjectF::Functor> JApplicationVariable::storeProjectF;
		std::unique_ptr<JApplicationVariable::LoadProjectF::Functor>JApplicationVariable::loadProjectF;
		//versionList
		//0 ~ n .. Latest version ~ old version
		std::vector<std::string> JApplicationVariable::versionList
		{
			"1.00",
		};
		std::vector<std::tuple<std::string, float>> JApplicationVariable::appTime;

		std::string JApplicationVariable::GetLatestVersion()noexcept
		{
			return versionList[0];
		}
		int JApplicationVariable::GetSubverionDigitRange()noexcept
		{
			return 2;
		}
		std::vector<std::string> JApplicationVariable::GetAppVersion()noexcept
		{
			return versionList;
		}
		std::wstring JApplicationVariable::GetEngineExePath()noexcept
		{
			return engineExePath;
		}
		std::wstring JApplicationVariable::GetEnginePath()noexcept
		{
			return enginePath;
		}
		std::wstring JApplicationVariable::GetEngineResourcePath()
		{
			return engineEngineResourcePath;
		}
		std::wstring JApplicationVariable::GetEngineDefaultResourcePath()noexcept
		{
			return engineDefaultResourcePath;
		}
		std::wstring JApplicationVariable::GetEngineInfoPath()noexcept
		{
			return engineInfoPath;
		}
		std::wstring JApplicationVariable::GetEngineProjectListFilePath()noexcept
		{
			return engineProjectListFilePath;
		}
		std::wstring JApplicationVariable::GetShaderPath()noexcept
		{
			return shaderPath;
		}
		std::wstring JApplicationVariable::GetActivatedProjectPath()noexcept
		{
			return activatedProjectPath;
		}
		std::wstring JApplicationVariable::GetActivatedProjectName()noexcept
		{
			return activatedProjectName;
		}
		std::wstring JApplicationVariable::GetProjectPath()noexcept
		{
			return activatedProjectPath;
		}
		std::wstring JApplicationVariable::GetProjectContentPath()noexcept
		{
			return contentPath;
		}
		std::wstring JApplicationVariable::GetProjectSettingPath()noexcept
		{
			return projectSettingPath;
		}
		std::wstring JApplicationVariable::GetProjectLibraryPath()noexcept
		{
			return libraryPath;
		}
		std::wstring JApplicationVariable::GetProjectShaderMetafilePath()noexcept
		{
			return shaderMetafilePath;
		}
		std::wstring JApplicationVariable::GetProjectDefaultResourcePath()noexcept
		{
			return projectDefaultResourcePath;
		}
		std::wstring JApplicationVariable::GetProjectEditorResourcePath()noexcept
		{
			return projectEditorResoucePath;
		}
		std::wstring JApplicationVariable::GetProjectVersionFilePath()noexcept
		{
			return projectVersionFilePath;
		}
		std::wstring JApplicationVariable::GetProjectContentScenePath()noexcept
		{
			return sceneFolderPath;
		}
		std::wstring JApplicationVariable::GetProjectContentScriptPath()noexcept
		{
			return scriptFolderPath;
		}
		std::wstring JApplicationVariable::GetProjectContentResourcePath()noexcept
		{
			return resourceFolderPath;
		}
		J_APPLICATION_STATE JApplicationVariable::GetApplicationState()noexcept
		{
			return applicationState;
		}
		Core::J_LANGUAGE_TYPE JApplicationVariable::GetEngineLanguageType()noexcept
		{
			return engineLanguage;
		}
		bool JApplicationVariable::IsDefaultFolder(const std::wstring& path)noexcept
		{
			uint folderCount = (uint)projectFolderPath.size();
			for (uint i = 0; i < folderCount; ++i)
			{
				if (path == projectFolderPath[i])
					return true;
			}

			folderCount = (uint)engineFolderPath.size();
			for (uint i = 0; i < folderCount; ++i)
			{
				if (path == engineFolderPath[i])
					return true;
			}

			return false;
		}
		void JApplicationVariable::AddTime(const std::string& str, float time)noexcept
		{
			appTime.push_back(std::tuple(str, time));
		}
		void JApplicationVariable::UpdateTime(int index, float time)noexcept
		{
			appTime[index] = std::tuple(std::get<0>(appTime[index]), time);
		}
		void JApplicationVariable::ClearTime()noexcept
		{
			appTime.clear();
		}
		JApplicationVariable::TimeVec JApplicationVariable::GetTime()noexcept
		{
			return appTime;
		}
		void JApplicationVariable::SetApplicationState(const J_APPLICATION_STATE newState)noexcept
		{
			applicationState = newState;
		}
		void JApplicationVariable::Initialize()
		{
			TCHAR programpath[_MAX_PATH];
			GetModuleFileName(NULL, programpath, _MAX_PATH);
			engineExePath = programpath;
			size_t index = engineExePath.find_last_of(L"\\");
			enginePath = engineExePath.substr(0, index);

			index = enginePath.find_last_of(L"\\");
			enginePath.erase(enginePath.begin() + index, enginePath.end());
			index = enginePath.find_last_of(L"\\");
			enginePath.erase(enginePath.begin() + index, enginePath.end());
			 
			enginePath = enginePath + L"\\JinEngine";
			engineEngineResourcePath = enginePath + L"\\EngineResource";
			engineDefaultResourcePath = engineEngineResourcePath + L"\\BasicResource";
			engineInfoPath = enginePath + L"\\EngineInfo";
			engineProjectListFilePath = engineInfoPath + L"\\ProejctList.txt";
			shaderPath = engineEngineResourcePath + L"\\Shader";

			engineFolderPath =
			{
				enginePath,
				engineEngineResourcePath,
				engineDefaultResourcePath,
				engineInfoPath,
				shaderPath,
			};

			setlocale(LC_ALL, "");
			std::string localeStr = setlocale(LC_ALL, NULL);
			if (localeStr == "Korean_Korea.949")
				engineLanguage = Core::J_LANGUAGE_TYPE::KOREAN;
			else
				engineLanguage = Core::J_LANGUAGE_TYPE::ENGLISH;

			MakeEngineFolder();
		}
		bool JApplicationVariable::MakeEngineFolder()
		{
			uint folderCount = (uint)engineFolderPath.size();
			for (uint i = 0; i < folderCount; ++i)
			{
				if (_waccess(engineFolderPath[i].c_str(), 00) == -1)
				{
					if (_wmkdir(engineFolderPath[i].c_str()) == -1)
						return false;
				}
			}
			return true;
		}
		void JApplicationVariable::RegisterFunctor(JApplication* app,
			JApplicationVariable::StoreProjectF::Ptr storeF,
			JApplicationVariable::LoadProjectF::Ptr loadF)
		{
			storeProjectF = std::make_unique<StoreProjectF::Functor>(storeF, app);
			loadProjectF = std::make_unique<LoadProjectF::Functor>(loadF, app);
		}
		void JApplicationVariable::ExecuteAppCommand()
		{
			const uint commandCount = (uint)commandQueue.size();
			for (uint i = 0; i < commandCount; ++i)
				commandQueue[i]->InvokeCompletelyBind();
			commandQueue.clear();
		}

		std::vector<std::unique_ptr<JApplicationProject::JProjectInfo>> JApplicationProject::projectList;
		std::unique_ptr<JApplicationProject::JProjectInfo> JApplicationProject::nextProjectInfo;
		bool JApplicationProject::startProjectOnce = false;
		JApplicationProject::JProjectInfo::JProjectInfo(const std::wstring& name,
			const std::wstring& path,
			std::wstring version)
			:name(name), path(path), version(version)
		{}
		std::wstring JApplicationProject::JProjectInfo::GetName()const noexcept
		{
			return name;
		}
		std::wstring JApplicationProject::JProjectInfo::GetPath()const noexcept
		{
			return path;
		}
		std::wstring JApplicationProject::JProjectInfo::GetVersion()const noexcept
		{
			return version;
		}
		uint JApplicationProject::GetProjectInfoCount()noexcept
		{
			return (uint)projectList.size();
		}
		JApplicationProject::JProjectInfo* JApplicationProject::GetProjectInfo(uint index)noexcept
		{
			return projectList[index].get();
		}
		std::unique_ptr<JApplicationProject::JProjectInfo> JApplicationProject::MakeProjectInfo(const std::wstring& projectPath)
		{
			bool isValid = false;
			const uint projectCount = (uint)projectList.size();
			for (uint i = 0; i < projectCount; ++i)
			{
				if (projectList[i]->GetPath() == projectPath)
					return std::make_unique<JProjectInfo>(projectList[i]->GetName(), projectList[i]->GetPath(), projectList[i]->GetVersion());
			}

			std::wstring projectVersionFilePath = projectPath + L"\\" + L"ProjectSetting" + L"\\" + L"ProjectVersion.txt";
			if (_waccess(projectVersionFilePath.c_str(), 00) != -1)
			{
				std::wstring name;
				std::wstring folderPath;
				JCUtil::DecomposeFolderPath(projectPath, folderPath, name);
				std::string version;
				Core::J_FILE_IO_RESULT res = LoadProejctVersion(version);
				if (res != Core::J_FILE_IO_RESULT::SUCCESS)
					return std::make_unique<JProjectInfo>(name, projectPath, JCUtil::U8StrToWstr(version));
			}
			return nullptr;
		}
		void JApplicationProject::SetNextProjectInfo(std::unique_ptr<JProjectInfo>&& nextProjectInfo)
		{
			JApplicationProject::nextProjectInfo = std::move(nextProjectInfo);
		}
		void JApplicationProject::MakeProjectFolderPath(const std::wstring& projectName, const std::wstring& projectPath)
		{
			JApplicationVariable::activatedProjectName = JCUtil::EraseSideWChar(projectName, L' ');
			JApplicationVariable::activatedProjectPath = JCUtil::EraseSideWChar(projectPath, L' ');

			JApplicationVariable::contentPath = JApplicationVariable::activatedProjectPath + L"\\" + L"Content";
			JApplicationVariable::projectSettingPath = JApplicationVariable::activatedProjectPath + L"\\" + L"ProjectSetting";
			JApplicationVariable::libraryPath = JApplicationVariable::activatedProjectPath + L"\\" + L"Library";
			JApplicationVariable::shaderMetafilePath = JApplicationVariable::libraryPath + L"\\" + L"ShaderMetafile";
			JApplicationVariable::projectDefaultResourcePath = JApplicationVariable::libraryPath + L"\\" + L"DefaultResource";
			JApplicationVariable::projectEditorResoucePath = JApplicationVariable::projectSettingPath + L"\\" + L"Editor";
			JApplicationVariable::projectVersionFilePath = JApplicationVariable::projectSettingPath + L"\\" + L"ProjectVersion.txt";

			JApplicationVariable::sceneFolderPath = JApplicationVariable::contentPath + L"\\" + L"Scene";
			JApplicationVariable::scriptFolderPath = JApplicationVariable::contentPath + L"\\" + L"Script";
			JApplicationVariable::resourceFolderPath = JApplicationVariable::contentPath + L"\\" + L"Resource";

			JApplicationVariable::projectFolderPath.clear();
			JApplicationVariable::projectFolderPath =
			{
				JApplicationVariable::activatedProjectPath,
				JApplicationVariable::contentPath,
				JApplicationVariable::projectSettingPath,
				JApplicationVariable::libraryPath,
				JApplicationVariable::shaderMetafilePath,
				JApplicationVariable::projectDefaultResourcePath,
				JApplicationVariable::projectEditorResoucePath,
				JApplicationVariable::sceneFolderPath,
				JApplicationVariable::scriptFolderPath,
				JApplicationVariable::resourceFolderPath,
			};
		}
		bool JApplicationProject::MakeProjectFolder()
		{
			uint folderCount = (uint)JApplicationVariable::projectFolderPath.size();
			for (uint i = 0; i < folderCount; ++i)
			{
				if (_waccess(JApplicationVariable::projectFolderPath[i].c_str(), 00) == -1)
				{
					if (_wmkdir(JApplicationVariable::projectFolderPath[i].c_str()) == -1)
						return false;
				}
			}
			return true;
		}
		bool JApplicationProject::MakeProjectVersionFile(const std::string& pVersion)
		{
			if (!IsValidVersion(pVersion))
				return false;
			else
			{
				if (_waccess(JApplicationVariable::GetProjectVersionFilePath().c_str(), 00) == -1)
					return StoreProjectVersion(pVersion) == Core::J_FILE_IO_RESULT::SUCCESS;
				else
					return true;
			}
		}
		bool JApplicationProject::StartNewProject()
		{
			if (nextProjectInfo == nullptr)
				return false;
			SetStartProjectOnce();
			return true;
		}
		bool JApplicationProject::Initialize()
		{
			const uint projectListCount = (uint)projectList.size();
			bool hasProject = false;
			for (uint i = 0; i < projectListCount; ++i)
			{
				if (projectList[i]->GetPath() == nextProjectInfo->GetPath())
				{
					hasProject = true;
					break;
				}
			}
			if (!hasProject)
			{
				projectList.emplace_back(std::make_unique<JProjectInfo>(nextProjectInfo->GetName(),
					nextProjectInfo->GetPath(),
					nextProjectInfo->GetVersion()));
				StoreProjectList();
			}
			//dependency order
			MakeProjectFolderPath(nextProjectInfo->GetName(), nextProjectInfo->GetPath());
			bool res00 = MakeProjectFolder();
			bool res01 = MakeProjectVersionFile(JCUtil::WstrToU8Str(nextProjectInfo->GetVersion()));
			nextProjectInfo.reset();
			startProjectOnce = false;
			if (res00 && res01)
				JApplicationVariable::SetApplicationState(J_APPLICATION_STATE::EDIT_GAME);
			return res00 && res01;
		}
		void JApplicationProject::SetStartProjectOnce()
		{
			startProjectOnce = true;
		}
		bool JApplicationProject::IsValidVersion(const std::string& pVersion)
		{
			bool isValidVersion = false;
			const uint versionCount = (uint)JApplicationVariable::versionList.size();
			for (uint i = 0; i < versionCount; ++i)
			{
				if (JApplicationVariable::versionList[i] == pVersion)
				{
					isValidVersion = true;
					break;
				}
			}
			return isValidVersion;
		}
		bool JApplicationProject::CanStartProject()noexcept
		{
			return startProjectOnce;
		}
		Core::J_FILE_IO_RESULT JApplicationProject::StoreProjectVersion(const std::string& pVersion)
		{
			std::wofstream stream;
			stream.open(JApplicationVariable::GetProjectVersionFilePath().c_str(), std::ios::out | std::ios::binary);
			if (stream.is_open())
			{
				JFileIOHelper::StoreJString(stream, L"Symbol:", JApplicationVariable::engineProjectSymbol);
				JFileIOHelper::StoreJString(stream, L"Version:", JCUtil::StrToWstr(pVersion));
				stream.clear();
				return Core::J_FILE_IO_RESULT::SUCCESS;
			}
			else
				return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
		}
		Core::J_FILE_IO_RESULT JApplicationProject::LoadProejctVersion(_Out_ std::string& pVersion)
		{
			std::wifstream stream;
			stream.open(JApplicationVariable::GetProjectVersionFilePath().c_str(), std::ios::in | std::ios::binary);
			if (stream.is_open() && JFileIOHelper::SkipLine(stream, JApplicationVariable::engineProjectSymbol))
			{
				std::wstring version;
				JFileIOHelper::LoadJString(stream, version);
				stream.clear();
				pVersion = JCUtil::WstrToU8Str(version);
				return Core::J_FILE_IO_RESULT::SUCCESS;
			}
			else
				return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
		}
		void JApplicationProject::RequestStoreProject()
		{
			using StoreBind = JApplicationVariable::StoreProjectF::CompletelyBind;
			JApplicationVariable::commandQueue.push_back(std::make_unique<StoreBind>(*JApplicationVariable::storeProjectF));
		}
		void JApplicationProject::RequestLoadProject()
		{
			using LoadBind = JApplicationVariable::LoadProjectF::CompletelyBind;
			JApplicationVariable::commandQueue.push_back(std::make_unique<LoadBind>(*JApplicationVariable::loadProjectF));
		}
		void JApplicationProject::StoreProjectList()
		{
			//std::locale::global(std::locale("Korean"));
			std::wstring dirPath = JApplicationVariable::GetEngineProjectListFilePath();

			std::wofstream stream;
			stream.open(dirPath, std::ios::out | std::ios::binary);

			JFileIOHelper::StoreAtomicData(stream, L"ProjectCount:", (uint)projectList.size());
			for (const auto& data : projectList)
			{
				JFileIOHelper::StoreJString(stream, L"Name:", data->GetName());
				JFileIOHelper::StoreJString(stream, L"Path:", data->GetPath());
				JFileIOHelper::StoreJString(stream, L"Version:", data->GetVersion());
			}
			stream.close();
		}
		void JApplicationProject::LoadProjectList()
		{
			//std::locale::global(std::locale("Korean"));
			std::wstring dirPath = JApplicationVariable::GetEngineProjectListFilePath();
			std::wifstream stream;
			stream.open(dirPath, std::ios::in | std::ios::binary);

			uint projectCount;
			std::wstring name;
			std::wstring path;
			std::wstring version;

			JFileIOHelper::LoadAtomicData(stream, projectCount);
			if (stream.is_open())
			{
				// stream.eof == 파일끝을지나 읽기시도시
				//https://stackoverflow.com/questions/4533063/how-does-ifstreams-eof-work

				for (uint i = 0; i < projectCount; ++i)
				{
					JFileIOHelper::LoadJString(stream, name);
					JFileIOHelper::LoadJString(stream, path);
					JFileIOHelper::LoadJString(stream, version);
					if (_waccess(path.c_str(), 00) == 0)
						projectList.emplace_back(std::make_unique<JProjectInfo>(name, path, version));
				}
			}
		}
	}
}
