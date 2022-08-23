#include"JPreviewSceneEditor.h" 
#include"JEditorCameraControl.h"
#include"../../JEditorAttribute.h"
#include"../../../Utility/JEditorUtility.h"  
#include"../../../../Object/Resource/Scene/Preview/JPreviewScene.h"
#include"../../../../Object/Component/Camera/JCamera.h"  
#include"../../../../Object/Resource/JResourceManager.h" 
#include"../../../../Graphic/JGraphic.h"
#include"../../../../Graphic/JGraphicResourceManager.h" 
#include"../../../../../Lib/imgui/imgui.h" 

namespace JinEngine
{
	namespace Editor
	{
		JPreviewSceneEditor::JPreviewSceneEditor(std::unique_ptr<JEditorAttribute> attribute, const size_t ownerPageGuid)
			:JSceneEditor(std::move(attribute), ownerPageGuid)
		{}
		JPreviewSceneEditor::~JPreviewSceneEditor() {}
		void JPreviewSceneEditor::Initialize(JPreviewScene* previewScene, const uint cameraIndex)noexcept
		{
			JPreviewSceneEditor::previewScene = previewScene;
			JPreviewSceneEditor::cameraIndex = cameraIndex;
		}
		bool JPreviewSceneEditor::Activate(JEditorUtility* editorUtility)
		{
			if (Editor::Activate(editorUtility))
			{
				//previewScene->GetPreviewCamera(cameraIndex)->SetCameraState(J_CAMERA_STATE::RENDER);
				return true;
			}
			else
				return false;
		}
		bool JPreviewSceneEditor::DeActivate(JEditorUtility* editorUtility)
		{
			if (Editor::DeActivate(editorUtility))
			{
				//previewScene->GetPreviewCamera(cameraIndex)->SetCameraState(J_CAMERA_STATE::IDEL);
				return true;
			}
			else
				return false;
		}
		void JPreviewSceneEditor::UpdateWindow(JEditorUtility* editorUtility)
		{
			JEditorWindow::UpdateWindow(editorUtility);
			if (IsFocus())
			{
				//if (ImGui::IsMouseHoveringRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize()))
			//		editorCamCtrl->MouseMove(previewScene->GetPreviewCamera(cameraIndex), ImGui::GetMousePos().x, ImGui::GetMousePos().y);
			//	editorCamCtrl->KeyboardInput(previewScene->GetPreviewCamera(cameraIndex));
			}

			//ImVec2 windowSize = ImGui::GetWindowSize();
			//ImGui::Image((ImTextureID)JGraphic::Instance().ResourceInterface()->GetGpuSrvDescriptorHandle(previewScene->GetPreviewCamera(cameraIndex)->GetRsSrvHeapIndex()).ptr,
			//	ImVec2(windowSize.x, windowSize.y));
		}
	}
}