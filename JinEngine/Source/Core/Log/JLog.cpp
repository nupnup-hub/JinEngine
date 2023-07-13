#include"JLog.h"
#include<algorithm>
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Core
	{
		JLogBase::JLogBase(const std::string& title, const std::string& body, const bool pushSpacePreBody)
			:title(title), body(body), pushSpacePreBody(pushSpacePreBody), time(JRealTime::GetNowTime())
		{}
		JLogBase::JLogBase(const std::wstring& title, const std::wstring& body, const bool pushSpacePreBody)
			: title(JCUtil::WstrToU8Str(title)), body(JCUtil::WstrToU8Str(body)), pushSpacePreBody(pushSpacePreBody), time(JRealTime::GetNowTime())
		{}
		bool JLogBase::operator>(const JLogBase& data)const noexcept
		{
			return GetTime() > data.GetTime();
		}
		bool JLogBase::operator<(const JLogBase& data)const noexcept
		{
			return GetTime() < data.GetTime();
		} 
		std::string JLogBase::GetLog()const noexcept
		{
			if (body.empty())
				return title;
			else
			{
				if (pushSpacePreBody)
					return title + "\n" +body;
				else
					return title + body;
			} 
		}
		void JLogBase::AddTitle(const std::string postTitle)noexcept
		{
			title += postTitle;
		}
		void JLogBase::AddBody(const std::string postBody)noexcept
		{
			body += postBody;
		}

		std::vector<JLogBase*> JLogHandler::GetLogVec()const noexcept
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
		std::vector<JLogBase*> JLogHandler::GetLogVec(const bool useSort)noexcept
		{
			if (useSort)
				Sort();

			return GetLogVec();
		}
		void JLogHandler::SetCapacity(const uint newCapacity)noexcept
		{
			if(logVec.size() > 0)
				Sort();

			if(newCapacity >= maxCapacity)
				logVec.resize(maxCapacity);
			else
				logVec.resize(newCapacity);
		}
		void JLogHandler::PushLog(std::unique_ptr<JLogBase> newLog)noexcept
		{
			logVec[index] = std::move(newLog);
			++index;
			if (index >= logVec.size())
				index = 0;
		}
		void JLogHandler::Clear()noexcept
		{
			index = 0;  
			logVec.clear(); 
		}
		void JLogHandler::Sort()noexcept
		{
			bool isCirculateState = logVec[index] != nullptr; 
			std::sort(logVec.begin(), logVec.end());		 
			if (isCirculateState)
				index = logVec.size() - 1;
		}
	}
}