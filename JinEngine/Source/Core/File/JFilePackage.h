/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once 
#include"../JCoreEssential.h"

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