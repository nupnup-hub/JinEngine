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