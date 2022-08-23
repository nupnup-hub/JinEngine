#include"JMainSceneEditor.h" 
#include"JEditorCameraControl.h"
#include"../../JEditorAttribute.h" 
#include"../../../../Object/Component/Camera/JCamera.h"  
#include"../../../../Object/Component/Transform/JTransform.h"
#include"../../../../Object/Resource/Scene/JScene.h"
#include"../../../../Object/Resource/JResourceManager.h"
#include"../../../../Object/GameObject/JGameObject.h" 
#include"../../../../Object/GameObject/JGameObjectFactoryUtility.h"
#include"../../../../Utility/JCommonUtility.h"
#include"../../../../Graphic/JGraphic.h"
#include"../../../../Graphic/JGraphicResourceManager.h" 
#include"../../../../Core/GameTimer/JGameTimer.h"
#include"../../../../../Lib/imgui/imgui.h"  

namespace JinEngine
{
	namespace Editor
	{
		JMainSceneEditor::JMainSceneEditor(std::unique_ptr<JEditorAttribute> attribute, const size_t ownerPageGuid)
			:JSceneEditor(std::move(attribute), ownerPageGuid)
		{}
		JMainSceneEditor::~JMainSceneEditor()
		{

		}
		void JMainSceneEditor::Initialize()noexcept
		{

		}
		bool JMainSceneEditor::Activate()
		{
			if (JEditorWindow::Activate())
			{
				const J_OBJECT_FLAG flag = (J_OBJECT_FLAG)(OBJECT_FLAG_EDITOR_OBJECT | OBJECT_FLAG_HIDDEN);

				mainScene = JResourceManager::Instance().GetMainScene();
				camObj = JGFU::CreateCamera(*mainScene->GetRootGameObject(), flag, false, "MainSceneEditorCamera");
				editorCamera = camObj->GetComponent<JCamera>();
				//editorCamera->SetCameraState(J_CAMERA_STATE::RENDER);
				editorCamera->GetTransform()->SetPosition(position);
				editorCamera->GetTransform()->SetRotation(rotation);
				frustumObj = JGFU::CreateDebugGameObject(*mainScene->GetRootGameObject(), flag, J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_FRUSTUM, J_DEFAULT_MATERIAL::DEBUG_LINE_YELLOW);
				return true;
			}
			else
				return false;
		}
		bool JMainSceneEditor::DeActivate()
		{
			if (JEditorWindow::DeActivate())
			{
				position = editorCamera->GetTransform()->GetPosition();
				rotation = editorCamera->GetTransform()->GetRotation();
				camObj->Destroy();
				frustumObj->Destroy(); 
				camObj = nullptr;
				editorCamera = nullptr;
				frustumObj = nullptr;
				return true;
			}
			else
				return false;
		}
		void JMainSceneEditor::StoreEditorWindow(std::wofstream& stream)
		{
			JEditorWindow::StoreEditorWindow(stream);
			stream << position.x << " " << position.y << " " << position.z << '\n';
			stream << rotation.x << " " << rotation.y << " " << rotation.z << '\n';
		}
		void JMainSceneEditor::LoadEditorWindow(std::wifstream& stream)
		{
			JEditorWindow::LoadEditorWindow(stream);
			stream >> position.x >> position.y >> position.z;
			stream >> rotation.x >> rotation.y >> rotation.z;
		}
		void JMainSceneEditor::UpdateWindow()
		{
			JEditorWindow::UpdateWindow();

			if (frustumObj != nullptr)
			{
				JCamera* mainCamera = mainScene->GetMainCamera();
				JTransform* frumstumR = frustumObj->GetTransform();
				frumstumR->SetRotation(mainCamera->GetTransform()->GetRotation());
				frumstumR->SetPosition(mainCamera->GetTransform()->GetPosition());

				float camFar = mainCamera->GetFar();
				float camWidth = (float)mainCamera->GetViewWidth();
				float camHeight = (float)mainCamera->GetViewHeight();
				frumstumR->SetScale(DirectX::XMFLOAT3(camWidth, camHeight, camFar));
			}

			if (IsFocus())
			{
				if (ImGui::IsMouseHoveringRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize()))
					editorCamCtrl->MouseMove(editorCamera, ImGui::GetMousePos().x, ImGui::GetMousePos().y);
				editorCamCtrl->KeyboardInput(editorCamera);

				//if (editorUtility->selectedObject != nullptr && editorUtility->selectedObject->GetObjectType() == J_OBJECT_TYPE::GAME_OBJECT)
				//	DrawGameObjectWidget(editorUtility);
			}
			ImVec2 windowSize = ImGui::GetWindowSize();
			//ImGui::Image((ImTextureID)JGraphic::Instance().ResourceInterface()->GetGpuSrvDescriptorHandle(editorCamera->GetRsSrvHeapIndex()).ptr,
			//	ImVec2(windowSize.x, windowSize.y));
		}
		void JMainSceneEditor::DrawGameObjectWidget()
		{
			JGameObject* selectedGameObj = dynamic_cast<JGameObject*>(editorUtility->selectedObject);
			JTransform* transform = selectedGameObj->GetTransform();

			const float lineLength = 2.5f;
			ImDrawList* drawList = ImGui::GetWindowDrawList();
		}
	}
}