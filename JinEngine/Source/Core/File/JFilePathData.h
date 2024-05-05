#pragma once 
#include"../JCoreEssential.h"

namespace JinEngine
{
	namespace Core
	{
		struct JFilePathData
		{
		public:
			std::wstring name;
			std::wstring fullName;
			std::wstring folderPath;
			std::wstring path;
			std::wstring format;
		public:
			JFilePathData(const std::wstring& oriPath);
		};

		struct JAssetFilePathData : public JFilePathData
		{
		public: 
			std::wstring metaFilePath; 
		public:
			JAssetFilePathData(const std::wstring& engineFilePath);
		};

		struct JFileImportPathData : public JAssetFilePathData
		{
		public:
			const std::wstring oriFileWPath;
			const std::string oriFilePath; 
		public:
			std::wstring oriFileFormat;
		public:
			int flag;
		public:
			JFileImportPathData(const std::wstring& oriPath, const int flag = 0);
		};
	}
}