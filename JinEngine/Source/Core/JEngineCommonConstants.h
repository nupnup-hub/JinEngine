#pragma once

#define DEVELOP 1 

namespace JinEngine
{
	static constexpr int invalidIndex = -1;
	

	template<typename ...Param>
	static bool HasInvalidIndex(Param... index)noexcept
	{
		return ((index != invalidIndex) || ...);
	}
}