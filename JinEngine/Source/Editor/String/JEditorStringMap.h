#pragma once 
#include"../../Core/JCoreEssential.h"
 
namespace JinEngine
{
	namespace Editor
	{
		//manage editor string per language type
		class JEditorStringMap
		{
		private:
			std::unordered_map<size_t, std::vector<std::string>> strMap;
		public:
			//Add String
			//0.. English 1.. Korean
			bool AddString(const size_t key, const std::vector<std::string>& strVec)noexcept;
			const std::string GetString(const size_t key)const noexcept;
		};
	}
}