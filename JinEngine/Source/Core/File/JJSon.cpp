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


#include"JJSon.h"
#include"../Log/JLogMacro.h"
#include<fstream>

namespace JinEngine::Core
{
	namespace Private
	{}
	JJSon::CurrentData::CurrentData(Json::Value& v)
		:iter(v.begin()), ptr(&v)
	{}
	JJSon::JJSon(const std::string& path, const bool isContentsMemberArray)
		:path(path), isContentsMemberArray(isContentsMemberArray)
	{
		if (isContentsMemberArray)
			contents = Json::Value(Json::arrayValue);
		currentStack.emplace(contents);
	}
	JJSon::~JJSon()
	{ 
		//contents.clear();
	}
	bool JJSon::Load()
	{
		while (currentStack.size() > 0)
			currentStack.pop();
		 
		std::ifstream file;
		file.open(path.c_str(), std::ios::in | std::ios::binary);
		if (!file.is_open())
			return false;
		  
		//if (isContentsMemberArray)
		//	contents = Json::Value(Json::arrayValue);

		Json::CharReaderBuilder builder;
		std::string errors;
		if (!Json::parseFromStream(builder, file, &contents, &errors))
		{
			J_LOG_PRINT_OUT("Json parse error", errors);
			if (file.is_open())
				file.close();
			return false;
		}

		file.close();   
		currentStack.emplace(contents);  
		return true;
	}
	bool JJSon::Store()
	{  
		std::ofstream file;
		file.open(path.c_str(), std::ios::out | std::ios::binary);
		if (!file.is_open())
			return false;
		 
		Json::StreamWriterBuilder builder;
		builder["indentation"] = "\t";
		std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
		writer->write(contents, &file);
		file.close(); 
		return true;
	}
	bool JJSon::PushArrayOwner(const std::string& key)
	{
		if (currentStack.size() == 0)
			return false;
 
		CurrentData& current = currentStack.top();
		const bool isArray = current.ptr->isArray();

		if (isArray)
			currentStack.emplace(current.ptr->append(Json::Value(Json::arrayValue)));
		else
		{
			(*current.ptr)[key] = Json::Value(Json::arrayValue);
			currentStack.emplace((*current.ptr)[key]);
		}
		return true;
	}
	bool JJSon::PushArrayMember()
	{
		if (currentStack.size() == 0)
			return false;

		CurrentData& current = currentStack.top(); 
		if (!current.ptr->isArray())
			return false;

		currentStack.emplace(current.ptr->append(Json::Value()));
		return true;
	}
	bool JJSon::PushMapMember(const std::string& key)
	{
		if (currentStack.size() == 0)
			return false;

		CurrentData& current = currentStack.top(); 
		if (current.ptr->isArray())
			return false;

		currentStack.emplace((*current.ptr)[key]);
		return true;
	}
	bool JJSon::PushExistStack(const std::string& key)
	{
		if (currentStack.size() == 0)
			return false;

		CurrentData& current = currentStack.top();
		const bool isArray = current.ptr->isArray();

		if (current.ptr->isNull())
			return false;

		if (isArray)
		{ 
			int index = current.iter.index();
			if (!current.ptr->isValidIndex(index))
				return false;

			currentStack.emplace((*current.ptr)[index]);
			++current.iter;
		}
		else
		{
			Json::Value& value = (*current.ptr)[key];
			if (value.isNull())
				return false;

			currentStack.emplace(value);
		}
		return true;
	}
	bool JJSon::PopStack()
	{
		if (currentStack.size() == 0)
			return false;
			 
		currentStack.pop();
		return true;
	}
	uint JJSon::GetCurrentMemberCount()const noexcept
	{
		return currentStack.size() != 0 ? currentStack.top().ptr->size() : 0;
	}
	bool JJSon::IsCurrentHasArrayContainer()const noexcept
	{
		return currentStack.size() != 0 ? currentStack.top().ptr->isArray() : false;
	}
	bool JJSon::HasCurrentStack()const noexcept
	{
		return currentStack.size() != 0;
	}
}