#pragma once   
#include"JMeshFileLoaderUtility.h" 
#include"../../../../Singleton/JSingletonHolder.h"  
#include<string>
#include<vector>
 
//fbx, obj를 제외한 포맷에 대한 load를 지원하도록 한다
//미구현	 
namespace JinEngine 
{ 
	namespace Core
	{
		struct JStaticMeshData;
		struct JFileImportHelpData;
		class JMeshFileLoader
		{
		public:
			bool LoadBinFile(const JFileImportHelpData& pathData, JBinFileMeshData& binMeshData, std::vector<JBinFileMaterialData>& binMatData);
		};
	}
}