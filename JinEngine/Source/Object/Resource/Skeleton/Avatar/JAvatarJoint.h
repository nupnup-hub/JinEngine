#pragma once 
#include<string>   
#include<DirectXMath.h>
#include"../../../../Core/JDataType.h" 

namespace JinEngine
{
	struct JAvatarInterpolation
	{
	public:
		DirectX::XMFLOAT3 translation;
		DirectX::XMFLOAT4 quaternion; 
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