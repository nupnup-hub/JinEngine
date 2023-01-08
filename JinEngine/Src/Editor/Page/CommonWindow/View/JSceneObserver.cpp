#include"JSceneObserver.h" 
#include"../../JEditorPageShareData.h"
#include"../../JEditorAttribute.h" 
#include"../../../Menubar/JEditorMenuBar.h"
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../Utility/JEditorBinaryTreeView.h"
#include"../../../Utility/JEditorGameObjectSurpportTool.h"
#include"../../../Utility/JEditorCameraControl.h" 
#include"../../../../Core/File/JFileIOHelper.h"
#include"../../../../Object/Component/Camera/JCamera.h"  
#include"../../../../Object/Component/Transform/JTransform.h"
#include"../../../../Object/Component/RenderItem/JRenderItem.h"
#include"../../../../Object/Component/Light/JLight.h" 
#include"../../../../Object/Resource/Scene/JScene.h"  
#include"../../../../Object/Resource/JResourceManager.h"  
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
		namespace
		{
			//Debug
			/*static constexpr int debugMaterialCount = 8;
			JMaterial* debugMaterial[debugMaterialCount];
			JVector4<float> color[debugMaterialCount] = { {0.85f, 0.15f, 0.15f, 0.75f},
			{0.85f, 0.4f, 0.15f, 0.75f} ,
			{0.85f, 0.85f, 0.15f, 0.75f} ,
			{0.15f, 0.85f, 0.15f, 0.75f} ,
			{0.15f, 0.15f, 0.85f, 0.75f} ,
			{0.3f, 0.85f, 0.45f, 0.75f} ,
			{0.6f, 0.15f, 0.9f, 0.75f} ,
			{0.25f, 0.25f, 0.25f, 0.75f} };*/
			//uint streamCount = 0;
			static constexpr uint optionTypeSubTypeCount = 3;
		}
		
		JSceneObserver::JSceneObserver(const std::string& name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE pageType)
			:JEditorWindow(name, std::move(attribute), pageType)
		{
			editorCamCtrl = std::make_unique<JEditorCameraControl>();
			editorBTreeView = std::make_unique<JEditorBinaryTreeView>();		 
			openNodeFunctor = std::make_unique<OpenMenuNodeT::Functor>(&JSceneObserver::OpenObserverSettingNode, this);
			activateNodeFunctor = std::make_unique<ActivateMenuNodeT::Functor>(&JSceneObserver::ActivateObserverSetting, this);
			deActivateNodeFunctor = std::make_unique<DeActivateMenuNodeT::Functor>(&JSceneObserver::DeActivateObserverSetting, this);
			updateNodeFunctor = std::make_unique<UpdateMenuNodeT::Functor>(&JSceneObserver::UpdateObserverSetting, this);
 
			std::unique_ptr<JEditorMenuNode> root = std::make_unique<JEditorMenuNode>("Root", true, false, nullptr, nullptr);
			menubar = std::make_unique<JEditorMenuBar>(std::move(root), false);
			 
			JEditorMenuNode* rootNode = menubar->GetRootNode();
			std::unique_ptr<JEditorMenuNode> settingNode = std::make_unique<JEditorMenuNode>("Setting", false, false, nullptr, rootNode);
			std::unique_ptr<JEditorMenuNode> viewNode = std::make_unique<JEditorMenuNode>("View", false, false, nullptr, rootNode);
			std::unique_ptr<JEditorMenuNode> editorToolNode = std::make_unique<JEditorMenuNode>("Tool", false, false, nullptr, rootNode);

			int statIndex[optionTypeSubTypeCount]
			{
				(int)J_OBSERVER_SETTING_TYPE::OPTION_SPACE_SPATIAL,
				(int)J_OBSERVER_SETTING_TYPE::VIEW_SETTING_SPACE_SPATIAL_TREE,
				(int)J_OBSERVER_SETTING_TYPE::TOOL_CAM_FRUSTUM
			};
			int endIndex[optionTypeSubTypeCount]
			{
				(int)J_OBSERVER_SETTING_TYPE::VIEW_SETTING_SPACE_SPATIAL_TREE,
				(int)J_OBSERVER_SETTING_TYPE::TOOL_CAM_FRUSTUM,
				(int)J_OBSERVER_SETTING_TYPE::COUNT
			};
			JEditorMenuNode* optionParent[optionTypeSubTypeCount]
			{
				settingNode.get(), 
				viewNode.get(),
				editorToolNode.get()
			};

			menubar->AddNode(std::move(settingNode));
			menubar->AddNode(std::move(viewNode));
			menubar->AddNode(std::move(editorToolNode));

			for (uint i = 0; i < optionTypeSubTypeCount; ++i)
			{
				for (uint j = statIndex[i]; j < endIndex[i]; ++j)
					CreateMenuLeafNode(optionParent[i], (J_OBSERVER_SETTING_TYPE)j);
			}

			auto menubarIconLam = [](JSceneObserver* ob)
			{
				JVector2<int> alphabetSize = JImGuiImpl::GetAlphabetSize();
				float iconSizeFactor = alphabetSize.x > alphabetSize.y ? alphabetSize.x : alphabetSize.y;
				iconSizeFactor *= 2;

				JTexture* posIcon = JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::POSITION_ARROW);
				JTexture* rotIcon = JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::ROTATION_ARROW);
				JTexture* scaleIcon = JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::SCALE_ARROW);
				 
				const bool openPos = ob->nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS].isOpen;
				const bool openRot = ob->nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_ROT].isOpen;
				const bool openScale = ob->nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_SCALE].isOpen;

				if (openPos)
					JImGuiImpl::SetColorToDeep(ImGuiCol_Button, -0.15f);
				if (JImGuiImpl::ImageButton(*posIcon, JVector2<float>(iconSizeFactor, iconSizeFactor)))
					ob->OpenObserverSettingNode(J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS);
				if (openPos)
					JImGuiImpl::SetColorToDefault(ImGuiCol_Button);

				ImGui::SameLine();
				if (openRot)
					JImGuiImpl::SetColorToDeep(ImGuiCol_Button, -0.15f);
				if (JImGuiImpl::ImageButton(*rotIcon, JVector2<float>(iconSizeFactor, iconSizeFactor)))
					ob->OpenObserverSettingNode(J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_ROT);
				ImGui::SameLine();
				if (openRot)
					JImGuiImpl::SetColorToDefault(ImGuiCol_Button);

				if (openScale)
					JImGuiImpl::SetColorToDeep(ImGuiCol_Button, -0.15f);
				if (JImGuiImpl::ImageButton(*scaleIcon, JVector2<float>(iconSizeFactor, iconSizeFactor)))
					ob->OpenObserverSettingNode(J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_SCALE);
				if (openScale)
					JImGuiImpl::SetColorToDefault(ImGuiCol_Button);
			};

			menuIconFunctor = std::make_unique<MenuIconT::Functor>(menubarIconLam);
			menubar->RegisterExtraWidgetBind(std::make_unique< MenuIconT::CompletelyBind>(*menuIconFunctor, this));
		}
		JSceneObserver::~JSceneObserver() {}
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
			EnterWindow(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);
			UpdateDocking();
			if (IsActivated() && scene.IsValid() && cameraObj.IsValid())
			{
				UpdateMouseClick();
				if (IsFocus())
				{
					if (ImGui::IsMouseHoveringRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize()))
						editorCamCtrl->MouseMove(cameraComp.Get(), ImGui::GetMousePos().x, ImGui::GetMousePos().y);
					editorCamCtrl->KeyboardInput(cameraComp.Get());
				}

				auto selected = JEditorPageShareData::GetSelectedObj(GetOwnerPageType());
				const bool isValidObject = selected.IsValid() && selected->GetObjectType() == J_OBJECT_TYPE::GAME_OBJECT;
				if (ImGui::IsMouseDown(0))
				{
					JGameObject* hitObj = nullptr;
					//JGameObject* hitObj = JEditorGameObjectSurpportTool::SceneIntersect(scene, cameraComp, Core::J_SPACE_SPATIAL_LAYER::COMMON_OBJECT);
					if (hitObj != nullptr)
					{
						const bool canSetSelected = !selected.IsValid() || (selected.IsValid() &&
							selected->GetGuid() != hitObj->GetGuid());
						if(canSetSelected)
							RequestSelectObject(JEditorSelectObjectEvStruct(GetOwnerPageType(), Core::GetUserPtr(hitObj)));
					}
				}

				menubar->Update(true);
				//Warning!
				//Has order dependency for ImGui::CursorPos 
				for (uint i = 0; i < toolCount; ++i)
				{
					J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE toolType = toolVec[i]->GetToolType();
					J_OBSERVER_SETTING_TYPE settingType = ConvertToolToSettingType(toolType);
					if (toolVec[i]->IsActivated())
						toolVec[i]->Update(JEditorPageShareData::GetSelectedObj(GetOwnerPageType()), cameraComp);
				} 
				//JImGuiImpl::Image(*camera, ImGui::GetMainViewport()->WorkSize);
				JImGuiImpl::Image(*cameraComp.Get(), ImGui::GetWindowSize());
			}
			CloseWindow();
		}
		void JSceneObserver::CreateMenuLeafNode(JEditorMenuNode* parent, J_OBSERVER_SETTING_TYPE type)noexcept
		{
			std::string name;
			switch (type)
			{
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::OPTION_SPACE_SPATIAL:
			{
				name = "Space Sptail";
				break;
			}
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::VIEW_SETTING_SPACE_SPATIAL_TREE:
			{
				name = "Space Sptail Tree";
				break;
			}
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::VIEW_SHADOW_VIEWER:
			{
				name = "Shadow Map";
				break;
			}
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::VIEW_OCCLUSION_VIEWER:
			{
				name = "Occlusion Map";
				break;
			}
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::TOOL_CAM_FRUSTUM:
			{
				name = "Cam Frustum";
				break;
			}
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS:
			{
				name = "Position Arrow";
				break;
			}
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_ROT:
			{
				name = "Rotation Arrow";
				break;
			} 
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_SCALE:
			{
				name = "Scale Arrow";
				break;
			}
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::TOOL_MAKE_DEBUG_OBJECT:
			{
				name = "Make Debug Shape";
				break;
			}
			default:
				break;
			}
			std::unique_ptr<JEditorMenuNode> node = std::make_unique<JEditorMenuNode>(name,
				false, true,
				&nodeUtilData[(int)type].isOpen,
				parent);
			 
			node->RegisterBindHandle(std::make_unique<OpenMenuNodeT::CompletelyBind>(*openNodeFunctor, std::move(type)),
				std::make_unique<ActivateMenuNodeT::CompletelyBind>(*activateNodeFunctor, std::move(type)),
				std::make_unique<DeActivateMenuNodeT::CompletelyBind>(*deActivateNodeFunctor, std::move(type)),
				std::make_unique<UpdateMenuNodeT::CompletelyBind>(*updateNodeFunctor, std::move(type)));
		 
			menubar->AddNode(std::move(node));
		}
		void JSceneObserver::OpenObserverSettingNode(const J_OBSERVER_SETTING_TYPE type)noexcept
		{ 
			nodeUtilData[(int)type].isOpen = !nodeUtilData[(int)type].isOpen;
			if (nodeUtilData[(int)type].isOpen)
				ActivateObserverSetting(type);
			else
				DeActivateObserverSetting(type);
		}
		void JSceneObserver::ActivateObserverSetting(const J_OBSERVER_SETTING_TYPE type)noexcept
		{
			switch (type)
			{
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::TOOL_CAM_FRUSTUM:
			{
				if (!mainCamFrustum.IsValid())
					MakeMainCamFrustum();
				else
					JObject::BeginDestroy(mainCamFrustum.Release());
				break;
			}
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS:
			{
				ActivateToolType(ConvertSettingToToolType(type));
				break;
			}
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_ROT:
			{
				ActivateToolType(ConvertSettingToToolType(type));
				break;
			}
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_SCALE:
			{
				ActivateToolType(ConvertSettingToToolType(type));
				break;
			}
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::TOOL_MAKE_DEBUG_OBJECT:
			{
				CreateShapeGroup(J_DEFAULT_SHAPE::DEFAULT_SHAPE_CUBE, 6, 1, 6);
				nodeUtilData[(int)type].isOpen = false;
				break;
			}
			default:
				break;
			}		 
		}
		void JSceneObserver::DeActivateObserverSetting(const J_OBSERVER_SETTING_TYPE type)noexcept
		{
			switch (type)
			{
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::TOOL_CAM_FRUSTUM:
			{
				if (mainCamFrustum.IsValid())
					JObject::BeginDestroy(mainCamFrustum.Release());
				break;
			}
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS:
			{
				DeActivateToolType(ConvertSettingToToolType(type));
				break;
			}
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_ROT:
			{
				DeActivateToolType(ConvertSettingToToolType(type));
				break;
			}
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_SCALE:
			{
				DeActivateToolType(ConvertSettingToToolType(type));
				break;
			}
			default:
				break;
			}
		}
		void JSceneObserver::UpdateObserverSetting(const J_OBSERVER_SETTING_TYPE type)noexcept
		{
			switch (type)
			{
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::OPTION_SPACE_SPATIAL:
			{
				SceneSpaceSpatialOptionOnScreen();
				break;
			}
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::VIEW_SETTING_SPACE_SPATIAL_TREE:
			{
				DebugTreeOnScreen();
				break;
			}
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::VIEW_SHADOW_VIEWER:
			{
				ShadowMapViewerOnScreen();
				break;
			}
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::VIEW_OCCLUSION_VIEWER:
			{
				OcclusionResultOnScreen();
				break;
			}
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::TOOL_CAM_FRUSTUM:
			{
				UpdateMainCamFrustum();
				break;
			}
			default:
				break;
			}
		}
		void JSceneObserver::SceneSpaceSpatialOptionOnScreen()
		{
			auto data = &nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::OPTION_SPACE_SPATIAL];
			JImGuiImpl::BeginWindow("##SpatialSpaceWindow", &data->isOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);
			Core::JEnumInfo* enumInfo = Core::JReflectionInfo::Instance().GetEnumInfo(typeid(Core::J_SPACE_SPATIAL_TYPE).name());
			if (JImGuiImpl::BeginCombo("Type##SceneObserver", enumInfo->ElementName(enumInfo->EnumValue(data->selectedIndex)).c_str(), ImGuiComboFlags_HeightLarge))
			{
				const uint enumCount = enumInfo->GetEnumCount();
				for (uint i = 0; i < enumCount; i++)
				{
					bool isSelected = (data->selectedIndex == i);
					if (JImGuiImpl::Selectable(enumInfo->ElementName(enumInfo->EnumValue(i)), &isSelected))
						data->selectedIndex = i;
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if (data->selectedIndex == (int)Core::J_SPACE_SPATIAL_TYPE::OCTREE)
				OctreeOptionOnScreen();
			else if (data->selectedIndex == (int)Core::J_SPACE_SPATIAL_TYPE::BVH)
				BvhOptionOnScreen(); 
			else if (data->selectedIndex == (int)Core::J_SPACE_SPATIAL_TYPE::KD_TREE)
				KdTreeOptionOnScreen();
			JImGuiImpl::EndWindow();
		}
		void JSceneObserver::OctreeOptionOnScreen()
		{
			JSceneSpaceSpatialInterface* iSceneSpace = scene->SpaceSpatialInterface();
			Core::JOctreeOption octreeOption = iSceneSpace->GetOctreeOption(Core::J_SPACE_SPATIAL_LAYER::COMMON_OBJECT);
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
				iSceneSpace->SetOctreeOption(Core::J_SPACE_SPATIAL_LAYER::COMMON_OBJECT, octreeOption);
		}
		void JSceneObserver::BvhOptionOnScreen()
		{
			JSceneSpaceSpatialInterface* iSceneSpace = scene->SpaceSpatialInterface();
			Core::JBvhOption bvhOption = iSceneSpace->GetBvhOption(Core::J_SPACE_SPATIAL_LAYER::COMMON_OBJECT);

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
				iSceneSpace->SetBvhOption(Core::J_SPACE_SPATIAL_LAYER::COMMON_OBJECT, bvhOption);
		}
		void JSceneObserver::KdTreeOptionOnScreen()
		{
			JSceneSpaceSpatialInterface* iSceneSpace = scene->SpaceSpatialInterface();
			Core::JKdTreeOption kdTreeOption = iSceneSpace->GetKdTreeOption(Core::J_SPACE_SPATIAL_LAYER::COMMON_OBJECT);

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
				iSceneSpace->SetKdTreeOption(Core::J_SPACE_SPATIAL_LAYER::COMMON_OBJECT, kdTreeOption);
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
		void JSceneObserver::DebugTreeOnScreen()
		{ 
			auto spaceData = &nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::OPTION_SPACE_SPATIAL];
			auto treeData = &nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::VIEW_SETTING_SPACE_SPATIAL_TREE];
		 
			if (spaceData->selectedIndex != -1)
			{
				Core::J_SPACE_SPATIAL_TYPE type = (Core::J_SPACE_SPATIAL_TYPE)spaceData->selectedIndex;
				JSceneSpaceSpatialInterface* iSceneSpace = scene->SpaceSpatialInterface();
				editorBTreeView->Clear();
				iSceneSpace->BuildDebugTree(type, Core::J_SPACE_SPATIAL_LAYER::COMMON_OBJECT, *editorBTreeView);
				editorBTreeView->TreeOnScreen(Core::GetName(type), treeData->isOpen);
			}
		}
		void JSceneObserver::ShadowMapViewerOnScreen()
		{
			auto data = &nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::VIEW_SHADOW_VIEWER];
			if (JImGuiImpl::BeginWindow("##ShadowMapViewerWindow", &data->isOpen, ImGuiWindowFlags_NoDocking))
			{
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
					if (JImGuiImpl::BeginCombo("Light##SceneObserver", JCUtil::WstrToU8Str(shadowLitVec[data->selectedIndex]->GetName()).c_str(), ImGuiComboFlags_HeightLarge))
					{
						for (uint i = 0; i < shadowLitCount; ++i)
						{
							bool isSelected = (data->selectedIndex == i);
							if (JImGuiImpl::Selectable(JCUtil::WstrToU8Str(shadowLitVec[i]->GetName()), &isSelected))
								data->selectedIndex = i;
							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						JImGuiImpl::EndCombo();
					}
					JImGuiImpl::Image(*shadowLitVec[data->selectedIndex], ImGui::GetWindowSize());
				}
				JImGuiImpl::EndWindow();
			}
		}
		void JSceneObserver::OcclusionResultOnScreen()
		{ 
			auto data = &nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::VIEW_OCCLUSION_VIEWER];
			if(JImGuiImpl::BeginWindow("##OcclusionResultWindow1", &data->isOpen, ImGuiWindowFlags_NoDocking))
			{
				CD3DX12_GPU_DESCRIPTOR_HANDLE mainDsvDebugHandle = JGraphic::Instance().GetGpuDescriptorHandle(
					Graphic::J_GRAPHIC_RESOURCE_TYPE::MAIN_DEPTH_STENCIL_DEBUG, Graphic::J_GRAPHIC_BIND_TYPE::SRV, 0, 0);

				ImGui::Image((ImTextureID)(mainDsvDebugHandle).ptr, ImVec2(400, 200));

				Graphic::JGraphicInfo ginfo = JGraphic::Instance().GetGraphicInfo();
				for (uint i = 0; i < ginfo.occlusionMapCount; ++i)
				{
					CD3DX12_GPU_DESCRIPTOR_HANDLE occDebugHandle = JGraphic::Instance().GetGpuDescriptorHandle(
						Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_DEBUG_MAP, Graphic::J_GRAPHIC_BIND_TYPE::UAV, 0, i);
					ImGui::Image((ImTextureID)(occDebugHandle).ptr, ImVec2(400, 200));
					if (i == 0 || (i % 3) > 0)
						ImGui::SameLine();
				}
				//ImGui::SameLine();
				//ImGui::Image((ImTextureID)(JGraphic::Instance().GetDebugSrvHandle(1)).ptr, ImVec2(400, 250));
				JImGuiImpl::EndWindow();

				//JImGuiImpl::BeginWindow("##OcclusionResultWindow2", &isOpenOcclusionMapViewer, ImGuiWindowFlags_NoDocking); 
				//ImGui::Image((ImTextureID)(JGraphic::Instance().GetDebugSrvHandle(0)).ptr, ImGui::GetWindowSize());
				//JImGuiImpl::EndWindow();
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

				frumstumT->SetScale(DirectX::XMFLOAT3((camWidth / camDistance) * 50, (camHeight / camDistance) * 50, 50));
				frumstumT->SetRotation(mainCamera->GetTransform()->GetRotation());
				frumstumT->SetPosition(mainCamera->GetTransform()->GetPosition());
			}
		}
		void JSceneObserver::MakeMainCamFrustum()noexcept
		{
			mainCamFrustum = Core::GetUserPtr(JGFU::CreateDebugLineShape(*scene->GetRootGameObject(),
				OBJECT_FLAG_EDITOR_OBJECT,
				J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_FRUSTUM,
				J_DEFAULT_MATERIAL::DEBUG_LINE_RED));
		}
		void JSceneObserver::ActivateToolType(const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE type)
		{
			if (lastActivatedToolType != J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::NONE)
			{
				nodeUtilData[(int)ConvertToolToSettingType(lastActivatedToolType)].isOpen = false;
				DeActivateToolType(lastActivatedToolType);
			}
			 
			for (uint i = 0; i < toolCount; ++i)
			{
				if (toolVec[i]->GetToolType() == type && !toolVec[i]->IsActivated())
				{
					toolVec[i]->ActivateTool();  
					lastActivatedToolType = type;
					break;
				}
			} 
		}
		void JSceneObserver::DeActivateToolType(const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE type)
		{
			for (uint i = 0; i < toolCount; ++i)
			{
				if (toolVec[i]->GetToolType() == type && toolVec[i]->IsActivated())
				{
					toolVec[i]->DeActivateTool();  
					lastActivatedToolType = J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::NONE;
					return;
				}
			}
		}
		J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE JSceneObserver::ConvertSettingToToolType(const J_OBSERVER_SETTING_TYPE type)const noexcept
		{
			switch (type)
			{
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS:
				return J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::POSITION_ARROW;
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_ROT:
				return J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::ROTATION_ARROW;
			case JinEngine::Editor::JSceneObserver::J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_SCALE:
				return J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::SCALE_ARROW;
			default:
				return J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::NONE;
			} 
		}
		JSceneObserver::J_OBSERVER_SETTING_TYPE JSceneObserver::ConvertToolToSettingType(const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE type)const noexcept
		{
			switch (type)
			{ 
			case JinEngine::Editor::J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::POSITION_ARROW:
				return J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS;
			case JinEngine::Editor::J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::ROTATION_ARROW:
				return J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_ROT;
			case JinEngine::Editor::J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::SCALE_ARROW:
				return J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_SCALE;
			default:
				return J_OBSERVER_SETTING_TYPE::COUNT;		//Invalid
			}
		}
		void JSceneObserver::DoSetOpen()noexcept
		{
			JEditorWindow::DoSetOpen();

			positionTool = std::make_unique<JEditorTransformTool>(J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::POSITION_ARROW,
				J_DEFAULT_SHAPE::DEFAULT_SHAPE_POSITION_ARROW, 1 / 16);
			positionTool->SetDebugRoot(GetUserPtr(scene->GetDebugRootGameObject()));

			rotationTool = std::make_unique<JEditorTransformTool>(J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::ROTATION_ARROW,
				J_DEFAULT_SHAPE::DEFAULT_SHAPE_CIRCLE, 1 / 16);
			rotationTool->SetDebugRoot(GetUserPtr(scene->GetDebugRootGameObject()));
			 
			scaleTool = std::make_unique<JEditorTransformTool>(J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::SCALE_ARROW,
				J_DEFAULT_SHAPE::DEFAULT_SHAPE_SCALE_ARROW, 1 / 16);
			scaleTool->SetDebugRoot(GetUserPtr(scene->GetDebugRootGameObject()));

			toolVec.resize(toolCount);
			toolVec[0] = positionTool.get();
			toolVec[1] = rotationTool.get();
			toolVec[2] = scaleTool.get();
		}
		void JSceneObserver::DoSetClose()noexcept
		{
			JEditorWindow::DoSetClose();
			positionTool.reset();
			rotationTool.reset();
			scaleTool.reset();
			toolVec.clear();
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
				cameraComp = GetUserPtr(cameraObj->GetComponent<JCamera>());
				cameraComp->StateInterface()->SetCameraState(J_CAMERA_STATE::RENDER);

				menubar->ActivateOpenNode(true);
			}
		}
		void JSceneObserver::DoDeActivate()noexcept
		{
			JEditorWindow::DoDeActivate();
			if (cameraObj.IsValid())
			{ 
				lastCamPos = cameraComp->GetTransform()->GetPosition();
				lastCamRot = cameraComp->GetTransform()->GetRotation();
				JObject::BeginDestroy(cameraObj.Release());
			} 
			menubar->DeActivateOpenNode(true);
		}
		void JSceneObserver::StoreEditorWindow(std::wofstream& stream)
		{
			JEditorWindow::StoreEditorWindow(stream);
			JFileIOHelper::StoreXMFloat3(stream, L"LastCamPos:", lastCamPos.ConvertXMF());
			JFileIOHelper::StoreXMFloat3(stream, L"LastCamRot:", lastCamRot.ConvertXMF());
			JFileIOHelper::StoreEnumData(stream, L"LastActivatedToolType:", lastActivatedToolType);
			for (uint i = 0; i < (uint)J_OBSERVER_SETTING_TYPE::COUNT; ++i)
			{
				auto data = &nodeUtilData[i];
				JFileIOHelper::StoreAtomicData(stream, L"IsOpen:", data->isOpen);
				JFileIOHelper::StoreAtomicData(stream, L"SeletedIndex:", data->selectedIndex);
			}
		}
		void JSceneObserver::LoadEditorWindow(std::wifstream& stream)
		{
			DirectX::XMFLOAT3 lastPos;
			DirectX::XMFLOAT3 lastRot;

			JEditorWindow::LoadEditorWindow(stream);
			JFileIOHelper::LoadXMFloat3(stream, lastPos);
			JFileIOHelper::LoadXMFloat3(stream, lastRot);
			JFileIOHelper::LoadEnumData(stream, lastActivatedToolType);
			for (uint i = 0; i < (uint)J_OBSERVER_SETTING_TYPE::COUNT; ++i)
			{
				auto data = &nodeUtilData[i];
				JFileIOHelper::LoadAtomicData(stream, data->isOpen);
				JFileIOHelper::LoadAtomicData(stream, data->selectedIndex);
			}

			lastCamPos = lastPos;
			lastCamRot = lastRot;
		}
		void JSceneObserver::CreateShapeGroup(const J_DEFAULT_SHAPE& shape, const uint xDim, const uint yDim, const uint zDim)
		{
			for (uint i = 0; i < xDim; ++i)
				{
					for (uint j = 0; j < yDim; ++j)
					{
						for (uint k = 0; k < zDim; ++k)
						{
							JGameObject* cube = JGFU::CreateShape(*scene->GetRootGameObject(), OBJECT_FLAG_NONE, shape);
							cube->GetTransform()->SetScale(DirectX::XMFLOAT3(8, 8, 8));
							cube->GetTransform()->SetRotation(DirectX::XMFLOAT3(45, 45, 0));
							cube->GetTransform()->SetPosition(DirectX::XMFLOAT3(16 * i, 16 * j, 16 * k));
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