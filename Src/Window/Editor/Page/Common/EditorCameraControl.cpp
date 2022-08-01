#include"EditorCameraControl.h" 
#include"../../../../Object/Component/Camera/JCamera.h"
#include"../../../../Object/Component/Transform/JTransform.h"
#include"../../../../Core/GameTimer/JGameTimer.h"   
#include"../../../JWindows.h"
#include"../../../../../Lib/imgui/imgui.h"  
#include<DirectXMath.h> 

namespace JinEngine
{
	using namespace DirectX;

	EditorCameraControl::EditorCameraControl() 
	{
		preMousePos.x = 0;
		preMousePos.y = 0;
	}
	EditorCameraControl::~EditorCameraControl() {}
	void EditorCameraControl::MouseDown(JCamera* sceneCamera, float x, float y)
	{
		preMousePos.x = x;
		preMousePos.y = y;
		SetCapture(JWindow::Instance().HandleInterface()->GetHandle());
	}
	void EditorCameraControl::MouseUp(JCamera* sceneCamera, float x, float y)
	{
		ReleaseCapture();
	}
	void EditorCameraControl::MouseMove(JCamera* sceneCamera, float x, float y)
	{
		if (ImGui::IsMouseDown(1))
		{
			// Make each pixel correspond to a quarter of a degree.
			float dx = XMConvertToRadians(x - preMousePos.x) * 7.5f;
			float dy = XMConvertToRadians(y - preMousePos.y) * 7.5f;
			XMFLOAT3 newRot = sceneCamera->GetTransform()->GetRotation();
			newRot.y += dx;
			newRot.x += dy;
			sceneCamera->GetTransform()->SetRotation(newRot);
		}
		preMousePos.x = x;
		preMousePos.y = y;
	}
	void EditorCameraControl::KeyboardInput(JCamera* sceneCamera)
	{
		const float dt = JGameTimer::Instance().DeltaTime();
		bool isChanged = false;
		JTransform* camTransform = sceneCamera->GetTransform();
		XMFLOAT3 oldPosF = camTransform->GetPosition();
		XMVECTOR newPosV = XMLoadFloat3(&oldPosF);

		if (GetAsyncKeyState('W') & 0x8000)
			newPosV += XMVectorScale(camTransform->GetFront(), 0.01f);
		if (GetAsyncKeyState('S') & 0x8000)
			newPosV += XMVectorScale(-camTransform->GetFront(), 0.01f);
		if (GetAsyncKeyState('A') & 0x8000)
			newPosV += XMVectorScale(-camTransform->GetRight(), 0.01f);
		if (GetAsyncKeyState('D') & 0x8000)
			newPosV += XMVectorScale(camTransform->GetRight(), 0.01f);

		XMFLOAT3 newPosF;
		XMStoreFloat3(&newPosF, newPosV);
		if(newPosF.x != oldPosF.x || newPosF.y != oldPosF.y || newPosF.z != oldPosF.z)
			camTransform->SetPosition(newPosF);
	}
}
