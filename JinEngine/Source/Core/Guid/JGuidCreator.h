#pragma once
#include"../JCoreEssential.h" 
#include <chrono>   

namespace JinEngine
{
	namespace Core
	{ 
		class JGuidCreator
		{
		public:
			static size_t MakeGuid()noexcept;
		};

		static size_t MakeGuid()noexcept
		{
			return JGuidCreator::MakeGuid();
		}
	}
}