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

#include"JRGBUnit.h"
 
#define COL32_R_SHIFT    0
#define COL32_G_SHIFT    8
#define COL32_B_SHIFT    16
#define COL32_A_SHIFT    24
#define COLFACTOR(f)  (f > 1.0f ? 1.0f : (f < 0.0f ? 0.0f : f))
#define COL32(R,G,B,A)    (((JRGBUint)(A)<<COL32_A_SHIFT) | ((JRGBUint)(B)<<COL32_B_SHIFT) | ((JRGBUint)(G)<<COL32_G_SHIFT) | ((JRGBUint)(R)<<COL32_R_SHIFT))
 
#define COL8_RANGE (1 << 8) - 1 
#define COLV(U)     (JRGBVector((U & COL8_RANGE),((U >> COL32_G_SHIFT) & COL8_RANGE), ((U >> COL32_B_SHIFT) & COL8_RANGE), ((U >> COL32_A_SHIFT) & COL8_RANGE)))


namespace JinEngine::Core
{
	JRGBUint JRGBConvert::ToColorU(const JRGBVector& color)noexcept
	{
		return COL32(COLFACTOR(color.x) * 255,
			COLFACTOR(color.y) * 255,
			COLFACTOR(color.z) * 255,
			COLFACTOR(color.w) * 255);
	}
	JRGBVector JRGBConvert::ToColorV(const JRGBUint& color)noexcept
	{  
		return COLV(color);
	}     
	JRGBVector JRGBColorDefine::Red()noexcept
	{
		return JRGBVector(255, 0, 0, 255);
	}
	JRGBVector JRGBColorDefine::Orange()noexcept
	{
		return JRGBVector(255, 165, 0, 255);
	}
	JRGBVector JRGBColorDefine::Yellow()noexcept
	{
		return JRGBVector(255, 255, 255, 255);
	}
	JRGBVector JRGBColorDefine::Green()noexcept
	{
		return JRGBVector(0, 128, 0, 255);
	}
	JRGBVector JRGBColorDefine::Blue()noexcept
	{
		return JRGBVector(0, 0, 255, 255);
	}
	JRGBVector JRGBColorDefine::Navy()noexcept
	{
		return JRGBVector(75, 0, 130, 255);
	}
	JRGBVector JRGBColorDefine::Purple()noexcept
	{
		return JRGBVector(238, 130, 238, 255);
	}  
	JRGBVector JRGBColorDefine::FixedColor(const uint index)
	{
		return FixedColor(std::min((COLOR_TYPE)index, COLOR_TYPE::BLACK));
	}
	JRGBVector JRGBColorDefine::FixedColor(const COLOR_TYPE type)
	{
		switch (type)
		{
		case JinEngine::Core::JRGBColorDefine::COLOR_TYPE::RED:
			return Red();
		case JinEngine::Core::JRGBColorDefine::COLOR_TYPE::ORANGE:
			return Orange();
		case JinEngine::Core::JRGBColorDefine::COLOR_TYPE::YELLOW:
			return Yellow();
		case JinEngine::Core::JRGBColorDefine::COLOR_TYPE::GREEN:
			return Green();
		case JinEngine::Core::JRGBColorDefine::COLOR_TYPE::BLUE:
			return Blue();
		case JinEngine::Core::JRGBColorDefine::COLOR_TYPE::NAVY:
			return Navy();
		case JinEngine::Core::JRGBColorDefine::COLOR_TYPE::PURPLE:
			return Purple();
		case JinEngine::Core::JRGBColorDefine::COLOR_TYPE::WHITE:
			return White();
		case JinEngine::Core::JRGBColorDefine::COLOR_TYPE::BLACK:
		default:
			return Black();
		}
	}
}