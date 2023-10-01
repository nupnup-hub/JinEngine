#pragma once
#include"../../Core/JCoreEssential.h"
#include"../../Core/Math/JMatrix.h" 

namespace JinEngine
{
	namespace Graphic
	{ 
		struct JOutlineConstants
		{
			JMatrix4x4 world = JMatrix4x4::Identity();

			//invalid red ornage yellow green blue purple black
			JVector4<float> colors[8] =
			{
				JVector4<float>(0.0f ,0.0f, 0.0f, 0.0f),
				JVector4<float>(1.0f ,0.0f, 0.0f, 1.0f),	//same as DefaultColor
				JVector4<float>(1.0f ,0.5f, 0.0f, 1.0f),
				JVector4<float>(1.0f ,1.0f, 0.0f, 1.0f),
				JVector4<float>(0.0f ,1.0f, 0.0f, 1.0f),
				JVector4<float>(0.0f ,0.0f, 1.0f, 1.0f),
				JVector4<float>(0.5f ,0.0f, 0.75f, 1.0f),
				JVector4<float>(0.05f ,0.05f, 0.05f, 1.0f)
			};
			float threshold = 0.01f;
			float thickness = 4;
			uint offset = 0;
			uint width = 0;
			uint height = 0;
			uint outlinePad00 = 0;
			uint outlinePad01 = 0;
			uint outlinePad02 = 0;
		}; 
	}
}