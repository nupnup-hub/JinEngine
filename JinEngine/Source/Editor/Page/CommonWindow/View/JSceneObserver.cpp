#include"JSceneObserver.h"  
#include"../../JEditorPageShareData.h"
#include"../../JEditorAttribute.h" 
#include"../../../Menubar/JEditorMenuBar.h" 
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../Event/JEditorEvent.h"
#include"../../../EditTool/JEditorViewStructure.h"
#include"../../../EditTool/JEditorCoordGrid.h"
#include"../../../EditTool/JEditorGameObjectSurpportTool.h"
#include"../../../EditTool/JEditorCameraControl.h" 
#include"../../../../Core/File/JFileIOHelper.h"
#include"../../../../Object/Component/Camera/JCamera.h"  
#include"../../../../Object/Component/Camera/JCameraPrivate.h"  
#include"../../../../Object/Component/Transform/JTransform.h"
#include"../../../../Object/Component/RenderItem/JRenderItem.h"
#include"../../../../Object/Component/Light/JLight.h" 
#include"../../../../Object/Resource/Scene/JScene.h"  
#include"../../../../Object/Resource/Scene/JScenePrivate.h"  
#include"../../../../Object/Resource/JResourceManager.h"  
#include"../../../../Object/Resource/Texture/JTexture.h" 
#include"../../../../Object/GameObject/JGameObject.h"  
#include"../../../../Object/GameObject/JGameObjectCreator.h"   
#include"../../../../Graphic/JGraphic.h"
#include"../../../../Graphic/JGraphicPrivate.h"
#include"../../../../Graphic/GraphicResource/JGraphicResourceInterface.h" 
#include"../../../../Graphic/GraphicResource/JGraphicResourceUserAccess.h" 
#include"../../../../Utility/JCommonUtility.h"
#include"../../../../../ThirdParty/DirectX/TK/Src/d3dx12.h"

//test
#include"../../../../Object/Directory/JDirectory.h"
#include"../../../../Object/Directory/JFile.h"
#include"../../../../Object/Resource/Mesh/JMeshGeometry.h" 
#include"../../../../Object/Component/RenderItem/JRenderItemPrivate.h"

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
			using SceneTimeInterface = JScenePrivate::TimeInterface; 
			using SceneDebugInterface = JScenePrivate::DebugInterface;
			using SceneCashInterface = JScenePrivate::CashInterface; 
			using GraphicResourceInterface = Graphic::JGraphicPrivate::ResourceInterface;
			using RItemFrameIndex = JRenderItemPrivate::FrameIndexInterface;
		}
		namespace Constants
		{
			static constexpr uint optionTypeSubTypeCount = 3; 
			static constexpr uint toolCount = 3;
		}

		JSceneObserver::FrustumInfo::FrustumInfo(JUserPtr<JGameObject> frustum, JUserPtr<JCamera> cam)
			:frustum(frustum), cam(cam)
		{

		}
		void JSceneObserver::FrustumInfo::Clear()
		{
			if (frustum.IsValid())
				JGameObject::BeginDestroy(frustum.Get());
		}

		JSceneObserver::JSceneObserver(const std::string& name,
			std::unique_ptr<JEditorAttribute> attribute,
			const J_EDITOR_PAGE_TYPE pageType,
			const J_EDITOR_WINDOW_FLAG windowFlag,
			const std::vector<J_OBSERVER_SETTING_TYPE> useSettingType)
			:JEditorWindow(name, std::move(attribute), pageType, windowFlag)
		{ 
			editorCamCtrl = std::make_unique<JEditorCameraControl>();
			coordGrid = std::make_unique<JEditorSceneCoordGrid>();
			editorBTreeView = std::make_unique<JEditorBinaryTreeView>();
			selectNodeFunctor = std::make_unique<SelectMenuNodeT::Functor>(&JSceneObserver::SelectObserverSettingNode, this);
			activateNodeFunctor = std::make_unique<ActivateMenuNodeT::Functor>(&JSceneObserver::ActivateObserverSetting, this);
			deActivateNodeFunctor = std::make_unique<DeActivateMenuNodeT::Functor>(&JSceneObserver::DeActivateObserverSetting, this);
			updateNodeFunctor = std::make_unique<UpdateMenuNodeT::Functor>(&JSceneObserver::UpdateObserverSetting, this);

			BuildMenuBar(useSettingType);
			BuildMenuIcon(useSettingType);
		}
		JSceneObserver::~JSceneObserver()
		{}
		void JSceneObserver::BuildMenuBar(const std::vector<J_OBSERVER_SETTING_TYPE> useSettingType)
		{
			if (menubar != nullptr)
				return;

			std::unique_ptr<JEditorMenuNode> root = std::make_unique<JEditorMenuNode>("Root", true, false, true, nullptr, nullptr);
			menubar = std::make_unique<JEditorMenuBar>(std::move(root), false);

			JEditorMenuNode* rootNode = menubar->GetRootNode();
			std::unique_ptr<JEditorMenuNode> settingNode = std::make_unique<JEditorMenuNode>("Setting", false, false, true, nullptr, rootNode);
			std::unique_ptr<JEditorMenuNode> viewNode = std::make_unique<JEditorMenuNode>("View", false, false, true, nullptr, rootNode);
			std::unique_ptr<JEditorMenuNode> editorToolNode = std::make_unique<JEditorMenuNode>("Tool", false, false, true, nullptr, rootNode);

			int statIndex[Constants::optionTypeSubTypeCount]
			{
				(int)J_OBSERVER_SETTING_TYPE::OPTION_SPACE_SPATIAL,
				(int)J_OBSERVER_SETTING_TYPE::VIEW_SETTING_SPACE_SPATIAL_TREE,
				(int)J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS
			};
			int endIndex[Constants::optionTypeSubTypeCount]
			{
				(int)J_OBSERVER_SETTING_TYPE::VIEW_SETTING_SPACE_SPATIAL_TREE,
				(int)J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS,
				(int)J_OBSERVER_SETTING_TYPE::COUNT
			};
			JEditorMenuNode* optionParent[Constants::optionTypeSubTypeCount]
			{
				settingNode.get(),
				viewNode.get(),
				editorToolNode.get()
			};

			menubar->AddNode(std::move(settingNode));
			menubar->AddNode(std::move(viewNode));
			menubar->AddNode(std::move(editorToolNode));

			const uint useSettingCount = (uint)useSettingType.size();
			for (uint i = 0; i < useSettingCount; ++i)
			{
				int useIndex = (int)useSettingType[i];
				int parentIndex = 0;
				for (uint i = 0; i < Constants::optionTypeSubTypeCount; ++i)
				{
					if (useIndex >= statIndex[i] && useIndex < endIndex[i])
					{
						parentIndex = i;
						break;
					}
				}
				CreateMenuLeafNode(optionParent[parentIndex], (J_OBSERVER_SETTING_TYPE)useIndex);
				nodeUtilData[useIndex].isUse = true;
			}
		}
		void JSceneObserver::BuildMenuIcon(const std::vector<J_OBSERVER_SETTING_TYPE> useSettingType)
		{
			using SwitchIcon = JEditorMenuBar::SwitchIcon;
			using GetGResourcePtr = SwitchIcon::GetGResourceF::Ptr;
			using GetGResourceFunctor = SwitchIcon::GetGResourceF::Functor;

			//Has order dependency 

			size_t switchIconGuid[menuSwitchIconCount]
			{
				Core::MakeGuid(),Core::MakeGuid(),Core::MakeGuid(),Core::MakeGuid(),
				Core::MakeGuid(),Core::MakeGuid(),Core::MakeGuid()
			};
			J_OBSERVER_SETTING_TYPE settingVec[menuSwitchIconCount]
			{
				J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS,
				J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_ROT,
				J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_SCALE,
				J_OBSERVER_SETTING_TYPE::VIEW_FRUSTUM_LINE,
				J_OBSERVER_SETTING_TYPE::VIEW_SCENE_COORD_GRID,
				J_OBSERVER_SETTING_TYPE::TOOL_PLAY_SCENE_TIME,
				J_OBSERVER_SETTING_TYPE::TOOL_PAUSE_SCENE_TIME
			};

			GetGResourcePtr getGLamVec[menuSwitchIconCount]
			{
				[]() -> Graphic::JGraphicSingleResourceUserAccess* {return _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::POSITION_ARROW).Get(); },
				[]() -> Graphic::JGraphicSingleResourceUserAccess* {return _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::ROTATION_ARROW).Get(); },
				[]() -> Graphic::JGraphicSingleResourceUserAccess* {return _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::SCALE_ARROW).Get(); },
				[]() -> Graphic::JGraphicSingleResourceUserAccess* {return _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::VIEW_FRUSTUM_ICON).Get(); },
				[]() -> Graphic::JGraphicSingleResourceUserAccess* {return _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::COORD_GRID_ICON).Get(); },
				[]() -> Graphic::JGraphicSingleResourceUserAccess* {return _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::PLAY_SCENE_TIME).Get(); },
				[]() -> Graphic::JGraphicSingleResourceUserAccess* {return _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::PAUSE_SCENE_TIME).Get(); }
			};

			MenuSwitchIconOnF::Ptr onPtrVec[menuSwitchIconCount] =
			{
				 [](JSceneObserver* ob) {ob->ActivateObserverSetting(J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS); },
				 [](JSceneObserver* ob) {ob->ActivateObserverSetting(J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_ROT); },
				 [](JSceneObserver* ob) {ob->ActivateObserverSetting(J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_SCALE); },
				 [](JSceneObserver* ob) {ob->ActivateObserverSetting(J_OBSERVER_SETTING_TYPE::VIEW_FRUSTUM_LINE); },
				 [](JSceneObserver* ob) {ob->ActivateObserverSetting(J_OBSERVER_SETTING_TYPE::VIEW_SCENE_COORD_GRID); },
				 [](JSceneObserver* ob) {ob->ActivateObserverSetting(J_OBSERVER_SETTING_TYPE::TOOL_PLAY_SCENE_TIME); },
				 [](JSceneObserver* ob) {ob->ActivateObserverSetting(J_OBSERVER_SETTING_TYPE::TOOL_PAUSE_SCENE_TIME); }
			};

			MenuSwitchIconOffF::Ptr offPtrVec[menuSwitchIconCount] =
			{
				 [](JSceneObserver* ob) {ob->DeActivateObserverSetting(J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS); },
				 [](JSceneObserver* ob) {ob->DeActivateObserverSetting(J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_ROT); },
				 [](JSceneObserver* ob) {ob->DeActivateObserverSetting(J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_SCALE); },
				 [](JSceneObserver* ob) {ob->DeActivateObserverSetting(J_OBSERVER_SETTING_TYPE::VIEW_FRUSTUM_LINE); },
				 [](JSceneObserver* ob) {ob->DeActivateObserverSetting(J_OBSERVER_SETTING_TYPE::VIEW_SCENE_COORD_GRID); },
				 [](JSceneObserver* ob) {ob->DeActivateObserverSetting(J_OBSERVER_SETTING_TYPE::TOOL_PLAY_SCENE_TIME); },
				 [](JSceneObserver* ob) {ob->DeActivateObserverSetting(J_OBSERVER_SETTING_TYPE::TOOL_PAUSE_SCENE_TIME); }
			};

			std::vector<std::string> guideVec[menuSwitchIconCount]
			{
				{ "gameObject position control" , u8"위치 조정" },
			    { "gameObject rotation control" , u8"회전 조정" },
				{ "gameObject scale control" , u8"크기 조정" },
				{ "camera view frustum" , u8"카메라 절두체" },
				{ "coord grid" , u8"좌표계" },
				{ "play" , u8"재생" },
				{ "pause" , u8"정지" },
			};

			const uint useCount = (uint)useSettingType.size();
			for (uint i = 0; i < menuSwitchIconCount; ++i)
			{
				bool isUse = false;
				for (uint j = 0; j < useCount; ++j)
				{
					if (useSettingType[j] == settingVec[i])
					{
						isUse = true;
						break;
					}
				}
				if(!isUse)
					continue;
				 
				switchIconOnFunctorVec[i] = std::make_unique<MenuSwitchIconOnF::Functor>(onPtrVec[i]);
				switchIconOffFunctorVec[i] = std::make_unique<MenuSwitchIconOffF::Functor>(offPtrVec[i]);
				menubar->RegisterExtraWidget(std::make_unique<SwitchIcon>(switchIconGuid[i],
					std::make_unique<GetGResourceFunctor>(getGLamVec[i]),
					std::make_unique<MenuSwitchIconOnF::CompletelyBind>(*switchIconOnFunctorVec[i], this),
					std::make_unique<MenuSwitchIconOffF::CompletelyBind>(*switchIconOffFunctorVec[i], this),
					&nodeUtilData[(int)settingVec[i]].isOpen));

				menubar->RegisterEditorString(switchIconGuid[i], guideVec[i]);
			}
		}
		J_EDITOR_WINDOW_TYPE JSceneObserver::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::SCENE_OBSERVER;
		}
		void JSceneObserver::Initialize(JUserPtr<JScene> newScene, const std::wstring& editorCameraName)noexcept
		{
			scene = newScene;
			editCamData.name = editorCameraName;
			editCamData.lastPos = { 0,0,0 };
			editCamData.lastRot = { 0,0,0 };
		}
		void JSceneObserver::UpdateWindow()
		{
			EnterWindow(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);
			UpdateDocking();
			if (IsActivated() && scene.IsValid())
			{
				if (!isCreateHelperGameObj)
					CreateHelperGameObject();

				UpdateMouseClick();
				if (IsFocus())
				{
					if (ImGui::IsMouseHoveringRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize()))
						editorCamCtrl->MouseMove(editCamData.cam.Get(), ImGui::GetMousePos().x, ImGui::GetMousePos().y);
					editorCamCtrl->KeyboardInput(editCamData.cam.Get());
				}  

				JUserPtr<JTransform> camTransform = editCamData.cam->GetTransform();
				coordGrid->Update(JVector2<float>(camTransform->GetPosition().x, camTransform->GetPosition().z));
				menubar->Update(true);

				if (ImGui::IsMouseClicked(0))
				{
					JUserPtr<JGameObject> hitObj = JEditorGameObjectSurpportTool::SceneIntersect(scene, editCamData.cam, Core::J_SPACE_SPATIAL_LAYER::COMMON_OBJECT, ImGui::GetCursorPos());
					if (hitObj != nullptr)
					{ 
						RequestPushSelectObject(hitObj);
						//SetSelectedGameObjectTrigger(static_cast<JGameObject*>(hitObj), true); 
						SetContentsClick(true);
					}
				} 

				for (uint i = 0; i < Constants::toolCount; ++i)
				{
					J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE toolType = toolVec[i]->GetToolType();
					J_OBSERVER_SETTING_TYPE settingType = ConvertToolToSettingType(toolType);

					//activate된 tool만 update
					//selectedGobj 가 nullptr일 경우 tool object = nullptr (내부 valid 값으로 컨트롤)
					if (toolVec[i]->IsActivated())
						toolVec[i]->Update(selectedGobj, editCamData.cam, ImGui::GetCursorPos());
				}
				//JImGuiImpl::Image(*camera, ImGui::GetMainViewport()->WorkSize); 
				JImGuiImpl::Image(editCamData.cam.Get(), Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, ImGui::GetWindowSize());
			 
				editCamData.lastPos = editCamData.cam->GetTransform()->GetPosition();
				editCamData.lastRot = editCamData.cam->GetTransform()->GetRotation();
			}
			CloseWindow();
		}
		void JSceneObserver::CreateMenuLeafNode(JEditorMenuNode* parent, J_OBSERVER_SETTING_TYPE type)noexcept
		{
			std::string name;
			switch (type)
			{
			case J_OBSERVER_SETTING_TYPE::OPTION_SPACE_SPATIAL:
			{
				name = "Space sptail";
				break;
			}
			case J_OBSERVER_SETTING_TYPE::OPTION_EDITOR_OBSERVER:
			{
				name = "Editor";
				break;
			}
			case J_OBSERVER_SETTING_TYPE::OPTION_TEST_DATA:
			{
				name = "Test data";
				break;
			}
			case J_OBSERVER_SETTING_TYPE::VIEW_SETTING_SPACE_SPATIAL_TREE:
			{
				name = "Space sptatial tree";
				break;
			}
			case J_OBSERVER_SETTING_TYPE::VIEW_SHADOW_VIEWER:
			{
				name = "Shadow map";
				break;
			}
			case J_OBSERVER_SETTING_TYPE::VIEW_RENDER_RESULT:
			{
				name = "Render result";
				break;
			}
			case J_OBSERVER_SETTING_TYPE::VIEW_SCENE_COORD_GRID:
			{
				name = "Coord grid";
				break;
			}
			case J_OBSERVER_SETTING_TYPE::VIEW_FRUSTUM_LINE:
			{
				name = "Cam frustum";
				break;
			}
			case J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS:
			{
				name = "Position arrow";
				break;
			}
			case J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_ROT:
			{
				name = "Rotation arrow";
				break;
			}
			case J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_SCALE:
			{
				name = "Scale arrow";
				break;
			}
			default:
				break;
			}
			std::unique_ptr<JEditorMenuNode> node = std::make_unique<JEditorMenuNode>(name,
				false, true, true,
				&nodeUtilData[(int)type].isOpen,
				parent);

			node->RegisterBindHandle(std::make_unique<SelectMenuNodeT::CompletelyBind>(*selectNodeFunctor, std::move(type)),
				std::make_unique<SelectMenuNodeT::CompletelyBind>(*selectNodeFunctor, std::move(type)),
				std::make_unique<ActivateMenuNodeT::CompletelyBind>(*activateNodeFunctor, std::move(type)),
				std::make_unique<DeActivateMenuNodeT::CompletelyBind>(*deActivateNodeFunctor, std::move(type)),
				std::make_unique<UpdateMenuNodeT::CompletelyBind>(*updateNodeFunctor, std::move(type)));

			menubar->AddNode(std::move(node));
		}
		void JSceneObserver::SelectObserverSettingNode(const J_OBSERVER_SETTING_TYPE type)noexcept
		{ 
			if (nodeUtilData[(int)type].isOpen)
				ActivateObserverSetting(type);
			else
				DeActivateObserverSetting(type);
		}
		void JSceneObserver::ActivateObserverSetting(const J_OBSERVER_SETTING_TYPE type)noexcept
		{
			switch (type)
			{
			case J_OBSERVER_SETTING_TYPE::VIEW_SCENE_COORD_GRID:
			{
				if (!coordGrid->HasCoordGrid())
					coordGrid->MakeCoordGrid(scene->GetDebugRootGameObject());
				break;
			}
			case J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS:
			{
				ActivateTransformToolType(ConvertSettingToToolType(type));
				break;
			}
			case J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_ROT:
			{
				ActivateTransformToolType(ConvertSettingToToolType(type));
				break;
			}
			case J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_SCALE:
			{
				ActivateTransformToolType(ConvertSettingToToolType(type));
				break;
			}
			case J_OBSERVER_SETTING_TYPE::TOOL_PLAY_SCENE_TIME:
			{
				SceneTimeInterface::ActivateSceneTime(scene);
				nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::TOOL_PAUSE_SCENE_TIME].isOpen = false;
				break;
			}
			case J_OBSERVER_SETTING_TYPE::TOOL_PAUSE_SCENE_TIME:
			{
				if(scene->IsActivatedSceneTime() && !scene->IsPauseSceneTime())
					SceneTimeInterface::PlaySceneTimer(scene, false);
				else
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
			case J_OBSERVER_SETTING_TYPE::VIEW_SCENE_COORD_GRID:
			{
				if (coordGrid->HasCoordGrid())
					coordGrid->Clear();
				break;
			}
			case J_OBSERVER_SETTING_TYPE::VIEW_FRUSTUM_LINE:
			{
				for (auto& data : camFrustumMap)
					data.second.Clear();
				camFrustumMap.clear();
				break;
			}
			case J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS:
			{
				DeActivateTransformToolType(ConvertSettingToToolType(type));
				break;
			}
			case J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_ROT:
			{
				DeActivateTransformToolType(ConvertSettingToToolType(type));
				break;
			}
			case J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_SCALE:
			{
				DeActivateTransformToolType(ConvertSettingToToolType(type));
				break;
			}
			case J_OBSERVER_SETTING_TYPE::TOOL_PLAY_SCENE_TIME:
			{
				SceneTimeInterface::DeActivateSceneTime(scene);
				nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::TOOL_PAUSE_SCENE_TIME].isOpen = false;
				break;
			}
			case J_OBSERVER_SETTING_TYPE::TOOL_PAUSE_SCENE_TIME:
			{
				if (scene->IsActivatedSceneTime() && scene->IsPauseSceneTime())
					SceneTimeInterface::PlaySceneTimer(scene, true); 
				else
					nodeUtilData[(int)type].isOpen = true;
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
			case J_OBSERVER_SETTING_TYPE::OPTION_SPACE_SPATIAL:
			{
				SceneSpaceSpatialOptionOnScreen();
				break;
			}
			case J_OBSERVER_SETTING_TYPE::OPTION_EDITOR_OBSERVER:
			{
				EditorCameraOptionOnScreen();
				break;
			}
			case J_OBSERVER_SETTING_TYPE::OPTION_TEST_DATA:
			{
				EngineTestOptionOnScreen();
				break;
			}
			case J_OBSERVER_SETTING_TYPE::VIEW_SETTING_SPACE_SPATIAL_TREE:
			{
				DebugTreeOnScreen();
				break;
			}
			case J_OBSERVER_SETTING_TYPE::VIEW_SHADOW_VIEWER:
			{
				ShadowMapViewerOnScreen();
				break;
			}
			case J_OBSERVER_SETTING_TYPE::VIEW_RENDER_RESULT:
			{
				RenderResultOnScreen();
				break;
			}
			case J_OBSERVER_SETTING_TYPE::VIEW_FRUSTUM_LINE:
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
			JImGuiImpl::ComboEnumSet<Core::J_SPACE_SPATIAL_TYPE>("##SpatialSpaceWindow", data->selectedIndex);
			if (data->selectedIndex == (int)Core::J_SPACE_SPATIAL_TYPE::OCTREE)
				OctreeOptionOnScreen();
			else if (data->selectedIndex == (int)Core::J_SPACE_SPATIAL_TYPE::BVH)
				BvhOptionOnScreen();
			else if (data->selectedIndex == (int)Core::J_SPACE_SPATIAL_TYPE::KD_TREE)
				KdTreeOptionOnScreen();
			JImGuiImpl::EndWindow();
		}
		void JSceneObserver::EditorCameraOptionOnScreen()
		{ 
			auto data = &nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::OPTION_EDITOR_OBSERVER];
			JImGuiImpl::BeginWindow("##EditorOption", &data->isOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);	 
			  
			JImGuiImpl::Text("Camera");
			if (JImGuiImpl::CheckBox("display debug##JSceneObserver", editOption.allowDisplayDebug))
				editCamData.cam->SetAllowDisplayDebug(editOption.allowDisplayDebug);
			if (JImGuiImpl::CheckBox("frustum culling##JSceneObserver", editOption.allowFrustumCulling))
				editCamData.cam->SetAllowFrustumCulling(editOption.allowFrustumCulling);
			if (JImGuiImpl::CheckBox("occ culling##JSceneObserver", editOption.allowOccCulling))
				editCamData.cam->SetAllowHzbOcclusionCulling(editOption.allowOccCulling);
			if (JImGuiImpl::CheckBox("reflect othrer cam culling##JSceneObserver", editOption.allowReflectCullingResult))
				JCameraPrivate::EditorSettingInterface::SetAllowAllCullingResult(editCamData.cam, editOption.allowReflectCullingResult);
			 
			ImGui::Separator();
			JImGuiImpl::Text("Grid");
			int lineCount = coordGrid->GetLineCount();
			int lineStep = coordGrid->GetLineStep();

			if (JImGuiImpl::SliderInt("line count##JSceneObserverSceneCoord", &lineCount, coordGrid->GetMinLineCount(), coordGrid->GetMaxLineCount()))
				coordGrid->SetLineCount(lineCount);
			if (JImGuiImpl::SliderInt("line step##JSceneObserverSceneCoord", &lineStep, coordGrid->GetMinLineStep(), coordGrid->GetMaxLineStep()))
				coordGrid->SetLineStep(lineStep);
			JImGuiImpl::EndWindow();
		}
		void JSceneObserver::EngineTestOptionOnScreen()
		{
			auto data = &nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::OPTION_TEST_DATA];
			JImGuiImpl::BeginWindow("##EngineTestingOption", &data->isOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);
			JImGuiImpl::Text("Creation option"); 

			JImGuiImpl::Text("x count");
			ImGui::SameLine();
			JImGuiImpl::InputInt("##JSceneObserverSceneCoord x count", &testData.xCount);
			testData.xCount = std::clamp(testData.xCount, testData.minObjCount, testData.maxObjCount);

			JImGuiImpl::Text("y count");
			ImGui::SameLine();
			JImGuiImpl::InputInt("##JSceneObserverSceneCoord y count", &testData.yCount);
			testData.yCount = std::clamp(testData.yCount, testData.minObjCount, testData.maxObjCount);

			JImGuiImpl::Text("z count");
			ImGui::SameLine();
			JImGuiImpl::InputInt("##JSceneObserverSceneCoord z count", &testData.zCount);
			testData.zCount = std::clamp(testData.zCount, testData.minObjCount, testData.maxObjCount);

			std::string tLabel[3] = {"Cube", "Sphere", "Dragon"};
			J_DEFAULT_SHAPE shapeType[3] = {J_DEFAULT_SHAPE::CUBE,  J_DEFAULT_SHAPE::SPHERE,  J_DEFAULT_SHAPE::DRAGON };
			bool testMeshCheck[3] = { testData.meshType == J_DEFAULT_SHAPE::CUBE,
				testData.meshType == J_DEFAULT_SHAPE::SPHERE,
				testData.meshType == J_DEFAULT_SHAPE::DRAGON };

			for (uint i = 0; i < 3; ++i)
			{
				if (JImGuiImpl::CheckBox(tLabel[i] + "##JSceneObserverSceneCoord", testMeshCheck[i]))
					testData.meshType = shapeType[i];
				if (i != 2)
					ImGui::SameLine();
			}

			std::string tableColumnLabel[4] = { "Name", "x", "y", "z" };
			std::string tableRowLabel[4] = { "position", "rotation", "scale", "distance" };
			JVector3<float>* value[4] = { &testData.offsetPos, &testData.offsetRot, &testData.offsetScale, &testData.distance };
			
			Core::JGuiFlag tableFlag = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersV |
				ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ContextMenuInBody;
			const ImGuiTableColumnFlags_ columnDefaultFlag = ImGuiTableColumnFlags_WidthStretch;

			JImGuiImpl::BeginTable("##JSceneObserverSceneCoordValueTable", 4, tableFlag);
			for (uint i = 0; i < 4; ++i)
				JImGuiImpl::TableSetupColumn(tableColumnLabel[i], columnDefaultFlag);
			JImGuiImpl::TableHeadersRow();
			JImGuiImpl::TableNextRow();

			for (uint i = 0; i < 4; ++i)
			{
				JImGuiImpl::TableSetColumnIndex(0);
				JImGuiImpl::Text(tableRowLabel[i]);
				JImGuiImpl::TableSetColumnIndex(1);
				JImGuiImpl::InputFloat("##JSceneObserverSceneCoord" + tableRowLabel[i] + "X", &value[i]->x);
				JImGuiImpl::TableSetColumnIndex(2);
				JImGuiImpl::InputFloat("##JSceneObserverSceneCoord" + tableRowLabel[i] + "Y", &value[i]->y);
				JImGuiImpl::TableSetColumnIndex(3);
				JImGuiImpl::InputFloat("##JSceneObserverSceneCoord" + tableRowLabel[i] + "Z", &value[i]->z);
				if (i + 1 < 4)
					JImGuiImpl::TableNextRow();
			}
			JImGuiImpl::EndTable();

			if (JImGuiImpl::Button("Create##EngineTestingOption"))
				CreateShapeGroup();
			JImGuiImpl::EndWindow();
		}
		void JSceneObserver::OctreeOptionOnScreen()
		{ 
			Core::JOctreeOption octreeOption = scene->GetOctreeOption(Core::J_SPACE_SPATIAL_LAYER::COMMON_OBJECT);
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
			{
				scene->SetOctreeOption(Core::J_SPACE_SPATIAL_LAYER::COMMON_OBJECT, octreeOption);
				SetModifiedBit(scene, true);
			}
		}
		void JSceneObserver::BvhOptionOnScreen()
		{
			bool isUpdated = false; 
			Core::JBvhOption bvhOption = scene->GetBvhOption(Core::J_SPACE_SPATIAL_LAYER::COMMON_OBJECT);
			Core::JEnumInfo* buildEnumInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(Core::J_SPACE_SPATIAL_BUILD_TYPE).name());
			Core::JEnumInfo* splitEnumInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(Core::J_SPACE_SPATIAL_SPLIT_TYPE).name());
			int buildIndex = buildEnumInfo->GetEnumIndex((int)bvhOption.buildType);
			int splitIndex = splitEnumInfo->GetEnumIndex((int)bvhOption.splitType);
			int preBuildIndex = buildIndex;
			int preSplitIndex = splitIndex;

			JImGuiImpl::ComboEnumSet<Core::J_SPACE_SPATIAL_BUILD_TYPE>("BuildType##Bvh_SceneObserver", buildIndex);
			if (buildIndex != preBuildIndex)
				isUpdated = true;

			JImGuiImpl::ComboEnumSet<Core::J_SPACE_SPATIAL_SPLIT_TYPE>("SplitType##Bvh_SceneObserver", splitIndex);
			if (splitIndex != preSplitIndex)
				isUpdated = true;

			isUpdated |= CommonOptionOnScreen("Bvh", bvhOption.commonOption);
			if (isUpdated)
			{
				scene->SetBvhOption(Core::J_SPACE_SPATIAL_LAYER::COMMON_OBJECT, bvhOption);
				SetModifiedBit(scene, true);
			}
		}
		void JSceneObserver::KdTreeOptionOnScreen()
		{
			bool isUpdated = false; 
			Core::JKdTreeOption kdTreeOption = scene->GetKdTreeOption(Core::J_SPACE_SPATIAL_LAYER::COMMON_OBJECT);
			Core::JEnumInfo* buildEnumInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(Core::J_SPACE_SPATIAL_BUILD_TYPE).name());
			Core::JEnumInfo* splitEnumInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(Core::J_SPACE_SPATIAL_SPLIT_TYPE).name());
			int buildIndex = buildEnumInfo->GetEnumIndex((int)kdTreeOption.buildType);
			int splitIndex = splitEnumInfo->GetEnumIndex((int)kdTreeOption.splitType);
			int preBuildIndex = buildIndex;
			int preSplitIndex = splitIndex;

			JImGuiImpl::ComboEnumSet<Core::J_SPACE_SPATIAL_BUILD_TYPE>("BuildType##Kd_SceneObserver", buildIndex);
			if (buildIndex != preBuildIndex)
				isUpdated = true;

			JImGuiImpl::ComboEnumSet<Core::J_SPACE_SPATIAL_SPLIT_TYPE>("SplitType##Kd_SceneObserver", splitIndex);
			if (splitIndex != preSplitIndex)
				isUpdated = true;

			isUpdated |= CommonOptionOnScreen("Kd", kdTreeOption.commonOption);
			if (isUpdated)
			{
				scene->SetKdTreeOption(Core::J_SPACE_SPATIAL_LAYER::COMMON_OBJECT, kdTreeOption);
				SetModifiedBit(scene, true);
			}
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
				editorBTreeView->ClearNode();
				SceneDebugInterface::BuildDebugTree(scene, type, Core::J_SPACE_SPATIAL_LAYER::COMMON_OBJECT, *editorBTreeView);
				if (editorBTreeView->BeginView(Core::GetName(type) + +"##DebugTreeView", &treeData->isOpen, ImGuiWindowFlags_NoDocking))
				{
					editorBTreeView->OnScreen();
					editorBTreeView->EndView();
				}
			}
		}
		void JSceneObserver::ShadowMapViewerOnScreen()
		{
			auto data = &nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::VIEW_SHADOW_VIEWER];
			if (JImGuiImpl::BeginWindow("##ShadowMapViewerWindow", &data->isOpen, ImGuiWindowFlags_NoDocking))
			{
				const std::vector<JUserPtr<JComponent>> litVec = SceneCashInterface::GetComponentCashVec(scene, J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT);
				const uint litCount = (uint)litVec.size();

				std::vector<JLight*> shadowLitVec;
				for (uint i = 0; i < litCount; ++i)
				{
					JLight* lit = static_cast<JLight*>(litVec[i].Get());
					if (lit->IsShadowActivated())
						shadowLitVec.push_back(lit);
				}

				const uint shadowLitCount = (uint)shadowLitVec.size();
				if (shadowLitCount > 0)
				{
					JImGuiImpl::ComboSet("Light##SceneObserve", data->selectedIndex, shadowLitVec);
					JImGuiImpl::Image(shadowLitVec[data->selectedIndex], Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, ImGui::GetWindowSize());
				}
				JImGuiImpl::EndWindow();
			}
		}
		void JSceneObserver::RenderResultOnScreen()
		{
			auto data = &nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::VIEW_RENDER_RESULT];
			if (JImGuiImpl::BeginWindow("##RenderResultWindow1", &data->isOpen, ImGuiWindowFlags_NoDocking))
			{				 
				auto camVec = scene->GetComponentVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA);
				for (const auto& camData : camVec)
				{
					JCamera* cam = static_cast<JCamera*>(camData.Get());
					auto gRInterface = cam->GraphicResourceUserInterface();
					JImGuiImpl::Text(JCUtil::WstrToU8Str(cam->GetName()));

					if (cam->AllowDisplayDepthMap())
					{
						JImGuiImpl::Text("Depth Map");
						auto sceneDsDebugHandle = gRInterface.GetGpuHandle(Graphic::J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL_DEBUG, Graphic::J_GRAPHIC_BIND_TYPE::SRV, 0);
						ImGui::Image((ImTextureID)(sceneDsDebugHandle).ptr, ImVec2(400, 200));
					}

					if (cam->AllowHzbOcclusionCulling() && cam->AllowDisplayOccCullingDepthMap())
					{
						JImGuiImpl::Text("Occlusion Depth Map");
						Graphic::JGraphicInfo ginfo = JGraphic::Instance().GetGraphicInfo();
						for (uint i = 0; i < ginfo.occlusionMapCount; ++i)
						{
							auto occDebugHandle = gRInterface.GetGpuHandle(Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG, Graphic::J_GRAPHIC_BIND_TYPE::UAV, i);
							ImGui::Image((ImTextureID)(occDebugHandle).ptr, ImVec2(400, 200));
							if (i == 0 || (i % 3) > 0)
								ImGui::SameLine();
						}
					}			 
					ImGui::NewLine();
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
			auto camVec = scene->GetComponentVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA);
			for (const auto& data : camVec)
			{
				if (!data->GetOwner()->IsEditorObject())
				{ 
					if (camFrustumMap.find(data->GetGuid()) == camFrustumMap.end())
						CreateCamFrustum(Core::ConnectChildUserPtr<JCamera>(data));
				}
			}
			std::vector<size_t> invalidVec;
			for (const auto& data : camFrustumMap)
			{
				if (!data.second.cam.IsValid() || !data.second.cam->IsActivated())
					invalidVec.push_back(data.first);
				else
				{
					JCamera* cam = data.second.cam.Get();
					JUserPtr<JTransform> frumstumT = data.second.frustum->GetTransform();

					const float camWidth = cam->GetFarViewWidth();
					const float camHeight = cam->GetFarViewHeight();
					const float camNear = cam->GetNear();
					const float camFar = cam->GetFar();
					const float camDistance = camFar - camNear;

					frumstumT->SetScale(DirectX::XMFLOAT3((camWidth / camDistance) * 50, (camHeight / camDistance) * 50, 50));
					frumstumT->SetRotation(cam->GetTransform()->GetRotation());
					frumstumT->SetPosition(cam->GetTransform()->GetPosition());
				}
			}
			for (const auto& data : invalidVec)
			{
				camFrustumMap.find(data)->second.Clear();
				camFrustumMap.erase(data);
			}
		}
		void JSceneObserver::CreateCamFrustum(JUserPtr<JCamera> cam)noexcept
		{
			if (cam == nullptr)
				return;

			auto frustum = JGCI::CreateDebugLineShape(scene->GetRootGameObject(),
				OBJECT_FLAG_EDITOR_OBJECT,
				J_DEFAULT_SHAPE::BOUNDING_FRUSTUM,
				J_DEFAULT_MATERIAL::DEBUG_LINE_RED,
				true);

			if (frustum.IsValid())
				camFrustumMap.emplace(cam->GetGuid(), FrustumInfo(frustum, cam));
		}
		void JSceneObserver::ActivateTransformToolType(const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE type)
		{
			for (uint i = 0; i < Constants::toolCount; ++i)
				DeActivateTransformToolType((J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE)i);
	
			for (uint i = 0; i < Constants::toolCount; ++i)
			{
				if (toolVec[i]->GetToolType() == type && !toolVec[i]->IsActivated())
				{
					toolVec[i]->ActivateTool(); 
					nodeUtilData[(int)ConvertToolToSettingType(type)].isOpen = true;
					break;
				}
			}
		}
		void JSceneObserver::DeActivateTransformToolType(const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE type)
		{
			for (uint i = 0; i < Constants::toolCount; ++i)
			{
				if (toolVec[i]->GetToolType() == type && toolVec[i]->IsActivated())
				{
					toolVec[i]->DeActivateTool();
					nodeUtilData[(int)ConvertToolToSettingType(type)].isOpen = false;
					return;
				}
			}
		}
		J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE JSceneObserver::ConvertSettingToToolType(const J_OBSERVER_SETTING_TYPE type)const noexcept
		{
			switch (type)
			{
			case J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS:
				return J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::POSITION_ARROW;
			case J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_ROT:
				return J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::ROTATION_ARROW;
			case J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_SCALE:
				return J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::SCALE_ARROW;
			default:
				return J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::POSITION_ARROW;
			}
		}
		J_OBSERVER_SETTING_TYPE JSceneObserver::ConvertToolToSettingType(const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE type)const noexcept
		{
			switch (type)
			{
			case J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::POSITION_ARROW:
				return J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS;
			case J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::ROTATION_ARROW:
				return J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_ROT;
			case J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::SCALE_ARROW:
				return J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_SCALE;
			default:
				return J_OBSERVER_SETTING_TYPE::COUNT;		//Invalid
			}
		}
		void JSceneObserver::CreateHelperGameObject()
		{
			JUserPtr<JGameObject> camObj = JGCI::CreateDebugCamera(scene->GetRootGameObject(), OBJECT_FLAG_UNIQUE_EDITOR_OBJECT, editCamData.name);
			camObj->SetName(L"Observer");
			camObj->GetTransform()->SetPosition(editCamData.lastPos.ConvertXMF());
			camObj->GetTransform()->SetRotation(editCamData.lastRot.ConvertXMF());
			editCamData.cam = camObj->GetComponent<JCamera>();
			editCamData.cam->SetCameraState(J_CAMERA_STATE::RENDER);

			if (nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS].isUse)
				positionTool->SetDebugRoot(scene->GetDebugRootGameObject());
			if (nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_ROT].isUse)
				rotationTool->SetDebugRoot(scene->GetDebugRootGameObject());
			if (nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_SCALE].isUse)
				scaleTool->SetDebugRoot(scene->GetDebugRootGameObject());
			 
			//자동으로 activate 불가능
			nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::TOOL_PLAY_SCENE_TIME].isOpen = false;
			nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::TOOL_PAUSE_SCENE_TIME].isOpen = false;

			menubar->ActivateOpenNode(true);
			isCreateHelperGameObj = true;
		}
		void JSceneObserver::DestroyHelperGameObject()
		{
			if (editCamData.cam.IsValid())
			{
				JObject::BeginDestroy(editCamData.cam->GetOwner().Get());
				editCamData.cam.Clear();
			}
			menubar->DeActivateOpenNode(true);
			isCreateHelperGameObj = false;
		}
		void JSceneObserver::DoSetOpen()noexcept
		{
			JEditorWindow::DoSetOpen();
			positionTool = std::make_unique<JEditorTransformTool>(J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::POSITION_ARROW,
				J_DEFAULT_SHAPE::POSITION_ARROW, 1 / 16);

			rotationTool = std::make_unique<JEditorTransformTool>(J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::ROTATION_ARROW,
				J_DEFAULT_SHAPE::CIRCLE, 1 / 16);

			scaleTool = std::make_unique<JEditorTransformTool>(J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE::SCALE_ARROW,
				J_DEFAULT_SHAPE::SCALE_ARROW, 1 / 16);

			iconTexture.push_back(_JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::POSITION_ARROW));
			iconTexture.push_back(_JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::ROTATION_ARROW));
			iconTexture.push_back(_JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::SCALE_ARROW));
			iconTexture.push_back(_JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::VIEW_FRUSTUM_ICON));
			iconTexture.push_back(_JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::COORD_GRID_ICON));
			iconTexture.push_back(_JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::PLAY_SCENE_TIME));
			iconTexture.push_back(_JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::PAUSE_SCENE_TIME)); 

			toolVec.resize(Constants::toolCount);
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
			iconTexture.clear();
			toolVec.clear();
		}
		void JSceneObserver::DoActivate()noexcept
		{
			JEditorWindow::DoActivate(); 
			if (scene.IsValid())
				CreateHelperGameObject();
			 
			editCamData.cam->SetAllowFrustumCulling(editOption.allowFrustumCulling);
			editCamData.cam->SetAllowHzbOcclusionCulling(editOption.allowOccCulling);
			editCamData.cam->SetAllowDisplayDebug(editOption.allowDisplayDebug);
			JCameraPrivate::EditorSettingInterface::SetAllowAllCullingResult(editCamData.cam, editOption.allowReflectCullingResult);

			std::vector<J_EDITOR_EVENT> listenEvTypeVec{ J_EDITOR_EVENT::PUSH_SELECT_OBJECT };
			AddEventListener(*JEditorEvent::EvInterface(), GetGuid(), listenEvTypeVec);

			if (scene.IsValid())
			{
				auto vec = scene->GetGameObjectVec();
				for (const auto& data : vec)
				{
					if (data->IsSelected())
						PushSelectedObject(data);
				}
			}
		}
		void JSceneObserver::DoDeActivate()noexcept
		{
			JEditorWindow::DoDeActivate();	 
			DestroyHelperGameObject();
			RemoveListener(*JEditorEvent::EvInterface(), GetGuid());
		}
		void JSceneObserver::StoreEditorWindow(std::wofstream& stream)
		{
			JEditorWindow::StoreEditorWindow(stream); 
			JFileIOHelper::StoreXMFloat3(stream, L"LastCamPos:", editCamData.lastPos.ConvertXMF());
			JFileIOHelper::StoreXMFloat3(stream, L"LastCamRot:", editCamData.lastRot.ConvertXMF());
			for (uint i = 0; i < (uint)J_OBSERVER_SETTING_TYPE::COUNT; ++i)
			{
				auto data = &nodeUtilData[i];
				JFileIOHelper::StoreAtomicData(stream, L"IsOpen:", data->isOpen);
				JFileIOHelper::StoreAtomicData(stream, L"SeletedIndex:", data->selectedIndex);
			}
			JFileIOHelper::StoreAtomicData(stream, L"allowDisplayDebug", editOption.allowDisplayDebug);
			JFileIOHelper::StoreAtomicData(stream, L"allowFrustumCulling", editOption.allowFrustumCulling);
			JFileIOHelper::StoreAtomicData(stream, L"allowOccCulling", editOption.allowOccCulling);
			JFileIOHelper::StoreAtomicData(stream, L"allowReflectCullingResult", editOption.allowReflectCullingResult);

			JFileIOHelper::StoreAtomicData(stream, L"CoordLineCount", coordGrid->GetLineCount()); 
			JFileIOHelper::StoreAtomicData(stream, L"CoordLineStep", coordGrid->GetLineStep());
		}
		void JSceneObserver::LoadEditorWindow(std::wifstream& stream)
		{ 
			DirectX::XMFLOAT3 lastPos;
			DirectX::XMFLOAT3 lastRot;
			int lineCount = 0; 
			int lineStep = 0;

			JEditorWindow::LoadEditorWindow(stream); 
			JFileIOHelper::LoadXMFloat3(stream, lastPos);
			JFileIOHelper::LoadXMFloat3(stream, lastRot); 
			for (uint i = 0; i < (uint)J_OBSERVER_SETTING_TYPE::COUNT; ++i)
			{
				auto data = &nodeUtilData[i];
				JFileIOHelper::LoadAtomicData(stream, data->isOpen);
				JFileIOHelper::LoadAtomicData(stream, data->selectedIndex);
			}
			JFileIOHelper::LoadAtomicData(stream, editOption.allowDisplayDebug);
			JFileIOHelper::LoadAtomicData(stream, editOption.allowFrustumCulling);
			JFileIOHelper::LoadAtomicData(stream, editOption.allowOccCulling);
			JFileIOHelper::LoadAtomicData(stream, editOption.allowReflectCullingResult);

			JFileIOHelper::LoadAtomicData(stream, lineCount); 
			JFileIOHelper::LoadAtomicData(stream, lineStep);

			lastPos = lastPos;
			lastRot = lastRot;
			coordGrid->SetLineCount(lineCount); 
			coordGrid->SetLineStep(lineStep);
		}
		void JSceneObserver::CreateShapeGroup()
		{
			for (int i = 0; i < testData.xCount; ++i)
			{
				for (int j = 0; j < testData.yCount; ++j)
				{
					for (int k = 0; k < testData.zCount; ++k)
					{ 
						JUserPtr<JGameObject> gObj = JGCI::CreateShape(scene->GetRootGameObject(), OBJECT_FLAG_NONE, testData.meshType);
						gObj->GetTransform()->SetScale(testData.offsetScale.ConvertXMF());
						gObj->GetTransform()->SetRotation(testData.offsetRot.ConvertXMF());
						gObj->GetTransform()->SetPosition((testData.offsetPos + JVector3<float>(i * testData.distance.x, j * testData.distance.y, k * testData.distance.z)).ConvertXMF());
					}
				}
			}
			SetModifiedBit(scene, true);
		}
		void JSceneObserver::OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)
		{
			JEditorWindow::OnEvent(senderGuid, eventType, eventStruct);	
			if (eventType == J_EDITOR_EVENT::PUSH_SELECT_OBJECT && eventStruct->pageType == GetOwnerPageType())
			{
				JEditorPushSelectObjectEvStruct* evstruct = static_cast<JEditorPushSelectObjectEvStruct*>(eventStruct);
				JUserPtr< Core::JIdentifier> gameObj = evstruct->GetLastMatchedTypeObject(JGameObject::StaticTypeInfo());
				if (gameObj.IsValid())
				{
					if (!selectedGobj.IsValid() || selectedGobj->GetGuid() != gameObj->GetGuid())
						selectedGobj.ConnnectChild(std::move(gameObj));
				}
			}
			if (senderGuid == GetGuid())
				return;
		}
		/*
		void JSceneObserver::CreateDebugMaterial()noexcept
		{
			for (uint i = 0; i < debugMaterialCount; ++i)
			{
				JMaterial::InitData initData = JMaterial::InitData(L"TestM" + std::to_wstring(i),
					Core::MakeGuid(),
					OBJECT_FLAG_EDITOR_OBJECT,
					_JResourceManager::Instance().GetEditorResourceDirectory());

				debugMaterial[i] = JRFI<JMaterial>::Create(Core::JPtrUtil::MakeOwnerPtr<JMaterial::InitData>(L"TestM" + std::to_wstring(i),
					Core::MakeGuid(),
					OBJECT_FLAG_EDITOR_OBJECT,
					_JResourceManager::Instance().GetEditorResourceDirectory()));
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

//kdtree align debug
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

//frustum culling debug
/*

				auto rVec = scene->GetComponentVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM);
				for (const auto& data : rVec)
				{
					if(data->GetOwner()->GetName() != L"Cube")
						continue;

					auto rItem = static_cast<JRenderItem*>(data.Get());
					int frameIndex = RItemFrameIndex::GetBoundingFrameIndex(rItem);
					auto cam = scene->FindFirstSelectedCamera(false);
					auto cullingInterface = cam->CullingUserInterface();
					ImGui::Text((JCUtil::WstrToU8Str(data->GetOwner()->GetName()) + " " +
						std::to_string(cullingInterface.IsCulled(Graphic::J_CULLING_TYPE::FRUSTUM,frameIndex)) + " "+
						std::to_string(cullingInterface.IsCulled(Graphic::J_CULLING_TYPE::OCCLUSION, frameIndex))).c_str());

					auto frustum = cam->GetBoundingFrustum();
					auto bbox = rItem->GetBoundingBox();

					auto nearFrustum = frustum;
					nearFrustum.Near = 0;
					nearFrustum.Far = frustum.Near;
					auto result = bbox.Contains(DirectX::XMLoadFloat3(&frustum.Origin));

					ImGui::SameLine();
					ImGui::Text(" Pos C: ");
					ImGui::SameLine();
					if (result == DirectX::CONTAINS)
						ImGui::Text("Contain");
					else if (result == DirectX::INTERSECTS)
						ImGui::Text("Intersect");
					else if (result == DirectX::DISJOINT)
						ImGui::Text("Disjoint");

					ImGui::SameLine();
					ImGui::Text("B Frustum C: ");
					ImGui::SameLine();
					result = bbox.Contains(nearFrustum);
					if (result == DirectX::CONTAINS)
						ImGui::Text("Contain");
					else if (result == DirectX::INTERSECTS)
						ImGui::Text("Intersect");
					else if (result == DirectX::DISJOINT)
						ImGui::Text("Disjoint");

					ImGui::SameLine();
					ImGui::Text("F Frustum C: ");
					ImGui::SameLine();
					result = nearFrustum.Contains(bbox);
					if (result == DirectX::CONTAINS)
						ImGui::Text("Contain");
					else if (result == DirectX::INTERSECTS)
						ImGui::Text("Intersect");
					else if (result == DirectX::DISJOINT)
						ImGui::Text("Disjoint");
				}

*/