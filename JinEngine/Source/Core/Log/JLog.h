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


#pragma once
#include"../JCoreEssential.h"
#include"../Time/JRealTime.h"
#include"../Utility/JMacroUtility.h" 

namespace JinEngine
{
	namespace Core
	{
		class JLogBase
		{
		private:
			std::string title;
			std::string body;
			const Core::JRealTime::JTime time;
		private:
			bool pushSpacePreBody;
		public:
			JLogBase(const std::string& title, const std::string& body = "", const bool pushSpacePreBody = true);
			JLogBase(const std::wstring& title, const std::wstring& body = L"", const bool pushSpacePreBody = true);
			bool operator>(const JLogBase& data)const noexcept;
			bool operator<(const JLogBase& data)const noexcept;
		public:
			J_SIMPLE_GET(Core::JRealTime::JTime, time, Time); 
			J_SIMPLE_GET(std::string, title, Title);
			J_SIMPLE_GET(std::string, body, Body);
		public: 
			std::string GetLog()const noexcept;
		public:
			bool HasTitle()const noexcept;
			bool HasContents()const noexcept;
		public:
			void AddTitle(const std::string postTitle)noexcept;
			void AddBody(const std::string postBody)noexcept;
		};
	} 

}