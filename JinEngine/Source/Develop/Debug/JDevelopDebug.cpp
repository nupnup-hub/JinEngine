#include"JDevelopDebug.h"
#include"JDevelopDebugConstants.h"
#include"../../Core/Utility/JCommonUtility.h"
#include"../../Application/JApplicationProject.h" 
#include"../../Application/JApplicationEngine.h" 
#include<unordered_map>

namespace JinEngine::Develop
{
	namespace Private
	{
		static constexpr bool canStreamOut = true;
		static bool isActivated = true;
		struct JDevelopDebugData
		{
		public:
			std::unordered_map<std::string, std::unique_ptr<Core::JPublicLogHolder>> holderMap;
		};

		static std::unique_ptr<JDevelopDebugData> data;
		static JDevelopDebugData* GetData()
		{ 
			if (data == nullptr)
				data = std::make_unique<JDevelopDebugData>();
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
#ifdef DEVELOP
		return (uint)Private::GetData()->holderMap.size();
#else
		return invalidIndex;
#endif
	} 
	std::vector<Core::JLogBase*> JDevelopDebug::GetLogVec(const std::string& name)
	{
#ifdef DEVELOP
		auto handler = Private::GetHandle(name);
		return handler != nullptr ? handler->GetLogVec() : std::vector<Core::JLogBase*>{};
#else
		return std::vector<Core::JLogBase*>{};
#endif
	}
	void JDevelopDebug::PushLog(const std::string& contents)
	{
#ifdef DEVELOP 
		if (!Private::CanUse())
			return;

		auto handler = Private::GetHandle(Constants::defualtLogHandlerName);
		if (handler == nullptr)
		{
			PushPublicLogHandler(Constants::defualtLogHandlerName);
			handler = Private::GetHandle(Constants::defualtLogHandlerName);
		}
		handler->PushLog(std::make_unique<Core::JLogBase>("", contents));
#else
		return;
#endif
	}
	void JDevelopDebug::PushLog(const std::wstring& contents)
	{
#ifdef DEVELOP 
		if (!Private::CanUse())
			return;
		PushLog(JCUtil::WstrToU8Str(contents));
#else
		return;
#endif
	}
	void JDevelopDebug::PushLog(const std::string& name, const std::string& contents)
	{
#ifdef DEVELOP
		if (!Private::CanUse())
			return;

		auto handler = Private::GetHandle(name);
		if (handler == nullptr)
			return;
		handler->PushLog(std::make_unique<Core::JLogBase>(name, contents));
#else
		return;
#endif
	}
	bool JDevelopDebug::PushPublicLogHandler(const std::string& name)
	{
#ifdef DEVELOP
		if (!Private::CanUse()|| !Core::JPublicLogHolder::IsUniqueName(name))
			return false;

		Private::GetData()->holderMap.emplace(name, std::make_unique<Core::JPublicLogHolder>(name));
		auto data = Private::GetData()->holderMap.find(name);
		if (Private::canStreamOut)
			data->second->SetOption(Private::CreatetreamOption(name));

		std::string time = Core::JRealTime::GetNowTime().ToString();
		data->second->PushLog(std::make_unique<Core::JLogBase>("", name + "  Time: " + time));
		return true;
#else
		return false;
#endif
	}
	bool JDevelopDebug::PopPublicLogHandler(const std::string& name)
	{
#ifdef DEVELOP
		if (!Private::CanUse())
			return false;

		auto& map = Private::GetData()->holderMap;
		auto data = map.find(name);
		if (data == map.end())
			return false;

		map.erase(name);
		return true;
#else
		return false;
#endif
	}
	void JDevelopDebug::Write()
	{
#ifdef DEVELOP
		if (!Private::CanUse())
			return;
		Private::Write();
#else
#endif
	}
	void JDevelopDebug::Clear()
	{
#ifdef DEVELOP
		if (!Private::CanUse())
			return;
		Private::ClearData();
#else
#endif
	}
	void JDevelopDebug::Activate()
	{
#ifdef DEVELOP
		Private::isActivated = true;
#else
#endif
	}
	void JDevelopDebug::DeActivate()
	{
#ifdef DEVELOP
		Clear();
		Private::isActivated = false;
#else
#endif
	}
	bool JDevelopDebug::IsActivate()
	{
#ifdef DEVELOP 
		return Private::isActivated;
#else
#endif
	}


}