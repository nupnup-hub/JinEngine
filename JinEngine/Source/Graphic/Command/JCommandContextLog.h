#pragma once
#include"../../Core/JCoreEssential.h" 
#include<string>
#include<vector>

namespace JinEngine
{
	namespace Graphic
	{
		struct JCommandContextLog
		{
		public:
			std::string name; 
			float lastUpdatedMsTime = 0;
			bool isLastUpdaetd = false;
		public:
			JCommandContextLog() = default;
			JCommandContextLog(const std::string& name);
		};
	}
}