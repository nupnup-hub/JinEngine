#include"JApplicationLog.h"
#include"../Core/Log/JLog.h"

namespace JinEngine
{
	namespace Application
	{ 
		namespace
		{
			static std::unique_ptr<Core::JLogHandler> appLog;
		}

		void JApplicaitonLog::AddLog(std::unique_ptr<Core::JLogBase>&& log)
		{
			appLog->PushLog(std::move(log));
		}
		void JApplicaitonLog::AddLog(const std::wstring& body)
		{
			appLog->PushLog(std::make_unique<Core::JLogBase>(L" ", body));
		}
		void JApplicaitonLog::AddLog(const std::wstring& title, const std::wstring& body)
		{
			appLog->PushLog(std::make_unique<Core::JLogBase>(title, body));
		}

		void JApplicationLogPrivate::Initialize()noexcept
		{
			appLog = std::make_unique<Core::JLogHandler>();
		}
		void JApplicationLogPrivate::Clear()noexcept
		{
			appLog.reset();
		}
		std::vector<Core::JLogBase*> JApplicationLogPrivate::GetLog()noexcept
		{
			return appLog->GetLogVec();
		}
	}
}