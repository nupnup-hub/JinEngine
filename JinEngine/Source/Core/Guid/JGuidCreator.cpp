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


#include"JGuidCreator.h"
#include <random>
#include <sstream>
#include <chrono> 

namespace JinEngine::Core
{
	namespace
	{
		static size_t CreateGuidUseTime()noexcept
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
		static size_t CreateGuidUseTimeAndRand()noexcept
		{  
			auto now = std::chrono::system_clock::now();
			auto timestamp = now.time_since_epoch().count();
			  
			//version
			timestamp |= 0x1000000000000000;
			 
			static std::random_device rd;
			static std::mt19937 gen(rd());
			static std::uniform_int_distribution<size_t> dis(0, SIZE_MAX);

			return std::hash<std::string>{}(std::to_string(timestamp) + std::to_string(dis(gen)));
		}
	}
	size_t JGuidCreator::MakeGuid()noexcept
	{
		return CreateGuidUseTimeAndRand();
	}
}