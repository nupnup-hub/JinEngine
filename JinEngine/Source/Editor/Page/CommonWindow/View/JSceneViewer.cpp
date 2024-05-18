/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#include"JSceneViewer.h"   
#include"../../JEditorAttribute.h" 
#include"../../../Event/JEditorEvent.h"
#include"../../../Gui/JGui.h"
#include"../../../Gui/JGuiImageInfo.h"
#include"../../../EditTool/JEditorIdentifierList.h" 
#include"../../../EditTool/JEditorCameraControl.h" 
#include"../../../../Object/Component/Camera/JCamera.h" 
#include"../../../../Object/Component/Light/JDirectionalLight.h" 
#include"../../../../Object/Component/Transform/JTransform.h" 

#include"../../../../Object/Resource/Scene/JScene.h" 
#include"../../../../Object/GameObject/JGameObject.h" 
#include"../../../../Core/File/JFileIOHelper.h" 
#include"../../../../Graphic/JGraphic.h"
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
				if (selectedCam.IsValid() && selectedCam->IsActivated())
				{
					editorCamCtrl->Update(selectedCam, JGui::GetMousePos().x, JGui::GetMousePos().y, J_GUI_FOCUS_FLAG_CHILD_WINDOW);
					JGuiImageInfo info(selectedCam.Get(), Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON);
					if (selectedCam->AllowPostProcess())
					{
						auto gInterface = selectedCam->GraphicResourceUserInterface();
						info.dataIndex = gInterface.GetResourceDataIndex(Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, Graphic::J_GRAPHIC_TASK_TYPE::APPLY_POST_PROCESS_RESULT);
					}
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
#if DEVELOP
				TestLight();
#endif
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
		void JSceneViewer::TestLight()
		{
			if (JGraphic::Instance().GetGraphicOptionRef().debugging.testTrigger02)
			{
				//static float xDeltaFloat = 0.025f;
				static float zDeltaFloat = 0.075f;
				//static int xDir = 1;
				static int zDir = 1;

				auto dLight = scene->GetFirstDirectionalLight();
				if (dLight == nullptr)
					return;

				auto t = dLight->GetOwner()->GetTransform();
				auto curRot = t->GetRotation();

				//if (curRot.x > 90)
				//	xDir = -1;
				//else if (curRot.x < -90)
				//	xDir = 1;
				if (curRot.z > 90)
					zDir = -1;
				else if (curRot.z <= 0)
					zDir = 1;

				t->SetRotation(curRot + JVector3F(0, 0, zDeltaFloat * zDir));

				curRot = t->GetRotation();
				//JVector3F xp90 = JVector3F(100, 120, 255);
				//JVector3F xn90 = JVector3F(233, 240, 250);
				JVector3F zp90 = JVector3F(20, 125, 255);
				JVector3F zp0 = JVector3F(255, 125, 20);

				//JVector3F xFactor = JVector3F(0, 0, 0);
				JVector3F zFactor = JVector3F(0, 0, 0);
				//xFactor = JVector3F::EWMA(xn90, xp90, (curRot.x + 90) / 180.0f);
				zFactor = JVector3F::EWMA(zp90, zp0, max(curRot.z, 1e-06) / 90.0f);

				dLight->SetColor(zFactor / 255.0f);
			}
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