#pragma once 
#include <string>
#include"../Math/JMatrix.h"

namespace JinEngine
{ 
	namespace Core
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
}