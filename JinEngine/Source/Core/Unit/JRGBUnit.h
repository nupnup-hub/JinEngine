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
#include"../JCoreEssential.h"
#include"../Math/JVector.h"

namespace JinEngine
{
	namespace Core
	{
		using JRGBUint = uint32;
		using JRGBVector = JVector4F;

		class JRGBConvert
		{
		public:
			static JRGBUint ToColorU(const JRGBVector& color)noexcept;
			static JRGBVector ToColorV(const JRGBUint& color)noexcept;
		};

		class JRGBColorDefine
		{
		public:
			enum class COLOR_TYPE
			{
				RED,
				ORANGE,
				YELLOW,
				GREEN,
				BLUE,
				NAVY,
				PURPLE,
				WHITE,
				BLACK,
				COUNT
			}; 
		public:
			static JRGBVector Red()noexcept;
			static JRGBVector Orange()noexcept;
			static JRGBVector Yellow()noexcept;
			static JRGBVector Green()noexcept;
			static JRGBVector Blue()noexcept;
			static JRGBVector Navy()noexcept;
			static JRGBVector Purple()noexcept;
			static JRGBVector White()noexcept;
			static JRGBVector Black()noexcept;
		public: 
			static JRGBVector FixedColor(const uint index)noexcept;
			static JRGBVector FixedColor(const COLOR_TYPE type)noexcept;
		}; 
		
	}
}