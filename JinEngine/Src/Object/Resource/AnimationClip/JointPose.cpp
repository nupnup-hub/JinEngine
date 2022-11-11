#include"JointPose.h"

namespace JinEngine {

	using namespace DirectX;
	JointPose::JointPose(const DirectX::XMFLOAT3& s, const DirectX::XMFLOAT3& r,const DirectX::XMFLOAT3& t, float stTime)noexcept
		: Scale(s.x, s.y, s.z),
		Translation(t.x, t.y, t.z),
		stTime(stTime)
	{
		const XMVECTOR q = XMQuaternionRotationRollPitchYaw(r.x, r.y, r.z);
		XMStoreFloat4(&RotationQuat, q);
	}
	JointPose::JointPose(const DirectX::XMFLOAT3& s, const DirectX::XMFLOAT4& q, const DirectX::XMFLOAT3& t, float stTime)noexcept
		: Scale(s.x, s.y, s.z), 
		RotationQuat(q.x, q.y, q.z, q.w),
		Translation(t.x, t.y, t.z),
		stTime(stTime)
	{}
	JointPose::JointPose(const XMFLOAT4X4& fm, float stTime)noexcept
		:stTime(stTime)
	{ 
		const XMMATRIX m = XMLoadFloat4x4(&fm);
		XMVECTOR t;
		XMVECTOR q;
		XMVECTOR s;
		XMMatrixDecompose(&s, &q, &t, m);

		XMStoreFloat4(&RotationQuat, q);
		XMStoreFloat3(&Translation, t);
		XMStoreFloat3(&Scale, s);
	}
}