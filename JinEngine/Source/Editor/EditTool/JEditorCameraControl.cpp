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
			static constexpr float camDeltaFactor = 0.1f;
			static constexpr float movementFactorMax = 10.0f;
			static constexpr float movementFactorMin = 0.5f;
		}
		JEditorCameraControl::JEditorCameraControl()
		{
			preMousePos.x = 0;
			preMousePos.y = 0;
		}
		JEditorCameraControl::~JEditorCameraControl() {}
		void JEditorCameraControl::Update(JCamera* sceneCamera, float x, float y, const J_GUI_FOCUS_FLAG_ wndFocusFlag)
		{
			if (JGui::IsCurrentWindowFocused(wndFocusFlag))
			{
				if (JGui::IsMouseHoveringRect(JGui::GetWindowPos(), JGui::GetWindowPos() + JGui::GetWindowSize()))
					MouseMove(sceneCamera, x, y);
				KeyboardInput(sceneCamera);
			}	 
			preMousePos.x = x;
			preMousePos.y = y;
		}
		void JEditorCameraControl::SetMousePos(const JVector2F mousePos)
		{
			preMousePos = mousePos;
		}
		void JEditorCameraControl::MouseDown(JCamera* sceneCamera, float x, float y)
		{
			preMousePos.x = x;
			preMousePos.y = y;
			SetCapture(Window::JWindowPrivate::HandleInterface::GetHandle());
		}
		void JEditorCameraControl::MouseUp(JCamera* sceneCamera, float x, float y)
		{
			ReleaseCapture();
		}
		void JEditorCameraControl::MouseMove(JCamera* sceneCamera, float x, float y)
		{
			if (JGui::IsMouseDown(Core::J_MOUSE_BUTTON::RIGHT))
			{ 
				float dx = (x - preMousePos.x) * camDeltaFactor;
				float dy = (y - preMousePos.y) * camDeltaFactor;  
				sceneCamera->GetTransform()->SetRotation(sceneCamera->GetTransform()->GetRotation() + JVector3F(dy, dx, 0));
			}
		}
		void JEditorCameraControl::KeyboardInput(JCamera* sceneCamera)
		{
			const float dt = JEngineTimer::Data().DeltaTime();
			bool isChanged = false;
			JUserPtr<JTransform> camTransform = sceneCamera->GetTransform();
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
			  
			camTransform->SetPosition(newPos);
		}
		void JEditorCameraControl::AddMovementFactor(const float delta)
		{
			movementFactor = std::clamp(movementFactor + delta, movementFactorMin, movementFactorMax);
		}
	}
}
