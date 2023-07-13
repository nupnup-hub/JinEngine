#pragma once
#include<string>
#include<unordered_map>

namespace JinEngine
{
	namespace Core
	{
		class JFilePackage
		{
		public:
			struct FileInfo
			{
			public:
				const std::wstring name;
				const size_t guid;
				const size_t fileSize;
			public:
				std::wstring oriPath;		//use by CreateTarFile
			public:
				FileInfo(const std::wstring name, const size_t guid, const size_t fileSize);
				FileInfo(const std::wstring name, const size_t guid, const size_t fileSize, const std::wstring& oriPath);
			};
			struct Header
			{
			public:
				std::unordered_map<size_t, FileInfo> fileInfo;
			public:
				bool isEncrypted;
			};
		private:
			std::wstring packagePath;
			Header header;
		public:
			void SetPackagePath(const std::wstring& path);
		public:
			bool AddFile(const std::wstring& path);
			bool PopFile(const std::wstring& path);
		public:
			bool StorePackage(const bool applyEncryption, const bool clearIfSuccess = true);
			bool LoadPackage(Header& header, const bool clearIfSuccess = true);
		public:
			void Clear();
		};
	}
}