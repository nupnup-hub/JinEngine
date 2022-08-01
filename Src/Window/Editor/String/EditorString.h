#pragma once
#include<string>
#include<unordered_map>
 
namespace JinEngine
{
	class EditorString
	{
	private:
		std::unordered_map<size_t, std::vector<std::string>> strMap;
	public:
		//Add String
		//0.. English 1.. Korean
		bool AddString(const size_t key, const std::vector<std::string>& strVec);
		const std::string GetString(const size_t key);
	};
}