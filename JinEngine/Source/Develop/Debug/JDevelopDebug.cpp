#include"JDevelopDebug.h"
#include"JDevelopDebugConstants.h"
#include"../../Core/Utility/JCommonUtility.h"
#include"../../Application/JApplicationProject.h" 
#include"../../Application/JApplicationEngine.h" 
#include<unordered_map>
#include<stack>

namespace JinEngine::Develop
{
	namespace Private
	{
		static constexpr bool canStreamOut = true;
		static bool isActivated = false;
#ifdef DEVELOP
		static constexpr bool developMode = true;
#else
		static constexpr bool developMode = false;
#endif
		struct JDevelopDebugData
		{
		public:
			std::unordered_map<std::string, std::unique_ptr<Core::JPublicLogHolder>> holderMap;
			std::stack<std::string> defaultLogNameStack;
		};

		static std::unique_ptr<JDevelopDebugData> data;
		static JDevelopDebugData* GetData()
		{
			if (data == nullptr)
			{
				data = std::make_unique<JDevelopDebugData>();
				data->defaultLogNameStack.push(Constants::defualtLogHandlerName); 
			}
			return data.get();
		}
		static Core::JPublicLogHolder* GetHandle(const std::string& name)
		{
			auto& map = Private::GetData()->holderMap;
			auto data = map.find(name);
			if (data == map.end())
				return nullptr;

			return data->second.get();
		}
		static void Write()
		{
			if (data != nullptr)
			{
				for (const auto& data : data->holderMap)
				{
					if (data.second->GetStreamOption().autoStreamOutIfFullVec)
						data.second->Write();
				}
			}
		}
		static void ClearData()
		{
			Write();
			data = nullptr;
		}
		static bool CanUse()
		{
			return 	isActivated;
		}
		static Core::JPublicLogHolder::StreamOutOption CreatetreamOption(const std::string& name)
		{
			Core::JPublicLogHolder::StreamOutOption option;
			if (JApplicationEngine::GetApplicationState() == J_APPLICATION_STATE::PROJECT_SELECT)
			{
				option.path = JCUtil::WstrToU8Str(JApplicationEngine::LogPath()) + "\\" + name + ".txt";
				option.autoStreamOutIfFullVec = true;
				option.overWrite = true;
			}
			else
			{
				option.path = JCUtil::WstrToU8Str(JApplicationProject::DevelopLogPath()) + "\\" + name + ".txt";
				option.autoStreamOutIfFullVec = true;
				option.overWrite = true;
			}
			return option;
		}
	}
	uint JDevelopDebug::GetPublicLogSetCount() noexcept
	{
		if constexpr (Private::developMode)
			return (uint)Private::GetData()->holderMap.size();
		else
			return invalidIndex;
	}
	std::vector<Core::JLogBase*> JDevelopDebug::GetLogVec(const std::string& name)
	{
		if constexpr (Private::developMode)
		{
			auto handler = Private::GetHandle(name);
			return handler != nullptr ? handler->GetLogVec() : std::vector<Core::JLogBase*>{};
		}
		else
			return std::vector<Core::JLogBase*>{};
	}
	bool JDevelopDebug::HasLogHandler(const std::string& name)
	{
		if constexpr (Private::developMode)
			return Private::GetHandle(name) != nullptr;
		return false;
	}
	void JDevelopDebug::PushLog(const std::string& contents)
	{
		if constexpr (Private::developMode)
		{
			if (!Private::CanUse())
				return;

			std::string logName = Private::GetData()->defaultLogNameStack.top();
			auto handler = Private::GetHandle(logName);
			if (handler == nullptr)
			{
				if (logName == Constants::defualtLogHandlerName)
				{
					CreatePublicLogHandler(Constants::defualtLogHandlerName);
					handler = Private::GetHandle(Constants::defualtLogHandlerName);
				}
				else
					return;
			}
			handler->PushLog(std::make_unique<Core::JLogBase>("", contents));
		}
	}
	void JDevelopDebug::PushLog(const std::wstring& contents)
	{
		if constexpr (Private::developMode)
		{
			if (!Private::CanUse())
				return;
			PushLog(JCUtil::WstrToU8Str(contents));
		}
	}
	void JDevelopDebug::PushLog(const std::string& logName, const std::string& contents)
	{
		if constexpr (Private::developMode)
		{
			if (!Private::CanUse())
				return;

			auto handler = Private::GetHandle(logName);
			if (handler == nullptr)
				return;
			handler->PushLog(std::make_unique<Core::JLogBase>("", contents));
		}
	}
	void JDevelopDebug::PushLog(const std::string& logName, const std::wstring& contents)
	{
		if constexpr (Private::developMode)
			PushLog(logName, JCUtil::WstrToU8Str(contents));
	}
	bool JDevelopDebug::CreatePublicLogHandler(const std::string& name)
	{
		if constexpr (Private::developMode)
		{
			if (!Private::CanUse() || !Core::JPublicLogHolder::IsUniqueName(name))
				return false;

			Private::GetData()->holderMap.emplace(name, std::make_unique<Core::JPublicLogHolder>(name));
			auto data = Private::GetData()->holderMap.find(name);
			if (Private::canStreamOut)
				data->second->SetOption(Private::CreatetreamOption(name));

			std::string time = Core::JRealTime::GetNowTime().ToString();
			data->second->PushLog(std::make_unique<Core::JLogBase>("", name + "  Time: " + time));
			return true;
		}
		else
			return false;
	}
	bool JDevelopDebug::DestroyPublicLogHandler(const std::string& name)
	{
		if constexpr (Private::developMode)
		{
			if (!Private::CanUse())
				return false;

			auto& map = Private::GetData()->holderMap;
			auto data = map.find(name);
			if (data == map.end())
				return false;

			map.erase(name);
			return true;
		}
		else
			return false;
	}
	bool JDevelopDebug::PushDefaultLogHandler(const std::string& name)
	{
		if constexpr (Private::developMode)
		{
			if (Private::GetHandle(name) == nullptr)
				return false;
			Private::GetData()->defaultLogNameStack.push(name);
		}
		else
			return false;
	}
	bool JDevelopDebug::PopDefaultLogHandler(const std::string& name)
	{
		if constexpr (Private::developMode)
		{
			if (name == Constants::defualtLogHandlerName || Private::GetData()->defaultLogNameStack.top() != name)
				return false; 

			Private::GetData()->defaultLogNameStack.pop();
			return true;
		}
		else
			return false;
	}
	void JDevelopDebug::Write()
	{
		if constexpr (Private::developMode)
		{
			if (!Private::CanUse())
				return;
			Private::Write();
		}
	}
	void JDevelopDebug::Clear()
	{
		if constexpr (Private::developMode)
		{
			if (!Private::CanUse())
				return;
			Private::ClearData();
		}
	}
	void JDevelopDebug::Activate()
	{ 
		if constexpr (Private::developMode)
			Private::isActivated = true;
	}
	void JDevelopDebug::DeActivate()
	{
		if constexpr (Private::developMode)
		{
			Clear();
			Private::isActivated = false;
		}
	}
	bool JDevelopDebug::IsActivate()
	{
		if constexpr (Private::developMode)
			return Private::isActivated;
		else
			return false;
	}
}