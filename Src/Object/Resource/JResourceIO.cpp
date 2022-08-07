#include"JResourceIO.h"     
#include"JResourceObject.h"
#include"JResourceObjectInterface.h"
#include"JResourceObjectFactory.h"
#include"../Directory/JDirectory.h"
#include"../../Core/JDataType.h"
#include"../../Utility/JCommonUtility.h"
#include"../Directory/JDirectoryFactory.h"
#include"../../Graphic/JGraphic.h"
#include"../../Graphic/JGraphicResourceManager.h"
#include"../../Application/JApplicationVariable.h"  
#include"../../Core/Guid/GuidCreator.h"
#include<Windows.h>

namespace fs = std::filesystem;
using namespace std;

namespace JinEngine
{
	JResourceIO::JResourceIO() {}
	JResourceIO::~JResourceIO() {}
	void JResourceIO::LoadEngineDirectory(JDirectory* engineRootDir)
	{
		SearchDirectory(engineRootDir, true, (JOBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_INERASABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNEDITABLE));
	}
	void JResourceIO::LoadProjectDirectory(JDirectory* projectRootDir)
	{
		std::vector<std::string> defaultFolderPath
		{
			JApplicationVariable::GetProjectSettingPath(),
			JApplicationVariable::GetProjectLibraryPath(),
			JApplicationVariable::GetProjectContentPath()
		};

		std::vector<JOBJECT_FLAG> flag
		{
			(JOBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_INERASABLE),
			(JOBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_INERASABLE),
			(JOBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_INERASABLE)
		};

		std::vector<JDirectory*> defaultFolderCash;
		for (uint i = 0; i < defaultFolderPath.size(); ++i)
		{
			std::string dirPath = defaultFolderPath[i];
			std::string dirFolderPath;
			std::string dirName;
			std::string dirFormat;
			JCommonUtility::DecomposeFilePath(dirPath, dirFolderPath, dirName, dirFormat);
			defaultFolderCash.push_back(JDFI::Create(dirName, Core::MakeGuid(), flag[i], *projectRootDir));
		}
		/*
		* dependence resource
		* need to search In the order below.
		*/
		// defaultFolderCash[0] = ProjectSetting
		// defaultFolderCash[1] = projectLibraryDir
		// defaultFolderCash[2] = projectContentsDir

		const JOBJECT_FLAG projectOtherDir = (JOBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_INERASABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNEDITABLE);
		const JOBJECT_FLAG projectContentDir = (JOBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_INERASABLE | OBJECT_FLAG_UNEDITABLE);
		const JOBJECT_FLAG userDefineDir = OBJECT_FLAG_NONE;

		SearchDirectory(defaultFolderCash[0], true, projectOtherDir);
		SearchDirectory(defaultFolderCash[1], true, projectOtherDir);
		SearchDirectory(defaultFolderCash[2], true, projectContentDir);
		SearchDirectory(defaultFolderCash[2], false, userDefineDir);
	}
	void JResourceIO::LoadProjectResource(JDirectory* projectRootDir)
	{
		std::vector<JRI::RTypeHint> rInfo = JRI::GetRTypeHintVec(RESOURCE_ALIGN_TYPE::DEPENDENCY);
		const uint rInfoCount = (uint)rInfo.size();
		for (uint i = 0; i < rInfoCount; ++i)
		{
			JDirectory* searchRootDir;
			if (rInfo[i].thisType == J_RESOURCE_TYPE::SHADER)
				searchRootDir = projectRootDir->SearchDirectory(JApplicationVariable::GetProjectShaderMetafilePath());
			else
				searchRootDir = projectRootDir->SearchDirectory(JApplicationVariable::GetProjectContentPath());

			if (rInfo[i].hasGraphicResource)
			{
				JGraphic::Instance().CommandInterface()->FlushCommandQueue();
				JGraphic::Instance().CommandInterface()->StartCommand();
				SearchResource(rInfo[i].thisType, searchRootDir);
				JGraphic::Instance().CommandInterface()->EndCommand();
				JGraphic::Instance().CommandInterface()->FlushCommandQueue();
			}
			else
				SearchResource(rInfo[i].thisType, searchRootDir);
		}
	}
	void JResourceIO::SearchDirectory(JDirectory* parentDir, bool searchDefaultFolder, const JOBJECT_FLAG dirFlag)
	{
		WIN32_FIND_DATA  findFileData;
		HANDLE hFindFile = FindFirstFile((parentDir->GetWPath() + L"\\*.*").c_str(), &findFileData);
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
					const std::string cFileName = JCommonUtility::WstringToU8String(findFileData.cFileName);
					if (!parentDir->HasChild(cFileName))
					{
						JDirectory* next = JDFI::Create(cFileName, Core::MakeGuid(), dirFlag, *parentDir);
						SearchDirectory(next, searchDefaultFolder, dirFlag);
					}
				}
			}
			bResult = FindNextFile(hFindFile, &findFileData);
		}
		FindClose(hFindFile);
	}
	void JResourceIO::SearchResource(const J_RESOURCE_TYPE rType, JDirectory* directory)
	{
		const std::wstring dirWPath = directory->GetWPath();
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
					JResourcePathData pathData(directory->GetWPath() + L"\\" + findFileData.cFileName);
					if (JRI::CallIsValidFormat(rType, pathData.format))
						JRFI<JResourceObject>::LoadByName(JRI::CallGetTypeName(rType), *directory, pathData);
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