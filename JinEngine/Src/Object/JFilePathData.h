#pragma once
#include<string>

namespace JinEngine
{
	namespace Core
	{
		struct JAssetFileLoadPathData
		{
		public:
			const std::wstring engineFilePath;
			const std::wstring engineMetaFilePath;
			std::wstring folderPath;
			std::wstring name;
		public:
			JAssetFileLoadPathData(const std::wstring& engineFilePath, const std::wstring engineMetaFilePath);
		};

		struct JFileImportHelpData
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
			JFileImportHelpData(const std::wstring& oriPath,
				const std::wstring& fileFormat,
				const std::wstring& metafileFormat);
		};
	}
}