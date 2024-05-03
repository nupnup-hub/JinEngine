#pragma once 
#include"../Device/JGraphicDeviceUser.h"
#include"../../Core/JCoreEssential.h" 
#include"../../Core/Math/JVector.h"
#include<string>

namespace JinEngine
{
	namespace Graphic
	{
		class JGpuAcceleratorHolder : public JGraphicDeviceUser
		{
		public:
			virtual void Clear()noexcept = 0;
		};
	}
}