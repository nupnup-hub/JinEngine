#include"PreviewSceneEditor.h" 
#include"EditorCameraControl.h"
#include"../EditorAttribute.h"
#include"../../Utility/EditorUtility.h"  
#include"../../../../Object/Resource/Scene/Preview/PreviewScene.h"
#include"../../../../Object/Component/Camera/JCamera.h"  
#include"../../../../Object/Resource/JResourceManager.h" 
#include"../../../../Graphic/JGraphic.h"
#include"../../../../Graphic/JGraphicResourceManager.h" 
#include"../../../../../Lib/imgui/imgui.h" 

namespace JinEngine
{
	PreviewSceneEditor::PreviewSceneEditor(std::unique_ptr<EditorAttribute> attribute, const size_t ownerPageGuid)
		:SceneEditor(std::move(attribute), ownerPageGuid)
	{}
	PreviewSceneEditor::~PreviewSceneEditor(){}
	void PreviewSceneEditor::Initialize(PreviewScene* previewScene, const uint cameraIndex)noexcept
	{
		PreviewSceneEditor::previewScene = previewScene;
		PreviewSceneEditor::cameraIndex = cameraIndex; 
	}
	bool PreviewSceneEditor::Activate(EditorUtility* editorUtility)
	{
		if (Editor::Activate(editorUtility))
		{
			previewScene->GetPreviewCamera(cameraIndex)->SetCameraState(J_CAMERA_STATE::RENDER);
			return true;
		}
		else
			return false;
	}
	bool PreviewSceneEditor::DeActivate(EditorUtility* editorUtility)
	{
		if (Editor::DeActivate(editorUtility))
		{
			previewScene->GetPreviewCamera(cameraIndex)->SetCameraState(J_CAMERA_STATE::IDEL);
			return true;
		}
		else
			return false;
	}
	void PreviewSceneEditor::UpdateWindow(EditorUtility* editorUtility)
	{
		EditorWindow::UpdateWindow(editorUtility);
		if (IsFocus())
		{
			if (ImGui::IsMouseHoveringRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize()))
				editorCamCtrl->MouseMove(previewScene->GetPreviewCamera(cameraIndex), ImGui::GetMousePos().x, ImGui::GetMousePos().y);
			editorCamCtrl->KeyboardInput(previewScene->GetPreviewCamera(cameraIndex));
		}

		ImVec2 windowSize = ImGui::GetWindowSize();
		ImGui::Image((ImTextureID)JGraphic::Instance().ResourceInterface()->GetGpuSrvDescriptorHandle(previewScene->GetPreviewCamera(cameraIndex)->GetRsSrvHeapIndex()).ptr,
			ImVec2(windowSize.x, windowSize.y));
	}
}
