#pragma once
#include<string>

namespace JinEngine
{
	struct JResourcePathData
	{
	public:
		const std::wstring wstrPath;
		std::string strPath;
		//ex) Drive:A\\B
		std::string folderPath;
		//ex) Drive:A\\B\\name
		std::string name;
		//ex) Drive:A\\B\\name.format
		std::string format;
	public:
		JResourcePathData(const std::wstring& oriPath);
	};
}