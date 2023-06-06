#pragma once
#include <chrono>  
#include <string>   

namespace JinEngine
{
	namespace Core
	{ 
		//static std::hash<std::string> hash;
		static size_t MakeGuid()noexcept
		{
			std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
			auto duration = now.time_since_epoch();

			typedef std::chrono::duration<int, std::ratio_multiply<std::chrono::hours::period, std::ratio<8>>::type> Days;
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

			return std::hash<std::string>{}(std::to_string(days.count()) +
				std::to_string(hours.count()) + ":" +
				std::to_string(minutes.count()) + ":" +
				std::to_string(seconds.count()) + ":" +
				std::to_string(milliseconds.count()) + ":" +
				std::to_string(microseconds.count()) + ":" +
				std::to_string(nanoseconds.count()));
		}
	}
}