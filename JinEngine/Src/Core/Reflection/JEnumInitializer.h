#pragma once
#include<string>
#include<unordered_map>

namespace JinEngine
{
	namespace Core
	{
		class JEnumInfo; 

		using EnumNameMap = std::unordered_map<int, std::string>;
		using EnumElementVec = std::vector<int>;
		using EnumNameVec = std::vector<std::string>;

		class JEnumInitializer
		{
		private:
			friend class JEnumInfo; 
		private:
			const std::string name;
			const std::string fullName;
			const std::string element;
			const size_t enumSize;
			const EnumNameMap enumElementMap;
			const EnumElementVec enumElementVec;
			const bool isEnumClass;
			const bool isTwoSqureEnum;
		public:
			JEnumInitializer(const std::string& name, const std::string& fullName, const std::string element, const size_t enumSize, bool isEnumClass)
				:name(name), 
				fullName(fullName), 
				element(element), 
				enumSize(enumSize), 
				enumElementMap(CreateEnumMap()), 
				enumElementVec(CreateEnumVec()),
				isEnumClass(isEnumClass),
				isTwoSqureEnum(IsTwoSqureEnum())
			{}
		private: 
			EnumNameMap CreateEnumMap()noexcept;
			EnumElementVec CreateEnumVec()noexcept;
		private:
			bool IsTwoSqureEnum()const noexcept;
		};
	}
}