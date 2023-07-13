#pragma once 

namespace JinEngine
{
	namespace Core
	{
		enum class J_THREAD_USE_CASE_TYPE
		{
			GRAPHIC_DRAW,	//stuff command list
			GRAPHIC_UPDATE,
			COMMON
		};

		enum class J_THREAD_STATE_TYPE
		{
			WAIT,
			WORK
		};
	}
}