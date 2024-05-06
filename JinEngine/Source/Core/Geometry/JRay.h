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
#include"../Math/JVector.h"

namespace JinEngine
{
	namespace Core
	{
		class JRay2D
		{
		public:
			JVector2<float> p;
			JVector2<float> dir;
			float time;
			float maxTime;
		public:
			JRay2D(const DirectX::XMVECTOR pV, const DirectX::XMVECTOR dirV, float time = 0, float maxTime = FLT_MAX);
			JRay2D(const JVector2<float>& p, const JVector2<float>& dir, float time = 0, float maxTime = FLT_MAX);
		public:
			DirectX::XMVECTOR PosV()const noexcept;
			DirectX::XMVECTOR DirV()const noexcept;
		};

		class JRay
		{ 
		public:
			JVector3<float> p;
			JVector3<float> dir;
			float time;
			float maxTime;
		public: 
			JRay(const DirectX::XMVECTOR pV, const DirectX::XMVECTOR dirV, float time = 0, float maxTime = FLT_MAX);
			JRay(const JVector3<float>& p, const JVector3<float>& dir, float time = 0, float maxTime = FLT_MAX);
		public:
			DirectX::XMVECTOR PosV()const noexcept;
			DirectX::XMVECTOR DirV()const noexcept;
		};
	}
}