/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


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