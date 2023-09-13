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
#include"../../Core/JCoreEssential.h"
#include"../../Core/Identity/JIdenCreator.h"
#include"../../Core/File/JFileConstant.h"
#include"../../Core/File/JFilePathData.h"
#include"../../Core/File/JFileIOHelper.h"
#include"../../Core/Utility/JCommonUtility.h"
#include"../../Graphic/JGraphic.h"
#include"../../Graphic/GraphicResource/JGraphicResourceManager.h"
#include"../../Application/JApplicationEngine.h"  
#include"../../Application/JApplicationProject.h"  
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
		Core::JAssetFileLoadPathData pathData{ path };
		if (_waccess(pathData.engineFileWPath.c_str(), 00) != -1 &&
			_waccess(pathData.engineMetaFileWPath.c_str(), 00) != -1)
		{
			auto loadData = DirIOInterface::CreateLoadAssetDIData(nullptr, pathData);
			return Core::ConvertChildUserPtr<JDirectory>(DirIOInterface::LoadAssetData(loadData.get()));
		}
		else
			return JICI::Create<JDirectory>(pathData.engineFileWPath, Core::MakeGuid(), initFlag, nullptr);
	}
	void JResourceObjectIO::LoadEngineDirectory(const JUserPtr<JDirectory>& engineRootDir)
	{
		SearchDirectory(engineRootDir, false, true, (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNCOPYABLE));
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
		std::vector<std::wstring> defaultFolderPath
		{
			JApplicationProject::ConfigPath(),
			JApplicationProject::ProjectResourcePath(),
			JApplicationProject::DocumentPath(), 
			JApplicationProject::ContentsPath(),
		};

		std::vector<J_OBJECT_FLAG> flag
		{
			(J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNCOPYABLE),
			(J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNCOPYABLE),
			(J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNCOPYABLE),
			(J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNCOPYABLE)
		};

		std::vector<JUserPtr<JDirectory>> defaultFolderCash;
		for (uint i = 0; i < defaultFolderPath.size(); ++i)
		{
			Core::JAssetFileLoadPathData pathData{ defaultFolderPath[i] };   
			if (_waccess(pathData.engineFileWPath.c_str(), 00) != -1 &&
				_waccess(pathData.engineMetaFileWPath.c_str(), 00) != -1)
			{
				auto loadData = DirIOInterface::CreateLoadAssetDIData(projectRootDir, pathData);
				defaultFolderCash.push_back(Core::ConvertChildUserPtr<JDirectory>(DirIOInterface::LoadAssetData(loadData.get())));
			}
			else
				defaultFolderCash.push_back(JICI::Create<JDirectory>(pathData.name, Core::MakeGuid(), flag[i], projectRootDir));
		}
		/*
		* dependence resource
		* need to search In the order below.
		*/
		// defaultFolderCash[0] = ProjectSetting
		// defaultFolderCash[1] = projectResourceDir
		// defaultFolderCash[2] = projectLog
		// defaultFolderCash[3] = projectContentsDir

		//subDir Flag
		const J_OBJECT_FLAG projectOtherDirFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNCOPYABLE);
		const J_OBJECT_FLAG projectContentDirFlag = (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNCOPYABLE);
		const J_OBJECT_FLAG userDefineDirFlag = OBJECT_FLAG_NONE;

		SearchDirectory(defaultFolderCash[0], true, true, projectOtherDirFlag);
		SearchDirectory(defaultFolderCash[1], true, false, projectOtherDirFlag);	// default resource has folder
		SearchDirectory(defaultFolderCash[2], true, false, projectOtherDirFlag); 
		SearchDirectory(defaultFolderCash[3], true, true, projectContentDirFlag);
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
			std::wifstream stream;
			stream.open(data->GetMetaFilePath(), std::ios::in | std::ios::binary);
			if (!stream.is_open())
				continue;

			if (!JFileIOHelper::SkipSentence(stream, Core::JFileConstant::StreamTypeGuidSymbol()))
				return;

			size_t typeGuid = 0;
			stream >> typeGuid;
			Core::JTypeInfo* typeInfo = _JReflectionInfo::Instance().GetTypeInfo(typeGuid);

			if (!JFileIOHelper::SkipSentence(stream, Core::JFileConstant::StreamLastOpenSymbol(*typeInfo)))
				return;

			bool isOpen = false;
			stream >> isOpen;
			stream.close();
			if (!isOpen)
				continue;

			if (data->TryGetResourceUser() != nullptr)
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
						Core::JAssetFileLoadPathData pathData{ Core::JFileConstant::MakeFilePath(parentDir->GetPath(),  findFileData.cFileName) };
						if (!onlyDeafultFolder || isDefaultPtr(pathData.engineFileWPath))
						{
							if (_waccess(pathData.engineFileWPath.c_str(), 00) != -1 &&
								_waccess(pathData.engineMetaFileWPath.c_str(), 00) != -1)
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
		Core::JAssetFileLoadPathData pathData(directory->GetPath() + L"\\" + fileName);
		if (pathData.format != Core::JFileConstant::GetFileFormat())
			return;

		std::wifstream stream;
		stream.open(pathData.engineMetaFileWPath, std::ios::in | std::ios::binary);
		if (!stream.is_open())
			return;

		if (!JFileIOHelper::SkipSentence(stream, Core::JFileConstant::StreamObjGuidSymbol()))
			return;
		size_t guid = 0;
		stream >> guid;

		if (!JFileIOHelper::SkipSentence(stream, Core::JFileConstant::StreamTypeGuidSymbol()))
			return;
		size_t typeGuid = 0;
		stream >> typeGuid;

		if (!JFileIOHelper::SkipSentence(stream, Core::JFileConstant::StreamObjFlagSymbol()))
			return;
		int flag = -1;
		stream >> flag;

		Core::JTypeInfo* typeInfo = _JReflectionInfo::Instance().GetTypeInfo(typeGuid);
		if (!JFileIOHelper::SkipSentence(stream, Core::JFileConstant::StreamTypeSymbol<J_RESOURCE_TYPE>()))
			return;

		int storeType = 0;
		stream >> storeType;
		J_RESOURCE_TYPE storedRType = (J_RESOURCE_TYPE)storeType;
		if (storedRType != rType)
		{
			stream.close();
			return;
		}

		if (!JFileIOHelper::SkipSentence(stream, Core::JFileConstant::StreamFormatIndexSymbol()))
			return;

		int formatIndex = 0;
		stream >> formatIndex;
		JFileInitData initData(pathData.name, guid, *typeInfo, storedRType, (J_OBJECT_FLAG)flag, (uint8)formatIndex);

		if (storedRType == J_RESOURCE_TYPE::SCENE)
		{
			if (canLoadResource)
			{
				if (!JFileIOHelper::SkipSentence(stream, Core::JFileConstant::StreamLastOpenSymbol(*typeInfo)))
					return;

				bool isOpen = false;
				stream >> isOpen;
				stream.close();
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
			stream.close();
			auto file = DirFileInterface::CreateJFile(initData, directory);
			if (canLoadResource)
				file->TryGetResourceUser();
			//JRFIB::LoadByName(JRI::CallGetTypeInfo(storedRType).Name(), *directory, pathData);
		}
	}
}