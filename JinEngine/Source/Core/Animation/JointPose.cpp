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


#include"JointPose.h"

namespace JinEngine {

	using namespace DirectX;
	namespace Core
	{
		JointPose::JointPose(const JVector3<float>& s, const JVector3<float>& r, const JVector3<float>& t, float stTime)noexcept
			: scale(s.x, s.y, s.z),
			translation(t.x, t.y, t.z),
			rotationQuat(XMQuaternionRotationRollPitchYaw(r.x, r.y, r.z)),
			stTime(stTime)
		{
		}
		JointPose::JointPose(const JVector3<float>& s, const JVector4<float>& q, const JVector3<float>& t, float stTime)noexcept
			: scale(s.x, s.y, s.z),
			rotationQuat(q.x, q.y, q.z, q.w),
			translation(t.x, t.y, t.z),
			stTime(stTime)
		{}
		JointPose::JointPose(const JMatrix4x4& m, float stTime)noexcept
			:stTime(stTime)
		{
			XMVECTOR t;
			XMVECTOR q;
			XMVECTOR s;
			XMMatrixDecompose(&s, &q, &t, m.LoadXM());

			rotationQuat = q;
			translation = t;
			scale = s;
		}
	}
}