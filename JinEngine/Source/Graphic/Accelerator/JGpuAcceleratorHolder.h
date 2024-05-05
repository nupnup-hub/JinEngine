#pragma once 
#include"../Device/JGraphicDeviceUser.h" 
#include"../../Core/Math/JVector.h" 

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