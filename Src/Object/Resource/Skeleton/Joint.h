#pragma once 
#include <string>
#include<DirectXMath.h>
#include"../../../Core/JDataType.h"

namespace JinEngine
{ 
	struct Joint
	{
	public:  
		DirectX::XMFLOAT4X4 inbindPose;
		std::string name = "";
		uint8 parentIndex; 
		float length;
	};
}