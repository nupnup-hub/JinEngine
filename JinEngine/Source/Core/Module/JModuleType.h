#pragma once

namespace JinEngine
{
	namespace Core
	{
		enum class J_MODULE_TYPE
		{
			PROJECT,
			EDITOR
		};

		enum class J_MODULE_LINK_TYPE
		{
			STATIC,		//static library
			DYNAMIC		//dynamic library
		};


		enum class J_MODULE_LOAD_PHASE
		{
			DEFAULT,	//Load when engine initialize
			MANUAL			
		};
	}
}