#pragma once
namespace JinEngine
{
	enum class J_APPLICATION_STATE
	{
		PROJECT_SELECT,
		EDIT_GAME,
		PLAY_GAME
	};

	enum class J_APPLICATION_SUB_STATE
	{ 
		IDLE, 
		UPDATE_LOOP
	};
}