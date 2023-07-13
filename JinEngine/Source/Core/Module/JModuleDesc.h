#pragma once
#include"JModuleType.h"
#include<string>
#include<vector>

namespace JinEngine
{
	namespace Core
	{ 
		struct JModuleDesc
		{
		public:
			std::string version = "1.0.0";
		public: 
			J_MODULE_TYPE moduleType = J_MODULE_TYPE::PROJECT;
			J_MODULE_LINK_TYPE linkType = J_MODULE_LINK_TYPE::DYNAMIC;
			J_MODULE_LOAD_PHASE loadPhase = J_MODULE_LOAD_PHASE::DEFAULT;
		public:
			bool isLoaded = false;
		};
	}
}