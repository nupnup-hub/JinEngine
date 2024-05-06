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
#include"JResourceObjectType.h"
#include"../JObjectFlag.h"
#include<direct.h>	  

namespace JinEngine
{
	class JDirectory;
	class JResourceObject;

	namespace Core
	{
		struct JAssetFilePathData;
	}
	class JResourceObjectIO
	{ 
	public:
		JResourceObjectIO();
		~JResourceObjectIO();
	public:
		JUserPtr<JDirectory> LoadRootDirectory(const std::wstring& path, const J_OBJECT_FLAG initFlag);
		void LoadEngineDirectory(const JUserPtr<JDirectory>& engineRootDir);
		void LoadEngineResource(const JUserPtr<JDirectory>& engineRootDir);
		void LoadProjectDirectory(const JUserPtr<JDirectory>& projectRootDir);
		void LoadProjectResourceFile(const JUserPtr<JDirectory>& projectRootDir);
		void LoadProjectLastOpendScene(const JUserPtr<JDirectory>& projectRootDir);
	private:
		void SearchDirectory(const JUserPtr<JDirectory>& parentDir, const bool searchProjectFolder, const bool onlyDeafultFolder, const J_OBJECT_FLAG subDirFlag);
		void SearchFile (const J_RESOURCE_TYPE rType, const JUserPtr<JDirectory>& directory, const bool canLoadResource);
	private:
		void LoadFile(const J_RESOURCE_TYPE rType, const JUserPtr<JDirectory>& directory, const std::wstring& fileName, const bool canLoadResource);
	};
}
