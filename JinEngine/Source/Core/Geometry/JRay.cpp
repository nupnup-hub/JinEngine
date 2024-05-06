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


#include"JRay.h"

namespace JinEngine
{
	namespace Core
	{
		JRay2D::JRay2D(const DirectX::XMVECTOR pV, const DirectX::XMVECTOR dirV, float time, float maxTime)
			:time(time), maxTime(maxTime)
		{ 
			JRay2D::p = pV;
			JRay2D::dir = dirV;
		}
		JRay2D::JRay2D(const JVector2<float>& p, const JVector2<float>& dir, float time, float maxTime)
			: p(p), dir(dir), time(time), maxTime(maxTime)
		{}
		DirectX::XMVECTOR JRay2D::PosV()const noexcept
		{
			return DirectX::XMVectorSet(p.x, p.y, 0.0f, 1.0f);
		}
		DirectX::XMVECTOR JRay2D::DirV()const noexcept
		{
			return DirectX::XMVectorSet(dir.x, dir.y, 0.0f, 0.0f);
		}

		JRay::JRay(const DirectX::XMVECTOR pV, const DirectX::XMVECTOR dirV, float time, float maxTime)
			: p(pV), dir(dirV), time(time), maxTime(maxTime)
		{}
		JRay::JRay(const JVector3<float>& p, const JVector3<float>& dir, float time, float maxTime)
			:p(p), dir(dir), time(time), maxTime(maxTime)
		{}
		DirectX::XMVECTOR JRay::PosV()const noexcept
		{
			return DirectX::XMVectorSet(p.x, p.y, p.z, 1.0f);
		}
		DirectX::XMVECTOR JRay::DirV()const noexcept
		{
			return DirectX::XMVectorSet(dir.x, dir.y, dir.z, 0.0f);
		}
	}
}