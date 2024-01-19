#pragma once
#include"../../Core/JCoreEssential.h"
#include"../../Core/Math/JMatrix.h" 

namespace JinEngine
{
	namespace Graphic
	{ 
		struct JOutlineConstants
		{
			//instead use ndc vertex
			//JMatrix4x4 world = JMatrix4x4::Identity();

			//invalid red ornage yellow green blue purple black
			JVector4<float> colors[8] =
			{
				JVector4<float>(0.0f ,0.0f, 0.0f, 0.0f),
				JVector4<float>(0.85f ,0.2f, 0.2f, 0.9f),	//same as DefaultColor
				JVector4<float>(0.85f ,0.425f, 0.2f, 0.9f),
				JVector4<float>(0.85f ,0.85f, 0.2f, 0.9f),
				JVector4<float>(0.2f ,0.85f, 0.2f, 0.9f),
				JVector4<float>(0.2f ,0.2f, 0.85f, 0.9f),
				JVector4<float>(0.425f ,0.2f, 0.75f, 0.9f),
				JVector4<float>(0.05f ,0.05f, 0.05f, 0.9f)
			};
			float threshold = 0.01f;
			float thickness = 2;
			uint offset = 0;
			uint width = 0;
			uint height = 0;
			uint outlinePad00 = 0;
			uint outlinePad01 = 0;
			uint outlinePad02 = 0;
		}; 
	}
}