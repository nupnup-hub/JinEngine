#include"JShaderDataHolder.h"

namespace JinEngine::Graphic
{
	DeviceIdentification JGraphicShaderDataHolder::GetDeviceIdentification()const noexcept
	{
		return (DeviceIdentification)GetDeviceType();
	}
	bool JGraphicShaderDataHolder::IsSameDevice(const DeviceIdentification otherDeviceIdentification)const noexcept
	{
		return otherDeviceIdentification == (DeviceIdentification)GetDeviceType();
	}
 
	DeviceIdentification JComputeShaderDataHolder::GetDeviceIdentification()const noexcept
	{
		return (DeviceIdentification)GetDeviceType();
	}
	bool JComputeShaderDataHolder::IsSameDevice(const DeviceIdentification otherDeviceIdentification)const noexcept
	{
		return otherDeviceIdentification == (DeviceIdentification)GetDeviceType();
	}
}