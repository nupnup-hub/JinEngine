#include"JSceneObserver.h" 
#include"JEditorCameraControl.h"
#include"../../JEditorAttribute.h" 
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../../Core/File/JFileIOHelper.h"
#include"../../../../Object/Component/Camera/JCamera.h"  
#include"../../../../Object/Component/Transform/JTransform.h"
#include"../../../../Object/Resource/Scene/JScene.h" 
#include"../../../../Object/Resource/Mesh/JDefaultShapeType.h" 
#include"../../../../Object/Resource/Material/JDefaultMaterialType.h" 
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
				 
				UpdateMainCamFrustum();
				JImGuiImpl::Selectable("SpatialSpace##JSceneObserver", &isOpenSpatialOption, 0, JVector2<float>(ImGui::CalcTextSize("SpatialSpace").x, 0));
				ImGui::SameLine();
				if (JImGuiImpl::Selectable("MainCameraFrustum##JSceneObserver", &isMainCameraFrustumActivated, 0, JVector2<float>(ImGui::CalcTextSize("MainCameraFrustum").x, 0)))
				{
					if (isMainCameraFrustumActivated)
						MakeMainCamFrustum();
					else
						mainCamFrustum.Release()->BeginDestroy();
				}
				if(isOpenSpatialOption)
					SceneStructureOptionOnScreen();
				//JImGuiImpl::Image(*camera, ImGui::GetMainViewport()->WorkSize);
				JImGuiImpl::Image(*camera, ImGui::GetWindowSize());
			}
			CloseWindow();
		}
		void JSceneObserver::UpdateMainCamFrustum()noexcept
		{
			if (mainCamFrustum.IsValid())
			{
				JCamera* mainCamera = scene->GetMainCamera();
				JTransform* frumstumT = mainCamFrustum->GetTransform();

				float camWidth = (float)mainCamera->GetViewWidth();
				float camHeight = (float)mainCamera->GetViewHeight();
				float camNear = mainCamera->GetNear();
				float camFar = mainCamera->GetFar();
				float camDistance = camFar - camNear;

				frumstumT->SetScale(DirectX::XMFLOAT3((camWidth / camDistance) * 20, (camHeight / camDistance) * 20, 20));
				frumstumT->SetRotation(mainCamera->GetTransform()->GetRotation());
				frumstumT->SetPosition(mainCamera->GetTransform()->GetPosition());
			}
		}
		void JSceneObserver::SceneStructureOptionOnScreen()
		{   
			JImGuiImpl::BeginWindow("##SpatialSpaceWindow", &isOpenSpatialOption, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);
			JSceneSpaceSpatialInterface* iSceneSpace = scene->SpaceSpatialInterface();
			Core::JSceneSpatialStructureOption option = iSceneSpace->GetSpatialStructureOption();
			bool isSpatialStructureActivated = option.isSpatialStructureActivated;
			bool isDebugActivated = option.isDebugActivated;
			bool isDebugOnlyLeaf = option.isDebugOnlyLeaf;

			if (JImGuiImpl::CheckBox("OnSpaceSpatial##SceneObserver", isSpatialStructureActivated))
				iSceneSpace->SetSceneSpatialStructure(isSpatialStructureActivated);
			if (JImGuiImpl::CheckBox("Debug##SceneObserver", isDebugActivated))
				iSceneSpace->SetDebugBoundingBox(isDebugActivated);
			if (JImGuiImpl::CheckBox("DebugLeafOnly##SceneObserver", isDebugOnlyLeaf))
				iSceneSpace->SetDebugOnlyLeaf(isDebugOnlyLeaf);
			Core::JEnumInfo* enumInfo = Core::JReflectionInfo::Instance().GetEnumInfo(typeid(Core::J_SCENE_SPATIAL_STRUCTURE_TYPE).name());
			int selectedIndex = enumInfo->GetEnumIndex((Core::JEnum)option.type);
			 
			if (JImGuiImpl::BeginCombo("Type##SceneObserver", enumInfo->ElementName(enumInfo->EnumValue(selectedIndex)).c_str(), ImGuiComboFlags_HeightLarge))
			{
				const uint enumCount = enumInfo->GetEnumCount();
				int preSelected = selectedIndex;
				for (uint i = 0; i < enumCount; i++)
				{
					bool isSelected = (selectedIndex == i);
					if (JImGuiImpl::Selectable(enumInfo->ElementName(enumInfo->EnumValue(i)), &isSelected))
						selectedIndex = i;
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
				if (preSelected != selectedIndex)
					iSceneSpace->SetSceneSpatialStructureType((Core::J_SCENE_SPATIAL_STRUCTURE_TYPE)enumInfo->EnumValue(selectedIndex));
			}
			JImGuiImpl::EndWindow();
		}
		void JSceneObserver::MakeMainCamFrustum()
		{
			mainCamFrustum = Core::GetUserPtr(JGFU::CreateDebugGameObject(*scene->GetRootGameObject(),
				OBJECT_FLAG_EDITOR_OBJECT,
				J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_FRUSTUM,
				J_DEFAULT_MATERIAL::DEBUG_LINE_YELLOW));
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
				cameraObj = Core::GetUserPtr(camObj); 
				cameraObj->GetComponent<JCamera>()->StateInterface()->SetCameraState(J_CAMERA_STATE::RENDER);	
				if (isMainCameraFrustumActivated)
					MakeMainCamFrustum();
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
				if (isMainCameraFrustumActivated)
					mainCamFrustum.Release()->BeginDestroy();
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