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

namespace JinEngine
{
	namespace Graphic
	{
		namespace Constants
		{
			static constexpr float cullingUpdateFrequencyMin = 0.0f;
			static constexpr float cullingUpdateFrequencyMax = 1.0f;
			 
			static constexpr uint frustumNonCullingValue = 0;
			static constexpr uint hzbNonCullingValue = 0;
			static constexpr uint hdNonCullingValue = 1;
			 
			static constexpr uint lightPerClusterVariation = 3;
			static constexpr uint litClusterXVariation = 2;
			static constexpr uint litClusterYVariation = 2;
			static constexpr uint litClusterZVariation = 3;
			static constexpr uint litClusterVariation = litClusterXVariation * litClusterYVariation * litClusterZVariation;

			//0 is fixed value
			static constexpr uint litClusterXRange[litClusterXVariation] = { 24, 48 };
			static constexpr uint litClusterYRange[litClusterYVariation] = { 12, 24 };
			static constexpr uint litClusterZRange[litClusterZVariation] = { 32, 64, 128 };
			static constexpr uint maxLightPerClusterRange[lightPerClusterVariation] = { 15, 30, 45 };
			static constexpr float litClusterNear = 50.0f;
			static constexpr float litClusterNearMin = 1.0f;
			static constexpr float litClusterNearMax = 256.0f;

			static constexpr float litClusterRangeOffset = 1.1f;
			static constexpr float litClusterRangeOffsetMin = 0.75f;
			static constexpr float litClusterRangeOffsetMax = 2.0;
		}
	}
}