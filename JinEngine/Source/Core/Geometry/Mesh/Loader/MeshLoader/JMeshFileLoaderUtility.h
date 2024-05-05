#pragma once
#include"../../JMeshStruct.h" 
#include"../../../../ThirdParty/DirectX/DirectXMesh.h"   

namespace JinEngine
{
	namespace Core
	{
		struct JBinFileMeshData
		{
		public:
			JStaticMeshGroup meshGroup;
			std::vector<std::wstring> materialName;
		};
		struct JBinFileMaterialData
		{
		public:
			std::wstring name;
			std::wstring albedoTName;
		};
	}
}