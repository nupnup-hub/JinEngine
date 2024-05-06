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


#include"JSceneObserver.h"  
#include"../../JEditorPageShareData.h"
#include"../../JEditorAttribute.h" 
#include"../../../Menubar/JEditorMenuBar.h" 
#include"../../../Gui/JGui.h"
#include"../../../Gui/JGuiImageInfo.h"
#include"../../../Event/JEditorEvent.h"
#include"../../../EditTool/JEditorViewStructure.h"
#include"../../../EditTool/JEditorCoordGrid.h"
#include"../../../EditTool/JEditorGameObjectSurpportTool.h"
#include"../../../EditTool/JEditorSceneImageInteraction.h" 
#include"../../../EditTool/JEditorCameraControl.h" 
#include"../../../EditTool/JEditorMouseDragBox.h" 
#include"../../../EditTool/JEditorIdentifierList.h"
#include"../../../../Core/File/JFileIOHelper.h"
#include"../../../../Core/Identity/JIdenCreator.h"
#include"../../../../Core/Math/JVectorExtend.h"
#include"../../../../Core/Utility/JCommonUtility.h"
#include"../../../../Object/Component/Camera/JCamera.h"  
#include"../../../../Object/Component/Camera/JCameraPrivate.h"  
#include"../../../../Object/Component/Transform/JTransform.h"
#include"../../../../Object/Component/RenderItem/JRenderItem.h" 
#include"../../../../Object/Component/Light/JLight.h" 
#include"../../../../Object/Component/Light/JPointLight.h" 
#include"../../../../Object/Component/Light/JSpotLight.h" 
#include"../../../../Object/Component/Light/JRectLight.h" 
#include"../../../../Object/Resource/Scene/JScene.h"  
#include"../../../../Object/Resource/Scene/JScenePrivate.h"  
#include"../../../../Object/Resource/JResourceManager.h"  
#include"../../../../Object/Resource/Texture/JTexture.h" 
#include"../../../../Object/Resource/Material/JMaterial.h"
#include"../../../../Object/Resource/Mesh/JMeshGeometry.h"
#include"../../../../Object/GameObject/JGameObject.h"  
#include"../../../../Object/GameObject/JGameObjectCreator.h"   
#include"../../../../Graphic/JGraphic.h"
#include"../../../../Graphic/JGraphicPrivate.h"
#include"../../../../Graphic/GraphicResource/JGraphicResourceInterface.h" 
#include"../../../../Graphic/GraphicResource/JGraphicResourceUserAccess.h" 
#include"../../../../Graphic/Outline/JOutlineConstants.h"
#include"../../../../Graphic/FrameResource/JFrameUpdate.h"
#include"../../../../../ThirdParty/DirectX/TK/Src/d3dx12.h"

//test
//#include"../../../../Object/Directory/JDirectory.h"
//#include"../../../../Object/Directory/JFile.h"
//#include"../../../../Object/Resource/Mesh/JMeshGeometry.h" 
//#include"../../../../Object/Component/RenderItem/JRenderItemPrivate.h"

//#include<fstream>
//Debug
//#include"../../../../Object/Component/RenderItem/JRenderItem.h"
//#include"../../../../Object/Resource/Material/JMaterial.h"  
//#include"../../../../Object/Resource/JResourceObjectFactory.h"  
//#include"../../../../Core/File/JFileIOHelper.h"
//#include"../../../../Debug/JDebugTimer.h" 
//#include"../../../../Core/Time/JStopWatch.h" 

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
			//Debug
			//using RItemFrameIndex = JRenderItemPrivate::FrameIndexInterface;
		}
		namespace Private
		{
			static constexpr uint optionTypeSubTypeCount = 3;
			static constexpr uint toolCount = 3;

			static constexpr uint menuIconCount = 7;
			static constexpr uint sceneIconCount = 5; //4 lit 1 cam
			static constexpr uint camTextureIndex = sceneIconCount - 1; 
			 
			static constexpr int GetLightTextureIndex(const J_LIGHT_TYPE type)
			{
				switch (type)
				{
				case JinEngine::J_LIGHT_TYPE::DIRECTIONAL:
					return 0;
				case JinEngine::J_LIGHT_TYPE::POINT:
					return 1;
				case JinEngine::J_LIGHT_TYPE::SPOT:
					return 2;
				case JinEngine::J_LIGHT_TYPE::RECT:
					return 3;
				default:
					break;
				}
				return -1;
			}

		}
		namespace
		{
			static float rsImageSizeFactor = 1.0f;
			static JVector2<float> RenderResultImageSize()
			{
				JVector2F v = JVector2<float>(JGui::GetWindowSize()) / 2.5f;
				return JVector2F::Clamp(v, JVector2F(256, 256), v) * rsImageSizeFactor;
				//return JVector2<float>(JGui::GetWindowSize()) / 4.0f;
			} 
			static JVector2<float> TextureImageSize()
			{
				JVector2F v = JVector2<float>(JGui::GetWindowSize()) / 2.5f;
				return JVector2F::Clamp(v, JVector2F(256, 256), v);
			}
		}

		void JSceneObserver::TestData::Initialize()
		{
			JVector4<float> initCol[matCount]
			{
				JVector4<float>(1.0f, 0.1f, 0.1f, 0.65f),
				JVector4<float>(0.1f, 0.1f, 0.1f, 0.66f),
				JVector4<float>(0.1f, 0.1f, 1.0f, 0.67f),
				JVector4<float>(1.0f, 1.0f, 0.1f, 0.68f),
				JVector4<float>(1.0f, 0.1f, 1.0f, 0.69f),
				JVector4<float>(0.1f, 1.0f, 1.0f, 0.70f),
				JVector4<float>(1.0f, 1.0f, 1.0f, 0.71f)
			};

			matVec.resize(matCount);

			JUserPtr<JDirectory> dir = _JResourceManager::Instance().GetEditorResourceDirectory();
			for (uint i = 0; i < matCount; ++i)
			{
				JUserPtr<JMaterial> material = JICI::Create<JMaterial>(L"TestDataMat",
					Core::MakeGuid(),
					OBJECT_FLAG_EDITOR_OBJECT,
					JMaterial::GetDefaultFormatIndex(),
					dir);

				material->SetAlbedoColor(initCol[i]);
				material->SetLight(true);
				material->SetShadow(true);
				//material->SetRoughness(0.75f);
				//material->SetMetallic(0.1f);
				matVec[i] = material;
			}
		}
		void JSceneObserver::TestData::Clear()
		{
			for (auto& data : objParentVec)
				Core::JIdentifier::BeginDestroy(data.Release());
			for (auto& data : matVec)
				Core::JIdentifier::BeginDestroy(data.Release());
			objParentVec.clear();
			matVec.clear();
		}

		bool JSceneObserver::TextureDebug::HasValidHandle()const noexcept
		{
			if (dataHandle.size() == 0)
				return false;
			for (const auto& data : dataHandle)
			{
				if (!data.IsValid())
					return false;
			}
			return true;
		}

		JSceneObserver::JSceneObserver(const std::string& name,
			std::unique_ptr<JEditorAttribute> attribute,
			const J_EDITOR_PAGE_TYPE pageType,
			const J_EDITOR_WINDOW_FLAG windowFlag,
			const std::vector<J_OBSERVER_SETTING_TYPE> useSettingType,
			const std::vector<size_t>& listenWindowGuidVec)
			:JEditorWindow(name, std::move(attribute), pageType, windowFlag)
		{
			coordGrid = std::make_unique<JEditorSceneCoordGrid>();
			editBTreeView = std::make_unique<JEditorBinaryTreeView>();
			editCamCtrl = std::make_unique<JEditorCameraControl>();
			mouseBBox = std::make_unique<JEditorMouseDragSceneBox>();
			idenList = std::make_unique< JEditorIdentifierList>();
			selectNodeFunctor = std::make_unique<SelectMenuNodeT::Functor>(&JSceneObserver::SelectObserverSettingNode, this);
			activateNodeFunctor = std::make_unique<ActivateMenuNodeT::Functor>(&JSceneObserver::ActivateObserverSetting, this);
			deActivateNodeFunctor = std::make_unique<DeActivateMenuNodeT::Functor>(&JSceneObserver::DeActivateObserverSetting, this);
			updateNodeFunctor = std::make_unique<UpdateMenuNodeT::Functor>(&JSceneObserver::UpdateObserverSetting, this);

			for (const auto& data : listenWindowGuidVec)
				PushOtherWindowGuidForListenEv(data);
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

			int statIndex[Private::optionTypeSubTypeCount]
			{
				(int)J_OBSERVER_SETTING_TYPE::OPTION_SPACE_SPATIAL,
				(int)J_OBSERVER_SETTING_TYPE::VIEW_SETTING_ACCELERATOR_TREE,
				(int)J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS
			};
			int endIndex[Private::optionTypeSubTypeCount]
			{
				(int)J_OBSERVER_SETTING_TYPE::VIEW_SETTING_ACCELERATOR_TREE,
				(int)J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS,
				(int)J_OBSERVER_SETTING_TYPE::COUNT
			};
			JEditorMenuNode* optionParent[Private::optionTypeSubTypeCount]
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
				for (uint i = 0; i < Private::optionTypeSubTypeCount; ++i)
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
				J_OBSERVER_SETTING_TYPE::VIEW_CAMERA,
				J_OBSERVER_SETTING_TYPE::VIEW_LIGHT,
				J_OBSERVER_SETTING_TYPE::VIEW_SCENE_COORD_GRID,
				J_OBSERVER_SETTING_TYPE::TOOL_PLAY_SCENE_TIME,
				J_OBSERVER_SETTING_TYPE::TOOL_PAUSE_SCENE_TIME
			};

			GetGResourcePtr getGLamVec[menuSwitchIconCount]
			{
				[]() -> Graphic::JGraphicResourceUserAccess* {return _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::POSITION_ARROW).Get(); },
				[]() -> Graphic::JGraphicResourceUserAccess* {return _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::ROTATION_ARROW).Get(); },
				[]() -> Graphic::JGraphicResourceUserAccess* {return _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::SCALE_ARROW).Get(); },
				[]() -> Graphic::JGraphicResourceUserAccess* {return _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::VIEW_FRUSTUM_ICON).Get(); },
				[]() -> Graphic::JGraphicResourceUserAccess* {return _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::SPOT_LIGHT).Get(); },
				[]() -> Graphic::JGraphicResourceUserAccess* {return _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::COORD_GRID_ICON).Get(); },
				[]() -> Graphic::JGraphicResourceUserAccess* {return _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::PLAY_SCENE_TIME).Get(); },
				[]() -> Graphic::JGraphicResourceUserAccess* {return _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::PAUSE_SCENE_TIME).Get(); }
			};

			MenuSwitchIconOnF::Ptr onPtrVec[menuSwitchIconCount] =
			{
				 [](JSceneObserver* ob) {ob->ActivateObserverSetting(J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS); },
				 [](JSceneObserver* ob) {ob->ActivateObserverSetting(J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_ROT); },
				 [](JSceneObserver* ob) {ob->ActivateObserverSetting(J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_SCALE); },
				 [](JSceneObserver* ob) {ob->ActivateObserverSetting(J_OBSERVER_SETTING_TYPE::VIEW_CAMERA); },
				 [](JSceneObserver* ob) {ob->ActivateObserverSetting(J_OBSERVER_SETTING_TYPE::VIEW_LIGHT); },
				 [](JSceneObserver* ob) {ob->ActivateObserverSetting(J_OBSERVER_SETTING_TYPE::VIEW_SCENE_COORD_GRID); },
				 [](JSceneObserver* ob) {ob->ActivateObserverSetting(J_OBSERVER_SETTING_TYPE::TOOL_PLAY_SCENE_TIME); },
				 [](JSceneObserver* ob) {ob->ActivateObserverSetting(J_OBSERVER_SETTING_TYPE::TOOL_PAUSE_SCENE_TIME); }
			};

			MenuSwitchIconOffF::Ptr offPtrVec[menuSwitchIconCount] =
			{
				 [](JSceneObserver* ob) {ob->DeActivateObserverSetting(J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS); },
				 [](JSceneObserver* ob) {ob->DeActivateObserverSetting(J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_ROT); },
				 [](JSceneObserver* ob) {ob->DeActivateObserverSetting(J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_SCALE); },
				 [](JSceneObserver* ob) {ob->DeActivateObserverSetting(J_OBSERVER_SETTING_TYPE::VIEW_CAMERA); },
				 [](JSceneObserver* ob) {ob->DeActivateObserverSetting(J_OBSERVER_SETTING_TYPE::VIEW_LIGHT); },
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
				{ "light shape" , u8"조명 형상" },
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
				if (!isUse)
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
		void JSceneObserver::SetScenePlayProccess(std::unique_ptr<BeginScenePlayF::Functor> newBeginScenePlayF, std::unique_ptr<EndScenePlayF::Functor> newEndScenePlayF)
		{
			beginScenePlayF = std::move(newBeginScenePlayF);
			endScenePlayF = std::move(newEndScenePlayF);
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
			EnterWindow(J_GUI_WINDOW_FLAG_NO_SCROLL_BAR |
				J_GUI_WINDOW_FLAG_NO_SCROLL_WITH_MOUSE |
				J_GUI_WINDOW_FLAG_NO_COLLAPSE |
				J_GUI_WINDOW_FLAG_MENU_BAR);
			UpdateDocking();
			if (IsActivated() && scene.IsValid())
			{
				if (!isCreateHelperGameObj)
					CreateHelperGameObject();

				UpdateMouseClick();
				UpdateMouseWheel();
				editCamCtrl->Update(editCamData.cam, JGui::GetMousePos().x, JGui::GetMousePos().y, J_GUI_FOCUS_FLAG_CHILD_WINDOW);
			 
				JUserPtr<JTransform> camTransform = editCamData.cam->GetTransform();
				coordGrid->Update(JVector2<float>(camTransform->GetPosition().x, camTransform->GetPosition().z));
				menubar->Update(true);
				if (menubar->IsLastUpdateClickedContents() || menubar->IsNextUpdateClickedContents())
					SetContentsClick(true); 

				JVector2<float> sceneImageScreenPos = JGui::GetCursorScreenPos();
				JVector2<float> sceneImageCursorPos = JGui::GetCursorPos();
				const bool isLastLeftMouseCliked = JGui::IsLastMouseClicked(Core::J_MOUSE_BUTTON::LEFT);
				bool isHoveringToolObject = false;
				bool isDraggingToolObject = false;
				bool isLastUpdateSelectedToolObject = false;
				auto selectedVec = GetSelectedObjectVec<JGameObject>();
				geoTool->Update();

				for (uint i = 0; i < Private::toolCount; ++i)
				{
					J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE toolType = toolVec[i]->GetToolType();
					J_OBSERVER_SETTING_TYPE settingType = ConvertToolToSettingType(toolType);

					//activate된 tool만 update
					//selectedGobj 가 nullptr일 경우 tool object = nullptr (내부 valid 값으로 컨트롤)
					if (toolVec[i]->IsActivated())
					{
						toolVec[i]->Update(selectedVec, editCamData.cam, sceneImageScreenPos, !mouseBBox->IsActivated());
						isHoveringToolObject = isDraggingToolObject || toolVec[i]->IsHovering();
						isDraggingToolObject = isDraggingToolObject || toolVec[i]->IsDragging();
						isLastUpdateSelectedToolObject = isLastUpdateSelectedToolObject || toolVec[i]->IsLastUpdateSelected();
						if (selectedVec.size() == 0 && isLastLeftMouseCliked)
						{
							JUserPtr<JGameObject> lastSelecetd = toolVec[i]->GetLastSelected();
							if (lastSelecetd != nullptr && toolVec[i]->IsHitDebugObject(editCamData.cam, sceneImageScreenPos))
							{
								RequestPushSelectObject(lastSelecetd);
								SetContentsClick(true);
								isLastUpdateSelectedToolObject = true;
							}
						}
					}
				}

				const bool isToolContentsSelected = isLastUpdateSelectedToolObject || isDraggingToolObject;
				if (isToolContentsSelected)
					SetContentsClick(true);

				const bool isCurrentWndFocused = JGui::IsCurrentWindowFocused(J_GUI_FOCUS_FLAG_CHILD_WINDOW);
				bool canSelectSceneObject = isLastLeftMouseCliked && !isToolContentsSelected && !isHoveringToolObject;
				JGuiWindowInfo wndInfo;
				JGui::GetCurrentWindowInfo(wndInfo);

				//Debug
				/*
				if (GetSelectedObjectCount() == 1)
				{
					auto selected = GetSelectedObjectVec();
					auto fCam = scene->FindFirstSelectedCamera(false);
					auto gameObj = Core::ConvertChildUserPtr<JGameObject>(selected[0]);
					if (fCam != nullptr && gameObj != nullptr)
					{
						auto rItem = gameObj->GetRenderItem();
						if (rItem != nullptr)
						{
							//auto oBB = rItem->GetOrientedBoundingBox();
							//auto tBB = rItem->GetDetphTestBoundingBox();
							//auto frsutum = fCam->GetBoundingFrustum();

							//JGui::Text(JVector3F(oBB.Center).ToString() + " " + JVector3F(oBB.Extents).ToString());
							//JGui::Text(JVector3F(tBB.Center).ToString() + " " + JVector3F(tBB.Extents).ToString());
							//JGui::Text(std::to_string(frsutum.Contains(oBB)));
							//JGui::Text(std::to_string(frsutum.Contains(tBB)));
							//JGui::Text(std::to_string(JRenderItemPrivate::FrameIndexInterface::GetBoundingFrameIndex(rItem.Get())));
						}
					}
				}
				*/

				JGuiImageInfo imageInfo(editCamData.cam.Get(), Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON);
				JGui::Image(imageInfo, JGui::GetWindowSize());
				//JGui::Image(*camera, JGui::GetMainViewport()->WorkSize); 
 
				bool hasSelecetdIcon = false;
				DisplaySceneIcon(sceneImageScreenPos, canSelectSceneObject, hasSelecetdIcon);

				canSelectSceneObject = canSelectSceneObject && !hasSelecetdIcon;
				if (canSelectSceneObject)
				{
					JUserPtr<JGameObject> hitObj = JEditorSceneImageInteraction::Intersect(scene, editCamData.cam, J_ACCELERATOR_LAYER::COMMON_OBJECT, sceneImageScreenPos);
					if (hitObj != nullptr)
					{
						if (hitObj->IsSelected())
							RequestPopSelectObject(hitObj);
						else
							RequestPushSelectObject(hitObj);
						//SetSelectedGameObjectTrigger(static_cast<JGameObject*>(hitObj), true); 
						SetContentsClick(true);
					}
				}

				bool canAcitvateMouseBBox = !IsContentsClicked() && !isHoveringToolObject && JGui::IsMouseDragging(Core::J_MOUSE_BUTTON::LEFT);
				if (isCurrentWndFocused && !mouseBBox->IsActivated() && !hasSelecetdIcon && canAcitvateMouseBBox)
				{
					JUserPtr<JGameObject> hitObj = JEditorSceneImageInteraction::Intersect(scene, editCamData.cam, J_ACCELERATOR_LAYER::COMMON_OBJECT, sceneImageScreenPos);
					canAcitvateMouseBBox = canAcitvateMouseBBox && hitObj == nullptr;
				}
				
				if (JGui::IsCurrentWindowFocused(J_GUI_FOCUS_FLAG_CHILD_WINDOW))
				{
					using DragBoxUpdaetIn = JEditorMouseDragSceneBox::UpdateIn;
					using DragBoxUpdaetOut = JEditorMouseDragSceneBox::UpdateOut;
					DragBoxUpdaetIn in(canAcitvateMouseBBox, sceneImageCursorPos, JVector2F::Zero(), sceneImageScreenPos, scene, editCamData.cam, J_ACCELERATOR_LAYER::COMMON_OBJECT);
					DragBoxUpdaetOut out;

					mouseBBox->UpdateSceneImageDrag(in, out);
					if (out.newDeSelectedVec.size() > 0)
					{
						AddEventNotification(*JEditorEvent::EvInterface(),
							GetGuid(),
							J_EDITOR_EVENT::POP_SELECT_OBJECT,
							JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPopSelectObjectEvStruct>(GetOwnerPageType(), out.newDeSelectedVec, JEditorEvStruct::RANGE::ALL)));
					}
					if (out.newSelectedVec.size() > 0)
						AddEventNotification(*JEditorEvent::EvInterface(),
							GetGuid(),
							J_EDITOR_EVENT::PUSH_SELECT_OBJECT,
							JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPushSelectObjectEvStruct>(GetOwnerPageType(), GetWindowType(), out.newSelectedVec, JEditorEvStruct::RANGE::ALL))); 
					if (mouseBBox->GetSelectedCount() > 0)
						SetContentsClick(true);
				}
				if (editCamData.cam != nullptr)
				{
					editCamData.lastPos = editCamData.cam->GetTransform()->GetPosition();
					editCamData.lastRot = editCamData.cam->GetTransform()->GetRotation();
				}
			}
			CloseWindow();
		}
		void JSceneObserver::UpdateMouseWheel()
		{
			editCamCtrl->AddMovementFactor(JGui::GetMouseWheel());
		}
		void JSceneObserver::DisplaySceneIcon(const JVector2F sceneImagePos, const bool canSelectIcon, _Out_ bool& hasSelected)
		{
			hasSelected = false;
			if (scene == nullptr)
				return;

			auto calReduceRateLam = [](const float range, const float zValue)
			{
				return  ((range * 2 - (zValue * 0.5f)) / (range * 2));;
			};

			//obj to screen pos
			auto litVec = scene->GetComponentVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT);
			auto camVec = scene->GetComponentVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA);

			auto camFrustum = editCamData.cam->GetBoundingFrustum();
			auto validFrustum = camFrustum;
			const float frustumRange = camFrustum.Far - camFrustum.Near;
			const float iconRange = frustumRange / 0.25f;
			validFrustum.Far = validFrustum.Near + iconRange;

			auto viewM = editCamData.cam->GetView();
			auto projM = editCamData.cam->GetProj();

			const float camWidth = editCamData.cam->GetFarViewWidth();
			const float camHeight = editCamData.cam->GetFarViewHeight();

			const JVector2F posOffset = sceneImagePos;
			const JVector2F wndSize = JGui::GetWindowSize();
			const float iconSizeFactor = max(wndSize.x, wndSize.y) * 0.125f;

			const JVector4F color = JVector4F(0.8f, 0.8f, 0.8f, 0.75f);
			const JVector4F selectColor = JVector4F(1.0f, 1.0f, 1.0f, 1.0f);

			if (nodeUtilData[(uint)J_OBSERVER_SETTING_TYPE::VIEW_LIGHT].isOpen)
			{
				for (const auto& data : litVec)
				{
					const auto wPos = JVector4F(data->GetOwner()->GetTransform()->GetWorldPosition(), 1.0f).ToXmV();
					if (validFrustum.Contains(wPos) == DirectX::ContainmentType::DISJOINT)
						continue;

					const JLight* lit = static_cast<JLight*>(data.Get());
					JTexture* texture = sceneIconTexture[Private::GetLightTextureIndex(lit->GetLightType())].Get();
					const JVector2F offsetScaleRate = JVector2F(iconSizeFactor, iconSizeFactor) / JVector2F(texture->GetTextureWidth(), texture->GetTextureHeight());

					const JVector4F cPos = DirectX::XMVector4Transform(DirectX::XMVector4Transform(wPos, viewM), projM);
					const JVector2F ndcPos = JVector2F(cPos.x / cPos.w, cPos.y / cPos.w);
					const float reduceSizeFactor = calReduceRateLam(iconRange, cPos.w);

					const JVector2F iconCenterPos = JVector2F((ndcPos.x * wndSize.x + wndSize.x) / 2, (-ndcPos.y * wndSize.y + wndSize.y) / 2) + posOffset;
					const JVector2F iconHalfSize = JVector2F(iconSizeFactor * reduceSizeFactor, iconSizeFactor * reduceSizeFactor) * offsetScaleRate * 0.5f;
					const JVector2F iconMinPos = iconCenterPos - iconHalfSize;
					const JVector2F iconMaxPos = iconCenterPos + iconHalfSize;

					JGuiImageInfo info(texture);
					JGui::AddImage(info, iconMinPos, iconMaxPos, false, data->GetOwner()->IsSelected() ? selectColor : color);
					if (canSelectIcon && JGui::IsMouseInRectMM(iconMinPos, iconMaxPos))
					{
						AddEventNotification(*JEditorEvent::EvInterface(),
							GetGuid(),
							J_EDITOR_EVENT::PUSH_SELECT_OBJECT,
							JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPushSelectObjectEvStruct>(GetOwnerPageType(), GetWindowType(), data->GetOwner(), JEditorEvStruct::RANGE::ALL)));
						hasSelected = true;
					}
				}
			}
			if (nodeUtilData[(uint)J_OBSERVER_SETTING_TYPE::VIEW_CAMERA].isOpen)
			{
				for (const auto& data : camVec)
				{
					if (data->GetGuid() == editCamData.cam->GetGuid())
						continue;

					const auto wPos = JVector4F(data->GetOwner()->GetTransform()->GetWorldPosition(), 1.0f).ToXmV();
					if (validFrustum.Contains(wPos) == DirectX::ContainmentType::DISJOINT)
						continue;

					const JLight* lit = static_cast<JLight*>(data.Get());
					JTexture* texture = sceneIconTexture[Private::camTextureIndex].Get();
					const JVector2F offsetScaleRate = JVector2F(iconSizeFactor, iconSizeFactor) / JVector2F(texture->GetTextureWidth(), texture->GetTextureHeight());

					const JVector4F cPos = DirectX::XMVector4Transform(DirectX::XMVector4Transform(wPos, viewM), projM);
					const JVector2F ndcPos = JVector2F(cPos.x / cPos.w, cPos.y / cPos.w);
					const float reduceSizeFactor = calReduceRateLam(iconRange, cPos.w);

					const JVector2F iconCenterPos = JVector2F((ndcPos.x * wndSize.x + wndSize.x) / 2, (-ndcPos.y * wndSize.y + wndSize.y) / 2) + posOffset;
					const JVector2F iconHalfSize = JVector2F(iconSizeFactor * reduceSizeFactor, iconSizeFactor * reduceSizeFactor) * offsetScaleRate * 0.5f;
					const JVector2F iconMinPos = iconCenterPos - iconHalfSize;
					const JVector2F iconMaxPos = iconCenterPos + iconHalfSize;

					JGuiImageInfo info(texture);
					JGui::AddImage(info, iconMinPos, iconMaxPos, false, data->GetOwner()->IsSelected() ? selectColor : color);
					if (canSelectIcon && JGui::IsMouseInRectMM(iconMinPos, iconMaxPos))
					{
						AddEventNotification(*JEditorEvent::EvInterface(),
							GetGuid(),
							J_EDITOR_EVENT::PUSH_SELECT_OBJECT,
							JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPushSelectObjectEvStruct>(GetOwnerPageType(), GetWindowType(), data->GetOwner(), JEditorEvStruct::RANGE::ALL)));
						hasSelected = true;
					}
				}
			}
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
			case J_OBSERVER_SETTING_TYPE::OPTION_INSTANCE_TEST:
			{
				name = "Instance test";
				break;
			}
			case J_OBSERVER_SETTING_TYPE::VIEW_SETTING_ACCELERATOR_TREE:
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
				name = "Cam render result";
				break;
			}
			case J_OBSERVER_SETTING_TYPE::VIEW_TEXTURE_DETAIL:
			{
				name = "Texture detail";
				break;
			}
			case J_OBSERVER_SETTING_TYPE::VIEW_SCENE_COORD_GRID:
			{
				name = "Coord grid";
				break;
			}
			case J_OBSERVER_SETTING_TYPE::VIEW_CAMERA:
			{
				name = "Cam frustum";
				break;
			}
			case J_OBSERVER_SETTING_TYPE::VIEW_LIGHT:
			{
				name = "Light boundary";
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
				if (beginScenePlayF != nullptr)
					(*beginScenePlayF)();
				SceneTimeInterface::ActivateSceneTime(scene);
				nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::TOOL_PAUSE_SCENE_TIME].isOpen = false;
				break;
			}
			case J_OBSERVER_SETTING_TYPE::TOOL_PAUSE_SCENE_TIME:
			{
				if (scene->IsActivatedSceneTime() && !scene->IsPauseSceneTime())
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
			case J_OBSERVER_SETTING_TYPE::VIEW_TEXTURE_DETAIL:
			{
				if (textureDebug->HasValidHandle())
					Graphic::JGraphicResourceUserInterface::ClearFirstResourceMipmapBind(textureDebug->dataHandle);
				break;
			}
			case J_OBSERVER_SETTING_TYPE::VIEW_CAMERA:
			{
				geoTool->ClearTarget(JCamera::StaticTypeInfo());
				break;
			}
			case J_OBSERVER_SETTING_TYPE::VIEW_LIGHT:
			{
				geoTool->ClearTarget(JLight::StaticTypeInfo());
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
				if (endScenePlayF != nullptr)
					(*endScenePlayF)();
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
				SceneAcceleratorOptionOnScreen();
				break;
			}
			case J_OBSERVER_SETTING_TYPE::OPTION_EDITOR_OBSERVER:
			{
				EditorCameraOptionOnScreen();
				break;
			}
			case J_OBSERVER_SETTING_TYPE::OPTION_INSTANCE_TEST:
			{
				EngineTestOptionOnScreen();
				break;
			}
			case J_OBSERVER_SETTING_TYPE::VIEW_SETTING_ACCELERATOR_TREE:
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
			case J_OBSERVER_SETTING_TYPE::VIEW_TEXTURE_DETAIL:
			{
				TextureDetailOnScreen();
				break;
			}
			case J_OBSERVER_SETTING_TYPE::VIEW_CAMERA:
			{
				UpdateMainCamFrustum();
				break;
			}
			case J_OBSERVER_SETTING_TYPE::VIEW_LIGHT:
			{ 
				geoTool->TryCreateGeoView(GetSelectedObjectVec<JGameObject>(), scene->GetDebugRootGameObject());
				break;
			}
			default:
				break;
			}
		}
		void JSceneObserver::SceneAcceleratorOptionOnScreen()
		{
			auto data = &nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::OPTION_SPACE_SPATIAL];
			JGui::BeginWindow("##SpatialSpaceWindow", &data->isOpen, J_GUI_WINDOW_FLAG_NO_RESIZE | J_GUI_WINDOW_FLAG_NO_DOCKING);
			JGui::ComboEnumSet<J_ACCELERATOR_TYPE>("##SpatialSpaceWindow", data->selectedIndex);
			if (data->selectedIndex == (int)J_ACCELERATOR_TYPE::OCTREE)
				OctreeOptionOnScreen();
			else if (data->selectedIndex == (int)J_ACCELERATOR_TYPE::BVH)
				BvhOptionOnScreen();
			else if (data->selectedIndex == (int)J_ACCELERATOR_TYPE::KD_TREE)
				KdTreeOptionOnScreen();
			JGui::EndWindow();
		}
		void JSceneObserver::EditorCameraOptionOnScreen()
		{
			auto data = &nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::OPTION_EDITOR_OBSERVER];
			JGui::BeginWindow("##EditorOption", &data->isOpen, J_GUI_WINDOW_FLAG_NO_RESIZE | J_GUI_WINDOW_FLAG_NO_DOCKING);

			JGui::Text("Camera");
			if (JGui::CheckBox("display debug##"+ GetName(), editOption.allowDisplayDebugging))
				editCamData.cam->SetAllowDisplayDebugObject(editOption.allowDisplayDebugging);
			if (JGui::CheckBox("frustum culling##"+ GetName(), editOption.allowFrustumCulling))
				editCamData.cam->SetAllowFrustumCulling(editOption.allowFrustumCulling);
			if (JGui::CheckBox("occ culling##"+ GetName(), editOption.allowOccCulling))
				editCamData.cam->SetAllowHzbOcclusionCulling(editOption.allowOccCulling);
			if (JGui::CheckBox("reflect othrer cam culling##"+ GetName(), editOption.allowReflectCullingResult))
				JCameraPrivate::EditorSettingInterface::SetAllowAllCullingResult(editCamData.cam, editOption.allowReflectCullingResult);
			if (JGui::Button("fit main cam##"+ GetName()))
			{
				auto mainCam = scene->FindFirstSelectedCamera(false); 
				editCamData.cam->GetTransform()->SetTransform(mainCam->GetTransform()->GetWorldMatrix());
			}
			JGui::Separator();
			JGui::Text("Grid");
			int lineCount = coordGrid->GetLineCount();
			int lineStep = coordGrid->GetLineStep();

			if (JGui::SliderInt("line count##Coord" + GetName(), &lineCount, coordGrid->GetMinLineCount(), coordGrid->GetMaxLineCount()))
				coordGrid->SetLineCount(lineCount);
			if (JGui::SliderInt("line step##Coord" + GetName(), &lineStep, coordGrid->GetMinLineStep(), coordGrid->GetMaxLineStep()))
				coordGrid->SetLineStep(lineStep);
			JGui::EndWindow();
		}
		void JSceneObserver::EngineTestOptionOnScreen()
		{
			auto data = &nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::OPTION_INSTANCE_TEST];
			JGui::BeginWindow("##TestOption" + GetName(), &data->isOpen, J_GUI_WINDOW_FLAG_NO_RESIZE | J_GUI_WINDOW_FLAG_NO_DOCKING);
			JGui::Text("Creation option");

			JGui::Text("x count");
			JGui::SameLine();
			JGui::InputInt("##Coord x count" + GetName(), &testData.xCount);
			testData.xCount = std::clamp(testData.xCount, testData.minObjCount, testData.maxObjCount);

			JGui::Text("y count");
			JGui::SameLine();
			JGui::InputInt("##Coord y count" + GetName(), &testData.yCount);
			testData.yCount = std::clamp(testData.yCount, testData.minObjCount, testData.maxObjCount);

			JGui::Text("z count");
			JGui::SameLine();
			JGui::InputInt("##Coord z count" + GetName(), &testData.zCount);
			testData.zCount = std::clamp(testData.zCount, testData.minObjCount, testData.maxObjCount);

			std::string oLabel[2] = { "Shape", "Light"};
			TestData::OBJ_TYPE objType[2] = { TestData::OBJ_TYPE::SHAPE, TestData::OBJ_TYPE::LIGHT };
			bool testObjCheck[2] = { testData.objType == TestData::OBJ_TYPE::SHAPE,
				testData.objType == TestData::OBJ_TYPE::LIGHT};

			for (uint i = 0; i < 2; ++i)
			{
				if (JGui::CheckBox(oLabel[i] + "##Coord" + GetName(), testObjCheck[i]))
					testData.objType = objType[i];
				if (i != 1)
					JGui::SameLine();
			}

			if (testData.objType == TestData::OBJ_TYPE::SHAPE)
			{
				std::string tLabel[3] = { "Cube", "Sphere", "Dragon" };
				J_DEFAULT_SHAPE shapeType[3] = { J_DEFAULT_SHAPE::CUBE,  J_DEFAULT_SHAPE::SPHERE,  J_DEFAULT_SHAPE::DRAGON };
				bool testMeshCheck[3] = { testData.meshType == J_DEFAULT_SHAPE::CUBE,
					testData.meshType == J_DEFAULT_SHAPE::SPHERE,
					testData.meshType == J_DEFAULT_SHAPE::DRAGON };

				for (uint i = 0; i < 3; ++i)
				{
					if (JGui::CheckBox(tLabel[i] + "##JSceneObserverSceneCoord", testMeshCheck[i]))
						testData.meshType = shapeType[i];
					if (i != 2)
						JGui::SameLine();
				}
			} 
			else if (testData.objType == TestData::OBJ_TYPE::LIGHT)
			{
				std::string lLabel[3] = { "Point", "Spot", "Rect" };
				J_LIGHT_TYPE litType[3] = { J_LIGHT_TYPE::POINT, J_LIGHT_TYPE::SPOT, J_LIGHT_TYPE::RECT };
				bool testLitCheck[3] = { testData.litType == J_LIGHT_TYPE::POINT,
					testData.litType == J_LIGHT_TYPE::SPOT,
					testData.litType == J_LIGHT_TYPE::RECT };

				for (uint i = 0; i < 3; ++i)
				{
					if (JGui::CheckBox(lLabel[i] + "##Coord" + GetName(), testLitCheck[i]))
						testData.litType = litType[i];
					if (i != 2)
						JGui::SameLine();
				}
			}

			std::string tableColumnLabel[4] = { "Name", "x", "y", "z" };
			std::string tableRowLabel[4] = { "position", "rotation", "scale", "distance" };
			JVector3<float>* value[4] = { &testData.offsetPos, &testData.offsetRot, &testData.offsetScale, &testData.distance };

			J_GUI_TABLE_FLAG_ tableFlag = J_GUI_TABLE_FLAG_SIZING_FIXED_FIT | J_GUI_TABLE_FLAG_BORDER_V |
				J_GUI_TABLE_FLAG_BORDER_OUTHER_H | J_GUI_TABLE_FLAG_ROW_BG | J_GUI_TABLE_FLAG_CONTEXT_MENU_IN_BODY;
			const J_GUI_TABLE_COLUMN_FLAG_ columnDefaultFlag = J_GUI_TABLE_COLUMN_FLAG_WIDTH_STRETCH;

			if (JGui::BeginTable("##Table" + GetName(), 4, tableFlag))
			{
				for (uint i = 0; i < 4; ++i)
					JGui::TableSetupColumn(tableColumnLabel[i], columnDefaultFlag);
				JGui::TableHeadersRow();
				JGui::TableNextRow();

				for (uint i = 0; i < 4; ++i)
				{
					JGui::TableSetColumnIndex(0);
					JGui::Text(tableRowLabel[i]);
					JGui::TableSetColumnIndex(1);
					JGui::InputFloat("##Coord" + tableRowLabel[i] + "X" + GetName(), &value[i]->x);
					JGui::TableSetColumnIndex(2);
					JGui::InputFloat("##Coord" + tableRowLabel[i] + "Y" + GetName(), &value[i]->y);
					JGui::TableSetColumnIndex(3);
					JGui::InputFloat("##Coord" + tableRowLabel[i] + "Z" + GetName(), &value[i]->z);
					if (i + 1 < 4)
						JGui::TableNextRow();
				}
				JGui::EndTable();
			}

			if (JGui::Button("Create##TestOption" + GetName()))
			{
				if (testData.objType == TestData::OBJ_TYPE::SHAPE)
					CreateShapeGroup();
				else if (testData.objType == TestData::OBJ_TYPE::LIGHT)
					CreateLightGroup();
			}


			JGui::EndWindow();
		}
		void JSceneObserver::OctreeOptionOnScreen()
		{
			JOctreeOption octreeOption = scene->GetOctreeOption(J_ACCELERATOR_LAYER::COMMON_OBJECT);
			int minSize = octreeOption.minSize;
			int octreeSizeSquare = octreeOption.octreeSizeSquare;

			bool isChanged = false;
			isChanged |= JGui::InputInt("minSize##"+ GetName(), &minSize, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE);
			isChanged |= JGui::InputInt("octreeSizeSquare##"+ GetName(), &octreeSizeSquare, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE);
			isChanged |= JGui::InputFloat("looseFactor##"+ GetName(), &octreeOption.looseFactor, J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE);
			isChanged |= CommonOptionOnScreen("Octree", octreeOption.commonOption);

			octreeOption.minSize = minSize;
			octreeOption.octreeSizeSquare = octreeSizeSquare;
			if (isChanged)
			{
				scene->SetOctreeOption(J_ACCELERATOR_LAYER::COMMON_OBJECT, octreeOption);
				SetModifiedBit(scene, true);
			}
		}
		void JSceneObserver::BvhOptionOnScreen()
		{
			bool isUpdated = false;
			JBvhOption bvhOption = scene->GetBvhOption(J_ACCELERATOR_LAYER::COMMON_OBJECT);
			Core::JEnumInfo* buildEnumInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(J_ACCELERATOR_BUILD_TYPE).name());
			Core::JEnumInfo* splitEnumInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(J_ACCELERATOR_SPLIT_TYPE).name());
			int buildIndex = buildEnumInfo->GetEnumIndex((int)bvhOption.buildType);
			int splitIndex = splitEnumInfo->GetEnumIndex((int)bvhOption.splitType);
			int preBuildIndex = buildIndex;
			int preSplitIndex = splitIndex;

			JGui::ComboEnumSet<J_ACCELERATOR_BUILD_TYPE>("BuildType##Bvh_SceneObserver", buildIndex);
			if (buildIndex != preBuildIndex)
				isUpdated = true;

			JGui::ComboEnumSet<J_ACCELERATOR_SPLIT_TYPE>("SplitType##Bvh_SceneObserver", splitIndex);
			if (splitIndex != preSplitIndex)
				isUpdated = true;

			isUpdated |= CommonOptionOnScreen("Bvh", bvhOption.commonOption);
			if (isUpdated)
			{
				scene->SetBvhOption(J_ACCELERATOR_LAYER::COMMON_OBJECT, bvhOption);
				SetModifiedBit(scene, true);
			}
		}
		void JSceneObserver::KdTreeOptionOnScreen()
		{
			bool isUpdated = false;
			JKdTreeOption kdTreeOption = scene->GetKdTreeOption(J_ACCELERATOR_LAYER::COMMON_OBJECT);
			Core::JEnumInfo* buildEnumInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(J_ACCELERATOR_BUILD_TYPE).name());
			Core::JEnumInfo* splitEnumInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(J_ACCELERATOR_SPLIT_TYPE).name());
			int buildIndex = buildEnumInfo->GetEnumIndex((int)kdTreeOption.buildType);
			int splitIndex = splitEnumInfo->GetEnumIndex((int)kdTreeOption.splitType);
			int preBuildIndex = buildIndex;
			int preSplitIndex = splitIndex;

			JGui::ComboEnumSet<J_ACCELERATOR_BUILD_TYPE>("BuildType##Kd_SceneObserver", buildIndex);
			if (buildIndex != preBuildIndex)
				isUpdated = true;

			JGui::ComboEnumSet<J_ACCELERATOR_SPLIT_TYPE>("SplitType##Kd_SceneObserver", splitIndex);
			if (splitIndex != preSplitIndex)
				isUpdated = true;

			isUpdated |= CommonOptionOnScreen("Kd", kdTreeOption.commonOption);
			if (isUpdated)
			{
				scene->SetKdTreeOption(J_ACCELERATOR_LAYER::COMMON_OBJECT, kdTreeOption);
				SetModifiedBit(scene, true);
			}
		}
		bool JSceneObserver::CommonOptionOnScreen(const std::string& uniqueName, JAcceleratorOption& commonOption)
		{
			bool isChanged = false;
			isChanged |= JGui::CheckBox("Activate##"+ GetName() + uniqueName, commonOption.isAcceleratorActivated);
			isChanged |= JGui::CheckBox("DebugBBox##"+ GetName() + uniqueName, commonOption.isDebugActivated);
			isChanged |= JGui::CheckBox("DebugLeafOnly##"+ GetName() + uniqueName, commonOption.isDebugLeafOnly);
			isChanged |= JGui::CheckBox("CullingActivate##"+ GetName() + uniqueName, commonOption.isCullingActivated);
			return isChanged;
		}
		void JSceneObserver::DebugTreeOnScreen()
		{
			//수정필요
			//소수의 object에 대해서는 정확하나
			//다수의 object에는 정확한 대응을 하지못함
			//폐기하거나 수정하거나 추후에 결정할것.
			auto spaceData = &nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::OPTION_SPACE_SPATIAL];
			auto treeData = &nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::VIEW_SETTING_ACCELERATOR_TREE];

			if (spaceData->selectedIndex != -1)
			{
				J_ACCELERATOR_TYPE type = (J_ACCELERATOR_TYPE)spaceData->selectedIndex; 
				editBTreeView->ClearNode(); 
				SceneDebugInterface::BuildDebugTree(scene, type, J_ACCELERATOR_LAYER::COMMON_OBJECT, editBTreeView.get());
				if (editBTreeView->BeginView(Core::GetName(type) + +"##DebugTreeView", &treeData->isOpen, J_GUI_WINDOW_FLAG_NO_DOCKING))
				{
					editBTreeView->OnScreen();
					editBTreeView->EndView();
				}
			}
		}
		void JSceneObserver::ShadowMapViewerOnScreen()
		{
			auto data = &nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::VIEW_SHADOW_VIEWER];
			if (JGui::BeginWindow("ShadowMap##"+ GetName(), &data->isOpen, J_GUI_WINDOW_FLAG_NO_DOCKING))
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
					if (data->selectedIndex >= shadowLitCount)
						data->selectedIndex = 0;

					auto selectedLit = shadowLitVec[data->selectedIndex];
					JGui::ComboSet("Light##SceneObserve", data->selectedIndex, shadowLitVec);
					if (shadowLitVec[data->selectedIndex]->AllowDisplayShadowMap())
					{
						JGuiImageInfo info(shadowLitVec[data->selectedIndex],
							Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP,
							Graphic::J_GRAPHIC_BIND_TYPE::SRV); 
						JGui::Image(info, RenderResultImageSize());
					}
					else
						JGui::Text("Can display shadow map... please on allowDisplayShadowMap trigger");
				} 
			}
			JGui::EndWindow();
		}
		void JSceneObserver::RenderResultOnScreen()
		{
			auto data = &nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::VIEW_RENDER_RESULT];
			const size_t sceneGuid = scene->GetGuid();
			const uint pointLitCount = Graphic::JFrameUpdateData::GetAreaRegistedCount(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT, sceneGuid);
			const uint spotLitCount = Graphic::JFrameUpdateData::GetAreaRegistedCount(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT, sceneGuid);
			const uint rectLitCount = Graphic::JFrameUpdateData::GetAreaRegistedCount(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT, sceneGuid);
			const uint litSum = pointLitCount + spotLitCount + rectLitCount;		
		 
			if (JGui::BeginWindow("RenderResult##" + GetName(), &data->isOpen, J_GUI_WINDOW_FLAG_NO_DOCKING))
			{
				JGui::SetCursorPosX(JGui::GetWindowSize().x * 0.9f - JGui::GetFramePadding().x - JGui::GetScrollBarSize());
				JGui::SetNextItemWidth(JGui::GetWindowSize().x * 0.1f);
				JGui::SliderFloat("##RenderResult_Size", &rsImageSizeFactor, 0.001f, 2.0f, 2);

				auto graphicOption = JGraphic::Instance().GetGraphicOption();
				auto camVec = scene->GetComponentVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA);
				for (const auto& camData : camVec)
				{
					JCamera* cam = static_cast<JCamera*>(camData.Get());
					auto gInterface = cam->GraphicResourceUserInterface();
					const uint rtDataIndex = gInterface.GetResourceDataIndex(Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, Graphic::J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
 
					JGui::Text(JCUtil::WstrToU8Str(cam->GetOwner()->GetName()));
					if (!cam->AllowDisplayRenderResult())
						continue;

					if (gInterface.HasOption(Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP, rtDataIndex))
					{
						auto handle = gInterface.GetOptionGpuHandle(Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON,
							Graphic::J_GRAPHIC_BIND_TYPE::SRV,
							Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP,
							0,
							rtDataIndex);

						JGui::Text("Albedo Map");
						JGuiImageInfo info(handle); 
						JGui::Image(info, RenderResultImageSize());
					}
 
					//depth, normal, ssao		... except tangent
					using GI = Graphic::JGraphicResourceUserInterface; 
					using condFunc = bool(*)(const GI&);
					constexpr uint deubgMapCount = 6; 
					condFunc cond[deubgMapCount]
					{
						[](const GI& g)
						{
							auto index = g.GetResourceDataIndex(Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, Graphic::J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
							return g.HasOption(Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY, index);
						},
						[](const GI& g) {return true; },
						[](const GI& g) 
						{
							auto index = g.GetResourceDataIndex(Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, Graphic::J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
							return g.HasOption(Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP, index);
						},
						[](const GI& g)
						{
							auto index = g.GetResourceDataIndex(Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, Graphic::J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
							return g.HasOption(Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP, index);
						},
						[](const GI& g)
						{
							auto index = g.GetResourceDataIndex(Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, Graphic::J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
							return g.HasOption(Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY, index);
						},
						[](const GI& g){return g.HasHandle(Graphic::J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP); }
					};
					std::string name[deubgMapCount]
					{
						"Depth Map",
						"Specular Map",
						"Normal Map",
						"Tangent Map",
						"Velocity Map",
						"SSAO Map"
					};

					const uint debugMapCount = gInterface.GetDataCount(Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP);
					for (uint i = 0; i < debugMapCount; ++i)
					{
						if (!cond[i](gInterface))
							continue;

						int dataIndex = i;
						//swap display order depth & specular
						if (dataIndex == 0)
							dataIndex = 1;
						else if (dataIndex == 1)
							dataIndex = 0;

						JGui::Text(name[dataIndex]);
						auto handle = gInterface.GetGpuHandle(Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP, Graphic::J_GRAPHIC_BIND_TYPE::SRV, 0, dataIndex);
						JGuiImageInfo info(handle);
						JGui::Image(info, RenderResultImageSize());
					}

					if (cam->AllowDisplayLightCullingDebug() && litSum > 0)
					{ 
						JGui::Text("Light list visualize");
						JGuiImageInfo info(cam,
							Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON,
							Graphic::J_GRAPHIC_BIND_TYPE::SRV);

						info.dataIndex = gInterface.GetResourceDataIndex(Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, Graphic::J_GRAPHIC_TASK_TYPE::LIGHT_LIST_DRAW);
						info.displayAllType = false;

						JGui::Image(info, RenderResultImageSize());

						//unuse
						//rt resource has lightCulling class not camera
						/*
						if (pointLitCount > 0)
						{ 
							JGui::Text("First point light rt");
							JGuiImageInfo pointInfo(cam,
								Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING,
								Graphic::J_GRAPHIC_BIND_TYPE::SRV);
							pointInfo.dataIndex = 0;
							pointInfo.displayAllType = false;
							JGui::Image(pointInfo, RenderResultImageSize());
						}
						if (spotLitCount > 0)					
						{ 
							JGui::Text("First spot light rt");
							JGuiImageInfo spotInfo(cam,
								Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING,
								Graphic::J_GRAPHIC_BIND_TYPE::SRV);
							spotInfo.dataIndex = 1;
							spotInfo.displayAllType = false;
							JGui::Image(spotInfo, RenderResultImageSize());
						}
						if (rectLitCount > 0)
						{ 
							JGui::Text("First rect light rt");
							JGuiImageInfo rectInfo(cam,
								Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING,
								Graphic::J_GRAPHIC_BIND_TYPE::SRV);
							rectInfo.dataIndex = 2;
							rectInfo.displayAllType = false;
							JGui::Image(rectInfo, RenderResultImageSize());
						}
						*/
					}
					if ((cam->AllowHzbOcclusionCulling() || cam->AllowHdOcclusionCulling()) && cam->AllowDisplayOccCullingDepthMap())
					{
						//cam has one occ
						auto afterDisplayImagePtr = [](int i)
						{ 
							int mod = i % 4;
							if (0 == mod || mod == 1 || mod == 2)
								JGui::SameLine();
						};
						 
						JGui::Text("Occlusion Depth Map");
						JGuiImageInfo info(cam,
							Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG,
							Graphic::J_GRAPHIC_BIND_TYPE::SRV); 
						if (cam->AllowHzbOcclusionCulling())
							info.extraPerImagePtr = afterDisplayImagePtr;

						JGui::Image(info, RenderResultImageSize());
					} 
					if (graphicOption.CanUseRtGi() && cam->AllowRaytracingGI())
					{
						JGui::Text("RtGi");
						JGuiImageInfo info(cam,
							Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON,
							Graphic::J_GRAPHIC_BIND_TYPE::SRV); 
						info.dataIndex = gInterface.GetResourceDataIndex(Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, Graphic::J_GRAPHIC_TASK_TYPE::RAYTRACING_GI);
						JGui::Image(info, RenderResultImageSize());

						//finalColorSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::RAYTRACING_GI);
					}
					JGui::Separator();
				}
				//JGui::SameLine();
				//JGui::Image((ImTextureID)(JGraphic::Instance().GetDebugSrvHandle(1)).ptr, ImVec2(400, 250));

				//JGui::BeginWindow("##OcclusionResultWindow2", &isOpenOcclusionMapViewer, J_GUI_WINDOW_FLAG_NO_DOCKING); 
				//JGui::Image((ImTextureID)(JGraphic::Instance().GetDebugSrvHandle(0)).ptr, JGui::GetWindowSize());
				//JGui::EndWindow();
			}
			JGui::EndWindow();
		}
		void JSceneObserver::TextureDetailOnScreen()
		{
			auto data = &nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::VIEW_TEXTURE_DETAIL];
			const J_GUI_WINDOW_FLAG_ flag = J_GUI_WINDOW_FLAG_NO_SAVE |  J_GUI_WINDOW_FLAG_NO_DOCKING;
			if (JGui::BeginWindow(("TextureDetail##" + GetName()), &data->isOpen, flag))
			{
				auto selectLam = [](JSceneObserver* ob){Graphic::JGraphicResourceUserInterface::ClearFirstResourceMipmapBind(ob->textureDebug->dataHandle);};
				using SelectF = Core::JSFunctorType<void, JSceneObserver*>;
				
				JEditorIdentifierList::DisplayDataSet dataSet(GetName());
				dataSet.callWindowAutomatically = false;
				dataSet.selectB = Core::UniqueBind(std::make_unique<SelectF::Functor>(selectLam), this);

				idenList->SetTypeGuid(JTexture::StaticTypeInfo().TypeGuid());
				idenList->Display(dataSet);

				JGui::SameLine();
				JGui::SetNextItemWidth(JGui::GetWindowSize().x * 0.1f);
				JGui::SliderFloat("##TextureDetailOnScreen_Size", &textureDebug->sizeFactor, 0.001f, 1.0f, 2);

				const JVector2F imageSize = JGui::GetWindowSize() * textureDebug->sizeFactor;
				auto selectedTexture = idenList->GetSelectedUser<JTexture>(); 
				bool canDisplayMipmap = false;
			 
				auto gUser = selectedTexture->GraphicResourceUserInterface();
				if (gUser.GetFirstMipmapType() != Graphic::J_GRAPHIC_MIP_MAP_TYPE::NONE)
				{
					if (gUser.TryFirstResourceMipmapBind(textureDebug->gpuHandle, textureDebug->dataHandle))
						canDisplayMipmap = true;
				}

				JTexture* texture = static_cast<JTexture*>(selectedTexture.Get());
				//JVector2<uint> oriSize = JVector2<uint>(texture->GetTextureWidth(), texture->GetTextureHeight());
				JGuiImageInfo info(texture); 
				JGui::Image(info, imageSize);

				if (canDisplayMipmap)
				{ 
					//oriSize /= 2;
					const uint count = (uint)textureDebug->gpuHandle.size();
					for (uint i = 1; i < count; ++i)
					{
						JGuiImageInfo info(textureDebug->gpuHandle[i]);
						JGui::Text("Mipmap(" + std::to_string(i) + ")");
						JGui::Image(info, imageSize);
						//oriSize /= 2;
					}
					Graphic::JGraphicResourceUserInterface::ClearFirstResourceMipmapBind(textureDebug->dataHandle);
				} 			 
			}
			JGui::EndWindow();
		}
		void JSceneObserver::UpdateMainCamFrustum()noexcept
		{
			auto camVec = scene->GetComponentVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA);
			for (const auto& data : camVec)
			{
				if (!data->GetOwner()->IsEditorObject())
				{
					if (!geoTool->HasGeo(data->GetGuid()))
						geoTool->CreateFrustumView(Core::ConnectChildUserPtr<JCamera>(data), scene->GetDebugRootGameObject());
				}
			}
		}
		void JSceneObserver::ActivateTransformToolType(const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE type)
		{
			for (uint i = 0; i < Private::toolCount; ++i)
				DeActivateTransformToolType((J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE)i);

			for (uint i = 0; i < Private::toolCount; ++i)
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
			for (uint i = 0; i < Private::toolCount; ++i)
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
			camObj->GetTransform()->SetPosition(editCamData.lastPos);
			camObj->GetTransform()->SetRotation(editCamData.lastRot);
			editCamData.cam = camObj->GetComponent<JCamera>();
			editCamData.cam->SetCameraState(J_CAMERA_STATE::RENDER);

			if (nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS].isUse)
				positionTool->SetDebugRoot(scene->GetDebugRootGameObject());
			if (nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_ROT].isUse)
				rotationTool->SetDebugRoot(scene->GetDebugRootGameObject());
			if (nodeUtilData[(int)J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_SCALE].isUse)
				scaleTool->SetDebugRoot(scene->GetDebugRootGameObject());

			geoTool = std::make_unique<JEditorGeometryTool>();
			textureDebug = std::make_unique<TextureDebug>();
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
			geoTool = nullptr;
			textureDebug = nullptr;
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

			menuIconTexture.resize(Private::menuIconCount);
			menuIconTexture[0] = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::POSITION_ARROW);
			menuIconTexture[1] = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::ROTATION_ARROW);
			menuIconTexture[2] = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::SCALE_ARROW);
			menuIconTexture[3] = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::VIEW_FRUSTUM_ICON);
			menuIconTexture[4] = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::COORD_GRID_ICON);
			menuIconTexture[5] = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::PLAY_SCENE_TIME);
			menuIconTexture[6] = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::PAUSE_SCENE_TIME);

			sceneIconTexture.resize(Private::sceneIconCount);
			sceneIconTexture[0] = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::DIRECTIONAL_LIGHT);
			sceneIconTexture[1] = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::POINT_LIGHT);
			sceneIconTexture[2] = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::SPOT_LIGHT);
			sceneIconTexture[3] = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::RECT_LIGHT);
			sceneIconTexture[4] = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::CAMERA);

			toolVec.resize(Private::toolCount);
			toolVec[0] = positionTool.get();
			toolVec[1] = rotationTool.get();
			toolVec[2] = scaleTool.get();
			testData.Initialize();
		}
		void JSceneObserver::DoSetClose()noexcept
		{
			positionTool.reset();
			rotationTool.reset();
			scaleTool.reset();
			menuIconTexture.clear();
			toolVec.clear();
			testData.Clear();
			JEditorWindow::DoSetClose();
		}
		void JSceneObserver::DoActivate()noexcept
		{
			JEditorWindow::DoActivate();
			if (scene.IsValid())
				CreateHelperGameObject();

			editCamData.cam->SetAllowFrustumCulling(editOption.allowFrustumCulling);
			editCamData.cam->SetAllowHzbOcclusionCulling(editOption.allowOccCulling);
			editCamData.cam->SetAllowDisplayDebugObject(editOption.allowDisplayDebugging);
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

			editCamCtrl->SetMousePos(JGui::GetMousePos());
		}
		void JSceneObserver::DoDeActivate()noexcept
		{
			DestroyHelperGameObject();
			RemoveListener(*JEditorEvent::EvInterface(), GetGuid());
			JEditorWindow::DoDeActivate();
		}
		void JSceneObserver::LoadEditorWindow(JFileIOTool& tool)
		{
			JVector3<float> lastPos;
			JVector3<float> lastRot;
			int lineCount = 0;
			int lineStep = 0;
			float movementFactor = 0;

			JEditorWindow::LoadEditorWindow(tool);
			JFileIOHelper::LoadVector3(tool, lastPos, "LastCamPos:");
			JFileIOHelper::LoadVector3(tool, lastRot, "LastCamRot:");

			tool.PushExistStack("NodeData");
			for (uint i = 0; i < (uint)J_OBSERVER_SETTING_TYPE::COUNT; ++i)
			{
				auto data = &nodeUtilData[i];
				if (!tool.PushExistStack())
					break;

				JFileIOHelper::LoadAtomicData(tool, data->isOpen, "IsOpen:");
				JFileIOHelper::LoadAtomicData(tool, data->selectedIndex, "SeletedIndex:");
				tool.PopStack();
			}
			tool.PopStack();
			JFileIOHelper::LoadAtomicData(tool, editOption.allowDisplayDebugging, "allowDisplayDebugging");
			JFileIOHelper::LoadAtomicData(tool, editOption.allowFrustumCulling, "allowFrustumCulling");
			JFileIOHelper::LoadAtomicData(tool, editOption.allowOccCulling, "allowOccCulling");
			JFileIOHelper::LoadAtomicData(tool, editOption.allowReflectCullingResult, "allowReflectCullingResult");
			JFileIOHelper::LoadAtomicData(tool, lineCount, "CoordLineCount");
			JFileIOHelper::LoadAtomicData(tool, lineStep, "CoordLineStep");
			JFileIOHelper::LoadAtomicData(tool, movementFactor, "MovemnetFactor:");

			editCamData.lastPos = lastPos;
			editCamData.lastRot = lastRot;
			coordGrid->SetLineCount(lineCount);
			coordGrid->SetLineStep(lineStep);
			editCamCtrl->SetMovemnetFactor(movementFactor);
		}
		void JSceneObserver::StoreEditorWindow(JFileIOTool& tool)
		{
			JEditorWindow::StoreEditorWindow(tool);
			JFileIOHelper::StoreVector3(tool, editCamData.lastPos, "LastCamPos:");
			JFileIOHelper::StoreVector3(tool, editCamData.lastRot, "LastCamRot:");

			tool.PushArrayOwner("NodeData");
			for (uint i = 0; i < (uint)J_OBSERVER_SETTING_TYPE::COUNT; ++i)
			{
				auto data = &nodeUtilData[i];
				tool.PushArrayMember();
				JFileIOHelper::StoreAtomicData(tool, data->isOpen, "IsOpen:");
				JFileIOHelper::StoreAtomicData(tool, data->selectedIndex, "SeletedIndex:");
				tool.PopStack();
			}
			tool.PopStack();
			JFileIOHelper::StoreAtomicData(tool, editOption.allowDisplayDebugging, "allowDisplayDebugging");
			JFileIOHelper::StoreAtomicData(tool, editOption.allowFrustumCulling, "allowFrustumCulling");
			JFileIOHelper::StoreAtomicData(tool, editOption.allowOccCulling, "allowOccCulling");
			JFileIOHelper::StoreAtomicData(tool, editOption.allowReflectCullingResult, "allowReflectCullingResult");
			JFileIOHelper::StoreAtomicData(tool, coordGrid->GetLineCount(), "CoordLineCount");
			JFileIOHelper::StoreAtomicData(tool, coordGrid->GetLineStep(), "CoordLineStep");
			JFileIOHelper::StoreAtomicData(tool, editCamCtrl->GetMovemnetFactor(), "MovemnetFactor:"); 
		}
		void JSceneObserver::CreateShapeGroup()
		{
			bool isOctreeActivated = false;
			bool isBvhActivated = false;
			bool isKdTreeActivated = false;
			JOctreeOption octreeOpt = scene->GetOctreeOption(J_ACCELERATOR_LAYER::COMMON_OBJECT);
			JBvhOption bvhOpt = scene->GetBvhOption(J_ACCELERATOR_LAYER::COMMON_OBJECT);
			JKdTreeOption kdTreeOpt = scene->GetKdTreeOption(J_ACCELERATOR_LAYER::COMMON_OBJECT);

			if (scene->IsAcceleratorActivated())
			{
				isOctreeActivated = octreeOpt.commonOption.isAcceleratorActivated;
				isBvhActivated = bvhOpt.commonOption.isAcceleratorActivated;
				isKdTreeActivated = kdTreeOpt.commonOption.isAcceleratorActivated;

				octreeOpt.commonOption.isAcceleratorActivated = false;
				bvhOpt.commonOption.isAcceleratorActivated = false;
				kdTreeOpt.commonOption.isAcceleratorActivated = false;

				if (isOctreeActivated)
					scene->SetOctreeOption(J_ACCELERATOR_LAYER::COMMON_OBJECT, octreeOpt);
				if (isBvhActivated)
					scene->SetBvhOption(J_ACCELERATOR_LAYER::COMMON_OBJECT, bvhOpt);
				if (isKdTreeActivated)
					scene->SetKdTreeOption(J_ACCELERATOR_LAYER::COMMON_OBJECT, kdTreeOpt);
			}
			JUserPtr<JGameObject> parent = JGCI::CreateShape(scene->GetRootGameObject(), OBJECT_FLAG_NONE, J_DEFAULT_SHAPE::EMPTY);
			parent->SetName(L"Test Object Set");
			for (int i = 0; i < testData.xCount; ++i)
			{
				for (int j = 0; j < testData.yCount; ++j)
				{
					for (int k = 0; k < testData.zCount; ++k)
					{
						JUserPtr<JGameObject> gObj = JGCI::CreateShape(parent, OBJECT_FLAG_NONE, testData.meshType);
						gObj->GetTransform()->SetScale(testData.offsetScale);
						gObj->GetTransform()->SetRotation(testData.offsetRot);
						gObj->GetTransform()->SetPosition((testData.offsetPos + JVector3<float>(i * testData.distance.x, j * testData.distance.y, k * testData.distance.z)));
						gObj->GetRenderItem()->SetMaterial(0, testData.matVec[((i + j + k) % TestData::matCount)]);
					}
				}
			}
			testData.objParentVec.push_back(parent);
			if (isOctreeActivated)
			{
				octreeOpt.commonOption.isAcceleratorActivated = true;
				scene->SetOctreeOption(J_ACCELERATOR_LAYER::COMMON_OBJECT, octreeOpt);
			}
			if (isBvhActivated)
			{
				bvhOpt.commonOption.isAcceleratorActivated = true;
				scene->SetBvhOption(J_ACCELERATOR_LAYER::COMMON_OBJECT, bvhOpt);
			}
			if (isKdTreeActivated)
			{
				kdTreeOpt.commonOption.isAcceleratorActivated = true;
				scene->SetKdTreeOption(J_ACCELERATOR_LAYER::COMMON_OBJECT, kdTreeOpt);
			}

			SetModifiedBit(scene, true);
		}
		void JSceneObserver::CreateLightGroup()
		{
			JUserPtr<JGameObject> parent = JGCI::CreateShape(scene->GetRootGameObject(), OBJECT_FLAG_NONE, J_DEFAULT_SHAPE::EMPTY);
			parent->SetName(L"Test Light Set");
			for (int i = 0; i < testData.xCount; ++i)
			{
				for (int j = 0; j < testData.yCount; ++j)
				{
					for (int k = 0; k < testData.zCount; ++k)			{
						 
						JUserPtr<JGameObject> gObj = JGCI::CreateLight(parent, OBJECT_FLAG_NONE, testData.litType);
						//gObj->GetTransform()->SetScale(testData.offsetScale);
						auto lit = gObj->GetComponent<JLight>();
						if (lit->GetLightType() == J_LIGHT_TYPE::RECT)
							static_cast<JRectLight*>(lit.Get())->SetPower(6.0f);

						gObj->GetTransform()->SetRotation(testData.offsetRot);
						gObj->GetTransform()->SetPosition((testData.offsetPos + JVector3<float>(i * testData.distance.x, j * testData.distance.y, k * testData.distance.z)));
					}
				}
			}
			testData.objParentVec.push_back(parent);
			SetModifiedBit(scene, true);
		}
		void JSceneObserver::OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)
		{
			JEditorWindow::OnEvent(senderGuid, eventType, eventStruct);
			if (!eventStruct->CanExecuteOtherEv(senderGuid, GetGuid()))
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

				JSceneAcceleratorInterface* iSceneSpace = scene->AcceleratorInterface();
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
					JGui::Text((JCUtil::WstrToU8Str(data->GetOwner()->GetName()) + " " +
						std::to_string(cullingInterface.IsCulled(Graphic::J_CULLING_TYPE::FRUSTUM,frameIndex)) + " "+
						std::to_string(cullingInterface.IsCulled(Graphic::J_CULLING_TYPE::OCCLUSION, frameIndex))).c_str());

					auto frustum = cam->GetBoundingFrustum();
					auto bbox = rItem->GetBoundingBox();

					auto nearFrustum = frustum;
					nearFrustum.Near = 0;
					nearFrustum.Far = frustum.Near;
					auto result = bbox.Contains(DirectX::XMLoadFloat3(&frustum.Origin));

					JGui::SameLine();
					JGui::Text(" Pos C: ");
					JGui::SameLine();
					if (result == DirectX::CONTAINS)
						JGui::Text("Contain");
					else if (result == DirectX::INTERSECTS)
						JGui::Text("Intersect");
					else if (result == DirectX::DISJOINT)
						JGui::Text("Disjoint");

					JGui::SameLine();
					JGui::Text("B Frustum C: ");
					JGui::SameLine();
					result = bbox.Contains(nearFrustum);
					if (result == DirectX::CONTAINS)
						JGui::Text("Contain");
					else if (result == DirectX::INTERSECTS)
						JGui::Text("Intersect");
					else if (result == DirectX::DISJOINT)
						JGui::Text("Disjoint");

					JGui::SameLine();
					JGui::Text("F Frustum C: ");
					JGui::SameLine();
					result = nearFrustum.Contains(bbox);
					if (result == DirectX::CONTAINS)
						JGui::Text("Contain");
					else if (result == DirectX::INTERSECTS)
						JGui::Text("Intersect");
					else if (result == DirectX::DISJOINT)
						JGui::Text("Disjoint");
				}

*/