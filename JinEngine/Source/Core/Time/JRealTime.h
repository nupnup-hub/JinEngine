#pragma once 
#include"../JCoreEssential.h"

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
				const size_t orderNumber;
			public:
				JTime(const int year, const int month, const int day, const int hour, const int minute, const int sec);
				bool operator>(const JTime& rhs)const noexcept;
				bool operator<(const JTime& rhs)const noexcept;
			public:
				std::string ToString()const noexcept; 
			private:
				size_t CalOrderNumer()const noexcept;
			};
		public:
			static JTime GetNowTime()noexcept; 
		};
	}
}