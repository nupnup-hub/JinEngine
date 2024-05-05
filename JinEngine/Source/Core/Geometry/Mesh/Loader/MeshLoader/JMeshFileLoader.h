#pragma once   
#include"JMeshFileLoaderUtility.h" 
#include"../../../../Singleton/JSingletonHolder.h"   
 
//fbx, obj�� ������ ���˿� ���� load�� �����ϵ��� �Ѵ�
//�̱���	 
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