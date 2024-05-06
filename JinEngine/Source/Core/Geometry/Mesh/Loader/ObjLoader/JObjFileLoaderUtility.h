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
#include"../../JMeshStruct.h" 
#include"../../JMaterialProperty.h"
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
		public:
			JMaterialParameter mParam;
			//¹Ì±¸Çö
			JVector4F ambient; 
			JVector4F specular;
			JVector4F transmissionFilterColor;
			float specularWeight; 
			float opaqueFactor; 
			float opticalDensity;
			JOBJ_MATERIAL_FLAG flag;
		};
	}
}