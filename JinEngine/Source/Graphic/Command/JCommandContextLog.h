#pragma once
#include"../../Core/JCoreEssential.h"  

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