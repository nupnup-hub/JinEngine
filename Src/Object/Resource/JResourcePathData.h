#pragma once
#include<string>

namespace JinEngine
{
	struct JAssetFileLoadPathData
	{
	public:
		const std::wstring engineFilePath;
		const std::wstring engineMetaFilePath;
		std::wstring folderPath;
		std::wstring name;
	public:
		JAssetFileLoadPathData(const std::wstring& engineFilePath);
	};

	struct JFileImportPathData
	{
	public:
		const std::wstring oriFileWPath;
		const std::string oriFilePath;
		std::wstring engineFilePath;
		std::wstring engineMetaFilePath;
		std::wstring folderPath;
		std::wstring name;
		std::wstring fullName;
		std::wstring format;
	public:
		JFileImportPathData(const std::wstring& oriPath);
	};
}