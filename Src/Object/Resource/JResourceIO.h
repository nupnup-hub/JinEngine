#pragma once 
#include<direct.h>	 
#include<memory>  
#include"JResourceType.h"
#include"../JObjectFlag.h"

namespace JinEngine
{
	class JDirectory;
	class JResourceIO
	{ 
	public:
		JResourceIO();
		~JResourceIO();
		void LoadEngineDirectory(JDirectory* engineRootDir);
		void LoadProjectDirectory(JDirectory* projectRootDir);
		void LoadProjectResource(JDirectory* projectRootDir);
	private:
		void SearchDirectory(JDirectory* parentDir, bool searchDefaultFolder, const JOBJECT_FLAG dirFlag);
		void SearchResource(const J_RESOURCE_TYPE rType, JDirectory* directory);
	};
}
