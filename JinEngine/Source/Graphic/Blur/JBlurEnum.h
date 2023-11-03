#pragma once

namespace JinEngine
{
	namespace Graphic
	{
		enum class J_BLUR_TYPE
		{
			BOX,
			GAUSIAAN,
			//BILATERAL,
			COUNT
		};
		enum class J_BLUR_KENEL_SIZE
		{
			_3x3,
			_5x5,
			_7x7,
			COUNT
		};
	}
}