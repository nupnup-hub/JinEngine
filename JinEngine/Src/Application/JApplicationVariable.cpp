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
		namespace Private
		{
			//engine path
			static std::wstring engineExePath;
			static std::wstring enginePath;
			static std::wstring engineEngineResourcePath;
			static std::wstring engineDefaultResourcePath;
			static std::wstring engineInfoPath;

			static std::wstring engineProjectListFilePath;

			static std::wstring shaderPath;
			static std::vector<std::wstring> engineFolderPath;

			//Project path
			static std::wstring activatedProjectPath;
			static std::wstring activatedProjectName;

			static std::wstring contentPath;
			static std::wstring projectSettingPath;
			static std::wstring libraryPath;
			static std::wstring shaderMetafilePath;

			static std::wstring projectDefaultResourcePath;
			static std::wstring projectEditorResoucePath;
			static std::wstring projectResourceCashPath;
			static std::wstring projectVersionFilePath;

			static std::wstring sceneFolderPath;
			static std::wstring scriptFolderPath;
			static std::wstring resourceFolderPath;
			static std::vector<std::wstring> projectFolderPath;

			static J_APPLICATION_STATE applicationState;
			static Core::J_LANGUAGE_TYPE engineLanguage;

			static std::wstring engineProjectSymbol = L"JinEngineProject";

			static std::deque<std::unique_ptr<JApplicationVariable::AppCommandBind>> commandQueue;
			static std::unique_ptr<JApplicationVariable::StoreProjectF::Functor> storeProjectF;
			static std::unique_ptr<JApplicationVariable::LoadProjectF::Functor> loadProjectF;

			static std::vector<std::string> versionList
			{
				"1.00",
			};
			using TimeVec = std::vector<std::tuple<std::string, float>>;
			static TimeVec appTime;
		}

		//versionList
		//0 ~ n .. Latest version ~ old version 

		std::string JApplicationVariable::GetLatestVersion()noexcept
		{
			return Private::versionList[0];
		}
		int JApplicationVariable::GetSubverionDigitRange()noexcept
		{
			return 2;
		}
		std::vector<std::string> JApplicationVariable::GetAppVersion()noexcept
		{
			return Private::versionList;
		}
		std::wstring JApplicationVariable::GetEngineExePath()noexcept
		{
			return Private::engineExePath;
		}
		std::wstring JApplicationVariable::GetEnginePath()noexcept
		{
			return Private::enginePath;
		}
		std::wstring JApplicationVariable::GetEngineResourcePath()
		{
			return Private::engineEngineResourcePath;
		}
		std::wstring JApplicationVariable::GetEngineDefaultResourcePath()noexcept
		{
			return Private::engineDefaultResourcePath;
		}
		std::wstring JApplicationVariable::GetEngineInfoPath()noexcept
		{
			return Private::engineInfoPath;
		}
		std::wstring JApplicationVariable::GetEngineProjectListFilePath()noexcept
		{
			return Private::engineProjectListFilePath;
		}
		std::wstring JApplicationVariable::GetShaderPath()noexcept
		{
			return Private::shaderPath;
		}
		std::wstring JApplicationVariable::GetActivatedProjectPath()noexcept
		{
			return Private::activatedProjectPath;
		}
		std::wstring JApplicationVariable::GetActivatedProjectName()noexcept
		{
			return Private::activatedProjectName;
		}
		std::wstring JApplicationVariable::GetProjectPath()noexcept
		{
			return Private::activatedProjectPath;
		}
		std::wstring JApplicationVariable::GetProjectContentPath()noexcept
		{
			return Private::contentPath;
		}
		std::wstring JApplicationVariable::GetProjectSettingPath()noexcept
		{
			return Private::projectSettingPath;
		}
		std::wstring JApplicationVariable::GetProjectLibraryPath()noexcept
		{
			return Private::libraryPath;
		}
		std::wstring JApplicationVariable::GetProjectShaderMetafilePath()noexcept
		{
			return Private::shaderMetafilePath;
		}
		std::wstring JApplicationVariable::GetProjectDefaultResourcePath()noexcept
		{
			return Private::projectDefaultResourcePath;
		}
		std::wstring JApplicationVariable::GetProjectEditorResourcePath()noexcept
		{
			return Private::projectEditorResoucePath;
		}
		std::wstring JApplicationVariable::GetProjectResourceCashPath()noexcept
		{
			return Private::projectResourceCashPath;
		}
		std::wstring JApplicationVariable::GetProjectVersionFilePath()noexcept
		{
			return Private::projectVersionFilePath;
		}
		std::wstring JApplicationVariable::GetProjectContentScenePath()noexcept
		{
			return Private::sceneFolderPath;
		}
		std::wstring JApplicationVariable::GetProjectContentScriptPath()noexcept
		{
			return Private::scriptFolderPath;
		}
		std::wstring JApplicationVariable::GetProjectContentResourcePath()noexcept
		{
			return Private::resourceFolderPath;
		}
		J_APPLICATION_STATE JApplicationVariable::GetApplicationState()noexcept
		{
			return Private::applicationState;
		}
		Core::J_LANGUAGE_TYPE JApplicationVariable::GetEngineLanguageType()noexcept
		{
			return Private::engineLanguage;
		}
		bool JApplicationVariable::IsDefaultFolder(const std::wstring& path)noexcept
		{
			uint folderCount = (uint)Private::projectFolderPath.size();
			for (uint i = 0; i < folderCount; ++i)
			{
				if (path == Private::projectFolderPath[i])
					return true;
			}

			folderCount = (uint)Private::engineFolderPath.size();
			for (uint i = 0; i < folderCount; ++i)
			{
				if (path == Private::engineFolderPath[i])
					return true;
			}

			return false;
		}
		void JApplicationVariable::AddTime(const std::string& str, float time)noexcept
		{
			Private::appTime.push_back(std::tuple(str, time));
		}
		void JApplicationVariable::UpdateTime(int index, float time)noexcept
		{
			Private::appTime[index] = std::tuple(std::get<0>(Private::appTime[index]), time);
		}
		void JApplicationVariable::ClearTime()noexcept
		{
			Private::appTime.clear();
		}
		JApplicationVariable::TimeVec JApplicationVariable::GetTime()noexcept
		{
			return Private::appTime;
		}
		void JApplicationVariable::SetApplicationState(const J_APPLICATION_STATE newState)noexcept
		{
			Private::applicationState = newState;
		}
		void JApplicationVariable::Initialize()
		{
			TCHAR programpath[_MAX_PATH];
			GetModuleFileName(NULL, programpath, _MAX_PATH);
			Private::engineExePath = programpath;
			size_t index = Private::engineExePath.find_last_of(L"\\");
			Private::enginePath = Private::engineExePath.substr(0, index);

			index = Private::enginePath.find_last_of(L"\\");
			Private::enginePath.erase(Private::enginePath.begin() + index, Private::enginePath.end());
			index = Private::enginePath.find_last_of(L"\\");
			Private::enginePath.erase(Private::enginePath.begin() + index, Private::enginePath.end());
			 
			Private::enginePath = Private::enginePath + L"\\JinEngine";
			Private::engineEngineResourcePath = Private::enginePath + L"\\EngineResource";
			Private::engineDefaultResourcePath = Private::engineEngineResourcePath + L"\\DefaultResource";
			Private::engineInfoPath = Private::enginePath + L"\\EngineInfo";
			Private::engineProjectListFilePath = Private::engineInfoPath + L"\\ProejctList.txt";
			Private::shaderPath = Private::engineEngineResourcePath + L"\\Shader";

			Private::engineFolderPath =
			{
				Private::enginePath,
				Private::engineEngineResourcePath,
				Private::engineDefaultResourcePath,
				Private::engineInfoPath,
				Private::shaderPath,
			};

			setlocale(LC_ALL, "");
			std::string localeStr = setlocale(LC_ALL, NULL);
			if (localeStr == "Korean_Korea.949")
				Private::engineLanguage = Core::J_LANGUAGE_TYPE::KOREAN;
			else
				Private::engineLanguage = Core::J_LANGUAGE_TYPE::ENGLISH;

			MakeEngineFolder();
		}
		bool JApplicationVariable::MakeEngineFolder()
		{
			uint folderCount = (uint)Private::engineFolderPath.size();
			for (uint i = 0; i < folderCount; ++i)
			{
				if (_waccess(Private::engineFolderPath[i].c_str(), 00) == -1)
				{
					if (_wmkdir(Private::engineFolderPath[i].c_str()) == -1)
						return false;
				}
			}
			return true;
		}
		void JApplicationVariable::RegisterFunctor(JApplication* app,
			JApplicationVariable::StoreProjectF::Ptr storeF,
			JApplicationVariable::LoadProjectF::Ptr loadF)
		{
			Private::storeProjectF = std::make_unique<StoreProjectF::Functor>(storeF, app);
			Private::loadProjectF = std::make_unique<LoadProjectF::Functor>(loadF, app);
		}
		void JApplicationVariable::ExecuteAppCommand()
		{
			const uint commandCount = (uint)Private::commandQueue.size();
			for (uint i = 0; i < commandCount; ++i)
				Private::commandQueue[i]->InvokeCompletelyBind();
			Private::commandQueue.clear();
		}

		namespace Private
		{
			static std::vector<std::unique_ptr<JApplicationProject::JProjectInfo>> projectList;
			static std::unique_ptr<JApplicationProject::JProjectInfo> nextProjectInfo;
			static bool startProjectOnce = false;
			static bool endProject = false;
			static bool loadOtherProjectOnce = false;
		}

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
			return (uint)Private::projectList.size();
		}
		JApplicationProject::JProjectInfo* JApplicationProject::GetProjectInfo(uint index)noexcept
		{
			return Private::projectList[index].get();
		}
		void JApplicationProject::SetNextProjectInfo(std::unique_ptr<JProjectInfo>&& nextProjectInfo)noexcept
		{
			Private::nextProjectInfo = std::move(nextProjectInfo);
		}
		void JApplicationProject::TryLoadOtherProject()noexcept
		{
			Private::loadOtherProjectOnce = true;
		}
		void JApplicationProject::TryCloseProject()noexcept
		{
		}
		void JApplicationProject::CancelCloseProject()noexcept
		{
			if (Private::loadOtherProjectOnce)
				Private::nextProjectInfo.reset();
			Private::loadOtherProjectOnce = false;
		}
		void JApplicationProject::ConfirmCloseProject()noexcept
		{
			Private::endProject = true;
			if (Private::loadOtherProjectOnce)
			{
				Private::loadOtherProjectOnce = false;
				StartNewProject();
			}
		}
		std::unique_ptr<JApplicationProject::JProjectInfo> JApplicationProject::MakeProjectInfo(const std::wstring& projectPath)
		{
			bool isValid = false;
			const uint projectCount = (uint)Private::projectList.size();
			for (uint i = 0; i < projectCount; ++i)
			{
				if (Private::projectList[i]->GetPath() == projectPath)
				{
					return std::make_unique<JProjectInfo>(Private::projectList[i]->GetName(), \
						Private::projectList[i]->GetPath(),
						Private::projectList[i]->GetVersion());
				}
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
		void JApplicationProject::MakeProjectFolderPath(const std::wstring& projectName, const std::wstring& projectPath)
		{
			Private::activatedProjectName = JCUtil::EraseSideWChar(projectName, L' ');
			Private::activatedProjectPath = JCUtil::EraseSideWChar(projectPath, L' ');

			Private::contentPath = Private::activatedProjectPath + L"\\" + L"Content";
			Private::projectSettingPath = Private::activatedProjectPath + L"\\" + L"ProjectSetting";
			Private::libraryPath = Private::activatedProjectPath + L"\\" + L"Library";
			Private::shaderMetafilePath = Private::libraryPath + L"\\" + L"ShaderMetafile";
			Private::projectDefaultResourcePath = Private::libraryPath + L"\\" + L"DefaultResource";
			Private::projectEditorResoucePath = Private::projectSettingPath + L"\\" + L"Editor";
			Private::projectResourceCashPath = Private::projectSettingPath + L"\\" + L"Cash";
			Private::projectVersionFilePath = Private::projectSettingPath + L"\\" + L"ProjectVersion.txt";

			Private::sceneFolderPath = Private::contentPath + L"\\" + L"Scene";
			Private::scriptFolderPath = Private::contentPath + L"\\" + L"Script";
			Private::resourceFolderPath = Private::contentPath + L"\\" + L"Resource";

			Private::projectFolderPath.clear();
			Private::projectFolderPath =
			{
				Private::activatedProjectPath,
				Private::contentPath,
				Private::projectSettingPath,
				Private::libraryPath,
				Private::shaderMetafilePath,
				Private::projectDefaultResourcePath,
				Private::projectEditorResoucePath,
				Private::projectResourceCashPath,
				Private::sceneFolderPath,
				Private::scriptFolderPath,
				Private::resourceFolderPath,
			};
		}
		bool JApplicationProject::MakeProjectFolder()
		{
			uint folderCount = (uint)Private::projectFolderPath.size();
			for (uint i = 0; i < folderCount; ++i)
			{
				if (_waccess(Private::projectFolderPath[i].c_str(), 00) == -1)
				{
					if (_wmkdir(Private::projectFolderPath[i].c_str()) == -1)
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
			if (Private::nextProjectInfo == nullptr)
				return false;
			Private::startProjectOnce = true;
			return true;
		}
		bool JApplicationProject::Initialize()
		{
			const uint projectListCount = (uint)Private::projectList.size();
			bool hasProject = false;
			for (uint i = 0; i < projectListCount; ++i)
			{
				if (Private::projectList[i]->GetPath() == Private::nextProjectInfo->GetPath())
				{
					hasProject = true;
					break;
				}
			}
			if (!hasProject)
			{
				Private::projectList.emplace_back(std::make_unique<JProjectInfo>(Private::nextProjectInfo->GetName(),
					Private::nextProjectInfo->GetPath(),
					Private::nextProjectInfo->GetVersion()));
				StoreProjectList();
			}
			//dependency order
			MakeProjectFolderPath(Private::nextProjectInfo->GetName(), Private::nextProjectInfo->GetPath());
			bool res00 = MakeProjectFolder();
			bool res01 = MakeProjectVersionFile(JCUtil::WstrToU8Str(Private::nextProjectInfo->GetVersion()));
			Private::nextProjectInfo.reset();
			Private::startProjectOnce = false;
			Private::endProject = false;
			if (res00 && res01)
				JApplicationVariable::SetApplicationState(J_APPLICATION_STATE::EDIT_GAME);
			return res00 && res01;
		}
		bool JApplicationProject::IsValidVersion(const std::string& pVersion)
		{
			bool isValidVersion = false;
			const uint versionCount = (uint)Private::versionList.size();
			for (uint i = 0; i < versionCount; ++i)
			{
				if (Private::versionList[i] == pVersion)
				{
					isValidVersion = true;
					break;
				}
			}
			return isValidVersion;
		}
		bool JApplicationProject::CanStartProject()noexcept
		{
			return Private::startProjectOnce;
		}
		bool JApplicationProject::CanEndProject()noexcept
		{
			return Private::endProject;
		}
		Core::J_FILE_IO_RESULT JApplicationProject::StoreProjectVersion(const std::string& pVersion)
		{
			std::wofstream stream;
			stream.open(JApplicationVariable::GetProjectVersionFilePath().c_str(), std::ios::out | std::ios::binary);
			if (stream.is_open())
			{
				JFileIOHelper::StoreJString(stream, L"Symbol:", Private::engineProjectSymbol);
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
			if (stream.is_open() && JFileIOHelper::SkipLine(stream, Private::engineProjectSymbol))
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
		void JApplicationProject::StoreProject()
		{
			(*Private::storeProjectF)();
		}
		void JApplicationProject::LoadProject()
		{
			(*Private::loadProjectF)();
		}
		void JApplicationProject::StoreProjectList()
		{
			//std::locale::global(std::locale("Korean"));
			std::wstring dirPath = JApplicationVariable::GetEngineProjectListFilePath();

			std::wofstream stream;
			stream.open(dirPath, std::ios::out | std::ios::binary);

			JFileIOHelper::StoreAtomicData(stream, L"ProjectCount:", (uint)Private::projectList.size());
			for (const auto& data : Private::projectList)
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
						Private::projectList.emplace_back(std::make_unique<JProjectInfo>(name, path, version));
				}
			}
		}
	}
}
