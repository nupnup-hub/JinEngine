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
		};
	}
}