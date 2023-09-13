#pragma once 
#include <string>
#include"../../../Core/Math/JMatrix.h"
#include"../../../Core/JCoreEssential.h"

namespace JinEngine
{ 
	struct Joint
	{
	public:   
		JMatrix4x4 inbindPose;
		std::wstring name;
		uint8 parentIndex;
		float length;
	};
}