#pragma once 
#include<direct.h>	 
#include<memory>  
#include<string>
#include"JResourceType.h"
#include"../JObjectFlag.h"

namespace JinEngine
{
	class JDirectory;
	class JResourceObject;
	class JResourceIO
	{ 
	public:
		JResourceIO();
		~JResourceIO();
	public:
		JDirectory* LoadRootDirectory(const std::wstring& path, const J_OBJECT_FLAG initFlag);
		void LoadEngineDirectory(JDirectory* engineRootDir);
		void LoadEngineResource(JDirectory* engineRootDir);
		void LoadProjectDirectory(JDirectory* projectRootDir);
		void LoadProjectResource(JDirectory* projectRootDir);
	public:
		std::vector<JResourceObject*> ImportResource(JDirectory* dir, const std::wstring& path);
	private:
		void SearchDirectory(JDirectory* parentDir, bool searchDefaultFolder, const J_OBJECT_FLAG dirFlag);
		void SearchResource(const J_RESOURCE_TYPE rType, JDirectory* directory);
	};
}
