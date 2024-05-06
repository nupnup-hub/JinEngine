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


#include"JMeshFileLoader.h"
#include"../../../../File/JFileIOHelper.h"
#include"../../../../File/JFilePathData.h"

namespace JinEngine::Core
{
	//¹Ì±¸Çö
	bool JMeshFileLoader::LoadBinFile(const JFileImportPathData& pathData, JBinFileMeshData& binMeshData, std::vector<JBinFileMaterialData>& binMatData)
	{
		std::ifstream stream;
		stream.open(pathData.oriFileWPath, std::ios::binary | std::ios::in);
		if (!stream.is_open())
			return false;

		uint32 vertexCount = 0;
		uint32 indexCount = 0;

		stream.read((char*)&vertexCount, sizeof(uint32));
		stream.read((char*)&indexCount, sizeof(uint32));


		stream.close();
		return false;
	}
}