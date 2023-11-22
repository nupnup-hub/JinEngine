#include"JSceneViewer.h"   
#include"../../JEditorAttribute.h" 
#include"../../../Event/JEditorEvent.h"
#include"../../../Gui/JGui.h"
#include"../../../Gui/JGuiImageInfo.h"
#include"../../../EditTool/JEditorIdentifierList.h" 
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
			camList = std::make_unique<JEditorIdentifierList>();
			editorCamCtrl = std::make_unique<JEditorCameraControl>();

			camList->SetTypeGuid(JCamera::StaticTypeInfo().TypeGuid());
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
 
				auto preCursor = JGui::GetCursorPos();
				if (selectedCam.IsValid())
				{
					editorCamCtrl->Update(selectedCam, JGui::GetMousePos().x, JGui::GetMousePos().y, J_GUI_FOCUS_FLAG_CHILD_WINDOW);
					JGuiImageInfo info(selectedCam.Get(), Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON);
					info.useFirstHandle = false;
					JGui::Image(info, JGui::GetWindowSize());
				}	  
				JGui::SetCursorPos(preCursor);

				auto canDispalyLam = [](JEditorIdentifierList::TypeConditionDataSet set, Core::JIdentifier* iden)
				{
					return static_cast<JCamera*>(iden)->GetOwner()->GetOwnerGuid() == set.ownerGuid;
				};
				JEditorIdentifierList::DisplayDataSet dataSet(GetName());
				JEditorIdentifierList::TypeConditionDataSet conditionSet;
				conditionSet.ownerGuid = scene->GetGuid();
				dataSet.canDisplayB = Core::UniqueBind(std::make_unique<JEditorIdentifierList::CanDisplayF::Functor>(canDispalyLam),
					std::move(conditionSet), Core::empty);
				dataSet.callWindowAutomatically = false;
				dataSet.listWidth = JGui::GetClientWindowSize().x * 0.25f;
				dataSet.customComboFlag = J_GUI_COMBO_FLAG_HEIGHT_SMALL;
				camList->Display(dataSet);
				selectedCam = camList->GetSelectedUser<JCamera>(); 
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
		void JSceneViewer::LoadEditorWindow(JFileIOTool& tool)
		{
			size_t lastSelecetdCamGuid = 0;
			float movementFactor = 0;
			JEditorWindow::LoadEditorWindow(tool);
			JFileIOHelper::LoadAtomicData(tool, lastSelecetdCamGuid, "LastSelectedCamGuid:");
			JFileIOHelper::LoadAtomicData(tool, movementFactor, "MovemnetFactor:");
			camList->SetSelecetdGuid(lastSelecetdCamGuid); 
			editorCamCtrl->SetMovemnetFactor(movementFactor);
		}  
		void JSceneViewer::StoreEditorWindow(JFileIOTool& tool)
		{
			JEditorWindow::StoreEditorWindow(tool);
			JFileIOHelper::StoreAtomicData(tool, camList->GetSelectedGuid(), "LastSelectedCamGuid:");
			JFileIOHelper::StoreAtomicData(tool, editorCamCtrl->GetMovemnetFactor(), "MovemnetFactor:");
		}
	}
}
/*

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
					shiroBack.Clear();
*/