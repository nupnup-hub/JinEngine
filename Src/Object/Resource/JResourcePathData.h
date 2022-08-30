#pragma once
#include<string>

namespace JinEngine
{
	struct JResourcePathData
	{
	public:
		const std::wstring wstrPath;
		std::string strPath;
		std::wstring folderPath;
		std::wstring name;
		std::wstring fullName;
		std::wstring format;
	public:
		JResourcePathData(const std::wstring& oriPath);
	};
}