#pragma once
#include"../../../Core/Math/JVector.h"
#include"../../../Core/Math/JMatrix.h"

namespace JinEngine
{
	struct JointPose
	{
	public: 
		//World values
		JVector4<float> rotationQuat = { 0,0,0,0 };
		JVector3<float> translation = { 0,0,0 };
		JVector3<float> scale = {1,1,1 };  
		float stTime = 0;
	public:
		JointPose() = default;
		JointPose(const JVector3<float>& s, const JVector3<float>& r, const JVector3<float>& t, float stTime)noexcept;
		JointPose(const JVector3<float>& s, const JVector4<float>& q, const JVector3<float>& t, float stTime)noexcept;
		JointPose(const JMatrix4x4& m, float stTime)noexcept;
	};
}
