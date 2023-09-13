#include"JGuiBehaviorAdapter.h"

namespace JinEngine::Editor
{
	void JGuiBehaviorAdapter::AddAdaptee(std::unique_ptr<JGuiBehaviorAdaptee>&& newAdaptee)
	{
		if (newAdaptee == nullptr)
			return;

		const uint guiType = (uint)newAdaptee->GetGuiType();
		const uint deviceType = (uint)newAdaptee->GetDeviceType();
		adaptee[guiType][deviceType] = std::move(newAdaptee);
	}
	JGuiBehaviorAdaptee* JGuiBehaviorAdapter::GetAdaptee(const J_GUI_TYPE guiType, const Graphic::J_GRAPHIC_DEVICE_TYPE deviceType)const noexcept
	{
		return 	adaptee[(uint)guiType][(uint)deviceType].get();
	}
}