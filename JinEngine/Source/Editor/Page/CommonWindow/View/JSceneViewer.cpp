#include"JSceneViewer.h"  
#include"JSceneCameraList.h"
#include"../../JEditorAttribute.h" 
#include"../../../Event/JEditorEvent.h"
#include"../../../Gui/JGui.h"
#include"../../../Gui/JGuiImageInfo.h"
#include"../../../EditTool/JEditorCameraControl.h" 
#include"../../../../Object/Component/Camera/JCamera.h" 
#include"../../../../Object/Resource/Scene/JScene.h" 
#include"../../../../Object/GameObject/JGameObject.h" 
#include"../../../../Core/File/JFileIOHelper.h" 
#include"../../../../Graphic/GraphicResource/JGraphicResourceType.h"
#include"../../../../Graphic/GraphicResource/JGraphicResourceInterface.h"
#include"../../../../Graphic/GraphicResource/JGraphicResourceUserAccess.h"
#include"../../../../../ThirdParty/DirectX/TK/Src/d3dx12.h"
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
			EnterWindow(J_GUI_WINDOW_FLAG_NO_SCROLL_BAR |
				J_GUI_WINDOW_FLAG_NO_SCROLL_WITH_MOUSE |
				J_GUI_WINDOW_FLAG_NO_COLLAPSE);
			UpdateDocking();
			if (IsActivated() && scene.IsValid())
			{ 
				UpdateMouseClick(); 
				UpdateMouseWheel();
			 
				camList->Update(scene);
				selectedCam = camList->GetSelectedCam(scene);
				editorCamCtrl->Update(selectedCam.Get(), JGui::GetMousePos().x, JGui::GetMousePos().y, J_GUI_FOCUS_FLAG_CHILD_WINDOW);

				//camList->DisplayCameraList(scene.Get(), "Viewer", JGui::GetMainViewport()->Size * 0.15f);
 
				//Test Code
				/*
				* static bool allowShiroBack;
				JJGuiImpl::CheckBox("draw shiro background", allowShiroBack);

				if (allowShiroBack)
				{
					if (shiroBack == nullptr)
						shiroBack = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::PROJECT_SELECTOR_BACKGROUND);
					JJGuiImpl::Image(shiroBack.Get(), JVector2<float>(50, 50));
				}
				else
					shiroBack.Clear();*/

				if (selectedCam.IsValid())
				{ 
					JGuiImageInfo info(selectedCam.Get(), Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON);
					info.useFirstHandle = false;
					JGui::Image(info, JGui::GetWindowSize());
				}	 
				//JJGuiImpl::Image(*mainCamera, JGui::GetMainViewport()->WorkSize);
			}
			CloseWindow();
		}
		void JSceneViewer::UpdateMouseWheel()
		{
			editorCamCtrl->AddMovementFactor(JGui::GetMouseWheel());
		}
		void JSceneViewer::DoActivate()noexcept
		{
			JEditorWindow::DoActivate();  
			editorCamCtrl->SetMousePos(JGui::GetMousePos());
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