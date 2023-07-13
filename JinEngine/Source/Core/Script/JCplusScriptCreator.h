#pragma once
#include<string>
namespace JinEngine
{
	namespace Core
	{
		class JCplusScriptCreator
		{
		public:	// Not Wrapping
			static std::string CreateProjectDefaultHeader(const std::string& name);
			static std::string CreateProjectDefaultCpp(const std::string& name);
			static std::string CreateProjectDllMainCpp(const std::string& name); 
			static std::string CreateProjectPchHeader();
			static std::string CreateProjectPchCpp();
			static std::string CreateBehaviorHeader(const std::string& name);
			static std::string CreateBehaviorCpp(const std::string& name);
		};
	}
}
