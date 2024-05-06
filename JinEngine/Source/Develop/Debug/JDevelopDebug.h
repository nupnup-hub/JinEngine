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