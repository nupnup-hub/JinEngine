#pragma once
#include"JGuiBehaviorAdaptee.h"
#include<memory>

namespace JinEngine
{
	namespace Editor
	{
		//use communication
		//editor(high level layer) to gui lib caller func(low level layer)
		class JGuiBehaviorAdapter
		{
		public:
			std::unique_ptr<JGuiBehaviorAdaptee> adaptee[(uint)J_GUI_TYPE::COUNT][(uint)Graphic::J_GRAPHIC_DEVICE_TYPE::COUNT];
		public:
			void AddAdaptee(std::unique_ptr<JGuiBehaviorAdaptee>&& newAdaptee);
		public:
			JGuiBehaviorAdaptee* GetAdaptee(const J_GUI_TYPE guiType, const Graphic::J_GRAPHIC_DEVICE_TYPE deviceType)const noexcept;
		};
	}
}