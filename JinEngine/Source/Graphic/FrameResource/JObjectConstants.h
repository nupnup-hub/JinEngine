#pragma once  
#include"../../Core/Math/JVector.h"
#include"../../Core/Math/JMatrix.h"
#include"../../Core/JCoreEssential.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JObjectConstants
		{
			JMatrix4x4 world = JMatrix4x4::Identity();
			JMatrix4x4 texTransform = JMatrix4x4::Identity();
			uint     materialIndex = 0;
			uint     objPad0 = 0;
			uint     objPad1 = 0;
			uint     objPad2 = 0;
		};
	}
}