#include"JApplicationProject.h" 
#include"JApplicationProjectPrivate.h"  
#include"JApplicationEngine.h"
#include"JApplicationEnginePrivate.h"
#include"../Core/JDataType.h"
#include"../Core/File/JFileIOHelper.h"
#include"../Core/Guid/JGuidCreator.h"
#include"../Utility/JCommonUtility.h"
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
			using AppAccess = JApplicationProjectPrivate::AppAccess;
			using LifeInterface = JApplicationProjectPrivate::LifeInterface;
			using IOInterface = JApplicationProjectPrivate::IOInterface;
		}
		namespace
		{
			static const std::vector<std::string> versionList
			{
				"1.00",
			}; 
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
			bool startProjectOnce = false;
			bool endProject = false;
			bool loadOtherProjectOnce = false;
			bool tryEndProjectOnce = false;
		public:
			std::wstring ProjectVersionFilePath()noexcept
			{
				return JApplicationProject::SettingPath() + L"\\" + L"ProjectVersion.txt";
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
				const std::wstring projectVersionFilePath = projectPath + L"\\" + L"ProjectSetting" + L"\\" + L"ProjectVersion.txt";
				return _waccess(projectVersionFilePath.c_str(), 00) != -1;
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
			void BeginLoadOtherProject()noexcept
			{
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
				int index = JCUtil::GetJIndex(projectList, ptr, JApplicationProject::GetOpenProjectInfo()->GetGuid());
				projectList[index] = std::move(MakeProjectInfo(projectList[index].get()));
				StoreProjectList();
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
				bool res00 = MakeProjectFolder();
				bool res01 = MakeProjectVersionFile(JCUtil::WstrToU8Str(nextProjectInfo->GetVersion()));
				if (res00 && res01)
				{
					const bool hasProject = JApplicationProject::GetProjectInfo(nextProjectInfo->GetPath()) != nullptr;
					if (!hasProject)
						projectList.emplace_back(MakeProjectInfo(nextProjectInfo.get()));
					else
					{
						bool (*ptr)(JApplicationProjectInfo*, std::wstring) = [](JApplicationProjectInfo* a, std::wstring path) {return a->GetPath() == path; };
						int index = JCUtil::GetJIndex(projectList, ptr, nextProjectInfo->GetPath());
						projectList[index] = std::move(MakeProjectInfo(nextProjectInfo.get()));
					}
					StoreProjectList();
					(*setAppStateF)(J_APPLICATION_STATE::EDIT_GAME);
				}
				startProjectOnce = false;
				endProject = false;
				nextProjectInfo.reset();
				return res00 && res01;
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
					}
				}
			}
		};
 
		namespace
		{
			static JApplicationProjectImpl impl;
		}

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
		std::unique_ptr<JApplicationProjectInfo> JApplicationProjectInfo::GetUnique()noexcept
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
			return JApplicationEngine::ProjectLastRsPath() + L"\\" + std::to_wstring(GetGuid()) + L".dds";
		}

		std::wstring JApplicationProject::Name()noexcept
		{
			return impl.activatedProjectName;
		}
		std::wstring JApplicationProject::RootPath()noexcept
		{
			return impl.activatedProjectPath;
		}
		std::wstring JApplicationProject::ContentsPath()noexcept
		{
			return impl.activatedProjectPath + L"\\" + L"Content";
		}
		std::wstring JApplicationProject::SettingPath()noexcept
		{
			return impl.activatedProjectPath + L"\\" + L"Setting";
		}
		std::wstring JApplicationProject::LogPath()noexcept
		{
			return impl.activatedProjectPath + L"\\" + L"Log";
		}
		std::wstring JApplicationProject::ProjectResourcePath()noexcept
		{
			return impl.activatedProjectPath + L"\\" + L"ProjectResource";
		}
		//Contents
		std::wstring JApplicationProject::ContentScenePath()noexcept
		{
			return ContentsPath() + L"\\" + L"Scene";
		}
		std::wstring JApplicationProject::ContentScriptPath()noexcept
		{
			return ContentsPath() + L"\\" + L"Script";
		}
		std::wstring JApplicationProject::ContentResourcePath()noexcept
		{
			return ContentsPath() + L"\\" + L"Resource";
		}
		//Setting
		std::wstring JApplicationProject::EditorSettingPath()noexcept
		{
			return SettingPath() + L"\\" + L"Editor";
		}
		//Log
		std::wstring JApplicationProject::ProjectLogPath()noexcept
		{
			return LogPath() + L"\\" + L"ProjectLog";
		}
		//VersionFile
		std::wstring JApplicationProject::ProjectVersionFilePath()noexcept
		{
			return impl.ProjectVersionFilePath();
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
	 
		 
		uint JApplicationProject::GetProjectInfoCount()noexcept
		{
			return (uint)impl.projectList.size();
		}
		JApplicationProjectInfo* JApplicationProject::GetOpenProjectInfo()noexcept
		{
			return GetProjectInfo(impl.activatedProjectPath);
		}
		JApplicationProjectInfo* JApplicationProject::GetProjectInfo(uint index)noexcept
		{
			if (index >= impl.projectList.size())
				return nullptr;
			else
				return impl.projectList[index].get();
		}
		JApplicationProjectInfo* JApplicationProject::GetProjectInfo(const std::wstring& projectPath)noexcept
		{
			const uint projectCount = (uint)impl.projectList.size();
			for (uint i = 0; i < projectCount; ++i)
			{
				if (impl.projectList[i]->GetPath() == projectPath)
					return impl.projectList[i].get();
			}
			return nullptr;
		}
		std::vector<std::wstring> JApplicationProject::GetDefaultFolderPath()noexcept
		{
			return std::vector<std::wstring>
			{
				impl.activatedProjectPath,
					ContentsPath(),
					SettingPath(),
					LogPath(),
					ProjectResourcePath(),
					ShaderMetafilePath(),
					DefaultResourcePath(),
					ModResourceCachePath(),
					EditorSettingPath(),
					ProjectLogPath(),
					ContentScenePath(),
					ContentScriptPath(),
					ContentResourcePath()
			};
		}
		bool JApplicationProject::IsValidVersion(const std::string& pVersion)noexcept
		{
			return impl.IsValidVersion(pVersion);
		}
		bool JApplicationProject::IsValidPath(const std::wstring& projectPath)noexcept
		{
			return impl.IsValidPath(projectPath);
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
		void AppAccess::RegisterFunctor(std::unique_ptr<LoadProjectF>&& loadF, 
			std::unique_ptr<StoreProjectF>&& storeF,
			std::unique_ptr<SetAppStateF>&& setStateF)
		{ 
			impl.RegisterFunctor(std::move(loadF), std::move(storeF), std::move(setStateF));
		}
		bool AppAccess::Initialize()
		{
			return impl.Initialize();
		}
		void AppAccess::BeginLoadOtherProject()noexcept
		{
			impl.BeginLoadOtherProject();
		}
		void AppAccess::BeginCloseProject()noexcept
		{
			impl.BeginCloseProject();
		}
		void AppAccess::CloseProject()noexcept
		{
			impl.CloseProject();
		}
		bool AppAccess::CanStartProject()noexcept
		{
			return impl.CanStartProject();
		}
		bool AppAccess::CanEndProject()noexcept
		{
			return impl.CanEndProject();
		}


		std::unique_ptr<JApplicationProjectInfo> LifeInterface::MakeProjectInfo(const std::wstring& projectPath)noexcept
		{
			return impl.MakeProjectInfo(projectPath);
		}
		std::unique_ptr<JApplicationProjectInfo> LifeInterface::MakeProjectInfo(const std::wstring& projectPath, const std::string& pVersion)noexcept
		{
			return impl.MakeProjectInfo(projectPath, pVersion);
		}
		std::unique_ptr<JApplicationProjectInfo> LifeInterface::MakeProjectInfo(JApplicationProjectInfo* info)noexcept
		{
			return impl.MakeProjectInfo(info);
		}
		void LifeInterface::SetNextProjectInfo(std::unique_ptr<JApplicationProjectInfo>&& nextProjectInfo)noexcept
		{
			impl.SetNextProjectInfo(std::move(nextProjectInfo));
		}
		bool LifeInterface::SetStartNewProjectTrigger()
		{
			return impl.SetStartNewProjectTrigger();
		}
		void LifeInterface::CancelCloseProject()noexcept
		{
			impl.CancelCloseProject();
		}
		void LifeInterface::ConfirmCloseProject()noexcept
		{
			impl.ConfirmCloseProject();
		}
		 
		Core::J_FILE_IO_RESULT IOInterface::StoreProjectVersion(const std::string& pVersion)
		{
			return impl.StoreProjectVersion(pVersion);
		}
		Core::J_FILE_IO_RESULT IOInterface::LoadProejctVersion(_Out_ std::string& pVersion)
		{
			return impl.LoadProejctVersion(pVersion);
		}
		void IOInterface::StoreProject()
		{
			impl.StoreProject();
		}
		void IOInterface::LoadProject()
		{
			impl.LoadProject();
		}
		void IOInterface::StoreProjectList()
		{
			impl.StoreProjectList();
		}
		void IOInterface::LoadProjectList()
		{
			impl.LoadProjectList();
		}
	}
}
