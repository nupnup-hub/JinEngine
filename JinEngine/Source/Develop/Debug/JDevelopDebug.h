#pragma once
#include"../../Core/Log/JLogHandler.h"

namespace JinEngine
{
	namespace Develop
	{
		class JDevelopDebug
		{ 
		public:
			static uint GetPublicLogSetCount() noexcept; 
			static std::vector<Core::JLogBase*> GetLogVec(const std::string& name); 
		public:
			//use develop default log handler
			static void PushLog(const std::string& contents); 
			static void PushLog(const std::wstring& contents);
			static void PushLog(const std::string& name, const std::string& contents); 
		public:
			static bool PushPublicLogHandler(const std::string& name);
			static bool PopPublicLogHandler(const std::string& name);
		public:
			static void Write();
			static void Clear(); 
		public:
			static void Activate();
			static void DeActivate();
			static bool IsActivate();
		};
	}
}