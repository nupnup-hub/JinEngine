#include"JApplicationProject.h" 
#include"JApplicationProjectPrivate.h"  
#include"JApplicationEngine.h"
#include"JApplicationEnginePrivate.h"
#include"../Core/JCoreEssential.h"
#include"../Core/File/JFileIOHelper.h"
#include"../Core/Guid/JGuidCreator.h"
#include"../Core/Module/JModuleManager.h"
#include"../Core/Plugin/JPluginManager.h"
#include"../Core/Script/JCplusScriptCreator.h"
#include"../Core/Binary/JEngineTool.h"
#include"../Core/Platform/JPlatformInfo.h"
#include"../Core/Utility/JCommonUtility.h"
#include<Windows.h> 
#include <direct.h>	 
#include<fstream>  
#include<io.h>

#define ENGINE_PROJECT_SYMBOL L"JinEngineProject"

namespace JinEngine
{
	namespace Application
	{
		namespace
		{
			using MainAccess = JApplicationProjectPrivate::MainAccess;
			using LifeInterface = JApplicationProjectPrivate::LifeInterface;
			using IOInterface = JApplicationProjectPrivate::IOInterface;
		}
		namespace
		{
			static const std::vector<std::string> versionList
			{
				"1.00",
			};

			static std::string GetSrcFolderName()noexcept
			{
				return "Source";
			}
			static std::wstring GetSrcFolderWName()noexcept
			{
				return L"Source";
			}
			static std::wstring GetOutDirName()
			{
				return Core::GetSolutionPlatform() == "x64" ? L"Win64" : L"Win32";
			}
			static std::wstring ProjectDllPath()
			{
				return JApplicationProject::ProjectBinaryPath() + L"\\" + JApplicationProject::Name() + L".dll";
			}
			static void CreateProjectSolution()
			{
				//수정필요!
				//build system 개발 혹은 cmake등을 사용해서 빌드할 필요가있다.

				const std::string name = JCUtil::WstrToU8Str(JApplicationProject::Name());
				const std::string path = JCUtil::WstrToU8Str(JApplicationProject::SolutionPath());
				const std::string engineSrcPath = JCUtil::WstrToU8Str(JApplicationEngine::SouceCodePath());
				const std::string srcPath = JCUtil::WstrToU8Str(JApplicationProject::SouceCodePath());
				const std::string engineExeFolderPath = JCUtil::WstrToU8Str(JCUtil::GetFolderPath(JApplicationEngine::ExePath()));
				const std::string binaryPath = JCUtil::WstrToU8Str(JApplicationProject::ProjectBinaryPath());
				const std::string objFileFolderPath = JCUtil::WstrToU8Str(JApplicationEngine::SolutionPath()) + "\\JinEngine\\" + Core::GetSolutionPlatform() + "\\Release";
				const std::string thirdPartyPath = JCUtil::WstrToU8Str(JApplicationEngine::ThirdPartyPath());
				 
				std::vector<std::string> fconfig;
				fconfig.push_back("Property, OutDir, $(SolutionDir)..\\Binary\\" + JCUtil::WstrToU8Str(GetOutDirName()) + "\\");
				fconfig.push_back("Meta, AdditionalIncludeDirectories, " + engineSrcPath + ";$(SolutionDir)..\\Source;");
				fconfig.push_back("Meta, AdditionalLibraryDirectories, " + engineExeFolderPath + ";" +
					objFileFolderPath + ";" + 
					thirdPartyPath +"\\Cuda\\lib\\x64;" +
					thirdPartyPath + "\\fast-lzma2-master\\lib;"  + 
					thirdPartyPath + "\\Fbx\\lib\\vs2017\\x64\\release;" + 
					thirdPartyPath + "\\jsoncpp-master;");
				fconfig.push_back("Meta, AdditionalDependencies, JinEngine_static.lib;"
					"cudart_static.lib;" 
					"fast-lzma2.lib;"
					"libfbxsdk-md.lib;"
					"libxml2-md.lib;"
					"zlib-md.lib;"
					"jsoncpp_static.lib;");

				std::vector<std::string> sconfig;
				sconfig.push_back("Meta, AdditionalDependencies, " + objFileFolderPath + ", reference .obj");

				Core::JEngineTool::CreateNewProject(name, path, fconfig);
				Core::JEngineTool::SetProjectConfig(path + "\\" + name + ".sln", path + "\\" + name + "\\" + name + ".vcxproj", sconfig);
				Core::JEngineTool::RemoveAllProjectItem(path + "\\" + name + ".sln", name);

				const std::string projectPath = path + "\\" + name;
				const std::string dllMainPath = projectPath + "\\dllmain.cpp";
				const std::string frameworkPath = projectPath + "\\framework.h";
				const std::string pchHeaderpath = projectPath + "\\pch.h";
				const std::string pchCpppath = projectPath + "\\pch.cpp";

				if (_access(dllMainPath.c_str(), 00) != -1)
					remove(dllMainPath.c_str());
				if (_access(frameworkPath.c_str(), 00) != -1)
					remove(frameworkPath.c_str());
				if (_access(pchHeaderpath.c_str(), 00) != -1)
					remove(pchHeaderpath.c_str());
				if (_access(pchCpppath.c_str(), 00) != -1)
					remove(pchCpppath.c_str());
			}
			static void CreateProjectDirectory()
			{
				const std::string name = JCUtil::WstrToU8Str(JApplicationProject::Name());
				const std::string path = JCUtil::WstrToU8Str(JApplicationProject::SolutionPath());
				const std::string slnPath = path + "\\" + name + ".sln";

				Core::JEngineTool::CreateProjectVirtualDir(slnPath, name, GetSrcFolderName());
			}
			static void CreateProjectDefaultFile()
			{
				const std::string name = JCUtil::WstrToU8Str(JApplicationProject::Name());
				const std::string path = JCUtil::WstrToU8Str(JApplicationProject::SolutionPath());
				const std::string srcPath = JCUtil::WstrToU8Str(JApplicationProject::SouceCodePath());
				const std::string slnPath = path + "\\" + name + ".sln";

				const std::string aName = JCUtil::WstrToU8Str(JApplicationProject::Name());

				const std::string pchHeaderContetns = Core::JCplusScriptCreator::CreateProjectPchHeader();
				const std::string pchCppContetnsh = Core::JCplusScriptCreator::CreateProjectPchCpp();
				const std::string dllMainContetns = Core::JCplusScriptCreator::CreateProjectDllMainCpp(aName);
				const std::string projHeaderContents = Core::JCplusScriptCreator::CreateProjectDefaultHeader(aName);
				const std::string projCppContetns = Core::JCplusScriptCreator::CreateProjectDefaultCpp(aName);

				const std::string pchHeaderPath = srcPath + "\\pch.h";
				const std::string pchCppPath = srcPath + "\\pch.cpp";
				const std::string dllMainPath = srcPath + "\\dllmain.cpp";
				const std::string projHeaderPath = srcPath + "\\" + aName + ".h";
				const std::string projCppPath = srcPath + "\\" + aName + ".cpp";

				std::ofstream stream;
				stream.open(pchHeaderPath, std::ios::out | std::ios::binary);
				stream << pchHeaderContetns;
				stream.close();

				stream.open(pchCppPath, std::ios::out | std::ios::binary);
				stream << pchCppContetnsh;
				stream.close();

				stream.open(dllMainPath, std::ios::out | std::ios::binary);
				stream << dllMainContetns;
				stream.close();

				stream.open(projHeaderPath, std::ios::out | std::ios::binary);
				stream << projHeaderContents;
				stream.close();

				stream.open(projCppPath, std::ios::out | std::ios::binary);
				stream << projCppContetns;
				stream.close();

				std::vector<Core::JEngineTool::SolFileInfo> fileInfo;
				fileInfo.push_back(Core::JEngineTool::SolFileInfo(pchHeaderPath, GetSrcFolderName()));
				fileInfo.push_back(Core::JEngineTool::SolFileInfo(pchCppPath, GetSrcFolderName()));
				fileInfo.push_back(Core::JEngineTool::SolFileInfo(dllMainPath, GetSrcFolderName()));
				fileInfo.push_back(Core::JEngineTool::SolFileInfo(projHeaderPath, GetSrcFolderName()));
				fileInfo.push_back(Core::JEngineTool::SolFileInfo(projCppPath, GetSrcFolderName()));

				Core::JEngineTool::SolAddFileConfig config;
				config.allowBuild = true;

				Core::JEngineTool::AddMultiFile(slnPath, name, fileInfo, config);
			}
			static void CreateProjectModule()
			{
				std::wstring dllPath = ProjectDllPath();
				auto m = _JModuleManager::Instance().TryAddModule(dllPath, true);
				if (m == nullptr)
					MessageBox(0, L"Fail add module", 0, 0);
			}
			static void CreateProjectPlugin()
			{
				std::wstring name = JApplicationProject::Name();
				std::wstring folderPath = JApplicationProject::RootPath();

				auto mod = _JModuleManager::Instance().GetModule(ProjectDllPath());
				auto modVec = std::vector<JUserPtr<Core::JModule>>{ mod };

				Core::JPluginDesc desc;
				desc.version = JApplicationEngine::GetLatestVersion();
				desc.minEngineVersion = JApplicationEngine::GetAppVersion().back();

				auto p = _JPluginManager::Instance().TryAddPlugin(name, folderPath, modVec, desc);
				if (p == nullptr)
					MessageBox(0, L"Fail add plugin", 0, 0);
			}

			static void DestroyProjectPreviewAsset(const std::wstring& ddsPath, const bool destroyDDs)
			{
				if (destroyDDs)
					_wremove(ddsPath.c_str());

				_wremove((JCUtil::GetPathWithOutFormat(ddsPath) + L".jAsset").c_str());
				_wremove((JCUtil::GetPathWithOutFormat(ddsPath) + L".jAssetMeta").c_str());
			}
		}

		using LoadProjectF = JApplicationProjectPrivate::LoadProjectF;
		using StoreProjectF = JApplicationProjectPrivate::StoreProjectF;
		using SetAppStateF = JApplicationProjectPrivate::SetAppStateF;
		class JApplicationProjectImpl
		{
		public:
			std::wstring activatedProjectName;
			std::wstring activatedProjectPath;
			std::unique_ptr<LoadProjectF> loadProjectF;
			std::unique_ptr<StoreProjectF> storeProjectF;
			std::unique_ptr<SetAppStateF> setAppStateF;
			std::unique_ptr<JApplicationProjectInfo> nextProjectInfo;
			std::vector<std::unique_ptr<JApplicationProjectInfo>> projectList;
		public:
			bool startProjectOnce = false;
			bool endProject = false;
			bool loadOtherProjectOnce = false;
			bool tryEndProjectOnce = false;
		public:
			static JApplicationProjectImpl& Instance()
			{
				static JApplicationProjectImpl m;
				return m;
			}
		public:
			std::wstring ProjectVersionFilePath()noexcept
			{
				return JApplicationProject::ConfigPath() + L"\\" + L"ProjectVersion.txt";
			}
		public:
			void SetProjectFolderPath(const std::wstring& projectName, const std::wstring& projectPath)
			{
				//activatedProjectName = JCUtil::EraseSideWChar(projectName, L' ');
				//activatedProjectPath = JCUtil::EraseSideWChar(projectPath, L' ');
				activatedProjectName = projectName;
				activatedProjectPath = projectPath;
			}
			void SetNextProjectInfo(std::unique_ptr<JApplicationProjectInfo>&& newNextProjectInfo)noexcept
			{
				nextProjectInfo = std::move(newNextProjectInfo);
			}
			bool SetStartNewProjectTrigger()
			{
				if (nextProjectInfo == nullptr)
					return false;
				startProjectOnce = true;
				return true;
			}
		public:
			bool IsValidVersion(const std::string& pVersion)
			{
				bool isValidVersion = false;
				const uint versionCount = (uint)versionList.size();
				for (uint i = 0; i < versionCount; ++i)
				{
					if (versionList[i] == pVersion)
					{
						isValidVersion = true;
						break;
					}
				}
				return isValidVersion;
			}
			bool IsValidPath(const std::wstring& projectPath)noexcept
			{   
				return _waccess(ProjectVersionFilePath().c_str(), 00) != -1;
			}
			bool CanStartProject()noexcept
			{
				return startProjectOnce;
			}
			bool CanEndProject()noexcept
			{
				return endProject;
			}
		public:
			bool MakeProjectFolder()
			{
				auto defulatFolderPathVec = JApplicationProject::GetDefaultFolderPath();
				for (const auto& data : defulatFolderPathVec)
				{ 
					if (_waccess(data.c_str(), 00) == -1)
					{
						if (_wmkdir(data.c_str()) == -1)
							return false;
					}
				}
				return true;
			}
			bool MakeProjectVersionFile(const std::string& pVersion)
			{
				if (!IsValidVersion(pVersion))
					return false;
				else
				{
					if (_waccess(ProjectVersionFilePath().c_str(), 00) == -1)
						return StoreProjectVersion(pVersion) == Core::J_FILE_IO_RESULT::SUCCESS;
					else
						return true;
				}
			}
			bool MakeProjectSolution()
			{
				std::wstring filePath;
				if (!JCUtil::FindFirstFilePathByFormat(activatedProjectPath, L".sln", filePath))
				{
					CreateProjectSolution();
					if (!JCUtil::FindFirstFilePathByFormat(JApplicationProject::SolutionPath(), L".sln", filePath))
						return false;

					CreateProjectDirectory();
					CreateProjectDefaultFile();
					CreateProjectModule();
					CreateProjectPlugin();
				}
				return true;
			}
			std::unique_ptr<JApplicationProjectInfo> MakeProjectInfo(const std::wstring& projectPath)noexcept
			{
				std::wstring name;
				std::wstring folderPath;
				JCUtil::DecomposeFolderPath(projectPath, folderPath, name);
				std::string version;
				Core::J_FILE_IO_RESULT res = LoadProejctVersion(version);
				if (!IsValidVersion(version))
					return nullptr;

				Core::JRealTime::JTime time = Core::JRealTime::GetNowTime();
				return std::make_unique<JApplicationProjectInfo>(Core::MakeGuid(), name, projectPath, JCUtil::U8StrToWstr(version), time, time);
			}
			std::unique_ptr<JApplicationProjectInfo> MakeProjectInfo(const std::wstring& projectPath, const std::string& pVersion)noexcept
			{
				if (!IsValidVersion(pVersion))
					return nullptr;

				std::wstring name;
				std::wstring folderPath;
				JCUtil::DecomposeFolderPath(projectPath, folderPath, name);
				Core::JRealTime::JTime time = Core::JRealTime::GetNowTime();
				return std::make_unique<JApplicationProjectInfo>(Core::MakeGuid(), name, projectPath, JCUtil::U8StrToWstr(pVersion), time, time);
			}
			std::unique_ptr<JApplicationProjectInfo> MakeProjectInfo(JApplicationProjectInfo* info)noexcept
			{
				return std::make_unique<JApplicationProjectInfo>(info->GetGuid(),
					info->GetName(),
					info->GetPath(),
					info->GetVersion(),
					info->GetCreatedTime(),
					Core::JRealTime::GetNowTime());
			}
		public:
			void BeginLoadOtherProject(std::unique_ptr<JApplicationProjectInfo>&& nextProjInfo)noexcept
			{
				if (nextProjInfo == nullptr)
					return;

				SetNextProjectInfo(std::move(nextProjInfo));
				loadOtherProjectOnce = true;
			}
			void BeginCloseProject()noexcept
			{
				tryEndProjectOnce = true;
			}
		public:
			void CancelCloseProject()noexcept
			{
				if (!tryEndProjectOnce)
					return;

				if (loadOtherProjectOnce)
					nextProjectInfo.reset();

				tryEndProjectOnce = false;
				loadOtherProjectOnce = false;
			}
			void ConfirmCloseProject()noexcept
			{
				if (!tryEndProjectOnce)
					return;

				tryEndProjectOnce = false;
				endProject = true;
				if (loadOtherProjectOnce)
				{
					loadOtherProjectOnce = false;
					SetStartNewProjectTrigger();
				}
			}
		public:
			void CloseProject()noexcept
			{
				bool (*ptr)(JApplicationProjectInfo*, size_t) = [](JApplicationProjectInfo* a, size_t guid) {return a->GetGuid() == guid; };
				int index = JCUtil::GetIndex(projectList, ptr, JApplicationProject::GetOpenProjectInfo()->GetGuid());
				projectList[index] = std::move(MakeProjectInfo(projectList[index].get()));
				StoreProjectList();
				//for update new preview image
				DestroyProjectPreviewAsset(projectList[index]->lastRsPath(), false);
			}
			void DestroyProjectImmediately(const int projectIndex)
			{
				if (JApplicationEngine::GetApplicationState() != J_APPLICATION_STATE::PROJECT_SELECT || projectList.size() <= projectIndex)
					return;
				 
				if (nextProjectInfo != nullptr && nextProjectInfo->GetPath() == projectList[projectIndex]->GetPath())
				{
					nextProjectInfo = nullptr;
					startProjectOnce = false;
					loadOtherProjectOnce = false;
				}
				 
				DestroyProjectPreviewAsset(projectList[projectIndex]->lastRsPath(), true);
				JFileIOHelper::DestroyDirectory(projectList[projectIndex]->GetPath());
				projectList.erase(projectList.begin() + projectIndex);
			}
		public:
			void RegisterFunctor(std::unique_ptr<LoadProjectF>&& newLoadProjectF,
				std::unique_ptr<StoreProjectF>&& newStoreProjectF,
				std::unique_ptr<SetAppStateF>&& newSetAppStateF)
			{
				loadProjectF = std::move(newLoadProjectF);
				storeProjectF = std::move(newStoreProjectF);
				setAppStateF = std::move(newSetAppStateF);
			}
			bool Initialize()
			{
				const uint projectListCount = (uint)projectList.size();
				//dependency order
				SetProjectFolderPath(nextProjectInfo->GetName(), nextProjectInfo->GetPath());
				bool isValid = MakeProjectFolder();
				isValid = isValid && MakeProjectVersionFile(JCUtil::WstrToU8Str(nextProjectInfo->GetVersion()));
				
				//미구현
				//빌드시스템 완성후 추가
				//isValid = isValid && MakeProjectSolution();
				if (isValid)
				{
					const bool hasProject = JApplicationProject::GetProjectInfo(nextProjectInfo->GetPath()) != nullptr;
					if (!hasProject)
						projectList.emplace_back(MakeProjectInfo(nextProjectInfo.get()));
					else
					{
						bool (*ptr)(JApplicationProjectInfo*, std::wstring*) = [](JApplicationProjectInfo* a, std::wstring* path)
						{  
							return a->GetPath() == *path;
						};
						auto str = nextProjectInfo->GetPath();
						int index = JCUtil::GetIndex(projectList, ptr,  &str);
						//<JApplicationProjectInfo, const std::wstring&>
						projectList[index] = std::move(MakeProjectInfo(nextProjectInfo.get()));
					} 
					StoreProjectList();
					(*setAppStateF)(J_APPLICATION_STATE::EDIT_GAME);
				}
				startProjectOnce = false;
				endProject = false;
				nextProjectInfo.reset();
				return isValid;
			}
		public:
			void StoreJTime(std::wofstream& stream, const std::wstring& guide, const Core::JRealTime::JTime& time)
			{
				if (!stream.is_open())
					return;

				JFileIOHelper::StoreJString(stream, guide, L"");
				JFileIOHelper::StoreAtomicData(stream, L"Year", time.year, false);
				JFileIOHelper::StoreAtomicData(stream, L"Month", time.month, false);
				JFileIOHelper::StoreAtomicData(stream, L"Day", time.day, false);
				JFileIOHelper::StoreAtomicData(stream, L"Hour", time.hour, false);
				JFileIOHelper::StoreAtomicData(stream, L"Minute", time.minute, false);
				JFileIOHelper::StoreAtomicData(stream, L"Sec", time.sec);
			}
			const Core::JRealTime::JTime LoadJTime(std::wifstream& stream)
			{
				if (!stream.is_open())
					return Core::JRealTime::JTime(0, 0, 0, 0, 0, 0);

				std::wstring guide;
				int year = 0;
				int month = 0;
				int day = 0;
				int hour = 0;
				int minute = 0;
				int sec = 0;

				JFileIOHelper::LoadJString(stream, guide);
				JFileIOHelper::LoadAtomicData(stream, year);
				JFileIOHelper::LoadAtomicData(stream, month);
				JFileIOHelper::LoadAtomicData(stream, day);
				JFileIOHelper::LoadAtomicData(stream, hour);
				JFileIOHelper::LoadAtomicData(stream, minute);
				JFileIOHelper::LoadAtomicData(stream, sec);

				return Core::JRealTime::JTime(year, month, day, hour, minute, sec);
			}
			Core::J_FILE_IO_RESULT StoreProjectVersion(const std::string& pVersion)
			{
				std::wofstream stream;
				stream.open(ProjectVersionFilePath().c_str(), std::ios::out | std::ios::binary);
				if (stream.is_open())
				{
					JFileIOHelper::StoreJString(stream, L"Symbol:", ENGINE_PROJECT_SYMBOL);
					JFileIOHelper::StoreJString(stream, L"Version:", JCUtil::StrToWstr(pVersion));
					stream.clear();
					return Core::J_FILE_IO_RESULT::SUCCESS;
				}
				else
					return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
			}
			Core::J_FILE_IO_RESULT LoadProejctVersion(_Out_ std::string& pVersion)
			{
				std::wifstream stream;
				stream.open(ProjectVersionFilePath().c_str(), std::ios::in | std::ios::binary);
				if (stream.is_open() && JFileIOHelper::SkipLine(stream, ENGINE_PROJECT_SYMBOL))
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
			void StoreProject()
			{
				(*storeProjectF)();
			}
			void LoadProject()
			{
				(*loadProjectF)();
			}
			void StoreProjectList()
			{
				//std::locale::global(std::locale("Korean"));
				std::wstring dirPath = JApplicationEngine::ProjectListFilePath();
				std::wofstream stream;
				stream.open(dirPath, std::ios::out | std::ios::binary);

				JFileIOHelper::StoreAtomicData(stream, L"ProjectCount:", (uint)projectList.size());
				for (const auto& data : projectList)
				{
					JFileIOHelper::StoreAtomicData(stream, L"Guid:", data->GetGuid());
					JFileIOHelper::StoreJString(stream, L"Name:", data->GetName());
					JFileIOHelper::StoreJString(stream, L"Path:", data->GetPath());
					JFileIOHelper::StoreJString(stream, L"Version:", data->GetVersion());

					StoreJTime(stream, L"CreatedTime:", data->GetCreatedTime());
					StoreJTime(stream, L"LastUpdatedTime:", data->GetLastUpdateTime());
					JFileIOHelper::InputSpace(stream, 1);
				}
				stream.close();
			}
			void LoadProjectList()
			{
				//std::locale::global(std::locale("Korean"));
				std::wstring dirPath = JApplicationEngine::ProjectListFilePath();
				std::wifstream stream;
				stream.open(dirPath, std::ios::in | std::ios::binary);

				uint projectCount;
				size_t guid;
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
						JFileIOHelper::LoadAtomicData(stream, guid);
						JFileIOHelper::LoadJString(stream, name);
						JFileIOHelper::LoadJString(stream, path);
						JFileIOHelper::LoadJString(stream, version);
						Core::JRealTime::JTime createTime = LoadJTime(stream);
						Core::JRealTime::JTime lastUpdateTime = LoadJTime(stream);
 
						if (_waccess(path.c_str(), 00) == 0)
							projectList.emplace_back(std::make_unique<JApplicationProjectInfo>(guid, name, path, version, createTime, lastUpdateTime));
						else
						{ 
							std::wstring ddsPath = JApplicationProjectInfo::CombineLastRsPath(guid);
							DestroyProjectPreviewAsset(ddsPath.c_str(), true);
						}
					}
				}
			}
		};
		  
		JApplicationProjectInfo::JApplicationProjectInfo(const size_t guid,
			const std::wstring& name,
			const std::wstring& path,
			const std::wstring& version,
			const JTime& createdTime,
			const JTime& lastUpdateTime)
			:guid(guid),
			name(name),
			path(path),
			version(version),
			createdTime(createdTime),
			lastUpdateTime(lastUpdateTime)
		{}
		std::unique_ptr<JApplicationProjectInfo> JApplicationProjectInfo::CreateReplica()noexcept
		{
			return std::make_unique<JApplicationProjectInfo>(guid,
				name,
				path,
				version,
				lastUpdateTime,
				createdTime);
		}
		std::wstring JApplicationProjectInfo::lastRsPath()noexcept
		{
			return CombineLastRsPath(guid);
		}
		std::wstring JApplicationProjectInfo::CombineLastRsPath(const size_t guid)noexcept
		{
			return JApplicationEngine::ProjectLastRsPath() + L"\\" + std::to_wstring(guid) + L".dds";
		}

		std::wstring JApplicationProject::Name()noexcept
		{
			return JApplicationProjectImpl::Instance().activatedProjectName;
		}
		std::wstring JApplicationProject::RootPath()noexcept
		{
			return JApplicationProjectImpl::Instance().activatedProjectPath;
		}
		std::wstring JApplicationProject::ContentsPath()noexcept
		{
			return JApplicationProjectImpl::Instance().activatedProjectPath + L"\\" + L"Content";
		}
		std::wstring JApplicationProject::DocumentPath()noexcept
		{
			return JApplicationProjectImpl::Instance().activatedProjectPath + L"\\" + L"Document";
		}
		std::wstring JApplicationProject::ConfigPath()noexcept
		{
			return JApplicationProjectImpl::Instance().activatedProjectPath + L"\\" + L"Config";
		}
		std::wstring JApplicationProject::ProjectResourcePath()noexcept
		{
			return JApplicationProjectImpl::Instance().activatedProjectPath + L"\\" + L"Resource";
		}
		std::wstring JApplicationProject::BinaryPath()noexcept
		{
			return JApplicationProjectImpl::Instance().activatedProjectPath + L"\\" + L"Binary";
		}
		std::wstring JApplicationProject::SolutionPath()noexcept
		{
			return JApplicationProjectImpl::Instance().activatedProjectPath + L"\\Solution";
		}
		//Contents
		std::wstring JApplicationProject::ContentScenePath()noexcept
		{
			return ContentsPath() + L"\\" + L"Scene";
		}
		std::wstring JApplicationProject::ContentResourcePath()noexcept
		{
			return ContentsPath() + L"\\" + L"Resource";
		}
		//Log
		std::wstring JApplicationProject::LogPath()noexcept
		{
			return DocumentPath() + L"\\Log";
		}
		std::wstring JApplicationProject::DevelopLogPath()noexcept
		{
			return LogPath() + L"\\DevelopLog";
		}
		//Config
		std::wstring JApplicationProject::EditoConfigPath()noexcept
		{
			return ConfigPath() + L"\\" + L"Editor";
		}
		//VersionFile
		std::wstring JApplicationProject::ProjectVersionFilePath()noexcept
		{
			return JApplicationProjectImpl::Instance().ProjectVersionFilePath();
		}

		//ProjectResourcePath
		std::wstring JApplicationProject::DefaultResourcePath()noexcept
		{
			return ProjectResourcePath() + L"\\" + L"Default";
		}
		std::wstring JApplicationProject::ShaderMetafilePath()noexcept
		{
			return ProjectResourcePath() + L"\\" + L"ShaderMetafile";
		}
		std::wstring JApplicationProject::ModResourceCachePath()noexcept
		{
			return ProjectResourcePath() + L"\\" + L"ModResourceCache";
		}
		std::wstring JApplicationProject::ProjectBinaryPath()noexcept
		{
			return BinaryPath() + L"\\" + GetOutDirName();
		}
		std::wstring JApplicationProject::SouceCodePath()noexcept
		{
			return SolutionPath() + L"\\" + GetSrcFolderWName();
		}
		std::wstring JApplicationProject::OutDirPath()noexcept
		{
			return ProjectBinaryPath();
		}

		uint JApplicationProject::GetProjectInfoCount()noexcept
		{
			return (uint)JApplicationProjectImpl::Instance().projectList.size();
		}
		JApplicationProjectInfo* JApplicationProject::GetOpenProjectInfo()noexcept
		{
			return GetProjectInfo(JApplicationProjectImpl::Instance().activatedProjectPath);
		}
		JApplicationProjectInfo* JApplicationProject::GetProjectInfo(uint index)noexcept
		{
			if (index >= JApplicationProjectImpl::Instance().projectList.size())
				return nullptr;
			else
				return JApplicationProjectImpl::Instance().projectList[index].get();
		}
		JApplicationProjectInfo* JApplicationProject::GetProjectInfo(const std::wstring& projectPath)noexcept
		{
			const uint projectCount = (uint)JApplicationProjectImpl::Instance().projectList.size();
			for (uint i = 0; i < projectCount; ++i)
			{
				if (JApplicationProjectImpl::Instance().projectList[i]->GetPath() == projectPath)
					return JApplicationProjectImpl::Instance().projectList[i].get();
			}
			return nullptr;
		}
		std::vector<std::wstring> JApplicationProject::GetDefaultFolderPath()noexcept
		{
			return std::vector<std::wstring>
			{
				JApplicationProjectImpl::Instance().activatedProjectPath,
					ContentsPath(),
					DocumentPath(),
					ConfigPath(),
					ProjectResourcePath(),
					BinaryPath(),
					SolutionPath(),
					ShaderMetafilePath(),
					DefaultResourcePath(),
					ModResourceCachePath(),
					EditoConfigPath(),
					LogPath(),
					DevelopLogPath(),
					ContentScenePath(),
					ContentResourcePath(),
					ProjectBinaryPath(),
					SouceCodePath()
			};
		}
		bool JApplicationProject::IsValidVersion(const std::string& pVersion)noexcept
		{
			return JApplicationProjectImpl::Instance().IsValidVersion(pVersion);
		}
		bool JApplicationProject::IsValidPath(const std::wstring& projectPath)noexcept
		{
			return JApplicationProjectImpl::Instance().IsValidPath(projectPath);
		}
		bool JApplicationProject::IsDefaultFolder(const std::wstring& path)noexcept
		{
			auto vec = GetDefaultFolderPath();
			for (const auto& data : vec)
			{
				if (path == data)
					return true;
			}
			return false;
		}
		void MainAccess::RegisterFunctor(std::unique_ptr<LoadProjectF>&& loadF,
			std::unique_ptr<StoreProjectF>&& storeF,
			std::unique_ptr<SetAppStateF>&& setStateF)
		{
			JApplicationProjectImpl::Instance().RegisterFunctor(std::move(loadF), std::move(storeF), std::move(setStateF));
		}
		bool MainAccess::Initialize()
		{
			return JApplicationProjectImpl::Instance().Initialize();
		}
		void MainAccess::BeginLoadOtherProject(std::unique_ptr<JApplicationProjectInfo>&& nextProjInfo)noexcept
		{
			JApplicationProjectImpl::Instance().BeginLoadOtherProject(std::move(nextProjInfo));
		}
		void MainAccess::BeginCloseProject()noexcept
		{
			JApplicationProjectImpl::Instance().BeginCloseProject();
		}
		void MainAccess::CloseProject()noexcept
		{
			JApplicationProjectImpl::Instance().CloseProject();
		}
		bool MainAccess::CanStartProject()noexcept
		{
			return JApplicationProjectImpl::Instance().CanStartProject();
		}
		bool MainAccess::CanEndProject()noexcept
		{
			return JApplicationProjectImpl::Instance().CanEndProject();
		}

		std::unique_ptr<JApplicationProjectInfo> LifeInterface::MakeProjectInfo(const std::wstring& projectPath)noexcept
		{
			return JApplicationProjectImpl::Instance().MakeProjectInfo(projectPath);
		}
		std::unique_ptr<JApplicationProjectInfo> LifeInterface::MakeProjectInfo(const std::wstring& projectPath, const std::string& pVersion)noexcept
		{
			return JApplicationProjectImpl::Instance().MakeProjectInfo(projectPath, pVersion);
		}
		std::unique_ptr<JApplicationProjectInfo> LifeInterface::MakeProjectInfo(JApplicationProjectInfo* info)noexcept
		{
			return JApplicationProjectImpl::Instance().MakeProjectInfo(info);
		}
		void LifeInterface::SetNextProjectInfo(std::unique_ptr<JApplicationProjectInfo>&& nextProjectInfo)noexcept
		{
			JApplicationProjectImpl::Instance().SetNextProjectInfo(std::move(nextProjectInfo));
		}
		bool LifeInterface::SetStartNewProjectTrigger()
		{
			return JApplicationProjectImpl::Instance().SetStartNewProjectTrigger();
		}
		void LifeInterface::CancelCloseProject()noexcept
		{
			JApplicationProjectImpl::Instance().CancelCloseProject();
		}
		void LifeInterface::ConfirmCloseProject()noexcept
		{
			JApplicationProjectImpl::Instance().ConfirmCloseProject();
		}
		void LifeInterface::DestroyProject(const int projectIndex)noexcept
		{
			JApplicationProjectImpl::Instance().DestroyProjectImmediately(projectIndex);
		}

		Core::J_FILE_IO_RESULT IOInterface::StoreProjectVersion(const std::string& pVersion)
		{
			return JApplicationProjectImpl::Instance().StoreProjectVersion(pVersion);
		}
		Core::J_FILE_IO_RESULT IOInterface::LoadProejctVersion(_Out_ std::string& pVersion)
		{
			return JApplicationProjectImpl::Instance().LoadProejctVersion(pVersion);
		}
		void IOInterface::StoreProject()
		{
			JApplicationProjectImpl::Instance().StoreProject();
		}
		void IOInterface::LoadProject()
		{
			JApplicationProjectImpl::Instance().LoadProject();
		}
		void IOInterface::StoreProjectList()
		{
			JApplicationProjectImpl::Instance().StoreProjectList();
		}
		void IOInterface::LoadProjectList()
		{
			JApplicationProjectImpl::Instance().LoadProjectList();
		}
	}
}
