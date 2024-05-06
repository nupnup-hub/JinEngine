#include"JResourceObjectIO.h"     
#include"JResourceObject.h"  
#include"JResourceObjectHint.h"
#include"JResourceObjectImporter.h"
#include"Texture/JTexture.h"
#include"Mesh/JMeshGeometry.h"
#include"AnimationClip/JAnimationClip.h"

#include"../Directory/JDirectory.h" 
#include"../Directory/JDirectoryPrivate.h" 
#include"../Directory/JFile.h" 
#include"../Directory/JFileInitData.h"
#include"../JObjectFileIOHelper.h"
#include"../../Core/JCoreEssential.h"
#include"../../Core/Identity/JIdenCreator.h"
#include"../../Core/File/JFileConstant.h"
#include"../../Core/File/JFilePathData.h" 
#include"../../Core/Utility/JCommonUtility.h"
#include"../../Graphic/JGraphic.h"
#include"../../Graphic/GraphicResource/JGraphicResourceManager.h"
#include"../../Application/Engine/JApplicationEngine.h"  
#include"../../Application/Project/JApplicationProject.h"  
#include"../../Core/Guid/JGuidCreator.h"
#include<Windows.h>
#include<io.h>
#include<fstream>

namespace fs = std::filesystem;
using namespace std;

namespace JinEngine
{
	namespace
	{
		using DirIOInterface = JDirectoryPrivate::AssetDataIOInterface;
		using DirFileInterface = JDirectoryPrivate::FileInterface;
		using IsDefaultFolderPtr = bool(*)(const std::wstring& path);
	}
	namespace
	{
		static IsDefaultFolderPtr GetDefaultFolderPtr(const bool isProjectFolder)
		{
			if (isProjectFolder)
				return &JApplicationProject::IsDefaultFolder;
			else
				return &JApplicationEngine::IsDefaultFolder;
		}
	}


	JResourceObjectIO::JResourceObjectIO() {}
	JResourceObjectIO::~JResourceObjectIO() {}
	JUserPtr<JDirectory> JResourceObjectIO::LoadRootDirectory(const std::wstring& path, const J_OBJECT_FLAG initFlag)
	{
		Core::JAssetFilePathData pathData{ path };
		if (_waccess(pathData.path.c_str(), 00) != -1 &&
			_waccess(pathData.metaFilePath.c_str(), 00) != -1)
		{
			auto loadData = DirIOInterface::CreateLoadAssetDIData(nullptr, pathData);
			return Core::ConvertChildUserPtr<JDirectory>(DirIOInterface::LoadAssetData(loadData.get()));
		}
		else
			return JICI::Create<JDirectory>(pathData.path, Core::MakeGuid(), initFlag, nullptr);
	}
	void JResourceObjectIO::LoadEngineDirectory(const JUserPtr<JDirectory>& engineRootDir)
	{
		const J_OBJECT_FLAG flag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED |
			OBJECT_FLAG_UNDESTROYABLE |
			OBJECT_FLAG_HIDDEN | 
			OBJECT_FLAG_UNEDITABLE | 
			OBJECT_FLAG_UNCOPYABLE | 
			OBJECT_FLAG_RESTRICT_CONTROL_IDENTIFICABLE);
		SearchDirectory(engineRootDir, false, true, flag);
	}
	void JResourceObjectIO::LoadEngineResource(const JUserPtr<JDirectory>& engineRootDir)
	{
		auto rHintVec = RTypeCommonCall::GetRTypeHintVec(J_RESOURCE_ALIGN_TYPE::DEPENDENCY);
		for (const auto& hint : rHintVec)
			SearchFile(hint.thisType, engineRootDir, false);
	}
	void JResourceObjectIO::LoadProjectDirectory(const JUserPtr<JDirectory>& projectRootDir)
	{
		//has count dependency
		//defaultFolderCash
		//flag
		//subDirFlag
		//defaultFolderCash indexing

		static constexpr uint defaultResourceInedx = 4;
		std::vector<std::wstring> defaultFolderPath
		{
			JApplicationProject::ConfigPath(),
			JApplicationProject::ProjectPrivateResourcePath(),
			JApplicationProject::DocumentPath(), 
			JApplicationProject::ContentsPath(),
			JApplicationProject::DefaultResourcePath()
		};

		std::vector<J_OBJECT_FLAG> flag
		{
			(J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNCOPYABLE | OBJECT_FLAG_RESTRICT_CONTROL_IDENTIFICABLE),
			(J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNCOPYABLE | OBJECT_FLAG_RESTRICT_CONTROL_IDENTIFICABLE),
			(J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNCOPYABLE | OBJECT_FLAG_RESTRICT_CONTROL_IDENTIFICABLE),
			(J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNCOPYABLE | OBJECT_FLAG_RESTRICT_CONTROL_IDENTIFICABLE),
			(J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNCOPYABLE | OBJECT_FLAG_RESTRICT_CONTROL_IDENTIFICABLE)
		};
		 
		std::vector<JUserPtr<JDirectory>> defaultFolderCash;
		for (uint i = 0; i < defaultFolderPath.size(); ++i)
		{
			JUserPtr<JDirectory> parent = projectRootDir;
			if (i == defaultResourceInedx)
				parent = defaultFolderCash[3];

			Core::JAssetFilePathData pathData{ defaultFolderPath[i] };   
			if (_waccess(pathData.path.c_str(), 00) != -1 &&
				_waccess(pathData.metaFilePath.c_str(), 00) != -1)
			{
				auto loadData = DirIOInterface::CreateLoadAssetDIData(parent, pathData);
				defaultFolderCash.push_back(Core::ConvertChildUserPtr<JDirectory>(DirIOInterface::LoadAssetData(loadData.get())));
			}
			else
				defaultFolderCash.push_back(JICI::Create<JDirectory>(pathData.name, Core::MakeGuid(), flag[i], parent));
		}
 
		/*
		* dependence resource
		* need to search In the order below.
		*/
		// defaultFolderCash[0] = ProjectSetting
		// defaultFolderCash[1] = projectResourceDir
		// defaultFolderCash[2] = projectLog
		// defaultFolderCash[3] = projectContentsDir
		// defaultFolderCash[4] = projectContentsDefaultResourceDir

		//subDir Flag
		const J_OBJECT_FLAG projectOtherDirFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNCOPYABLE | OBJECT_FLAG_RESTRICT_CONTROL_IDENTIFICABLE);
		const J_OBJECT_FLAG projectContentDirFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNCOPYABLE | OBJECT_FLAG_RESTRICT_CONTROL_IDENTIFICABLE);
		const J_OBJECT_FLAG userDefineDirFlag = OBJECT_FLAG_NONE;

		SearchDirectory(defaultFolderCash[0], true, true, projectOtherDirFlag);
		SearchDirectory(defaultFolderCash[1], true, false, projectOtherDirFlag);	// default resource has folder
		SearchDirectory(defaultFolderCash[2], true, false, projectOtherDirFlag); 
		SearchDirectory(defaultFolderCash[3], true, true, projectContentDirFlag);
		SearchDirectory(defaultFolderCash[4], true, false, projectOtherDirFlag);
		SearchDirectory(defaultFolderCash[3], true, false, userDefineDirFlag);
	}
	void JResourceObjectIO::LoadProjectResourceFile(const JUserPtr<JDirectory>& projectRootDir)
	{
		//scene resource has all dependency
		auto rHintVec = RTypeCommonCall::GetRTypeHintVec(J_RESOURCE_ALIGN_TYPE::DEPENDENCY);
		for (const auto& hint : rHintVec)
			SearchFile(hint.thisType, projectRootDir, false);
	}
	void JResourceObjectIO::LoadProjectLastOpendScene(const JUserPtr<JDirectory>& projectRootDir)
	{
		auto fileVec = projectRootDir->GetDirectoryFileVec(true, J_RESOURCE_TYPE::SCENE);
		for (const auto& data : fileVec)
		{
			JFileIOTool tool;
			if (!tool.Begin(data->GetMetaFilePath(), JFileIOTool::TYPE::JSON, JFileIOTool::BEGIN_OPTION_JSON_TRY_LOAD_DATA))
				continue;

			size_t typeGuid = 0;
			if (JFileIOHelper::LoadAtomicData(tool, typeGuid, Core::JFileConstant::GetTypeGuidSymbol()) != Core::J_FILE_IO_RESULT::SUCCESS)
			{
				tool.Close();
				continue;
			}

			Core::JTypeInfo* typeInfo = _JReflectionInfo::Instance().GetTypeInfo(typeGuid);
			if (JFileIOHelper::LoadAtomicData(tool, typeGuid, Core::JFileConstant::GetTypeGuidSymbol()) != Core::J_FILE_IO_RESULT::SUCCESS)
			{
				tool.Close();
				continue;
			}

			bool isOpen = false;
			if (JFileIOHelper::LoadAtomicData(tool, isOpen, Core::JFileConstant::GetLastOpenSymbol(*typeInfo)) != Core::J_FILE_IO_RESULT::SUCCESS)
			{
				tool.Close();
				continue;
			}
			tool.Close();

			if (isOpen && data->TryGetResourceUser() != nullptr)
				return;
		}
	}
	void JResourceObjectIO::SearchDirectory(const JUserPtr<JDirectory>& parentDir, const bool searchProjectFolder, const bool onlyDeafultFolder, const J_OBJECT_FLAG subDirFlag)
	{ 
		WIN32_FIND_DATA  findFileData;
		HANDLE hFindFile = FindFirstFile((parentDir->GetPath() + L"\\*.*").c_str(), &findFileData);
		BOOL bResult = TRUE;
		if (hFindFile == INVALID_HANDLE_VALUE)
			return;
	 
		IsDefaultFolderPtr isDefaultPtr = GetDefaultFolderPtr(searchProjectFolder);
		if (onlyDeafultFolder && !isDefaultPtr(parentDir->GetPath()))
			return;

		while (bResult)
		{
			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (wcscmp(findFileData.cFileName, L".") && wcscmp(findFileData.cFileName, L".."))
				{
					JUserPtr<JDirectory> next = nullptr; 
					if (!parentDir->HasChild(findFileData.cFileName))
					{
						Core::JAssetFilePathData pathData{ Core::JFileConstant::MakeFilePath(parentDir->GetPath(),  findFileData.cFileName) };
						if (!onlyDeafultFolder || isDefaultPtr(pathData.path))
						{
							if (_waccess(pathData.path.c_str(), 00) != -1 &&
								_waccess(pathData.metaFilePath.c_str(), 00) != -1)
							{ 
								auto loadData = DirIOInterface::CreateLoadAssetDIData(parentDir, pathData);
								next = Core::ConvertChildUserPtr<JDirectory>(DirIOInterface::LoadAssetData(loadData.get())); 
							}
							else
								next = JICI::Create<JDirectory>(pathData.name, Core::MakeGuid(), subDirFlag, parentDir);
						}
					}
					else
						next = parentDir->GetChildDirctoryByName(findFileData.cFileName); 
					if (next != nullptr)
						SearchDirectory(next, searchProjectFolder, onlyDeafultFolder, subDirFlag);
				}
			}
			else
			{
				if (!Core::HasSQValueEnum(subDirFlag, OBJECT_FLAG_UNDESTROYABLE) && wcscmp(findFileData.cFileName, L".") && wcscmp(findFileData.cFileName, L".."))
				{
					Core::JAssetFilePathData pathData(parentDir->GetPath() + L"\\" + findFileData.cFileName);
					if (pathData.format == Core::JFileConstant::GetMetaFileFormatW())
					{
						const std::wstring name = JCUtil::GetFileNameWithOutFormat(findFileData.cFileName);
						const std::wstring dirPath = parentDir->GetPath() + L"\\" + name;
						const std::wstring assetPath = parentDir->GetPath() + L"\\" + name + Core::JFileConstant::GetFileFormatW();
						if (!JFileIOHelper::HasFile(dirPath) && !JFileIOHelper::HasFile(assetPath))
							JFileIOHelper::DestroyFile(pathData.metaFilePath);
					}
				}
			}
			bResult = FindNextFile(hFindFile, &findFileData);
		}
		FindClose(hFindFile);
	}
	void JResourceObjectIO::SearchFile(const J_RESOURCE_TYPE rType, const JUserPtr<JDirectory>& directory, const bool canLoadResource)
	{
		const std::wstring dirWPath = directory->GetPath();
		WIN32_FIND_DATA  findFileData;
		HANDLE hFindFile = FindFirstFile((dirWPath + L"\\*.*").c_str(), &findFileData);
		BOOL bResult = TRUE;

		if (hFindFile == INVALID_HANDLE_VALUE)
			return;

		while (bResult)
		{
			if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				if (wcscmp(findFileData.cFileName, L".") && wcscmp(findFileData.cFileName, L".."))
					LoadFile(rType, directory, findFileData.cFileName, canLoadResource);
			}
			bResult = FindNextFile(hFindFile, &findFileData);
		}
		FindClose(hFindFile);
		const uint childrenCount = directory->GetChildernDirctoryCount();
		for (uint i = 0; i < childrenCount; ++i)
		{
			JUserPtr<JDirectory> nextDir = directory->GetChildDirctory(i);
			if (nextDir != nullptr)
				SearchFile(rType, nextDir, canLoadResource);
		}
	}
	void JResourceObjectIO::LoadFile(const J_RESOURCE_TYPE rType, const JUserPtr<JDirectory>& directory, const std::wstring& fileName, const bool canLoadResource)
	{
		Core::JAssetFilePathData pathData(directory->GetPath() + L"\\" + fileName);
		if (pathData.format != Core::JFileConstant::GetFileFormatW())
		{
			if (pathData.format == Core::JFileConstant::GetMetaFileFormatW() && !Core::HasSQValueEnum(directory->GetFlag(), OBJECT_FLAG_UNDESTROYABLE))
			{
				const std::wstring dirPath = directory->GetPath() + L"\\" + pathData.name;
				const std::wstring assetPath = directory->GetPath() + L"\\" + pathData.name + Core::JFileConstant::GetFileFormatW();
				
				//asset file이 없는 경우 삭제 (directory는 meta만 있으므로 제외)
				if (!JFileIOHelper::HasFile(dirPath) && !JFileIOHelper::HasFile(assetPath))
					JFileIOHelper::DestroyFile(pathData.metaFilePath);
			}
			return;
		}
		JFileIOTool tool;
		if (!tool.Begin(pathData.metaFilePath, JFileIOTool::TYPE::JSON, JFileIOTool::BEGIN_OPTION_JSON_TRY_LOAD_DATA))
			return;
 
		size_t guid = 0;
		if (JFileIOHelper::LoadAtomicData(tool, guid, Core::JFileConstant::GetObjGuidSymbol()) != Core::J_FILE_IO_RESULT::SUCCESS)
		{
			tool.Close();
			return;
		}

		size_t typeGuid = 0;
		if (JFileIOHelper::LoadAtomicData(tool, typeGuid, Core::JFileConstant::GetTypeGuidSymbol()) != Core::J_FILE_IO_RESULT::SUCCESS)
		{
			tool.Close();
			return;
		}
 
		J_OBJECT_FLAG flag = OBJECT_FLAG_NONE;
		if (JFileIOHelper::LoadEnumData(tool, flag, Core::JFileConstant::GetObjFlagSymbol()) != Core::J_FILE_IO_RESULT::SUCCESS)
		{
			tool.Close();
			return;
		}
 
		J_RESOURCE_TYPE storedRType;
		Core::JTypeInfo* typeInfo = _JReflectionInfo::Instance().GetTypeInfo(typeGuid);
		if (JFileIOHelper::LoadEnumData(tool, storedRType, Core::JFileConstant::GetTypeSymbol<J_RESOURCE_TYPE>()) != Core::J_FILE_IO_RESULT::SUCCESS)
		{
			tool.Close();
			return;
		}

		if (storedRType != rType)
		{
			tool.Close();
			return;
		}
		int formatIndex = 0;
		if (JFileIOHelper::LoadAtomicData(tool, formatIndex, Core::JFileConstant::GetFormatIndexSymbol()) != Core::J_FILE_IO_RESULT::SUCCESS)
		{
			tool.Close();
			return;
		}
	 
		JFileInitData initData(pathData.name, guid, *typeInfo, storedRType, (J_OBJECT_FLAG)flag, (uint8)formatIndex);
		if (storedRType == J_RESOURCE_TYPE::SCENE)
		{
			if (canLoadResource)
			{
				bool isOpen = false;
				if (JFileIOHelper::LoadAtomicData(tool, isOpen, Core::JFileConstant::GetLastOpenSymbol(*typeInfo)) != Core::J_FILE_IO_RESULT::SUCCESS)
				{
					tool.Close();
					return;
				}				 
				auto file = DirFileInterface::CreateJFile(initData, directory);
				if (isOpen)
					file->TryGetResourceUser();
				//JRFIB::LoadByName(JRI::CallGetTypeInfo(storedRType).Name(), *directory, pathData);
			}
			else
				DirFileInterface::CreateJFile(initData, directory);
		}
		else
		{
			tool.Close();
			auto file = DirFileInterface::CreateJFile(initData, directory);
			if (canLoadResource)
				file->TryGetResourceUser();
			//JRFIB::LoadByName(JRI::CallGetTypeInfo(storedRType).Name(), *directory, pathData);
		}
	}
}