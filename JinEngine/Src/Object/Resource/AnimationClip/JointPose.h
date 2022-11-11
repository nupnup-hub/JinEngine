#pragma once
#include<DirectXMath.h>  

namespace JinEngine
{
	struct JointPose
	{
	public: 
		DirectX::XMFLOAT4 RotationQuat = { 0,0,0,0 };
		DirectX::XMFLOAT3 Translation = { 0,0,0 };
		DirectX::XMFLOAT3 Scale = {1,1,1 };  
		float stTime = 0;
	public:
		JointPose() = default;
		JointPose(const DirectX::XMFLOAT3& s, const DirectX::XMFLOAT3& r, const DirectX::XMFLOAT3& t, float stTime)noexcept;
		JointPose( const DirectX::XMFLOAT3& s, const DirectX::XMFLOAT4& q, const DirectX::XMFLOAT3& t, float stTime)noexcept;
		JointPose(const DirectX::XMFLOAT4X4& fm, float stTime)noexcept;
	};
}
