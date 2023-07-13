#pragma once
#include"JointPose.h"
#include"../../../Core/JDataType.h"
#include"../../../Utility/JD3DUtility.h"

namespace JinEngine
{ 
	struct JAnimationSample
	{
	public:
		std::vector<JointPose> jointPose;  
	};
}