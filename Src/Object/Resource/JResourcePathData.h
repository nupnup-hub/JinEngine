#pragma once
#include<string>

namespace JinEngine
{
	struct JResourcePathData
	{
	public:
		const std::wstring wstrPath;
		std::string strPath;
		std::string folderPath;
		std::string name;
		std::string fullName;
		std::string format;
	public:
		JResourcePathData(const std::wstring& oriPath);
	};
}