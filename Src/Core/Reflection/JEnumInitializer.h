#pragma once
#include<string>
#include<unordered_map>

namespace JinEngine
{
	namespace Core
	{
		class JEnumInitializer
		{
		public:
			using EnumElementMap = std::unordered_map<int, std::string>;
			using EnumElementVec = std::vector<int>;
		public:
			JEnumInitializer(const std::string& name, const std::string& fullName, const std::string element, const size_t enumSize)
				:name(name), fullName(fullName), element(element), enumSize(enumSize), enumElementMap(CreateEnumMap()), enumElementVec(CreateEnumVec())
			{}
		private:
			EnumElementMap CreateEnumMap()noexcept;
			EnumElementVec CreateEnumVec()noexcept;
		public:
			const std::string name;
			const std::string fullName;
			const std::string element;
			const size_t enumSize;
			const EnumElementMap enumElementMap;
			const EnumElementVec enumElementVec;
		};
	}
}