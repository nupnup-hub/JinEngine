#pragma once   
#include"JMeshFileLoaderUtility.h" 
#include"../../../../Singleton/JSingletonHolder.h"  
#include<string>
#include<vector>
 
//fbx, obj�� ������ ���˿� ���� load�� �����ϵ��� �Ѵ�
//�̱���	 
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