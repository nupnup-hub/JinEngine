#pragma once
namespace JinEngine
{
	namespace Graphic
	{
		enum class J_MAIN_THREAD_ORDER
		{
			BEGIN = 0,	//pre process before draw scene
			MID,		//post process after draw scene
			END,		//draw display image
			COUNT
		};
		
		enum class J_THREAD_TASK_TYPE
		{
			OCC,
			SHADOW_MAP,
			SCENE,
			COUNT
		};
	}
}