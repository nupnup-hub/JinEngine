#include"JEditorCameraControl.h" 
#include"../Gui/JGui.h"
#include"../../Object/Component/Camera/JCamera.h"
#include"../../Object/Component/Transform/JTransform.h"
#include"../../Core/Time/JGameTimer.h"  
#include"../../Window/JWindowPrivate.h"
#include<DirectXMath.h> 

using namespace DirectX;
namespace JinEngine
{
	namespace Editor
	{
		namespace 
		{
			static constexpr float camDeltaFactor = 5.0f;
			static constexpr float movementFactorMax = 10.0f;
			static constexpr float movementFactorMin = 1.0f;
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
				// Make each pixel correspond to a quarter of a degree.
				float dx = XMConvertToRadians(x - preMousePos.x) * camDeltaFactor;
				float dy = XMConvertToRadians(y - preMousePos.y) * camDeltaFactor;
				JVector3<float> newRot = sceneCamera->GetTransform()->GetRotation();
				newRot.y += dx;
				newRot.x += dy;
				sceneCamera->GetTransform()->SetRotation(newRot);
			}
		}
		void JEditorCameraControl::KeyboardInput(JCamera* sceneCamera)
		{
			const float dt = JEngineTimer::Data().DeltaTime();
			bool isChanged = false;
			JUserPtr<JTransform> camTransform = sceneCamera->GetTransform();
			JVector3<float> oldPosF = camTransform->GetPosition();
			XMVECTOR newPosV = camTransform->GetPosition().ToXmV();

			if (GetAsyncKeyState('W') & 0x8000)
				newPosV += XMVectorScale(camTransform->GetFront().ToXmV(), 0.01f * movementFactor);
			if (GetAsyncKeyState('S') & 0x8000)
				newPosV += XMVectorScale(-camTransform->GetFront().ToXmV(), 0.01f * movementFactor);
			if (GetAsyncKeyState('A') & 0x8000)
				newPosV += XMVectorScale(-camTransform->GetRight().ToXmV(), 0.01f * movementFactor);
			if (GetAsyncKeyState('D') & 0x8000)
				newPosV += XMVectorScale(camTransform->GetRight().ToXmV(), 0.01f * movementFactor);

			XMFLOAT3 newPosF;
			XMStoreFloat3(&newPosF, newPosV);
			if (newPosF.x != oldPosF.x || newPosF.y != oldPosF.y || newPosF.z != oldPosF.z)
				camTransform->SetPosition(newPosF);
		}
		void JEditorCameraControl::AddMovementFactor(const float delta)
		{ 
			movementFactor = std::clamp(movementFactor + delta, movementFactorMin, movementFactorMax);
		}
	}
}
