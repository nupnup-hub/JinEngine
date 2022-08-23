#pragma once
#include<string> 
#include"../../../../Lib/DirectX/DirectXMesh.h" 
#include"../../../Object/Resource/Mesh/JMeshStruct.h" 
#include"../../../Utility/JVector.h"
#include"../../JDataType.h" 

namespace JinEngine
{
	namespace Core
	{
		struct JObjMeshPartData
		{
		public:
			std::string meshName;
			std::string materialName;
			JStaticMeshData staticMeshData;
			DirectX::BoundingBox boundingBox;
			DirectX::BoundingSphere boundingSphere;
		};

		enum class JOBJ_MATERIAL_FLAG
		{
			NONE,
			HAS_ALBEDO_T = 1 << 0,
			HAS_NORMAL_T = 1 << 1,
			HAS_HEIGHT_T = 1 << 2,
			HAS_AMBIENT_T = 1 << 3,
			HAS_SPECULAR_COLOR_T = 1 << 4,
			HAS_SPECULAR_HIGHLIGHT_T = 1 << 5,
			HAS_ALPHA_T = 1 << 6,
			HAS_DECAL_T = 1 << 7,
		};

		struct JObjMatData
		{
		public:
			std::string name;
			std::string albedoTName;
			std::string normalTName;
			std::string heightTName;
			std::string ambientTName;
			//미구현
			std::string specularColorTName;
			//미구현
			std::string specularHighlightTName;
			std::string alphaTName;
			//미구현
			std::string decalTName;

			//미구현
			DirectX::XMFLOAT4 ambient;
			DirectX::XMFLOAT4 albedo;
			//미구현
			DirectX::XMFLOAT4 specular;
			//미구현
			DirectX::XMFLOAT4 transmissionFilterColor;
			//미구현
			float specularWeight;
			//미구현
			float opaqueFactor;
			//미구현
			float opticalDensity;

			JOBJ_MATERIAL_FLAG flag;
		};
	}
}