#include"JLogHandler.h"
#include"JLog.h"
#include"../Platform/JPlatformInfo.h"
#include<Windows.h> 
#include<algorithm>
#include<fstream>

namespace JinEngine::Core
{
	void JLogPrintOut::JLogPrintOut::PrintOut(const JLogBase& log)
	{
#if defined(OS_WINDOW_64)
		MessageBoxA(0, log.GetBody().c_str(), log.GetTitle().c_str(), 0);
#endif
	}
	void JLogPrintOut::JLogPrintOut::PrintOut(const std::string& file, 
		const std::string& line, 
		const std::string& title, 
		const std::string& contents)
	{ 
		std::string newContents;
		newContents += "File: " + file + '\n';
		newContents += "Line: " + line + '\n';
		newContents += contents;

		PrintOut(JLogBase(title, newContents));
	}
	void JLogPrintOut::JLogPrintOut::PrintOut(const std::wstring& file, 
		const std::wstring& line,
		const std::wstring& title, 
		const std::wstring& contents)
	{ 
		std::wstring newContents;
		newContents += L"File: " + file + L'\n';
		newContents += L"Line: " + line + L'\n';
		newContents += contents;

		PrintOut(JLogBase(title, newContents));
	}

	namespace
	{
		static constexpr uint initCapacity = 50;
		static constexpr uint maxCapacity = 1000;	

		using HolderGroup = std::unordered_map<std::string, JPublicLogHolder*>;
		using GetLogVecPtr = std::vector<JLogBase*>(*)(JLogHolderInterface* holder);

		static HolderGroup group = HolderGroup();
		static GetLogVecPtr getLogVecPtr = GetLogVecPtr();

		static bool AddPublieHolder(JPublicLogHolder* holder)noexcept
		{
			if (holder == nullptr)
				return false;

			auto data = group.find(holder->GetName());
			if (data != group.end())
				return false;

			group.emplace(holder->GetName(), holder);
			return true;
		}
		static void ErasePublicHolder(const std::string& holderName)noexcept
		{
			group.erase(holderName);
		}
		static JPublicLogHolder* GetPublicHolder(const std::string& holderName)noexcept
		{
			if (holderName.empty())
				return nullptr;

			auto data = group.find(holderName);
			return data != group.end() ? data->second : nullptr;
		}
		static std::vector<JLogBase*> GetHolderLogVec(JLogHolderInterface* holder)
		{
			if (getLogVecPtr == nullptr)
				return std::vector<JLogBase*>{};

			return getLogVecPtr(holder);
		}
	}

	class JLogHolderInterface::JLogHolderImpl
	{
	private:
		const std::string name;
		std::vector<std::unique_ptr<JLogBase>> logVec;
		JLogHolderInterface::StreamOutOption streamOption;
		int index = 0;
	public:
		JLogHolderImpl(const std::string& name)
			:name(name)
		{ 
			if (getLogVecPtr == nullptr)
			{
				getLogVecPtr = [](JLogHolderInterface* holder)
				{
					return holder->impl->GetLogVec();
				};
			}
			SetCapacity(initCapacity);
		}
	public:
		std::vector<JLogBase*> GetLogVec()const noexcept
		{
			std::vector<JLogBase*> rLogVec;
			//circulate staet
			if (logVec[index] != nullptr)
			{
				const uint logVecCount = (uint)logVec.size();
				for (int i = index; i < logVecCount; ++i)
					rLogVec.push_back(logVec[i].get());
			}
			for (int i = 0; i < index; ++i)
				rLogVec.push_back(logVec[i].get());
			return rLogVec;
		}
		std::vector<JLogBase*> GetLogVec(const bool useSort)noexcept
		{
			if (useSort)
				Sort();

			return GetLogVec();
		} 
		J_SIMPLE_GET(JLogHolderInterface::StreamOutOption, streamOption, StreamOption)
		J_SIMPLE_GET(std::string, name, Name)
	public:
		void SetCapacity(const uint newCapacity)noexcept
		{
			if (logVec.size() > 0)
				Sort();

			if (newCapacity >= maxCapacity)
				logVec.resize(maxCapacity);
			else
				logVec.resize(newCapacity);
		}
	public:
		void SetOption(const JLogHolderInterface::LogHolderOption& option)noexcept
		{
			if (option.capacity != logVec.size())
			{
				if (logVec.size() > 0)
					Sort();

				if (option.capacity >= maxCapacity)
					logVec.resize(maxCapacity);
				else
					logVec.resize(option.capacity);
			}
		}
		void SetOption(const JLogHolderInterface::StreamOutOption& option)noexcept
		{
			streamOption = option;
		}
	public:
		void Write()
		{
			if (streamOption.path.empty())
				return;

			std::ofstream stream;
			auto mode = (uint)std::ios::out;
			if (streamOption.overWrite)
				mode |= std::ios::app;

			stream.open(streamOption.path, mode);
			if (!stream.is_open())
				return;

			if (!streamOption.guide.empty())
				stream << streamOption.guide << "\n";
			for (const auto& data : logVec)
			{
				if (data == nullptr)
					continue;

				if (streamOption.customFormatPtr != nullptr)
					streamOption.customFormatPtr(data.get(), streamOption);
				else
				{
					if (streamOption.outTitle && data->HasTitle())
						stream << data->GetTitle() << '\n';
					if (streamOption.outBody && data->HasContents())
						stream << data->GetBody() << '\n';
				}
			}

			stream.close();
			if (streamOption.outAfterClear)
				ClearBuffer();
		}
	public:
		void PushLog(std::unique_ptr<JLogBase>&& newLog)noexcept
		{
			logVec[index] = std::move(newLog);
			++index;
			if (index >= logVec.size())
			{
				if (streamOption.autoStreamOutIfFullVec)
					Write();
				index = 0;
			}
		}
		void ClearBuffer()noexcept
		{
			index = 0;
			for (auto& data : logVec)
				data = nullptr;
		}
	public:
		void Sort()noexcept
		{
			bool isCirculateState = logVec[index] != nullptr;
			std::sort(logVec.begin(), logVec.end());
			if (isCirculateState)
				index = logVec.size() - 1;
		}
	};

 
	JLogHolderInterface::JLogHolderInterface(const std::string& name)
		:impl(std::make_unique< JLogHolderImpl>(name))
	{}
	std::string JLogHolderInterface::GetName()const noexcept
	{
		return impl->GetName();
	} 
	JLogHolderInterface::StreamOutOption JLogHolderInterface::GetStreamOption()const noexcept
	{
		return impl->GetStreamOption();
	}
	void JLogHolderInterface::SetOption(const LogHolderOption& option)noexcept
	{ 
		impl->SetOption(option);
	}
	void JLogHolderInterface::SetOption(const StreamOutOption& option)noexcept
	{
		impl->SetOption(option);
	}
	void JLogHolderInterface::Write()
	{
		impl->Write();
	}
	void JLogHolderInterface::Write(const StreamOutOption& option)
	{ 
		impl->SetOption(option);
		impl->Write();
	}
	void JLogHolderInterface::PushLog(std::unique_ptr<JLogBase>&& newLog)noexcept
	{
		impl->PushLog(std::move(newLog));
	} 
	void JLogHolderInterface::ClearBuffer()noexcept
	{
		impl->ClearBuffer();
	}

	JPublicLogHolder::JPublicLogHolder(const std::string& uniqueName)
		:JLogHolderInterface(uniqueName)
	{
		isValid = AddPublieHolder(this);
	}
	JPublicLogHolder::~JPublicLogHolder()
	{
		if (isValid)
			ErasePublicHolder(GetName());
	}
	std::vector<JLogBase*> JPublicLogHolder::GetLogVec()
	{
		return GetHolderLogVec(this);
	}
	std::vector<JLogBase*> JPublicLogHolder::GetLogVec(const std::string& uniqueName)
	{
		auto data = GetPublicHolder(uniqueName);
		if (data == nullptr)
			return std::vector<JLogBase*>{};

		return data->isValid ? GetHolderLogVec(data) : std::vector<JLogBase*>{};
	}
	bool JPublicLogHolder::IsValid()const noexcept
	{
		return isValid;
	}
	bool JPublicLogHolder::IsUniqueName(const std::string& uniqueName)
	{
		return GetPublicHolder(uniqueName) == nullptr;
	}
	void JPublicLogHolder::ClearBuffer(const std::string& uniqueName)
	{
		auto data = GetPublicHolder(uniqueName);
		if (data == nullptr)
			return;

		data->JLogHolderInterface::ClearBuffer();
	}
}