#pragma once 
#include"../JCoreEssential.h" 

namespace JinEngine
{
	namespace Core
	{ 
		struct JPluginDesc
		{
		public:
			std::string version;
			std::string minEngineVersion;
		public:
			bool isLoaded;
		};
	}
}