#pragma once
#include"../../Core/Reflection/JReflection.h"

namespace JinEngine
{
	namespace Editor
	{
		REGISTER_ENUM_CLASS(J_EDITOR_EVENT, int, MOUSE_CLICK,
			PUSH_SELECT_OBJECT, 
			POP_SELECT_OBJECT,
			CLEAR_SELECT_OBJECT,
			OPEN_PAGE,
			CLOSE_PAGE,
			ACTIVATE_PAGE,
			DEACTIVATE_PAGE,
			FOCUS_PAGE,
			UNFOCUS_PAGE,
			MAXIMIZE_PAGE,
			MINIMIZE_PAGE,
			PREVIOUS_SIZE_PAGE,
			OPEN_WINDOW,
			CLOSE_WINDOW,
			ACTIVATE_WINDOW,
			DEACTIVATE_WINDOW,
			FOCUS_WINDOW,
			UNFOCUS_WINDOW,
			OPEN_POPUP_WINDOW,
			CLOSE_POPUP_WINDOW,
			MAXIMIZE_WINDOW,
			PREVIOUS_SIZE_WINDOW,
			OPEN_SCENE,
			CLOSE_SCENE,
			BIND_FUNC,				
			T_BIND_FUNC);		

		//BIND_FUNC = Bind�� wrapping�� class�� ���� ����ȣ���Ѵ�.
		//T_BIND_FUNC = do, undo  Bind�� wrapping�� Ŭ������ ���� 
		//J_EDITOR_EVENT�� ��ϵ� transition ����� �����ϴ� �Լ��̿ܿ��� transition����� Ȱ���ϰ��Ѵ�.

		class JEditorEvType
		{
		public:
			static bool CanAllowNotifyCaller(const J_EDITOR_EVENT type)
			{
				switch (type)
				{ 
				case JinEngine::Editor::J_EDITOR_EVENT::PUSH_SELECT_OBJECT:
					return true;
				case JinEngine::Editor::J_EDITOR_EVENT::POP_SELECT_OBJECT:
					return true;
				case JinEngine::Editor::J_EDITOR_EVENT::CLEAR_SELECT_OBJECT:
					return true;
				default:
					return false;
				}
			}
		};

		//T_BIND_FUNC support redo undo

		/*
		enum class J_EDITOR_EVENT
		{
			MOUSE_CLICK,
			SELECT_OBJECT,
			DESELECT_OBJECT,

			OPEN_PAGE,
			CLOSE_PAGE,
			FRONT_PAGE,
			BACK_PAGE,
			ACTIVATE_PAGE,
			DEACTIVATE_PAGE,
			FOCUS_PAGE,
			UNFOCUS_PAGE,

			OPEN_WINDOW,
			CLOSE_WINDOW,
			FRONT_WINDOW,
			BACK_WINDOW,
			ACTIVATE_WINDOW,
			DEACTIVATE_WINDOW,
			FOCUS_WINDOW,
			UNFOCUS_WINDOW,

			OPEN_SCENE,
			CLOSE_SCENE,
			T_BIND_FUNC,
		};
		*/
	}
}