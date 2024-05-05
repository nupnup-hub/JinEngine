#pragma once 
#include"../JCoreEssential.h"
#include <sstream>

namespace JinEngine
{
	namespace Core
	{
		struct JCompressInfo
		{
		public:
			struct FileInfo
			{
			public:
				std::string path;
				size_t fileSize = 0;	//write compress  
			public:
				bool decomposeTarget = false;
			public:
				std::stringstream stream;
			};
		public:
			std::vector<FileInfo> fileInfo;
			std::string compressPath;
		public:
			std::string errLog;
		public:
			int preset;
		};

		class JFileCompression
		{
		public:
			static bool Compress(_Inout_ JCompressInfo& info);
			static bool DecompressToFile(_Inout_ JCompressInfo& info);
			static bool DecompressToStream(_Inout_ JCompressInfo& info);
		};
	}
}