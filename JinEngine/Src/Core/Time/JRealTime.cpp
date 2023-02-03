#include"JRealTime.h"
#include<chrono>

namespace JinEngine
{
	namespace Core
	{
		JRealTime::JTime::JTime(const int year, const int month, const int day, const int hour, const int minute, const int sec)
			:year(year), month(month), day(day), hour(hour), minute(minute), sec(sec)
		{}
		std::string JRealTime::JTime::ToString()const noexcept
		{
			std::string result = std::to_string(year) + "-";

			if (month < 10)
				result += std::to_string(0) + std::to_string(month) + "-";
			else
				result += std::to_string(month) + "-";

			if (day < 10)
				result += std::to_string(0) + std::to_string(day) + "-";
			else
				result += std::to_string(day) + "-";

			if (hour < 10)
				result += std::to_string(0) + std::to_string(hour) + ":";
			else
				result += std::to_string(hour) + ":";

			if (minute < 10)
				result += std::to_string(0) + std::to_string(minute) + ":";
			else
				result += std::to_string(minute) + ":";

			if (sec < 10)
				result += std::to_string(0) + std::to_string(sec);
			else
				result += std::to_string(sec);
			return result;
		}

		JRealTime::JTime JRealTime::GetNowTime()noexcept
		{
			struct tm newtime; 
			__time64_t long_time; 
			errno_t err;

			// Get time as 64-bit integer.
			_time64(&long_time);
			// Convert to local time.
			err = _localtime64_s(&newtime, &long_time);
			if (err)
			{
				printf("Invalid argument to _localtime64_s.");
				exit(1);
			}
			
			return JTime(newtime.tm_year + 1900,
				newtime.tm_mon + 1,
				newtime.tm_mday, 
				newtime.tm_hour,
				newtime.tm_min,
				newtime.tm_sec);
		} 
		std::string JRealTime::GetTime()noexcept
		{
			std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
			auto duration = now.time_since_epoch();

			typedef std::chrono::duration<int, std::ratio_multiply<std::chrono::hours::period, std::ratio<8>>::type> Days;
			// UTC: +8:00
			Days days = std::chrono::duration_cast<Days>(duration);
			duration -= days;
			auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
			duration -= hours;
			auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
			duration -= minutes;
			auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);

			return "hour: " + std::to_string(hours.count())
				+ "minute: " + std::to_string(minutes.count())
				+ "second: " + std::to_string(seconds.count());
		}
		std::string JRealTime::GetMiliTime()noexcept
		{
			std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
			auto duration = now.time_since_epoch();

			typedef std::chrono::duration<int, std::ratio_multiply<std::chrono::hours::period, std::ratio<8>>::type> Days;
			// UTC: +8:00
			Days days = std::chrono::duration_cast<Days>(duration);
			duration -= days;
			auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
			duration -= hours;
			auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
			duration -= minutes;
			auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
			duration -= seconds;
			auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

			return "hour: " + std::to_string(hours.count())
				+ "minute: " + std::to_string(minutes.count())
				+ "second: " + std::to_string(seconds.count())
				+ "milli: " + std::to_string(milliseconds.count());
		}
		std::string JRealTime::GetMicroTime()noexcept
		{
			std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
			auto duration = now.time_since_epoch();

			typedef std::chrono::duration<int, std::ratio_multiply<std::chrono::hours::period, std::ratio<8>>::type> Days;
			// UTC: +8:00
			Days days = std::chrono::duration_cast<Days>(duration);
			duration -= days;
			auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
			duration -= hours;
			auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
			duration -= minutes;
			auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
			duration -= seconds;
			auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
			duration -= milliseconds;
			auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration);

			return "hour: " + std::to_string(hours.count())
				+ "minute: " + std::to_string(minutes.count())
				+ "second: " + std::to_string(seconds.count())
				+ "milli: " + std::to_string(milliseconds.count())
				+ "micro: " + std::to_string(microseconds.count());
		}
		std::string JRealTime::GetNanoTime()noexcept
		{
			std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
			auto duration = now.time_since_epoch();

			typedef std::chrono::duration<int, std::ratio_multiply<std::chrono::hours::period, std::ratio<8>>::type> Days;
			// UTC: +8:00
			Days days = std::chrono::duration_cast<Days>(duration);
			duration -= days;
			auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
			duration -= hours;
			auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
			duration -= minutes;
			auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
			duration -= seconds;
			auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
			duration -= milliseconds;
			auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration);
			duration -= microseconds;
			auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);

			return "hour: " + std::to_string(hours.count())
				+ "minute: " + std::to_string(minutes.count())
				+ "second: " + std::to_string(seconds.count())
				+ "milli: " + std::to_string(milliseconds.count())
				+ "micro: " + std::to_string(microseconds.count())
				+ "nano: " + std::to_string(nanoseconds.count());
		}
	}
}