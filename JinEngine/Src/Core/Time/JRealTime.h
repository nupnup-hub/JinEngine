#pragma once
#include<string>

namespace JinEngine
{
	namespace Core
	{
		class JRealTime
		{
		public:
			class JTime
			{
			public:
				const int year;
				const int month;
				const int day;
				const int hour;
				const int minute;
				const int sec;
			public:
				JTime(const int year, const int month, const int day, const int hour, const int minute, const int sec);
			public:
				std::string ToString()const noexcept;
			};
		public:
			static JTime GetNowTime()noexcept;
			static std::string GetTime()noexcept;
			static std::string GetMiliTime()noexcept;
			static std::string GetMicroTime()noexcept;
			static std::string GetNanoTime() noexcept;
		};
	}
}