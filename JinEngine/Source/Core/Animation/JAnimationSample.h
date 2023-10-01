#pragma once
#include"JointPose.h" 

namespace JinEngine
{ 
	namespace Core
	{
		struct JAnimationSample
		{
		public:
			std::vector<JointPose> jointPose;
		};
	}
}