#pragma once
#include<string>
#include<vector>

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