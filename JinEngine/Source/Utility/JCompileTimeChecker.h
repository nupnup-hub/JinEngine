#pragma once

namespace JinEngine
{
	template<bool> struct JCompileTimeChecker;
	template<> struct JCompileTimeChecker<true> {};

#define STATIC_CHECKER(expr, msg) \
	{JCompileTimeChecker<((expr) != 0)> ERROR_##msg; (void)ERROR_##msg; } 
}