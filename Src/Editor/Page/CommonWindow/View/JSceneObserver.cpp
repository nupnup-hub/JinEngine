#include"JSceneObserver.h" 
#include"JEditorCameraControl.h"
#include"../../JEditorAttribute.h" 
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../../Core/File/JFileIOHelper.h"
#include"../../../../Object/Component/Camera/JCamera.h"  
#include"../../../../Object/Component/Transform/JTransform.h"
#include"../../../../Object/Resource/Scene/JScene.h" 
#include"../../../../Object/GameObject/JGameObject.h" 
#include"../../../../Object/GameObject/JGameObjectFactoryUtility.h"   
#include<fstream>

namespace JinEngine
{
	namespace Editor
	{
		JSceneObserver::JSceneObserver(const std::string& name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE pageType)
			:JEditorWindow(name, std::move(attribute), pageType)
		{
			editorCamCtrl = std::make_unique<JEditorCameraControl>();
		}
		JSceneObserver::~JSceneObserver(){}
		J_EDITOR_WINDOW_TYPE JSceneObserver::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::SCENE_OBSERVER;
		}
		void JSceneObserver::Initialize(Core::JUserPtr<JScene> newScene, const std::wstring& editorCameraName)noexcept
		{
			scene = newScene;
			JSceneObserver::editorCameraName = editorCameraName;
			lastCamPos = { 0,0,0 };
			lastCamRot = { 0,0,0 };
		}
		void JSceneObserver::UpdateWindow()
		{
			EnterWindow(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
			UpdateDocking();
			if (IsActivated() && scene.IsValid() && cameraObj.IsValid())
			{  
				UpdateMouseClick(); 
				JCamera* camera = cameraObj->GetComponent<JCamera>();
				if (IsFocus())
				{
					if (ImGui::IsMouseHoveringRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize()))
						editorCamCtrl->MouseMove(camera, ImGui::GetMousePos().x, ImGui::GetMousePos().y);
					editorCamCtrl->KeyboardInput(camera);
				} 
				JImGuiImpl::Image(*camera, ImGui::GetMainViewport()->WorkSize);
			}
			CloseWindow();
		}
		void JSceneObserver::DoActivate()noexcept
		{
			JEditorWindow::DoActivate();
			if (scene.IsValid())
			{
				JGameObject* camObj = JGFU::CreateCamera(*scene->GetRootGameObject(), OBJECT_FLAG_UNIQUE_EDITOR_OBJECT, false, editorCameraName);
				camObj->SetName(L"Observer");
				camObj->GetTransform()->SetPosition(lastCamPos.ConvertXM());
				camObj->GetTransform()->SetRotation(lastCamRot.ConvertXM());
				cameraObj = Core::GetUserPtr<JGameObject>(camObj->GetGuid()); 
				cameraObj->GetComponent<JCamera>()->StateInterface()->SetCameraState(J_CAMERA_STATE::RENDER);	
			}
		}
		void JSceneObserver::DoDeActivate()noexcept
		{
			JEditorWindow::DoDeActivate();
			if (cameraObj.IsValid())
			{
				JCamera* cam = cameraObj->GetComponent<JCamera>();
				lastCamPos = cam->GetTransform()->GetPosition();
				lastCamRot = cam->GetTransform()->GetRotation();
				cameraObj.Release()->BeginDestroy();
			}
		}
		void JSceneObserver::StoreEditorWindow(std::wofstream& stream)
		{
			JEditorWindow::StoreEditorWindow(stream);
			JFileIOHelper::StoreXMFloat3(stream, L"lastCamPos:", lastCamPos.ConvertXM());
			JFileIOHelper::StoreXMFloat3(stream, L"lastCamRot:", lastCamRot.ConvertXM());
		}
		void JSceneObserver::LoadEditorWindow(std::wifstream& stream)
		{ 
			DirectX::XMFLOAT3 lastPos;
			DirectX::XMFLOAT3 lastRot;

			JEditorWindow::LoadEditorWindow(stream);
			JFileIOHelper::LoadXMFloat3(stream, lastPos);
			JFileIOHelper::LoadXMFloat3(stream, lastRot);

			lastCamPos = lastPos;
			lastCamRot = lastRot;
		}
	}
}