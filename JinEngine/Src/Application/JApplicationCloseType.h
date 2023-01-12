#pragma once
namespace JinEngine
{
	namespace Application
	{
		enum class J_APPLICATION_CLOSE_TYPE
		{
			NONE = 0,
			CLOSE,
			CLOSE_DELETE_UNUSE_RESOURCE,
			CENCEL_CLOSE,
		};
	}
}