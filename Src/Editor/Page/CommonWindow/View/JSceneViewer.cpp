#include"JSceneViewer.h" 
#include"../JEditorAttribute.h"  
#include"../Common/JEditorCameraControl.h"
#include"../../Utility/JEditorUtility.h"
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
	namespace Editor
	{
		JSceneViewer::JSceneViewer(std::unique_ptr<JEditorAttribute> attribute, const size_t ownerPageGuid)
			:JEditorWindow(std::move(attribute), ownerPageGuid)
		{
			editorCamCtrl = std::make_unique<JEditorCameraControl>();
		}
		JSceneViewer::~JSceneViewer() {}
		bool JSceneViewer::Activate(JEditorUtility* editorUtility)
		{
			if (JEditorWindow::Activate(editorUtility))
			{
				if (selectedCamera != nullptr)
					;// selectedCamera->SetCameraState(J_CAMERA_STATE::RENDER);
				return true;
			}
			else
				return false;
		}
		bool JSceneViewer::DeActivate(JEditorUtility* editorUtility)
		{
			if (JEditorWindow::DeActivate(editorUtility))
			{
				if (selectedCamera != nullptr)
					;// selectedCamera->SetCameraState(J_CAMERA_STATE::IDEL);
				return true;
			}
			else
				return false;
		}
		void JSceneViewer::StoreEditorWindow(std::wofstream& stream)
		{
			JEditorWindow::StoreEditorWindow(stream);
			if (selectedCamera != nullptr)
			{
				stream << L"HasSelected: " << true << '\n';
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
		void JSceneViewer::LoadEditorWindow(std::wifstream& stream)
		{
			JEditorWindow::LoadEditorWindow(stream);
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
		void JSceneViewer::UpdateWindow(JEditorUtility* editorUtility)
		{
			JEditorWindow::UpdateWindow(editorUtility);

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