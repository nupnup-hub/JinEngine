#pragma once
#include"../JDataType.h"
#include"../Time/JRealTime.h"
#include"../../Utility/JMacroUtility.h"
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
			bool pushSpacePreBody;
			const Core::JRealTime::JTime time;
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
			void AddTitle(const std::string postTitle)noexcept;
			void AddBody(const std::string postBody)noexcept;
		};

		class JLogHandler
		{
		private:
			static constexpr uint initCapacity = 50;
			static constexpr uint maxCapacity = 1000;
		private: 
			int index = 0;
		private:
			std::vector<std::unique_ptr<JLogBase>> logVec;
		public:
			std::vector<JLogBase*> GetLogVec()const noexcept;
			std::vector<JLogBase*> GetLogVec(const bool useSort)noexcept;
			void SetCapacity(const uint newCapacity)noexcept;
		public:
			void PushLog(std::unique_ptr<JLogBase> newLog)noexcept;
			void Clear()noexcept; 
		private:
			void Sort()noexcept;
		};
	} 

}