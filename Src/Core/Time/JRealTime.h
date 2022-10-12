#pragma once
#include<string>

namespace JinEngine
{
	namespace Core
	{
		class JRealTime
		{
		public:
			static std::string GetTime()noexcept;
			static std::string GetMiliTime()noexcept;
			static std::string GetMicroTime()noexcept;
			static std::string GetNanoTime() noexcept;
		};
	}
}