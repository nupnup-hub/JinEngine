#pragma once 
#include<string>   
#include"../../../../Core/Math/JVector.h" 
#include"../../../../Core/JCoreEssential.h" 

namespace JinEngine
{
	struct JAvatarInterpolation
	{
	public:
		JVector3<float> translation;
		JVector4<float> quaternion;
		bool isAvatarJoint;
		bool isUpdated;
	};
	struct JAvatarJointGuide
	{
	public:
		std::string guideName;
		std::string defaultJointName;
		uint8 index;
	public:
		JAvatarJointGuide(const std::string& guideName, const std::string& defaultJointName, uint8 index);
	};
}