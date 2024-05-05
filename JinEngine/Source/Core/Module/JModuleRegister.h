#pragma once
#include"JModuleIntializer.h"
#include"JModuleManager.h"
#include"../Utility/JCommonUtility.h"
//#include"../../Application/Project/JApplicationProject.h"
#include"../File/JFilePathData.h" 
#include<windows.h>

namespace JinEngine
{
	namespace Core
	{ 
		template<typename Module>
		class JModuleRegister
		{
		public:
			JModuleRegister(const std::wstring& path,const JModuleDesc& desc = JModuleDesc())
			{ 
				//std::wstring wName = JCUtil::U8StrToWstr(name);
				//std::wstring modulePath = JApplicationProject::OutDirPath() + L"\\" + wName + L".dll";
				_JModuleManager::Instance().RegisterModule(JModuleIntializer{path, desc });
			}
		};
	}
}