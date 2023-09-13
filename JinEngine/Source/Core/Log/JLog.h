#pragma once
#include"../JCoreEssential.h"
#include"../Time/JRealTime.h"
#include"../Utility/JMacroUtility.h"
#include<vector>
#include<memory>

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