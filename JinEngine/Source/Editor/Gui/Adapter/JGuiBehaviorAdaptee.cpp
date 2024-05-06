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


#include"JGuiBehaviorAdaptee.h"

#define COL32_R_SHIFT    0
#define COL32_G_SHIFT    8
#define COL32_B_SHIFT    16
#define COL32_A_SHIFT    24
#define COLFACTOR(f)  (f > 1.0f ? 1.0f : (f < 0.0f ? 0.0f : f))
#define COL32(R,G,B,A)    (((uint32)(A)<<COL32_A_SHIFT) | ((uint32)(B)<<COL32_B_SHIFT) | ((uint32)(G)<<COL32_G_SHIFT) | ((uint32)(R)<<COL32_R_SHIFT))

namespace JinEngine::Editor
{  
	uint32 JGuiBehaviorAdaptee::ConvertUColor(const JVector4<float>& color)const noexcept
	{ 
		return COL32(COLFACTOR(color.x) * 255,
			COLFACTOR(color.y) * 255,
			COLFACTOR(color.z) * 255,
			COLFACTOR(color.w) * 255);
	}
	bool JGuiBehaviorAdaptee::CanFocusByMouseRightClick()noexcept
	{
		return false;
	}
}