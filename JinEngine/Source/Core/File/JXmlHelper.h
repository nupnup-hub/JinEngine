#pragma once
#include<string>
#include<vector>
#include<unordered_map>
#include"../JCoreEssential.h"

namespace JinEngine
{
	namespace Core
	{
		struct JXmlAttribute
		{
		public:
			std::string name;
			std::string value;
		public:
			JXmlAttribute(const std::string& name, const std::string& value);
		};
		//unuse
		struct JXmlNode
		{
		public: 
			int parentIndex = invalidIndex;
		public:
			std::string name;					//element
			std::string contents;
			std::vector<JXmlAttribute> attribute;
		public:
			JXmlNode(const std::string& name, const int parentIndex);
			JXmlNode(const std::string& name, const int parentIndex, const std::string& contents);
			JXmlNode(const std::string& name, const int parentIndex, const std::vector<JXmlAttribute>& attribute);
			JXmlNode(const std::string& name, const int parentIndex, const std::string& contents, const std::vector<JXmlAttribute>& attribute);
		};
		struct JXmlUserInfo
		{
		public:
			std::string path;
			std::string err;	//out
		public:
			JXmlUserInfo() = default;
			JXmlUserInfo(const std::string& path);
		};

		class JXmlIOHelper
		{
		public:
			static bool Load(_Inout_ JXmlUserInfo& userInfo, _Out_ std::vector<JXmlNode>& node);
			static bool Store(_Inout_ JXmlUserInfo& userInfo, _In_ const std::vector<JXmlNode>& node);
		};
	}
}