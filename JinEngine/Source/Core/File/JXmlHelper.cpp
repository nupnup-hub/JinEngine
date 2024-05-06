/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#include"JXmlHelper.h"
#include"../../../ThirdParty/tinyxml2-master/tinyxml2.h" 

namespace JinEngine
{
	using namespace tinyxml2;
	namespace Core
	{
		namespace
		{
			static void LoadNodeData(XMLDocument* doc, XMLElement* pElement, const int pIndex, _Out_ std::vector<JXmlNode>& node)
			{
				XMLElement* child = nullptr;
				if (pElement == nullptr)
					child = doc->FirstChildElement();
				else
					child = pElement->FirstChildElement();

				while (child != nullptr)
				{
					JXmlNode newNode = JXmlNode(child->Name(), pIndex);
					if (child->GetText() != 0)
						newNode.contents = child->GetText();

					const XMLAttribute* attribute = child->FirstAttribute();
					while (attribute != nullptr)
					{
						newNode.attribute.push_back(JXmlAttribute(attribute->Name(), attribute->Value()));
						attribute = attribute->Next();
					}

					node.push_back(std::move(newNode));
					LoadNodeData(doc, child, node.size() - 1, node);
					child = child->NextSiblingElement();
				}
			}
			static void StoreNodeData(XMLDocument* doc, XMLElement* pElement, const int pIndex, _In_ const std::vector<JXmlNode>& nodeVec)
			{
				const int nodeCount = (int)nodeVec.size();
				for (int i = pIndex + 1; i < nodeCount; ++i)
				{
					if (nodeVec[i].parentIndex == pIndex)
					{
						const JXmlNode* node = &nodeVec[i];
						XMLElement* cElement = doc->NewElement(node->name.c_str());

						if (pElement == nullptr)
							doc->LinkEndChild(cElement);
						else
							pElement->LinkEndChild(cElement);

						if (!node->contents.empty())
							cElement->SetText(node->contents.c_str());

						for (const auto& data : node->attribute)
							cElement->SetAttribute(data.name.c_str(), data.value.c_str());

						StoreNodeData(doc, cElement, i, nodeVec);
					}
				}
			}
		}

		JXmlAttribute::JXmlAttribute(const std::string& name, const std::string& value)
			:name(name), value(value)
		{}

		JXmlNode::JXmlNode(const std::string& name, const int parentIndex)
			: name(name), parentIndex(parentIndex)
		{}
		JXmlNode::JXmlNode(const std::string& name, const int parentIndex, const std::string& contents)
			: name(name), parentIndex(parentIndex), contents(contents)
		{}
		JXmlNode::JXmlNode(const std::string& name, const int parentIndex, const std::vector<JXmlAttribute>& attribute)
			: name(name), parentIndex(parentIndex), attribute(attribute)
		{}
		JXmlNode::JXmlNode(const std::string& name, const int parentIndex, const std::string& contents, const std::vector<JXmlAttribute>& attribute)
			: name(name), parentIndex(parentIndex), contents(contents), attribute(attribute)
		{}

		JXmlUserInfo::JXmlUserInfo(const std::string& path)
			: path(path)
		{}

		bool JXmlIOHelper::Load(_Inout_ JXmlUserInfo& userInfo, _Out_ std::vector<JXmlNode>& node)
		{
			XMLDocument doc;
			XMLError err = doc.LoadFile(userInfo.path.c_str());
			if (err != 0)
			{
				userInfo.err = "can't open xml file, it is invalid path";
				return false;
			}
			LoadNodeData(&doc, nullptr, invalidIndex, node);
			return true;
		}
		bool JXmlIOHelper::Store(_Inout_ JXmlUserInfo& userInfo, _In_ const std::vector<JXmlNode>& node)
		{
			XMLDocument* doc = new XMLDocument();
			if (!doc)
			{
				userInfo.err = "can't create xml document";
				return false;
			}

			XMLDeclaration* decl = doc->NewDeclaration();
			doc->InsertFirstChild(decl);

			if (node.size() > 0)
			{
				StoreNodeData(doc, nullptr, invalidIndex, node);
				doc->LinkEndChild(doc->FirstChildElement());
			}
			XMLError err = doc->SaveFile(userInfo.path.c_str());
			if (err != 0)
			{
				userInfo.err = "can't save xml file";
				return false;
			}
			return true;
		}
	}
}