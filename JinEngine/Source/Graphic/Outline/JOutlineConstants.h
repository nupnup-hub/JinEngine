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