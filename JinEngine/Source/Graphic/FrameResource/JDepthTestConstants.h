#pragma once 
#include"../../Core/Math/JMatrix.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JDepthTestPassConstants
		{ 
			JMatrix4x4 viewProj = JMatrix4x4::Identity();
		};
	}
}