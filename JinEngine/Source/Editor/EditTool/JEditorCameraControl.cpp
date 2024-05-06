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


#include"JEditorCameraControl.h" 
#include"../Gui/JGui.h"
#include"../../Object/Component/Camera/JCamera.h"
#include"../../Object/Component/Transform/JTransform.h" 
#include"../../Core/Time/JGameTimer.h"  
#include"../../Core/Math/JVectorExtend.h"
#include"../../Window/JWindowPrivate.h" 

namespace JinEngine
{
	namespace Editor
	{
		namespace
		{
			static constexpr float camDeltaMoveFactor = 0.1f;
			static constexpr float camDeltaRotateFactor = 0.1f;
			static constexpr float movementFactorMax = 10.0f;
			static constexpr float movementFactorMin = 0.1f;
		}
		JEditorCameraControl::JEditorCameraControl()
		{
			preMousePos.x = 0;
			preMousePos.y = 0;
		}
		JEditorCameraControl::~JEditorCameraControl() {}
		void JEditorCameraControl::Update(const JUserPtr<JCamera>& cam, float x, float y, const J_GUI_FOCUS_FLAG_ wndFocusFlag)
		{
			if (std::isnan(x))
				x = preMousePos.x;
			if (std::isnan(y))
				y = preMousePos.y;
			if (JGui::IsCurrentWindowFocused(wndFocusFlag))
			{
				if (JGui::IsMouseHoveringRect(JGui::GetWindowPos(), JGui::GetWindowPos() + JGui::GetWindowSize()))
					MouseMove(cam, x, y);
				KeyboardInput(cam);
			}	 
			preMousePos.x = x;
			preMousePos.y = y;
		}
		float JEditorCameraControl::GetMovemnetFactor()const noexcept
		{
			return movementFactor;
		}
		void JEditorCameraControl::SetMousePos(const JVector2F mousePos)
		{
			preMousePos = mousePos;
		}
		void JEditorCameraControl::SetMovemnetFactor(const float factor)noexcept
		{
			movementFactor = std::clamp(factor, movementFactorMin, movementFactorMax);
		}
		void JEditorCameraControl::MouseDown(const JUserPtr<JCamera>& cam, float x, float y)
		{
			preMousePos.x = x;
			preMousePos.y = y;
			SetCapture(Window::JWindowPrivate::HandleInterface::GetHandle());
		}
		void JEditorCameraControl::MouseUp(const JUserPtr<JCamera>& cam, float x, float y)
		{
			ReleaseCapture();
		}
		void JEditorCameraControl::MouseMove(const JUserPtr<JCamera>& cam, float x, float y)
		{
			if (JGui::IsMouseDown(Core::J_MOUSE_BUTTON::RIGHT))
			{ 
				const float dx = (x - preMousePos.x);
				const float dy = (y - preMousePos.y);
				const JVector3F dV = JVector3F(abs(dy) > 0 ? dy * camDeltaRotateFactor : 0, abs(dx) > 0 ? dx * camDeltaRotateFactor : 0, 0);
				cam->GetTransform()->SetRotation(cam->GetTransform()->GetRotation() + dV);
				SetModifiedBit(cam, true);
			}
		}
		void JEditorCameraControl::KeyboardInput(const JUserPtr<JCamera>& cam)
		{
			const float dt = JEngineTimer::Data().DeltaTime();
			bool isChanged = false;
			JUserPtr<JTransform> camTransform = cam->GetTransform();
			JVector3<float> oldPos = camTransform->GetPosition();
			JVector3<float> newPos = camTransform->GetPosition();

			if (GetAsyncKeyState('W') & 0x8000)
				newPos += camTransform->GetFront() * (0.01f * movementFactor);
			if (GetAsyncKeyState('S') & 0x8000)
				newPos += -camTransform->GetFront() * (0.01f * movementFactor);
			if (GetAsyncKeyState('A') & 0x8000)
				newPos += -camTransform->GetRight() * (0.01f * movementFactor);
			if (GetAsyncKeyState('D') & 0x8000)
				newPos += camTransform->GetRight() * (0.01f * movementFactor);
			  
			if (newPos == oldPos || newPos.HasNan())
				return;
		 
			SetModifiedBit(cam, true);
			camTransform->SetPosition(newPos);
		}
		void JEditorCameraControl::AddMovementFactor(const float delta)
		{
			movementFactor = std::clamp(movementFactor + std::clamp(delta, -camDeltaMoveFactor, camDeltaMoveFactor), movementFactorMin, movementFactorMax);
		}
	}
}
