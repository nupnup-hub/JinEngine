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
#include"JLog.h"

namespace JinEngine
{
	namespace Core
	{  
		class JLogPrintOut
		{ 
		public:
			static void PrintOut(const JLogBase& log);
			static void PrintOut(const std::string& file, const std::string& line, const std::string& title, const std::string& contents);
			static void PrintOut(const std::wstring& file, const std::wstring& line, const std::wstring& title, const std::wstring& contents);
		};

		class JLogHolderInterface
		{   
		public:
			struct StreamOutOption
			{
			public:
				using CustomFormatPtr = std::string(*)(JLogBase*, const JLogHolderInterface::StreamOutOption&);
			public:
				std::string path;
				std::string guide;
			public:
				CustomFormatPtr customFormatPtr = nullptr;
			public:
				bool outAfterClear = true;
				bool overWrite = true;
			public:
				bool outTitle = true;
				bool outBody = true;
			public:
				bool autoStreamOutIfFullVec = true;
			public:
				StreamOutOption() = default;
				StreamOutOption(const std::string& path,
					std::string guide = " ",
					bool outAfterClear = true,
					bool overWrite = true);
				StreamOutOption(const std::wstring& path,
					std::wstring guide = L" ",
					bool outAfterClear = true,
					bool overWrite = true);
			};
			struct LogHolderOption
			{ 
			public:
				size_t capacity = 0; 
			};
		private:
			class JLogHolderImpl;
			std::unique_ptr<JLogHolderImpl> impl;
		public:
			JLogHolderInterface(const std::string& name);
			virtual ~JLogHolderInterface() = default;
		public:
			std::string GetName()const noexcept; 
			StreamOutOption GetStreamOption()const noexcept;
		protected:
			void SetOption(const LogHolderOption& option)noexcept;
			void SetOption(const StreamOutOption& option)noexcept;
		protected:
			void Write();
			void Write(const StreamOutOption& option);
		protected:
			void PushLog(std::unique_ptr<JLogBase>&& newLog)noexcept; 
		protected:
			void ClearBuffer()noexcept;
		};
		 
		//can use JPublicLogHolder static func by name
		class JPublicLogHolder : public JLogHolderInterface
		{
		private:
			bool isValid;
		public:
			JPublicLogHolder(const std::string& uniqueName);
			~JPublicLogHolder();
		public: 
			using JLogHolderInterface::SetOption; 
			using JLogHolderInterface::Write;
			using JLogHolderInterface::PushLog; 
		public: 
			std::vector<JLogBase*> GetLogVec();
			static std::vector<JLogBase*> GetLogVec(const std::string& uniqueName);
		public:
			bool IsValid()const noexcept; 
			static bool IsUniqueName(const std::string& uniqueName);
		public:
			static void ClearBuffer(const std::string& uniqueName);
		};	 
	}
}