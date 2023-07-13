#include"JJSon.h"
#include<fstream>

namespace JinEngine::Core
{
	JJSon::JJSon(const std::wstring& path)
		:path(path)
	{}
	bool JJSon::Load()
	{ 
		value.clear();
		std::ifstream file;
		file.open(path.c_str(), std::ios::in | std::ios::binary);
		if (!file.is_open())
			return false;

		Json::Value json;
		Json::CharReaderBuilder builder;
		std::string errors;
		if (!Json::parseFromStream(builder, file, &json, &errors))
		{
			if (file.is_open())
				file.close();
			return false;
		}

		file.close(); 
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
		writer->write(value, &file);
		file.close();
		return true;
	}
}