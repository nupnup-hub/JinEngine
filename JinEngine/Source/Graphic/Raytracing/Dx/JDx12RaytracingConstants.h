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
#include"../../../Core/JCoreEssential.h"
namespace JinEngine
{
	namespace Graphic
	{
		namespace Constants
		{
			static constexpr uint objRefnstanceCapacity = 1 << 18;
			static constexpr uint pointLightInstanceCapacity = 1 << 12;
			static constexpr uint spotLightInstanceCapacity = 1 << 12;
			static constexpr uint rectLightInstanceCapacity = 1 << 12;

			static constexpr uint objRefAsInstanceIdoffset = 0;
			static constexpr uint pointLightAsInstanceIdOffset = objRefAsInstanceIdoffset + objRefnstanceCapacity;
			static constexpr uint spotLightAsInstanceIdOffset = pointLightAsInstanceIdOffset + pointLightInstanceCapacity;
			static constexpr uint rectLightAsInstanceIdOffset = spotLightAsInstanceIdOffset + spotLightInstanceCapacity;
			static constexpr uint lightAsInstanceIDed = rectLightAsInstanceIdOffset + rectLightInstanceCapacity;

			static constexpr uint renderItemMask = 1 << 1; 
			static constexpr uint pointLightMask = 1 << 2;
			static constexpr uint spotLightMask = 1 << 3;
			static constexpr uint rectLightMask = 1 << 4; 
		}
	}
}