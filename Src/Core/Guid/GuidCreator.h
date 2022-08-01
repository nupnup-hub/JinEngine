#pragma once
#include <chrono>  
#include <string>  

namespace JinEngine
{
	namespace Core
	{ 
		static size_t MakeGuid()
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
 
			std::string time = "hour: " + std::to_string(hours.count())
				+ "minute: " + std::to_string(minutes.count())
				+ "second: " + std::to_string(seconds.count())
				+ "milli: " + std::to_string(milliseconds.count())
				+ "micro: " + std::to_string(microseconds.count())
				+ "nano: " + std::to_string(nanoseconds.count());

			return std::hash<std::string>{}(time);
		}
	}
}

/*
* 	class JShader;
   class JSkeletonAsset;
   struct JSkeleton;

   template<>
   static size_t MakeGuid<JSkeletonAsset>();

   template<>
   static size_t MakeGuid<JSkeleton>();
*/