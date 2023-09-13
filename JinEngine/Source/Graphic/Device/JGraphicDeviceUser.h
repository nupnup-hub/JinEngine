#pragma once
#include"JGraphicDeviceType.h"

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicDevice;
		class JGraphicDeviceUser
		{
		public:
			virtual ~JGraphicDeviceUser();
		public:
			virtual J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept = 0;
		public:
			bool IsSameDevice(JGraphicDevice* device)const noexcept;
			bool IsSameDevice(JGraphicDeviceUser* user)const noexcept;
			bool IsSameDevice(const JGraphicDevice* device)const noexcept;
			bool IsSameDevice(const JGraphicDeviceUser* user)const noexcept;
		public:
			template<typename ...Param>
			bool IsAllSameDevice(Param... param)const noexcept
			{
				return (IsSameDevice(param) && ...);
			}
		};
	}
}