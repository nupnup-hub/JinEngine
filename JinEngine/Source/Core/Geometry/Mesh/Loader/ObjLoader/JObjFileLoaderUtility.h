#pragma once
#include<string> 
#include"../../JMeshStruct.h" 
#include"../../../../ThirdParty/DirectX/DirectXMesh.h"   

namespace JinEngine
{
	namespace Core
	{  
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

		struct JObjFileMaterial
		{
		public:
			std::wstring name;
			std::wstring albedoMapName;
			std::wstring normalMapName;
			std::wstring heightMapName;
			std::wstring ambientMapName; 
			std::wstring specularColorMapName; 
			std::wstring specularHighlightMapName;
			std::wstring alphaMapName; 
			std::wstring decalMapName;

			//¹Ì±¸Çö
			JVector4F ambient;
			JVector4F albedo;
			JVector4F specular;
			JVector4F transmissionFilterColor;
			float specularWeight; 
			float opaqueFactor; 
			float opticalDensity;

			JOBJ_MATERIAL_FLAG flag;
		};
	}
}