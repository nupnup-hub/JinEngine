/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


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

		//BIND_FUNC = Bind를 wrapping한 class를 통해 지연호출한다.	(unro, redo을 사용하고싶으면 수동으로 Transition호출이 필요하며 JEditorWindow class 들은 Request Bind로 이용가능)
		//T_BIND_FUNC Bind를 wrapping한 class를 Transition을 통해 지연호출한다.	(Transition에서 자체적으로 unro, redo, log 지원)
		//J_EDITOR_EVENT에 등록된 transition 기능을 지원하는 함수이외에도 transition기능을 활용하게한다.

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