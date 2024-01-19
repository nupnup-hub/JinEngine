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
			static bool HasLogHandler(const std::string& name);
		public:
			//use develop default log handler
			static void PushLog(const std::string& contents); 
			static void PushLog(const std::wstring& contents);
			static void PushLog(const std::string& logName, const std::string& contents);  
			static void PushLog(const std::string& logName, const std::wstring& contents);
		public:
			//create public log handler
			static bool CreatePublicLogHandler(const std::string& name);
			static bool DestroyPublicLogHandler(const std::string& name);
		public: 
			//push default log stack
			static bool PushDefaultLogHandler(const std::string& name);
			static bool PopDefaultLogHandler(const std::string& name);
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