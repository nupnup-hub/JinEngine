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

namespace JinEngine
{
	namespace Core
	{
		enum class J_PARAMETER_TYPE : uint8
		{
			UnKnown,
			Void,
			Bool, 
			Int, //short , int, long,  size_t 
			Uint,
			Float, 
			String,
			JVector2,	//Reflection support JVector<float> or JVector<int> or JVector<std::string>
			JVector3,
			JVector4,
			XMInt2,
			XMInt3,
			XMInt4,
			XMFloat2,
			XMFloat3,
			XMFloat4,
			STD_VECTOR,	//Reflection support std::vector<T> T = AtomicType..
						// std::vector<Class<T>> is not support gui
			STD_DEQUE,
			STD_MAP,
			STD_UNORDERED_MAP,
			USER_PTR,
			WEAK_PTR,
			Class,
			Enum,
		};
	}
}