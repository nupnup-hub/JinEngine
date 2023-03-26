#include"JResourceIO.h"     
#include"JResourceObject.h"
#include"JResourceObjectInterface.h"
#include"JResourceObjectFactory.h"
#include"JResourceImporter.h"
#include"Texture/JTexture.h"
#include"Mesh/JMeshGeometry.h"
#include"AnimationClip/JAnimationClip.h"

#include"../Directory/JDirectory.h"
#include"../Directory/JDirectoryFactory.h"
#include"../../Core/JDataType.h"
#include"../../Core/File/JFileConstant.h"
#include"../../Core/File/JFilePathData.h"
#include"../../Core/File/JFileIOHelper.h"
#include"../../Utility/JCommonUtility.h"
#include"../../Graphic/JGraphic.h"
#include"../../Graphic/GraphicResource/JGraphicResourceManager.h"
#include"../../Application/JApplicationVariable.h"  
#include"../../Core/Guid/GuidCreator.h"
#include<Windows.h>
#include<io.h>
#include<fstream>

namespace fs = std::filesystem;
using namespace std;

namespace JinEngine
{
	JResourceIO::JResourceIO() {}
	JResourceIO::~JResourceIO() {}
	JDirectory* JResourceIO::LoadRootDirectory(const std::wstring& path, const J_OBJECT_FLAG initFlag)
	{
		Core::JAssetFileLoadPathData pathData{ path };
		if (_waccess(pathData.engineFileWPath.c_str(), 00) != -1 &&
			_waccess(pathData.engineMetaFileWPath.c_str(), 00) != -1)
			return JDFI::LoadRoot(pathData);
		else
			return JDFI::CreateRoot(pathData.engineFileWPath, Core::MakeGuid(), initFlag);
	}
	void JResourceIO::LoadEngineDirectory(JDirectory* engineRootDir)
	{
		SearchDirectory(engineRootDir, true, (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNCOPYABLE));
	}
	void JResourceIO::LoadEngineResource(JDirectory* engineRootDir)
	{
		std::vector<JRI::RTypeHint> rInfo = JRI::GetRTypeHintVec(J_RESOURCE_ALIGN_TYPE::DEPENDENCY);
		const uint rInfoCount = (uint)rInfo.size();
		for (uint i = 0; i < rInfoCount; ++i)
			SearchResource(rInfo[i].thisType, engineRootDir);
	}
	void JResourceIO::LoadProjectDirectory(JDirectory* projectRootDir)
	{
		std::vector<std::wstring> defaultFolderPath
		{
			JApplicationVariable::GetProjectSettingPath(),
			JApplicationVariable::GetProjectLibraryPath(),
			JApplicationVariable::GetProjectContentPath(),
		};

		std::vector<J_OBJECT_FLAG> flag
		{
			(J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNCOPYABLE),
			(J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNCOPYABLE),
			(J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNCOPYABLE)
		};

		std::vector<JDirectory*> defaultFolderCash;
		for (uint i = 0; i < defaultFolderPath.size(); ++i)
		{
			Core::JAssetFileLoadPathData pathData{ defaultFolderPath[i] };
			if (_waccess(pathData.engineFileWPath.c_str(), 00) != -1 &&
				_waccess(pathData.engineMetaFileWPath.c_str(), 00) != -1)
				defaultFolderCash.push_back(JDFI::Load(*projectRootDir, pathData));
			else
				defaultFolderCash.push_back(JDFI::Create(pathData.name, Core::MakeGuid(), flag[i], *projectRootDir));
		}
		/*
		* dependence resource
		* need to search In the order below.
		*/
		// defaultFolderCash[0] = ProjectSetting
		// defaultFolderCash[1] = projectLibraryDir
		// defaultFolderCash[2] = projectContentsDir

		const J_OBJECT_FLAG projectOtherDirFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNCOPYABLE);
		const J_OBJECT_FLAG projectContentDirFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNCOPYABLE);
		const J_OBJECT_FLAG userDefineDirFlag = OBJECT_FLAG_NONE;

		SearchDirectory(defaultFolderCash[0], true, projectOtherDirFlag);
		SearchDirectory(defaultFolderCash[1], true, projectOtherDirFlag);
		SearchDirectory(defaultFolderCash[2], true, projectContentDirFlag);
		SearchDirectory(defaultFolderCash[2], false, userDefineDirFlag);
	}
	void JResourceIO::LoadProjectResource(JDirectory* projectRootDir)
	{
		std::vector<JRI::RTypeHint> rInfo = JRI::GetRTypeHintVec(J_RESOURCE_ALIGN_TYPE::DEPENDENCY);
		const uint rInfoCount = (uint)rInfo.size();
		for (uint i = 0; i < rInfoCount; ++i)
			SearchResource(rInfo[i].thisType, projectRootDir);
	}
	std::vector<JResourceObject*> JResourceIO::ImportResource(JDirectory* dir, const std::wstring& path)
	{
		return JResourceImporter::Instance().ImportResource(dir, path);
	}
	void JResourceIO::SearchDirectory(JDirectory* parentDir, bool searchDefaultFolder, const J_OBJECT_FLAG dirFlag)
	{ 
		WIN32_FIND_DATA  findFileData;
		HANDLE hFindFile = FindFirstFile((parentDir->GetPath() + L"\\*.*").c_str(), &findFileData);
		BOOL bResult = TRUE;
		if (hFindFile == INVALID_HANDLE_VALUE)
			return;

		if (searchDefaultFolder && !JApplicationVariable::IsDefaultFolder(parentDir->GetPath()))
			return;

		while (bResult)
		{
			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (wcscmp(findFileData.cFileName, L".") && wcscmp(findFileData.cFileName, L".."))
				{
					JDirectory* next = nullptr;
					if (!parentDir->HasChild(findFileData.cFileName))
					{
						Core::JAssetFileLoadPathData pathData{ Core::JFileConstant::MakeFilePath(parentDir->GetPath(),  findFileData.cFileName) };
						if (searchDefaultFolder)
						{
							if (JApplicationVariable::IsDefaultFolder(pathData.engineFileWPath))
							{
								if (_waccess(pathData.engineFileWPath.c_str(), 00) != -1 &&
									_waccess(pathData.engineMetaFileWPath.c_str(), 00) != -1)
									next = JDFI::Load(*parentDir, pathData);
								else
									next = JDFI::Create(pathData.name, Core::MakeGuid(), dirFlag, *parentDir);
							}
						}
						else
						{
							if (_waccess(pathData.engineFileWPath.c_str(), 00) != -1 &&
								_waccess(pathData.engineMetaFileWPath.c_str(), 00) != -1)
								next = JDFI::Load(*parentDir, pathData);
							else
								next = JDFI::Create(pathData.name, Core::MakeGuid(), dirFlag, *parentDir);
						}
					}
					else
						next = parentDir->GetChildDirctoryByName(findFileData.cFileName);
					if (next != nullptr)
						SearchDirectory(next, searchDefaultFolder, dirFlag);
				}
			}
			bResult = FindNextFile(hFindFile, &findFileData);
		}
		FindClose(hFindFile);
	}
	void JResourceIO::SearchResource(const J_RESOURCE_TYPE rType, JDirectory* directory)
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
				{
					Core::JAssetFileLoadPathData pathData(directory->GetPath() + L"\\" + findFileData.cFileName);
					if (pathData.format == Core::JFileConstant::GetFileFormat())
					{
						std::wifstream stream;
						stream.open(pathData.engineMetaFileWPath, std::ios::in | std::ios::binary);
						if (stream.is_open())
						{
							if (JFileIOHelper::SkipLine(stream, Core::JFileConstant::StreamTypeSymbol<J_RESOURCE_TYPE>()))
							{
								int storeType = 0;
								stream >> storeType;
								stream.close();
								if ((J_RESOURCE_TYPE)storeType == rType)
									JRFIB::LoadByName(JRI::CallGetTypeInfo((J_RESOURCE_TYPE)rType).Name(), *directory, pathData);
							}
						}
					}
				}
			}
			bResult = FindNextFile(hFindFile, &findFileData);
		}
		FindClose(hFindFile);
		const uint childrenCount = directory->GetChildernDirctoryCount();
		for (uint i = 0; i < childrenCount; ++i)
		{
			JDirectory* nextDir = directory->GetChildDirctory(i);
			if (nextDir != nullptr)
				SearchResource(rType, nextDir);
		}
	}
}