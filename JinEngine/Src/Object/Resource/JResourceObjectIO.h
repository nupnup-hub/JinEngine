#pragma once 
#include<direct.h>	 
#include<memory>  
#include<string>
#include"JResourceObjectType.h"
#include"../JObjectFlag.h"

namespace JinEngine
{
	class JDirectory;
	class JResourceObject;
	class JResourceObjectIO
	{ 
	public:
		JResourceObjectIO();
		~JResourceObjectIO();
	public:
		JDirectory* LoadRootDirectory(const std::wstring& path, const J_OBJECT_FLAG initFlag);
		void LoadEngineDirectory(JDirectory* engineRootDir);
		void LoadEngineResource(JDirectory* engineRootDir);
		void LoadProjectDirectory(JDirectory* projectRootDir);
		void LoadProjectResource(JDirectory* projectRootDir); 
	private:
		void SearchDirectory(JDirectory* parentDir, const bool searchProjectFolder, const bool onlyDeafultFolder, const J_OBJECT_FLAG dirFlag);
		void SearchFile (const J_RESOURCE_TYPE rType, JDirectory* directory, const bool canLoadResource);
	private:
		void LoadFile(const J_RESOURCE_TYPE rType, JDirectory* directory, const std::wstring& fileName, const bool canLoadResource);
	};
}
