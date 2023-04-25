#pragma once
#include<memory>
#include<string>
#include<vector>

namespace JinEngine
{
	namespace Core
	{
		class JLogBase;
		class JLogHandler;
	}

	namespace Application
	{
		class JApplicaitonLog
		{ 
		public:
			static void AddLog(std::unique_ptr<Core::JLogBase>&& log);
			static void AddLog(const std::wstring& body);
			static void AddLog(const std::wstring& title, const std::wstring& body);
		private:
			friend class JApplicationLogPrivate;
		};

		class JApplicationLogPrivate
		{
		private:
			static void Initialize()noexcept;
			static void Clear()noexcept;
		private:
			static std::vector<Core::JLogBase*> GetLog()noexcept; 
		};
	}
}