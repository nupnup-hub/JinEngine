#pragma once   
#include"JMeshFileLoaderUtility.h" 
#include"../../../../Singleton/JSingletonHolder.h"   
 
//fbx, obj를 제외한 포맷에 대한 load를 지원하도록 한다
//미구현	 
namespace JinEngine 
{ 
	namespace Core
	{
		struct JStaticMeshData;
		struct JFileImportPathData;
		class JMeshFileLoader
		{
		public:
			bool LoadBinFile(const JFileImportPathData& pathData, JBinFileMeshData& binMeshData, std::vector<JBinFileMaterialData>& binMatData);
		};
	}
}