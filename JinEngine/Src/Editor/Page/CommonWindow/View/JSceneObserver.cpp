#include"JSceneObserver.h" 
#include"JEditorCameraControl.h"
#include"../../JEditorAttribute.h" 
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../Utility/JEditorBinaryTreeView.h"
#include"../../../../Core/File/JFileIOHelper.h"
#include"../../../../Object/Component/Camera/JCamera.h"  
#include"../../../../Object/Component/Transform/JTransform.h"
#include"../../../../Object/Component/Light/JLight.h"
#include"../../../../Object/Resource/JResourceManager.h" 
#include"../../../../Object/Resource/Scene/JScene.h" 
#include"../../../../Object/Resource/Mesh/JDefaultShapeType.h" 
#include"../../../../Object/Resource/Material/JDefaultMaterialType.h" 
#include"../../../../Object/GameObject/JGameObject.h" 
#include"../../../../Object/GameObject/JGameObjectFactoryUtility.h"   
#include"../../../../Graphic/JGraphic.h"
#include"../../../../Utility/JCommonUtility.h"
//#include<fstream>

//Debug
//#include"../../../../Object/Component/RenderItem/JRenderItem.h"
//#include"../../../../Object/Resource/Material/JMaterial.h"  
//#include"../../../../Object/Resource/JResourceObjectFactory.h"  
//#include"../../../../Core/File/JFileIOHelper.h"
//#include"../../../../Debug/JDebugTimer.h"

namespace JinEngine
{
	namespace Editor
	{
		JSceneObserver::JSceneObserver(const std::string& name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE pageType)
			:JEditorWindow(name, std::move(attribute), pageType)
		{
			editorCamCtrl = std::make_unique<JEditorCameraControl>();
			editorBTreeView = std::make_unique<JEditorBinaryTreeView>();
		}
		JSceneObserver::~JSceneObserver() 	{}
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
						JObject::BeginDestroy(mainCamFrustum.Release());
				}
				ImGui::SameLine();
				JImGuiImpl::Selectable("ShadowMap##JSceneObserver", &isShadowViewer, 0, JVector2<float>(ImGui::CalcTextSize("ShadowMap").x, 0));
				ImGui::SameLine();
				JImGuiImpl::Selectable("OcclusionResult##JSceneObserver", &isOcclusionViewer, 0, JVector2<float>(ImGui::CalcTextSize("OcclusionResult").x, 0));

				if (isOpenSpatialOption)
					SceneStructureOptionOnScreen();
				if (isShadowViewer)
					ShadowMapViewerOnScreen();
				if (isOcclusionViewer)
					OcclusionResultOnScreen();
				//Debug
				ImGui::SameLine();
				CreateShapeGroup(J_DEFAULT_SHAPE::DEFAULT_SHAPE_CUBE);
				//JImGuiImpl::Image(*camera, ImGui::GetMainViewport()->WorkSize);
				JImGuiImpl::Image(*camera, ImGui::GetWindowSize());
			}
			CloseWindow(); 
		}
		void JSceneObserver::SceneStructureOptionOnScreen()
		{
			JImGuiImpl::BeginWindow("##SpatialSpaceWindow", &isOpenSpatialOption, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);
			Core::JEnumInfo* enumInfo = Core::JReflectionInfo::Instance().GetEnumInfo(typeid(Core::J_SPACE_SPATIAL_TYPE).name());
			if (JImGuiImpl::BeginCombo("Type##SceneObserver", enumInfo->ElementName(enumInfo->EnumValue(spaceSpatialIndex)).c_str(), ImGuiComboFlags_HeightLarge))
			{
				const uint enumCount = enumInfo->GetEnumCount();
				for (uint i = 0; i < enumCount; i++)
				{
					bool isSelected = (spaceSpatialIndex == i);
					if (JImGuiImpl::Selectable(enumInfo->ElementName(enumInfo->EnumValue(i)), &isSelected))
						spaceSpatialIndex = i;
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if (spaceSpatialIndex == (int)Core::J_SPACE_SPATIAL_TYPE::OCTREE)
				OctreeOptionOnScreen();
			else if (spaceSpatialIndex == (int)Core::J_SPACE_SPATIAL_TYPE::BVH)
			{
				BvhOptionOnScreen();
				DebugTreeOnScreen(Core::J_SPACE_SPATIAL_TYPE::BVH, "Bvh");
			}
			else if (spaceSpatialIndex == (int)Core::J_SPACE_SPATIAL_TYPE::KD_TREE)
			{
				KdTreeOptionOnScreen();
				DebugTreeOnScreen(Core::J_SPACE_SPATIAL_TYPE::KD_TREE, "Kd-Tree");
			}
			JImGuiImpl::EndWindow();
		}
		void JSceneObserver::OctreeOptionOnScreen()
		{
			JSceneSpaceSpatialInterface* iSceneSpace = scene->SpaceSpatialInterface();
			Core::JOctreeOption octreeOption = iSceneSpace->GetOctreeOption();
			int minSize = octreeOption.minSize;
			int octreeSizeSquare = octreeOption.octreeSizeSquare;

			bool isChanged = false;
			isChanged |= JImGuiImpl::InputInt("minSize##JSceneObserver", &minSize, ImGuiInputTextFlags_EnterReturnsTrue);
			isChanged |= JImGuiImpl::InputInt("octreeSizeSquare##JSceneObserver", &octreeSizeSquare, ImGuiInputTextFlags_EnterReturnsTrue);
			isChanged |= JImGuiImpl::InputFloat("looseFactor##JSceneObserver", &octreeOption.looseFactor, ImGuiInputTextFlags_EnterReturnsTrue);
			isChanged |= CommonOptionOnScreen("Octree", octreeOption.commonOption); 

			octreeOption.minSize = minSize;
			octreeOption.octreeSizeSquare = octreeSizeSquare;
			if (isChanged)
				iSceneSpace->SetOctreeOption(octreeOption);
		}
		void JSceneObserver::BvhOptionOnScreen()
		{
			JSceneSpaceSpatialInterface* iSceneSpace = scene->SpaceSpatialInterface();
			Core::JBvhOption bvhOption = iSceneSpace->GetBvhOption();

			bool isChanged = false;

			Core::JEnumInfo* buildEnumInfo = Core::JReflectionInfo::Instance().GetEnumInfo(typeid(Core::J_SPACE_SPATIAL_BUILD_TYPE).name());
			int buildIndex = buildEnumInfo->GetEnumIndex((int)bvhOption.buildType);

			if (JImGuiImpl::BeginCombo("BuildType##Bvh_SceneObserver", buildEnumInfo->ElementName(buildEnumInfo->EnumValue(buildIndex)).c_str(), ImGuiComboFlags_HeightLarge))
			{
				const uint enumCount = buildEnumInfo->GetEnumCount();
				for (uint i = 0; i < enumCount; i++)
				{
					bool isSelected = (buildIndex == i);
					if (JImGuiImpl::Selectable(buildEnumInfo->ElementName(buildEnumInfo->EnumValue(i)), &isSelected))
					{
						if (buildIndex != i)
							isChanged = true;
						buildIndex = i;
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			Core::JEnumInfo* splitEnumInfo = Core::JReflectionInfo::Instance().GetEnumInfo(typeid(Core::J_SPACE_SPATIAL_SPLIT_TYPE).name());
			int splitIndex = splitEnumInfo->GetEnumIndex((int)bvhOption.splitType);
			if (JImGuiImpl::BeginCombo("SplitType##Bvh_SceneObserver", splitEnumInfo->ElementName(splitEnumInfo->EnumValue(splitIndex)).c_str(), ImGuiComboFlags_HeightLarge))
			{
				const uint enumCount = splitEnumInfo->GetEnumCount();
				for (uint i = 0; i < enumCount; i++)
				{
					bool isSelected = (splitIndex == i);
					if (JImGuiImpl::Selectable(splitEnumInfo->ElementName(splitEnumInfo->EnumValue(i)), &isSelected))
					{
						if (splitIndex != i)
							isChanged = true;
						splitIndex = i;
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			isChanged |= CommonOptionOnScreen("Bvh", bvhOption.commonOption);
			if (isChanged)
				iSceneSpace->SetBvhOption(bvhOption);
		}
		void JSceneObserver::KdTreeOptionOnScreen()
		{
			JSceneSpaceSpatialInterface* iSceneSpace = scene->SpaceSpatialInterface();
			Core::JKdTreeOption kdTreeOption = iSceneSpace->GetKdTreeOption();

			bool isChanged = false;

			Core::JEnumInfo* buildEnumInfo = Core::JReflectionInfo::Instance().GetEnumInfo(typeid(Core::J_SPACE_SPATIAL_BUILD_TYPE).name());
			int buildIndex = buildEnumInfo->GetEnumIndex((int)kdTreeOption.buildType);

			if (JImGuiImpl::BeginCombo("BuildType##KdTree_SceneObserver", buildEnumInfo->ElementName(buildEnumInfo->EnumValue(buildIndex)).c_str(), ImGuiComboFlags_HeightLarge))
			{
				const uint enumCount = buildEnumInfo->GetEnumCount();
				for (uint i = 0; i < enumCount; i++)
				{
					bool isSelected = (buildIndex == i);
					if (JImGuiImpl::Selectable(buildEnumInfo->ElementName(buildEnumInfo->EnumValue(i)), &isSelected))
					{
						if (buildIndex != i)
							isChanged = true;
						buildIndex = i;
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			Core::JEnumInfo* splitEnumInfo = Core::JReflectionInfo::Instance().GetEnumInfo(typeid(Core::J_SPACE_SPATIAL_SPLIT_TYPE).name());
			int splitIndex = splitEnumInfo->GetEnumIndex((int)kdTreeOption.splitType);
			if (JImGuiImpl::BeginCombo("SplitType##KdTree_SceneObserver", splitEnumInfo->ElementName(splitEnumInfo->EnumValue(splitIndex)).c_str(), ImGuiComboFlags_HeightLarge))
			{
				const uint enumCount = splitEnumInfo->GetEnumCount();
				for (uint i = 0; i < enumCount; i++)
				{
					bool isSelected = (splitIndex == i);
					if (JImGuiImpl::Selectable(splitEnumInfo->ElementName(splitEnumInfo->EnumValue(i)), &isSelected))
					{
						if (splitIndex != i)
							isChanged = true;
						splitIndex = i;
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			isChanged |= CommonOptionOnScreen("KdTree", kdTreeOption.commonOption);
			if (isChanged)
				iSceneSpace->SetKdTreeOption(kdTreeOption);
		}
		bool JSceneObserver::CommonOptionOnScreen(const std::string& uniqueName, Core::JSpaceSpatialOption& commonOption)
		{
			bool isChanged = false;
			isChanged |= JImGuiImpl::CheckBox("Activate##JSceneObserver" + uniqueName, commonOption.isSpaceSpatialActivated);
			isChanged |= JImGuiImpl::CheckBox("DebugBBox##JSceneObserver" + uniqueName, commonOption.isDebugActivated);
			isChanged |= JImGuiImpl::CheckBox("DebugLeafOnly##JSceneObserver" + uniqueName, commonOption.isDebugLeafOnly);
			isChanged |= JImGuiImpl::CheckBox("CullingActivate##JSceneObserver" + uniqueName, commonOption.isCullingActivated);
			return isChanged;
		}
		void JSceneObserver::DebugTreeOnScreen(const Core::J_SPACE_SPATIAL_TYPE type, const std::string& uniqueLabel)
		{
			JImGuiImpl::CheckBox("TreeViewer##JSceneObserver" + uniqueLabel, isSpatialSpaceTreeViewer);
			if (isSpatialSpaceTreeViewer)
			{
				JSceneSpaceSpatialInterface* iSceneSpace = scene->SpaceSpatialInterface();
				editorBTreeView->Clear();
				iSceneSpace->BuildDebugTree(type, *editorBTreeView);
				editorBTreeView->TreeOnScreen(uniqueLabel, isSpatialSpaceTreeViewer);
			}
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
		void JSceneObserver::MakeMainCamFrustum()
		{ 
			mainCamFrustum = Core::GetUserPtr(JGFU::CreateDebugGameObject(*scene->GetRootGameObject(),
				OBJECT_FLAG_EDITOR_OBJECT,
				J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_FRUSTUM,
				J_DEFAULT_MATERIAL::DEBUG_LINE_YELLOW));
		}
		void JSceneObserver::ShadowMapViewerOnScreen()
		{
			JImGuiImpl::BeginWindow("##ShadowMapViewerWindow", &isShadowViewer, ImGuiWindowFlags_NoDocking);
			const std::vector<JComponent*> litVec = scene->CashInterface()->GetComponentCashVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT);
			const uint litCount = (uint)litVec.size();

			std::vector<JLight*> shadowLitVec;
			for (uint i = 0; i < litCount; ++i)
			{
				JLight* lit = static_cast<JLight*>(litVec[i]);
				if (lit->IsShadowActivated())
					shadowLitVec.push_back(lit);
			}

			const uint shadowLitCount = (uint)shadowLitVec.size();
			if (shadowLitCount > 0)
			{
				if (JImGuiImpl::BeginCombo("Light##SceneObserver", JCUtil::WstrToU8Str(shadowLitVec[shadowIndex]->GetName()).c_str(), ImGuiComboFlags_HeightLarge))
				{
					for (uint i = 0; i < shadowLitCount; ++i)
					{
						bool isSelected = (shadowIndex == i);
						if (JImGuiImpl::Selectable(JCUtil::WstrToU8Str(shadowLitVec[i]->GetName()), &isSelected))
							shadowIndex = i;
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					JImGuiImpl::EndCombo();
				}
				JImGuiImpl::Image(*shadowLitVec[shadowIndex], ImGui::GetWindowSize());
			}
			JImGuiImpl::EndWindow();
		}
		void JSceneObserver::OcclusionResultOnScreen()
		{
			JImGuiImpl::BeginWindow("##OcclusionResultWindow", &isOcclusionViewer, ImGuiWindowFlags_NoDocking);
			ImGui::Image((ImTextureID)(JGraphic::Instance().GetOcclusionSrvHandle()).ptr, ImGui::GetWindowSize());
			JImGuiImpl::EndWindow();
		}
		void JSceneObserver::DoActivate()noexcept
		{
			JEditorWindow::DoActivate();
			if (scene.IsValid())
			{
				JGameObject* camObj = JGFU::CreateCamera(*scene->GetRootGameObject(), OBJECT_FLAG_UNIQUE_EDITOR_OBJECT, false, editorCameraName);
				camObj->SetName(L"Observer");
				camObj->GetTransform()->SetPosition(lastCamPos.ConvertXMF());
				camObj->GetTransform()->SetRotation(lastCamRot.ConvertXMF());
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
				JObject::BeginDestroy(cameraObj.Release());
				if (isMainCameraFrustumActivated)
					JObject::BeginDestroy(mainCamFrustum.Release());
			}
		}
		void JSceneObserver::StoreEditorWindow(std::wofstream& stream)
		{
			JEditorWindow::StoreEditorWindow(stream);
			JFileIOHelper::StoreXMFloat3(stream, L"lastCamPos:", lastCamPos.ConvertXMF());
			JFileIOHelper::StoreXMFloat3(stream, L"lastCamRot:", lastCamRot.ConvertXMF());
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
		void JSceneObserver::CreateShapeGroup(const J_DEFAULT_SHAPE& shape)
		{
			if (JImGuiImpl::Button("MakeObject", JVector2<float>(ImGui::CalcTextSize("MakeObject").x, 0)))
			{
				for (uint i = 0; i < 30; ++i)
				{
					for (uint j = 0; j < 15; ++j)
					{
						JGameObject* cube = JGFU::CreateShape(*scene->GetRootGameObject(), OBJECT_FLAG_NONE, shape);
						cube->GetTransform()->SetScale(DirectX::XMFLOAT3(4, 4, 4));
						cube->GetTransform()->SetPosition(DirectX::XMFLOAT3(-8 * i, 0, -8 * j));
					}
					for (uint j = 0; j < 15; ++j)
					{
						JGameObject* cube = JGFU::CreateShape(*scene->GetRootGameObject(), OBJECT_FLAG_NONE, shape);
						cube->GetTransform()->SetScale(DirectX::XMFLOAT3(4, 4, 4));
						cube->GetTransform()->SetPosition(DirectX::XMFLOAT3(8 * i, 0, 8 * j));
					}
				}
			}
		}
		/*
		void JSceneObserver::CreateDebugMaterial()noexcept
		{
			for (uint i = 0; i < debugMaterialCount; ++i)
			{
				JMaterial::InitData initData = JMaterial::InitData(L"TestM" + std::to_wstring(i),
					Core::MakeGuid(),
					OBJECT_FLAG_EDITOR_OBJECT,
					JResourceManager::Instance().GetEditorResourceDirectory());

				debugMaterial[i] = JRFI<JMaterial>::Create(Core::JPtrUtil::MakeOwnerPtr<JMaterial::InitData>(L"TestM" + std::to_wstring(i),
					Core::MakeGuid(),
					OBJECT_FLAG_EDITOR_OBJECT,
					JResourceManager::Instance().GetEditorResourceDirectory()));
				debugMaterial[i]->SetAlbedoColor(color[i].ConvertXMF());
			}
		}
		void JSceneObserver::DestroyDebugMaterial()noexcept
		{
			for (uint i = 0; i < debugMaterialCount; ++i)
			{
				JObject::BeginDestroy(debugMaterial[i]);
				debugMaterial[i] = nullptr;
			}
		}*/

	}
}

/*

				JSceneSpaceSpatialInterface* iSceneSpace = scene->SpaceSpatialInterface();
				std::vector<JGameObject*> alignObject = iSceneSpace->GetAlignedObject(scene->GetMainCamera()->GetBoundingFrustum());

				const uint alignObjCount = (uint)alignObject.size();
				for (uint i = 0; i < alignObjCount; ++i)
				{
					if (i < debugMaterialCount)
						alignObject[i]->GetRenderItem()->SetMaterial(0, debugMaterial[i]);
				}

*/