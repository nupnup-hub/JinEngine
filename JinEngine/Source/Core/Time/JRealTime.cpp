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


#include"JRealTime.h"
#include<chrono>

namespace JinEngine
{
	namespace Core
	{
		JRealTime::JTime::JTime(const int year, const int month, const int day, const int hour, const int minute, const int sec)
			:year(year), month(month), day(day), hour(hour), minute(minute), sec(sec), orderNumber(CalOrderNumer())
		{}
		bool JRealTime::JTime::operator>(const JTime& rhs)const noexcept
		{
			return orderNumber > rhs.orderNumber;
		}
		bool JRealTime::JTime::operator<(const JTime& rhs)const noexcept
		{
			return orderNumber < rhs.orderNumber;
		}
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
		size_t JRealTime::JTime::CalOrderNumer()const noexcept
		{
			return (size_t)sec + 
				((size_t)minute *	100) +
				((size_t)hour   *	10000) +
				((size_t)day    *	1000000) +
				((size_t)month  *	100000000) +
				((size_t)year   *	10000000000);
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
	}
}