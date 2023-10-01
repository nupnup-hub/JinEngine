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