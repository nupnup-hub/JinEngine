#pragma once
#include"../../../Core/JCoreEssential.h"
namespace JinEngine
{
	namespace Graphic
	{
		namespace Constants
		{
			static constexpr uint objRefnstanceCapacity = 1 << 18;
			static constexpr uint pointLightInstanceCapacity = 1 << 12;
			static constexpr uint spotLightInstanceCapacity = 1 << 12;
			static constexpr uint rectLightInstanceCapacity = 1 << 12;

			static constexpr uint objRefAsInstanceIdoffset = 0;
			static constexpr uint pointLightAsInstanceIdOffset = objRefAsInstanceIdoffset + objRefnstanceCapacity;
			static constexpr uint spotLightAsInstanceIdOffset = pointLightAsInstanceIdOffset + pointLightInstanceCapacity;
			static constexpr uint rectLightAsInstanceIdOffset = spotLightAsInstanceIdOffset + spotLightInstanceCapacity;
			static constexpr uint lightAsInstanceIDed = rectLightAsInstanceIdOffset + rectLightInstanceCapacity;

			static constexpr uint renderItemMask = 1 << 1; 
			static constexpr uint pointLightMask = 1 << 2;
			static constexpr uint spotLightMask = 1 << 3;
			static constexpr uint rectLightMask = 1 << 4; 
		}
	}
}