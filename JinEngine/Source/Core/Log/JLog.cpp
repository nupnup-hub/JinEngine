#include"JLog.h"
#include<algorithm>
#include"../Utility/JCommonUtility.h"

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
		bool JLogBase::HasTitle()const noexcept
		{
			return !title.empty();
		}
		bool JLogBase::HasContents()const noexcept
		{
			return !body.empty();
		}
		void JLogBase::AddTitle(const std::string postTitle)noexcept
		{
			title += postTitle;
		}
		void JLogBase::AddBody(const std::string postBody)noexcept
		{
			body += postBody;
		}
	}
}