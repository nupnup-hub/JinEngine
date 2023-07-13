#pragma once
#include<string>
#include"../../Object/JObjectFlag.h"
namespace JinEngine
{
	namespace Core
	{
		struct JAssetFileLoadPathData
		{
		public:
			const std::wstring engineFileWPath; 
			std::wstring engineMetaFileWPath; 
			std::wstring folderPath;
			std::wstring name;
			std::wstring format;
		public:
			JAssetFileLoadPathData(const std::wstring& engineFilePath);
		};

		struct JFileImportHelpData
		{
		public:
			const std::wstring oriFileWPath; 
			const std::string oriFilePath;
			std::wstring engineFileWPath; 
			std::wstring engineMetaFileWPath; 
			std::wstring folderPath;
			std::wstring name;
			std::wstring fullName;
			std::wstring format;
		public:
			J_OBJECT_FLAG flag;
		public:
			JFileImportHelpData(const std::wstring& oriPath, const J_OBJECT_FLAG flag = OBJECT_FLAG_NONE);
		};
	}
}