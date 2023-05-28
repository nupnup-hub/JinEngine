#include"JSceneViewer.h"  
#include"JSceneCameraList.h"
#include"../../JEditorAttribute.h" 
#include"../../../Event/JEditorEvent.h"
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../EditTool/JEditorCameraControl.h" 
#include"../../../../Object/Component/Camera/JCamera.h" 
#include"../../../../Object/Resource/Scene/JScene.h" 
#include"../../../../Object/GameObject/JGameObject.h" 
#include"../../../../Core/File/JFileIOHelper.h" 
#include"../../../../Graphic/GraphicResource/JGraphicResourceType.h"
#include"../../../../Graphic/GraphicResource/JGraphicResourceInterface.h"
#include"../../../../Graphic/GraphicResource/JGraphicResourceUserAccess.h"
#include"../../../../../Lib/DirectX/TK/Src/d3dx12.h"
//Test
//#include"../../../../Object/Resource/JResourceManager.h" 
//#include"../../../../Object/Resource/Texture/JTexture.h" 
using namespace DirectX;

namespace JinEngine
{
	namespace Editor
	{
		JSceneViewer::JSceneViewer(const std::string& name,
			std::unique_ptr<JEditorAttribute> attribute,
			const J_EDITOR_PAGE_TYPE pageType,
			const J_EDITOR_WINDOW_FLAG windowFlag)
			:JEditorWindow(name, std::move(attribute), pageType, windowFlag)
		{
			camList = std::make_unique<JSceneCameraList>();
			editorCamCtrl = std::make_unique<JEditorCameraControl>();
		}
		JSceneViewer::~JSceneViewer() {}
		J_EDITOR_WINDOW_TYPE JSceneViewer::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::SCENE_VIEWER;
		}
		void JSceneViewer::Initialize(JUserPtr<JScene> newScene)
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
				camList->Update(scene);
				//camList->DisplayCameraList(scene.Get(), "Viewer", ImGui::GetMainViewport()->Size * 0.15f);
				selectedCam = camList->GetSelectedCam(scene);
 
				//Test Code
				/*
				* static bool allowShiroBack;
				JImGuiImpl::CheckBox("draw shiro background", allowShiroBack);

				if (allowShiroBack)
				{
					if (shiroBack == nullptr)
						shiroBack = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::PROJECT_SELECTOR_BACKGROUND);
					JImGuiImpl::Image(shiroBack.Get(), JVector2<float>(50, 50));
				}
				else
					shiroBack.Clear();*/

				if (selectedCam.IsValid())
				{
					if (IsFocus())
					{
						if (ImGui::IsMouseHoveringRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize()))
							editorCamCtrl->MouseMove(selectedCam.Get(), ImGui::GetMousePos().x, ImGui::GetMousePos().y);
						editorCamCtrl->KeyboardInput(selectedCam.Get());
					} 
					JImGuiImpl::Image(selectedCam.Get(), Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, ImGui::GetWindowSize());
				}	 
				//JImGuiImpl::Image(*mainCamera, ImGui::GetMainViewport()->WorkSize);
			}
			CloseWindow();
		}
		void JSceneViewer::DoActivate()noexcept
		{
			JEditorWindow::DoActivate();  
		}
		void JSceneViewer::DoDeActivate()noexcept
		{
			JEditorWindow::DoDeActivate();   
		}
		void JSceneViewer::StoreEditorWindow(std::wofstream& stream)
		{
			JEditorWindow::StoreEditorWindow(stream); 
			JFileIOHelper::StoreAtomicData(stream, L"LastSelectedCam:", camList->GetSelecetdIndex());
		}
		void JSceneViewer::LoadEditorWindow(std::wifstream& stream)
		{
			uint lastSelecetdCamIndex = 0;
			JEditorWindow::LoadEditorWindow(stream); 		 
			JFileIOHelper::LoadAtomicData(stream, lastSelecetdCamIndex);
			camList->SetSelecetdIndex(lastSelecetdCamIndex);
		}  
	}
}