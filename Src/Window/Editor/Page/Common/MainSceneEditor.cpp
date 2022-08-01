#include"MainSceneEditor.h" 
#include"EditorCameraControl.h"
#include"../EditorAttribute.h"
#include"../../Utility/EditorUtility.h"  
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
	MainSceneEditor::MainSceneEditor(std::unique_ptr<EditorAttribute> attribute, const size_t ownerPageGuid)
		:SceneEditor(std::move(attribute), ownerPageGuid)
	{}
	MainSceneEditor::~MainSceneEditor()
	{

	}
	void MainSceneEditor::Initialize(EditorUtility* editorUtility)noexcept
	{		 
 
	}
	bool MainSceneEditor::Activate(EditorUtility* editorUtility)
	{
		if (EditorWindow::Activate(editorUtility))
		{
			const JOBJECT_FLAG flag = (JOBJECT_FLAG)(OBJECT_FLAG_EDITOR_OBJECT | OBJECT_FLAG_HIDDEN);

			mainScene = JResourceManager::Instance().GetMainScene();
			camObj = JGFU::CreateCamera(*mainScene->GetRootGameObject(), flag, false, "MainSceneEditorCamera");
			editorCamera = camObj->GetComponent<JCamera>();
			editorCamera->SetCameraState(J_CAMERA_STATE::RENDER);
			editorCamera->GetTransform()->SetPosition(position); 
			editorCamera->GetTransform()->SetRotation(rotation);
			frustumObj = JGFU::CreateDebugGameObject(*mainScene->GetRootGameObject(), flag, J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_FRUSTUM, J_DEFAULT_MATERIAL::DEBUG_LINE_YELLOW);
			return true;
		}
		else
			return false;
	}
	bool MainSceneEditor::DeActivate(EditorUtility* editorUtility)
	{
		if (EditorWindow::DeActivate(editorUtility))
		{
			position = editorCamera->GetTransform()->GetPosition();
			rotation = editorCamera->GetTransform()->GetRotation();
			JGameObject::EraseGameObject(camObj);
			JGameObject::EraseGameObject(frustumObj); 
			camObj = nullptr;
			editorCamera = nullptr;
			frustumObj = nullptr;
			return true;
		}
		else
			return false;
	}
	void MainSceneEditor::StoreEditorWindow(std::wofstream& stream)
	{
		EditorWindow::StoreEditorWindow(stream); 
		stream << position.x << " " << position.y << " " << position.z << '\n';
		stream << rotation.x << " " << rotation.y << " " << rotation.z << '\n';
	}
	void MainSceneEditor::LoadEditorWindow(std::wifstream& stream)
	{
		EditorWindow::LoadEditorWindow(stream);
		stream >> position.x >> position.y >> position.z ;
		stream >> rotation.x >> rotation.y >> rotation.z; 
	}
	void MainSceneEditor::UpdateWindow(EditorUtility* editorUtility)
	{
		EditorWindow::UpdateWindow(editorUtility);

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

			if (editorUtility->selectedObject != nullptr && editorUtility->selectedObject->GetObjectType() == J_OBJECT_TYPE::GAME_OBJECT)
				DrawGameObjectWidget(editorUtility);
		}
		ImVec2 windowSize = ImGui::GetWindowSize();	  
		ImGui::Image((ImTextureID)JGraphic::Instance().ResourceInterface()->GetGpuSrvDescriptorHandle(editorCamera->GetRsSrvHeapIndex()).ptr,
			ImVec2(windowSize.x, windowSize.y));
	}
	void MainSceneEditor::DrawGameObjectWidget(EditorUtility* editorUtility)
	{
		JGameObject* selectedGameObj = dynamic_cast<JGameObject*>(editorUtility->selectedObject);
		JTransform* transform = selectedGameObj->GetTransform();

		const float lineLength = 2.5f;
		ImDrawList* drawList = ImGui::GetWindowDrawList();
	}
}
 