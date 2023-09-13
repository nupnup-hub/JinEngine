#pragma once
#include"../../../ThirdParty/jsoncpp-master/json.h"
#include<string>

namespace JinEngine
{
	namespace Core
	{
		class JJSon
		{
		public:
			std::wstring path;
			Json::Value value; 
		public:
			JJSon(const std::wstring& path);
		public:
			bool Load();
			bool Store();
		public:
			template<typename Enum>
			Enum LoadEnum(const std::string& label)
			{
				return (Enum)value[label].asInt();
			}
			template<typename Enum>
			void StoreEnum(const std::string& label, Enum e)
			{
				value[label] = (int)e;
			}
		};
	}
}