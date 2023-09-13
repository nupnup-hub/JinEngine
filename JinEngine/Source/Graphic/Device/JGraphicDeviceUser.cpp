#include"JGraphicDeviceUser.h"
#include"JGraphicDevice.h"

namespace JinEngine::Graphic
{
	JGraphicDeviceUser::~JGraphicDeviceUser()
	{	}
	bool JGraphicDeviceUser::IsSameDevice(JGraphicDevice* device)const noexcept
	{
		return device != nullptr ? GetDeviceType() == device->GetDeviceType() : false;
	}
	bool JGraphicDeviceUser::IsSameDevice(JGraphicDeviceUser* user)const noexcept
	{
		return user != nullptr ? GetDeviceType() == user->GetDeviceType() : false;
	}
	bool JGraphicDeviceUser::IsSameDevice(const JGraphicDevice* device)const noexcept
	{
		return device != nullptr ? GetDeviceType() == device->GetDeviceType() : false;
	}
	bool JGraphicDeviceUser::IsSameDevice(const JGraphicDeviceUser* user)const noexcept
	{
		return user != nullptr ? GetDeviceType() == user->GetDeviceType() : false;
	}
}