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
namespace JinEngine
{
	namespace Editor
	{
		enum class J_EDITOR_WINDOW_TYPE
		{
			AVATAR_EDITOR,
			WINDOW_DIRECTORY,
			LOG_VIEWER,
			ANIMATION_CONTROLLER_EDITOR, 
			GRAPHIC_WATCHER,
			OBJECT_EXPLORER,
			OBJECT_DETAIL,
			SCENE_VIEWER,
			SCENE_OBSERVER,
			PROJECT_SELECTOR_HUB,
			APPLICATION_WATCHER,
			ANIMATION_DIAGRAM_LIST,
			ANIMATION_CONDITION_LIST,
			ANIMATION_STATE_VIEW,
			GRAPHIC_OPTION,
			WINDOW_STATE,
			TEST_WINDOW
		};

		enum J_EDITOR_WINDOW_FLAG
		{
			J_EDITOR_WINDOW_NONE = 0,
			J_EDITOR_WINDOW_SUPROT_DOCK = 1 << 1,
			J_EDITOR_WINDOW_SUPPORT_WINDOW_CLOSING = 1 << 2,
			J_EDITOR_WINDOW_SUPPORT_SELECT = 1 << 3,
			J_EDITOR_WINDOW_LISTEN_OTHER_WINDOW_SELECT = 1 << 4,
			J_EDITOR_WINDOW_SUPPORT_POPUP = 1 << 5,
			J_EDITOR_WINDOW_SUPPORT_MAXIMIZE = 1 << 6,
		}; 
	}
}