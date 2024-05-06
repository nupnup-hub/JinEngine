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
#include"../Gui/JGuiType.h"
#include"../../Core/Math/JVector.h"
#include"../../Object/JObjectModifyInterface.h"

namespace JinEngine
{     
	class JCamera; 
	namespace Editor
	{
		class JEditorCameraControl : public JObjectModifyInterface
		{
		private:
			JVector2<float> preMousePos = JVector2<float>::Zero();
			//mulitiple cam pos delta
			float movementFactor = 1.0f;
		public:
			JEditorCameraControl();
			~JEditorCameraControl();
			JEditorCameraControl(const JEditorCameraControl& rhs) = delete;
			JEditorCameraControl& operator=(const JEditorCameraControl& rhs) = delete;
		public:
			float GetMovemnetFactor()const noexcept;
		public:
			void SetMousePos(const JVector2F mousePos);
			void SetMovemnetFactor(const float factor)noexcept;
		public:
			void Update(const JUserPtr<JCamera>& cam, float x, float y, const J_GUI_FOCUS_FLAG_ wndFocusFlag);
		private:
			void MouseDown(const JUserPtr<JCamera>& cam, float x, float y);
			void MouseUp(const JUserPtr<JCamera>& cam, float x, float y);
			void MouseMove(const JUserPtr<JCamera>& cam, float x, float y);
			void KeyboardInput(const JUserPtr<JCamera>& cam);
		public:
			void AddMovementFactor(const float delta);
		};
	}
}