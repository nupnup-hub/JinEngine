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