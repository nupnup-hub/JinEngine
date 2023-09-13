#pragma once
#include<DirectXMath.h>  
#include"../../Core/Math/JMatrix.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JBoundingObjectConstants
		{
			JMatrix4x4 boundWorld = JMatrix4x4::Identity();
		};
	}
}