#pragma once
#include"JointPose.h"
#include"../../../Core/JCoreEssential.h" 

namespace JinEngine
{ 
	struct JAnimationSample
	{
	public:
		std::vector<JointPose> jointPose;  
	};
}