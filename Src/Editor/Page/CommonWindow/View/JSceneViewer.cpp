#include"JSceneViewer.h" 
#include"JEditorCameraControl.h"
#include"../../JEditorAttribute.h"    
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../../Object/Component/Camera/JCamera.h" 
#include"../../../../Object/Resource/Scene/JScene.h" 
 
using namespace DirectX;

namespace JinEngine
{
	namespace Editor
	{
		JSceneViewer::JSceneViewer(const std::string& name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE pageType)
			:JEditorWindow(name, std::move(attribute), pageType)
		{
			editorCamCtrl = std::make_unique<JEditorCameraControl>();
		}
		JSceneViewer::~JSceneViewer() {}
		J_EDITOR_WINDOW_TYPE JSceneViewer::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::SCENE_VIEWER;
		}
		void JSceneViewer::Initialize(Core::JUserPtr<JScene> newScene)
		{
			scene = newScene;
		}
		void JSceneViewer::UpdateWindow()
		{
			EnterWindow(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
			UpdateDocking();
			if (IsActivated() && scene.IsValid())
			{ 
				UpdateMouseClick(); 
				JCamera* mainCamera = scene->GetMainCamera();
				if (IsFocus())
				{
					if (ImGui::IsMouseHoveringRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize()))
						editorCamCtrl->MouseMove(mainCamera, ImGui::GetMousePos().x, ImGui::GetMousePos().y);
					editorCamCtrl->KeyboardInput(mainCamera);
				}
				JImGuiImpl::Image(*mainCamera, ImGui::GetMainViewport()->WorkSize);
			}
			CloseWindow();
		}
	}
}