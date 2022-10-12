#include"JRealTime.h"
#include<chrono>

namespace JinEngine
{
	namespace Core
	{
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