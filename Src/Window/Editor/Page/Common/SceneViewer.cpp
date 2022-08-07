#include"SceneViewer.h" 
#include"../EditorAttribute.h"  
#include"../Common/EditorCameraControl.h"
#include"../../Utility/EditorUtility.h"
#include"../../../../Object/Resource/JResourceManager.h"

#include"../../../../Object/GameObject/JGameObject.h"
#include"../../../../Object/Component/Camera/JCamera.h"
#include"../../../../Object/Component/Transform/JTransform.h"
#include"../../../../Object/Resource/Scene/JScene.h" 

#include"../../../../Graphic/JGraphic.h"
#include"../../../../Graphic/JGraphicResourceManager.h" 
#include"../../../../Utility/JCommonUtility.h"
#include"../../../../../Lib/imgui/imgui.h"  

using namespace DirectX;

namespace JinEngine
{
	SceneViewer::SceneViewer(std::unique_ptr<EditorAttribute> attribute, const size_t ownerPageGuid)
		:EditorWindow(std::move(attribute), ownerPageGuid)
	{
		editorCamCtrl = std::make_unique<EditorCameraControl>();
	}
	SceneViewer::~SceneViewer(){}
	bool SceneViewer::Activate(EditorUtility* editorUtility)
	{
		if (EditorWindow::Activate(editorUtility))
		{
			if (selectedCamera != nullptr)
				;// selectedCamera->SetCameraState(J_CAMERA_STATE::RENDER);
			return true;
		}
		else
			return false;
	}
	bool SceneViewer::DeActivate(EditorUtility* editorUtility)
	{
		if (EditorWindow::DeActivate(editorUtility))
		{
			if (selectedCamera != nullptr)
				;// selectedCamera->SetCameraState(J_CAMERA_STATE::IDEL);
			return true;
		}
		else
			return false;
	}
	void SceneViewer::StoreEditorWindow(std::wofstream& stream)
	{
		EditorWindow::StoreEditorWindow(stream);
		if (selectedCamera != nullptr)
		{
			stream<< L"HasSelected: " << true << '\n';
			stream << L"CamOwnerGuid: " << selectedCamera->GetOwner()->GetGuid() << '\n';
			stream << L"CamGuid: " << selectedCamera->GetGuid() << '\n';

			XMFLOAT3 position = selectedCamera->GetTransform()->GetPosition();
			XMFLOAT3 rotation = selectedCamera->GetTransform()->GetRotation();
			stream << position.x << " " << position.y << " " << position.z << '\n';
			stream << rotation.x << " " << rotation.y << " " << rotation.z << '\n';
		}
		else
			stream << L"HasSelected: " << false << '\n';
	}
	void SceneViewer::LoadEditorWindow(std::wifstream& stream)
	{
		EditorWindow::LoadEditorWindow(stream);
		std::wstring guide;
		bool hasSelectedCamera;
		
		stream >> guide >> hasSelectedCamera;
		if (hasSelectedCamera)
		{
			size_t ownerGuid;
			size_t camGuid;
			XMFLOAT3 position;
			XMFLOAT3 rotation;
			stream >> guide >> ownerGuid;
			stream >> guide >> camGuid;

			stream >> position.x >> position.y >> position.z;
			stream >> rotation.x >> rotation.y >> rotation.z;

			JScene* mainScene = JResourceManager::Instance().GetMainScene();
			JGameObject* tarObj = mainScene->FindGameObject(ownerGuid);
			if (tarObj != nullptr)
			{
				JCamera* tarCam = tarObj->GetComponent<JCamera>();
				if (tarCam != nullptr)
				{
					tarCam->GetTransform()->SetPosition(position);
					tarCam->GetTransform()->SetRotation(rotation);
					selectedCamera = tarCam;
				}
			}
		}
	}
	void SceneViewer::UpdateWindow(EditorUtility* editorUtility)
	{
		EditorWindow::UpdateWindow(editorUtility);

		JCamera* mainCamera = JResourceManager::Instance().GetMainScene()->GetMainCamera();
		if (selectedCamera == nullptr)
		{
			selectedCamera = mainCamera;
			;// selectedCamera->SetCameraState(J_CAMERA_STATE::RENDER);
		}
		else if (selectedCamera->GetGuid() != mainCamera->GetGuid())
			selectedCamera = mainCamera;
		
		if (IsFocus())
		{
			if (ImGui::IsMouseHoveringRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize()))
				editorCamCtrl->MouseMove(selectedCamera, ImGui::GetMousePos().x, ImGui::GetMousePos().y);
			editorCamCtrl->KeyboardInput(selectedCamera);
		}

		if (ImGui::Checkbox("OnAnimation", &onAnimation))
		{  
			//수정필요
		}

		ImVec2 windowSize = ImGui::GetWindowSize();
		//ImGui::Image((ImTextureID)JGraphic::Instance().ResourceInterface()->GetGpuSrvDescriptorHandle(selectedCamera->GetRsSrvHeapIndex()).ptr,
		//	ImVec2(windowSize.x, windowSize.y)); 
	}
}

/*
		if (editorUtility->selectedObject != nullptr)
		{
			const J_OBJECT_TYPE objType = editorUtility->selectedObject->GetObjectType();
			if (objType == J_OBJECT_TYPE::GAME_OBJECT)
			{
				JGameObject* gameObj = dynamic_cast<JGameObject*>(editorUtility->selectedObject);
				JCamera* camera = gameObj->GetComponent<JCamera>();
				if (camera != nullptr && gameObj->GetFlag() != OBJECT_FLAG_EDITOR_OBJECT)
				{
					if (selectedCamera != nullptr)
						selectedCamera->SetCameraState(J_CAMERA_STATE::IDEL);
					selectedCamera = camera;
					selectedCamera->SetCameraState(J_CAMERA_STATE::RENDER);
				}
			}
		}
*/