#pragma once
#include<unordered_map>
namespace JinEngine
{
	namespace Core
	{
		struct JFSMLoadGuidMap
		{
		public:
			std::unordered_map<size_t, size_t> diagram;
			std::unordered_map<size_t, size_t> state;
			std::unordered_map<size_t, size_t> condition;
			bool isNewGuid = false;
		};
	}
}