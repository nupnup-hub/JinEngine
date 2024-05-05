#pragma once 
#include"JResourceObjectType.h"
#include"../JObjectFlag.h"
#include<direct.h>	  

namespace JinEngine
{
	class JDirectory;
	class JResourceObject;

	namespace Core
	{
		struct JAssetFilePathData;
	}
	class JResourceObjectIO
	{ 
	public:
		JResourceObjectIO();
		~JResourceObjectIO();
	public:
		JUserPtr<JDirectory> LoadRootDirectory(const std::wstring& path, const J_OBJECT_FLAG initFlag);
		void LoadEngineDirectory(const JUserPtr<JDirectory>& engineRootDir);
		void LoadEngineResource(const JUserPtr<JDirectory>& engineRootDir);
		void LoadProjectDirectory(const JUserPtr<JDirectory>& projectRootDir);
		void LoadProjectResourceFile(const JUserPtr<JDirectory>& projectRootDir);
		void LoadProjectLastOpendScene(const JUserPtr<JDirectory>& projectRootDir);
	private:
		void SearchDirectory(const JUserPtr<JDirectory>& parentDir, const bool searchProjectFolder, const bool onlyDeafultFolder, const J_OBJECT_FLAG subDirFlag);
		void SearchFile (const J_RESOURCE_TYPE rType, const JUserPtr<JDirectory>& directory, const bool canLoadResource);
	private:
		void LoadFile(const J_RESOURCE_TYPE rType, const JUserPtr<JDirectory>& directory, const std::wstring& fileName, const bool canLoadResource);
	};
}
