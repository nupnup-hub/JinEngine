#pragma once
#include"JModuleIntializer.h"
#include"JModuleManager.h"
#include"../../Application/JApplicationProject.h"
#include"../../Utility/JCommonUtility.h"
#include<string>  
#include<windows.h>

namespace JinEngine
{
	namespace Core
	{ 
		template<typename Module>
		class JModuleRegister
		{
		public:
			JModuleRegister(const std::string& name, const JModuleDesc& desc = JModuleDesc())
			{ 
				std::wstring wName = JCUtil::U8StrToWstr(name);
				std::wstring modulePath = JApplicationProject::OutDirPath() + L"\\" + wName + L".dll";
				_JModuleManager::Instance().RegisterModule(JModuleIntializer{ wName , modulePath, desc });
			}
		};
	}
}