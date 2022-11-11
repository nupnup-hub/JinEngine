#pragma once
#include <chrono>  
#include <string>  
#include"../Time/JRealTime.h"

namespace JinEngine
{
	namespace Core
	{ 
		//static std::hash<std::string> hash;
		static size_t MakeGuid()noexcept
		{
			return std::hash<std::string>{}(JRealTime::GetNanoTime());
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