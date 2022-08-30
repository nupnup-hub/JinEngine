#pragma once
#include<string>

namespace JinEngine
{
	struct JDirectoryPathData
	{
	public:
		const std::wstring wstrPath;
		std::string strPath;
		std::wstring folderPath;
		std::wstring name;
	public:
		JDirectoryPathData(const std::wstring& oripath);
	};
}