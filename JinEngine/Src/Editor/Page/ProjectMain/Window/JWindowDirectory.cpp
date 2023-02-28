#include"JWindowDirectory.h"
#include"../../JEditorPageShareData.h"
#include"../../JEditorAttribute.h"

#include"../../../Event/JEditorEvent.h"
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h" 
#include"../../../String/JEditorStringMap.h"
#include"../../../Popup/JEditorPopupMenu.h"
#include"../../../Popup/JEditorPopupNode.h"  
#include"../../../Helpers/JEditorSearchBarHelper.h"
#include"../../../Helpers/JEditorRenameHelper.h"
#include"../../../Align/JEditorAlignCalculator.h"
#include"../../../../Utility/JCommonUtility.h"   
#include"../../../../Utility/JMathHelper.h"   
#include"../../../../Core/Guid/GuidCreator.h" 
#include"../../../../Object/Component/Camera/JCamera.h"
#include"../../../../Object/Directory/JDirectoryFactory.h"
#include"../../../../Object/Directory/JDirectory.h"
#include"../../../../Object/Directory/JFile.h"
#include"../../../../Object/Resource/Scene/JScene.h"
#include"../../../../Object/Resource/AnimationController/JAnimationController.h"
#include"../../../../Object/Resource/Material/JMaterial.h" 
#include"../../../../Object/Resource/JResourceObjectFactory.h"
#include"../../../../Object/Resource/Scene/Preview/JPreviewScene.h" 
#include"../../../../Object/Resource/JResourceManager.h" 
#include"../../../../Object/Resource/JResourceImporter.h" 
#include"../../../../Window/JWindows.h"
#include"../../../../Application/JApplicationVariable.h"  
#include"../../../../../Lib/imgui/imgui.h"
#include<filesystem>


//#include"../../../../Core/File/JFileIOHelper.h"  
// #include "../../../../Graphic/GraphicResource/JGraphicResourceHandle.h"

namespace JinEngine
{
	class JAnimationController;
	class JMaterial;
	namespace Editor
	{
		namespace Constants
		{
			const std::string directoryViewName = "DirectoryView";
			const std::string fileViewName = "FileView";
			const std::wstring newDirectoryName = L"New Directory";
		}

		static int DirectoryLevelFinder(const std::string& path, std::wstring& copyTarget)
		{
			std::wstring copy = JCUtil::U8StrToWstr(path);
			int st = 0;
			size_t index = 0;
			int level = 0;
			while ((index = copy.find_first_of(L"\\")) != std::wstring::npos)
			{
				level++;
				copy.erase(st, index + 1);
			}
			copyTarget = copy;
			return level;
		}
		JWindowDirectory::JWindowDirectory(const std::string& name,
			std::unique_ptr<JEditorAttribute> attribute,
			const J_EDITOR_PAGE_TYPE ownerPageType,
			const J_EDITOR_WINDOW_FLAG windowFlag)
			:JEditorWindow(name, std::move(attribute), ownerPageType, windowFlag)
		{
			//JEditorStringMap Init
			editorString = std::make_unique<JEditorStringMap>();
			renameHelper = std::make_unique<JEditorRenameHelper>();
			searchBarHelper = std::make_unique<JEditorSearchBarHelper>(false);

			selectorIconSlidebarId = Core::MakeGuid();
			editorString->AddString(selectorIconSlidebarId, { "Icon Size" , u8"아이콘 크기" });

			BuildPopup();
		}
		JWindowDirectory::~JWindowDirectory() {}
		void JWindowDirectory::BuildPopup()
		{
			//directoryViewPopup.reset();
			fileviewPopup.reset();

			/*
			* Dir popup init
			std::unique_ptr<JEditorPopupNode> directoryViewPopupNode =
			std::make_unique<JEditorPopupNode>("Window JDirectory Directory Popup Root", J_EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

			std::unique_ptr<JEditorPopupNode> createDirectoryInDirectoryViewNode =
				std::make_unique<JEditorPopupNode>("Create Directory", J_EDITOR_POPUP_NODE_TYPE::LEAF, directoryViewPopupNode.get());
			editorString->AddString(createDirectoryInDirectoryViewNode->GetNodeId(), { "Create" , u8"폴더 생성" });

			std::unique_ptr<JEditorPopupNode> destoryDirectoryNode =
				std::make_unique<JEditorPopupNode>("Destroy Directory", J_EDITOR_POPUP_NODE_TYPE::LEAF, directoryViewPopupNode.get());
			editorString->AddString(destoryDirectoryNode->GetNodeId(), { "Delete" , u8"폴더 삭제" });

			std::unique_ptr<JEditorPopupNode> renameDirectoryNode =
				std::make_unique<JEditorPopupNode>("Rename Directory", J_EDITOR_POPUP_NODE_TYPE::LEAF, directoryViewPopupNode.get());
			editorString->AddString(renameDirectoryNode->GetNodeId(), { "Rename" , u8"새 이름" });

			*/
			//Popup Init
			std::unique_ptr<JEditorPopupNode> fileViewPopupRootNode =
				std::make_unique<JEditorPopupNode>("Window JDirectory FileView Popup Root", J_EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

			std::unique_ptr<JEditorPopupNode> createResourceNode =
				std::make_unique<JEditorPopupNode>("CreateResource", J_EDITOR_POPUP_NODE_TYPE::INTERNAL, fileViewPopupRootNode.get());
			editorString->AddString(createResourceNode->GetNodeId(), { "Create Resource" , u8"자원 생성" });

			std::unique_ptr<JEditorPopupNode> createMaterialNode =
				std::make_unique<JEditorPopupNode>("JMaterial", J_EDITOR_POPUP_NODE_TYPE::LEAF, createResourceNode.get());
			editorString->AddString(createMaterialNode->GetNodeId(), { "JMaterial" , u8"머테리얼" });

			std::unique_ptr<JEditorPopupNode> createSceneNode =
				std::make_unique<JEditorPopupNode>("JScene", J_EDITOR_POPUP_NODE_TYPE::LEAF, createResourceNode.get());
			editorString->AddString(createSceneNode->GetNodeId(), { "JScene" , u8"씬" });

			std::unique_ptr<JEditorPopupNode> createAnimationControllerNode =
				std::make_unique<JEditorPopupNode>("AnimationContorller", J_EDITOR_POPUP_NODE_TYPE::LEAF, createResourceNode.get());
			editorString->AddString(createAnimationControllerNode->GetNodeId(), { "AnimationContorller" , u8"애니메이션 컨트롤러" });

			std::unique_ptr<JEditorPopupNode> createDirctoryInFileViewNode =
				std::make_unique<JEditorPopupNode>("CreateDirctory", J_EDITOR_POPUP_NODE_TYPE::LEAF, fileViewPopupRootNode.get());
			editorString->AddString(createDirctoryInFileViewNode->GetNodeId(), { "Create Dirctory" , u8"폴더 생성" });

			std::unique_ptr<JEditorPopupNode> destroyNode =
				std::make_unique<JEditorPopupNode>("Destroy", J_EDITOR_POPUP_NODE_TYPE::LEAF, fileViewPopupRootNode.get());
			editorString->AddString(destroyNode->GetNodeId(), { "Destroy" , u8"삭제" });

			std::unique_ptr<JEditorPopupNode> importNode =
				std::make_unique<JEditorPopupNode>("ImportResource", J_EDITOR_POPUP_NODE_TYPE::LEAF, fileViewPopupRootNode.get());
			editorString->AddString(importNode->GetNodeId(), { "Import Resource" , u8"자원 불러오기" });

			std::unique_ptr<JEditorPopupNode> renameFileNode =
				std::make_unique<JEditorPopupNode>("Rename", J_EDITOR_POPUP_NODE_TYPE::LEAF, fileViewPopupRootNode.get());
			editorString->AddString(renameFileNode->GetNodeId(), { "Rename" , u8"새 이름" });

			auto createImportedResourceFLam = [](JWindowDirectory* wndDir, std::vector<JResourceObject*> rVec)
			{
				const uint resCount = (uint)rVec.size();
				for (uint i = 0; i < resCount; ++i)
				{
					if (rVec[i] != nullptr)
						wndDir->SetModifiedBit(Core::GetUserPtr(rVec[i]), true);
				}
				//search bar activate중에는 팝업생성 불가능
				wndDir->CreateDirectoryPreview(wndDir->opendDirctory.Get(), false);
			};
			auto renameLam = [](JWindowDirectory* wndDir)
			{
				wndDir->renameHelper->Activate(wndDir->selectedObj);
				wndDir->SetModifiedBit(wndDir->selectedObj, true);
			};

			createResourceF = std::make_unique<CreateObjectF::Functor>(&JWindowDirectory::CreateResourceObject, this);
			createDirectoryF = std::make_unique<CreateDirectoryF::Functor>(&JWindowDirectory::CreateDirectory, this);
			destroyObjectF = std::make_unique<DestroyObjectF::Functor>(&JWindowDirectory::DestroyObject, this);
			openNewDirF = std::make_unique<OpenNewDirectoryF::Functor>(&JWindowDirectory::OpenNewDirectory, this);
			importResourceF = std::make_unique<ImportResourceF::Functor>(&JWindowDirectory::ImportFile, this);
			createImportedResourceF = std::make_unique<CreateImportedResourceF::Functor>(createImportedResourceFLam);
			renameF = std::make_unique<RenameF::Functor>(renameLam);
			moveFileF = std::make_unique<MoveFIleF::Functor>(&JWindowDirectory::MoveFile, this); 
			regCreateRobjF = std::make_unique<RegisterCreateREvF::Functor>(&JWindowDirectory::RegisterCreateResourceObjectEv, this);
			regCreateDirF = std::make_unique<RegisterCreateDEvF::Functor>(&JWindowDirectory::RegisterCreateDirectoryEv, this);
			regDestroyObjF = std::make_unique<RegisterDestroyEvF::Functor>(&JWindowDirectory::RegisterDestroyResourceObjectEv, this);

			//createDirectoryInDirectoryViewNode->RegisterSelectBind(std::make_unique<RegisterCreateDEvF::CompletelyBind>(*regCreateDirF));
		//	destoryDirectoryNode->RegisterSelectBind(std::make_unique<RegisterDestroyEvF::CompletelyBind>(*regDestroyObjF));
			//renameDirectoryNode->RegisterSelectBind(std::make_unique<RenameF::CompletelyBind>(*renameF, this));

			createMaterialNode->RegisterSelectBind(std::make_unique<RegisterCreateREvF::CompletelyBind>(*regCreateRobjF, J_RESOURCE_TYPE::MATERIAL));
			createSceneNode->RegisterSelectBind(std::make_unique<RegisterCreateREvF::CompletelyBind>(*regCreateRobjF, J_RESOURCE_TYPE::SCENE));
			createAnimationControllerNode->RegisterSelectBind(std::make_unique<RegisterCreateREvF::CompletelyBind>(*regCreateRobjF, J_RESOURCE_TYPE::ANIMATION_CONTROLLER));
			createDirctoryInFileViewNode->RegisterSelectBind(std::make_unique<RegisterCreateDEvF::CompletelyBind>(*regCreateDirF));
			destroyNode->RegisterSelectBind(std::make_unique<RegisterDestroyEvF::CompletelyBind>(*regDestroyObjF));
			importNode->RegisterSelectBind(std::make_unique<ImportResourceF::CompletelyBind>(*importResourceF));
			renameFileNode->RegisterSelectBind(std::make_unique<RenameF::CompletelyBind>(*renameF, this));

			//directoryViewPopup = std::make_unique<JEditorPopupMenu>("Window JDirectory Directory Popup", std::move(directoryViewPopupNode));
			//directoryViewPopup->AddPopupNode(std::move(createDirectoryInDirectoryViewNode));
			//directoryViewPopup->AddPopupNode(std::move(destoryDirectoryNode));
			//directoryViewPopup->AddPopupNode(std::move(renameDirectoryNode));

			fileviewPopup = std::make_unique<JEditorPopupMenu>("Window JDirectory FileView Popup", std::move(fileViewPopupRootNode));
			fileviewPopup->AddPopupNode(std::move(createResourceNode));
			fileviewPopup->AddPopupNode(std::move(createMaterialNode));
			fileviewPopup->AddPopupNode(std::move(createSceneNode));
			fileviewPopup->AddPopupNode(std::move(createAnimationControllerNode));
			fileviewPopup->AddPopupNode(std::move(createDirctoryInFileViewNode));
			fileviewPopup->AddPopupNode(std::move(destroyNode));
			fileviewPopup->AddPopupNode(std::move(importNode));
			fileviewPopup->AddPopupNode(std::move(renameFileNode));
		}
		J_EDITOR_WINDOW_TYPE JWindowDirectory::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::WINDOW_DIRECTORY;
		}
		void JWindowDirectory::Initialize()
		{
			const std::wstring contentPath = JApplicationVariable::GetProjectContentPath();
			root = Core::GetUserPtr(JResourceManager::Instance().GetDirectory(contentPath));
			//searchBarHelper->RegisterDefaultObject(root);
			OpenNewDirectory(root);
		}
		void JWindowDirectory::UpdateWindow()
		{
			EnterWindow(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
			UpdateDocking();
			if (IsActivated() && opendDirctory.IsValid())
			{
				UpdateMouseClick();
				auto newSelected = JEditorPageShareData::GetSelectedObj(GetOwnerPageType());
				selectedObj = Core::JUserPtr<JObject>::ConvertChildUser(std::move(newSelected));

				const float yOffset = ImGui::GetStyle().WindowBorderSize + ImGui::GetStyle().WindowPadding.y;
				const float xOffset = ImGui::GetStyle().WindowBorderSize + ImGui::GetStyle().WindowPadding.x;
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + yOffset);
				ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.2f + xOffset);
				searchBarHelper->UpdateSearchBar();
				const bool hasData = searchBarHelper->HasInputData();
				const bool isUpdated = searchBarHelper->IsUpdateInputData();
				if (isUpdated)
				{
					if (!hasData)
					{
						ClearPreviewGroup();
						CreateDirectoryPreview(opendDirctory.Get(), false);
					}
					else
					{
						ClearPreviewGroup();
						CreateAllDirectoryPreview(root.Get(), true, JCUtil::U8StrToWstr(searchBarHelper->GetInputData()));
					}
					ClearMultiSelected();
				}

				ImGui::SameLine();
				const float fontSize = ImGui::GetCurrentContext()->FontSize;
				btnIconMaxSize = JImGuiImpl::GetDisplaySize().x * selectorIconMaxRate;
				btnIconMinSize = JImGuiImpl::GetDisplaySize().x * selectorIconMinRate;
				btnIconSize = JMathHelper::Clamp<float>(btnIconSize, btnIconMinSize, btnIconMaxSize);
				fileTitleBarSize = JVector2<float>(btnIconSize, fontSize * (btnIconMinSize / fontSize) + ImGui::GetWindowSize().y * 0.005f);

				const float preFrameY = ImGui::GetStyle().FramePadding.y;
				ImGui::GetStyle().FramePadding.y = 0;
				ImGui::SetCursorPosX(ImGui::GetWindowSize().x - JImGuiImpl::GetSliderWidth() - xOffset);
				ImGui::SetNextItemWidth(JImGuiImpl::GetSliderWidth());
				JImGuiImpl::SliderFloat("##" + GetName() + "_SizeSlider", &btnIconSize, btnIconMinSize, btnIconMaxSize, "", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoInput);
				if (ImGui::IsItemActive() || ImGui::IsItemHovered())
					ImGui::SetTooltip("%.1f", btnIconSize);
				ImGui::GetStyle().FramePadding.y = preFrameY;

				JVector2<float> nowCursor = ImGui::GetCursorPos();
				childWindowHeight = ImGui::GetWindowSize().y - nowCursor.y;
				//JImGuiImpl::DrawRectFrame(nowCursor, JVector2<float>(ImGui::GetWindowSize().x, childWindowHeight), 4, JImGuiImpl::GetUColor(ImGuiCol_FrameBg), true);
				BuildDirectoryView();
				ImGui::SameLine();
				BuildFileView();
			}
			CloseWindow();
		}
		void JWindowDirectory::BuildDirectoryView()
		{
			//editorString->GetString(selectorIconSlidebarId)
			const JVector2<float> windowSize = ImGui::GetWindowSize();
			const JVector2<float> viewSize = JVector2<float>(windowSize.x * 0.2f, childWindowHeight);
			JImGuiImpl::BeginChildWindow(Constants::directoryViewName.c_str(), viewSize, true, ImGuiWindowFlags_AlwaysAutoResize);

			const bool canSelect = !searchBarHelper->HasInputData();
			if (!canSelect)
				SetTreeNodeColor(0.2f);
			JDirectory* clickedDir = DirectoryViewOnScreen(root.Get(), canSelect);
			lastUpdateOpenNewDir = false;

			if (openNewDirB != nullptr)
			{
				openNewDirB->InvokeCompletelyBind();
				openNewDirB.reset();
			}
			if (clickedDir != nullptr && clickedDir->GetGuid() != opendDirctory->GetGuid())
			{
				OpenNewDirectory(Core::GetUserPtr(clickedDir));
				Core::JTransition::Log(JCUtil::WstrToU8Str(opendDirctory->GetName() + L" Open Directory"));
			}

			//보류
			//if (directoryViewPopup->IsOpen())
			//	directoryViewPopup->ExecutePopup(editorString.get());
			//directoryViewPopup->Update();
			JImGuiImpl::EndChildWindow();
		}
		void JWindowDirectory::BuildFileView()
		{
			const JVector2<float> windowSize = ImGui::GetWindowSize();
			const JVector2<float> viewSize = JVector2<float>(windowSize.x * 0.8f, childWindowHeight);
			JImGuiImpl::BeginChildWindow(Constants::fileViewName.c_str(), viewSize, true, ImGuiWindowFlags_AlwaysAutoResize);

			FileViewOnScreen();
			if (searchBarHelper->HasInputData() && fileviewPopup->IsOpen())
				fileviewPopup->SetOpen(false);
			if (fileviewPopup->IsOpen())
				fileviewPopup->ExecutePopup(editorString.get());

			bool canClearMultiSelected = HasMultiSelected() && (!IsFocus() || fileviewPopup->IsLastSelected());
			if (canClearMultiSelected)
			{
				ClearMultiSelected();
				auto dsEvStruct = std::make_unique<JEditorDeSelectObjectEvStruct>(GetOwnerPageType(), selectedObj->GetGuid());
				AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::DESELECT_OBJECT, JEditorEvent::RegisterEvStruct(std::move(dsEvStruct)));
			} 

			fileviewPopup->Update();
			JImGuiImpl::EndChildWindow();
		}
		JDirectory* JWindowDirectory::DirectoryViewOnScreen(JDirectory* directory, const bool canSelect)
		{
			ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_SpanAvailWidth |
				ImGuiTreeNodeFlags_Framed;

			bool isSelected = opendDirctory->GetGuid() == directory->GetGuid();
			if (isSelected && canSelect)
				SetTreeNodeColor(JImGuiImpl::GetSelectColorFactor());
			JDirectory* clickedDir = nullptr;
			if (lastUpdateOpenNewDir && opendDirctory->IsParent(directory))
				ImGui::SetNextItemOpen(true);

			bool isNodeOpen = JImGuiImpl::TreeNodeEx(JCUtil::WstrToU8Str(directory->GetName()).c_str(), baseFlags);
			if (ImGui::BeginDragDropTarget())
			{  
				if (!ImGui::IsMouseDragging(0) && JEditorPageShareData::IsEditableSelectedObject(GetOwnerPageType()))
					RequestMoveFile(directory, static_cast<JObject*>(JEditorPageShareData::GetSelectedObj(GetOwnerPageType()).Get()));
				ImGui::EndDragDropTarget();
			}
			if (isSelected && canSelect)
				SetTreeNodeColor(JImGuiImpl::GetSelectColorFactor() * -1);
			if (isNodeOpen)
			{
				if (ImGui::IsItemClicked(0) && canSelect)
					clickedDir = directory;

				const uint childDirCount = directory->GetChildernDirctoryCount();
				for (uint i = 0; i < childDirCount; ++i)
				{
					JDirectory* child = directory->GetChildDirctory(i);
					if (child == nullptr)
						continue;
					JDirectory* res = DirectoryViewOnScreen(child, canSelect);
					if (res != nullptr)
						clickedDir = res;
				}
				JImGuiImpl::TreePop();
			}
			return clickedDir;
		}
		void JWindowDirectory::FileViewOnScreen()
		{
			const JVector2<float> windowSize = ImGui::GetWindowSize();
			const JVector2<float> contentsSize = JVector2<float>(btnIconSize, btnIconSize + fileTitleBarSize.y);
			const JVector2<float> padding = JVector2<float>(windowSize.x * 0.015f, windowSize.y * 0.03f);
			const JVector2<float> spacing = windowSize * 0.015f;

			const JVector2<float> innerSize[2] = { btnIconSize, fileTitleBarSize * 0.8f };
			const JVector2<float> innerPosition[2] = { JVector2<float>(0, 0), fileTitleBarSize * 0.1f };

			JEditorDynamicAlignCalculator<2> widgetAlignCal;
			widgetAlignCal.Update(windowSize, contentsSize, padding, spacing, innerSize, innerPosition, J_EDITOR_INNER_ALGIN_TYPE::COLUMN, ImGui::GetCursorPos());
			JEditorTextAlignCalculator textAlignCal;

			const ImU32 contentsFrameColor = IM_COL32(175, 175, 185, 175);
			const ImU32 contentsBgColor = IM_COL32(95, 95, 115, 175);
			const ImU32 iconBgColor = IM_COL32(55, 55, 95, 175);
			const ImU32 textBgColor = IM_COL32(125, 125, 145, 175);
			const ImU32 bgRectDelta = IM_COL32(25, 25, 25, 0);

			const JVector4<float> sColFactor = JImGuiImpl::GetSelectColorFactor();
			SetTreeNodeColor(sColFactor);
			const ImU32 headerCol = JImGuiImpl::GetUColor(ImGuiCol_Header);
			const ImU32 hovCol = JImGuiImpl::GetUColor(ImGuiCol_HeaderHovered);
			const ImU32 hoveredAddedColor =  hovCol > headerCol ? hovCol - headerCol : headerCol - hovCol;
			const ImU32 selectedAddedColor = IM_COL32(sColFactor.x * 255, sColFactor.y * 255, sColFactor.z * 255, sColFactor.w * 255);

			JImGuiImpl::SetColor(JVector4<float>(0, 0, 0, 0), ImGuiCol_Header);
			JImGuiImpl::SetColor(JVector4<float>(0, 0, 0, 0), ImGuiCol_HeaderHovered);
			JImGuiImpl::SetColor(JVector4<float>(0, 0, 0, 0), ImGuiCol_HeaderActive);
			 
			const bool isMouseClick = ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1);
			const bool isPressedCtrl = ImGui::GetIO().KeyCtrl;
			bool isMouseInContents = false;
			if (isPressedCtrl)
			{
				if (!HasMultiSelected() && selectedObj.IsValid())
					AddMultiSelected(selectedObj->GetGuid(), selectedObj.Get());
			} 

			bool hasInvaildScene = false;
			const uint count = GetPreviewSceneCount();
			for (uint i = 0; i < count; ++i)
			{
				JPreviewScene* nowPreviewScene = GetPreviewScene(i);
				if (nowPreviewScene != nullptr)
				{
					Core::JUserPtr<JObject> nowObject = nowPreviewScene->GetJObject();
					if (!nowObject.IsValid())
					{
						hasInvaildScene = true;
						continue;
					}

					const J_OBJECT_FLAG flag = nowObject->GetFlag();
					if ((flag & OBJECT_FLAG_HIDDEN) > 0)
						continue;

					const J_OBJECT_TYPE objType = nowObject->GetObjectType();
					const bool isValidType = objType == J_OBJECT_TYPE::RESOURCE_OBJECT || objType == J_OBJECT_TYPE::DIRECTORY_OBJECT;
					if (!isValidType)
						continue;
					 
					bool isSelected = selectedObj.IsValid() && selectedObj->GetGuid() == nowObject->GetGuid();			
					if (HasMultiSelected())
						isSelected |= IsMultiSelected(nowObject->GetGuid());

					//Has order dependency
					const JVector2<float> iconSize = widgetAlignCal.GetInnerContentsSize();
					widgetAlignCal.SetNextContentsPosition();
					const bool isHorvered = JImGuiImpl::IsMouseInRect(ImGui::GetCurrentWindow()->DC.CursorPos, widgetAlignCal.GetTotalContentsSize());
					if (isHorvered)
						isMouseInContents = true;

					ImU32 addedColor = IM_COL32(0, 0, 0, 0);
					if (isHorvered)
						addedColor = hoveredAddedColor;
					if (isSelected)
						addedColor += selectedAddedColor;

					//draw bg frame rect
					JImGuiImpl::DrawRectFrame(ImGui::GetCurrentWindow()->DC.CursorPos,
						widgetAlignCal.GetTotalContentsSize(),
						5,
						contentsFrameColor + addedColor,
						true);

					//draw bg rect
					JImGuiImpl::DrawRectFilledMultiColor(ImGui::GetCurrentWindow()->DC.CursorPos,
						widgetAlignCal.GetTotalContentsSize(),
						contentsBgColor + addedColor,
						bgRectDelta,
						true);

					JVector2<float> preWorldCursorPos = ImGui::GetCurrentWindow()->DC.CursorPos;
					ImVec2 preCursor = ImGui::GetCursorPos();
					const std::string unqName = "##" + std::to_string(nowObject->GetGuid()) + "_Selectable";					
					if (JImGuiImpl::Selectable(unqName, false, ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_AllowItemOverlap, JVector2<float>(btnIconSize, btnIconSize + fileTitleBarSize.y)))
					{ 
						if (isPressedCtrl)
						{
							if (isSelected)
							{
								RemoveMultiSelected(nowObject->GetGuid());
								if (!HasMultiSelected())
								{
									auto dsEvStruct = std::make_unique<JEditorDeSelectObjectEvStruct>(GetOwnerPageType(), selectedObj->GetGuid());
									AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::DESELECT_OBJECT, JEditorEvent::RegisterEvStruct(std::move(dsEvStruct)));
								}
							}
							else
							{
								AddMultiSelected(nowObject->GetGuid(), nowObject.Get());
								if(!selectedObj.IsValid())
									RequestSelectObject(JEditorSelectObjectEvStruct{ GetOwnerPageType(), nowObject });
							}
						} 
						else
							RequestSelectObject(JEditorSelectObjectEvStruct{ GetOwnerPageType(), nowObject });
						if (objType == J_OBJECT_TYPE::RESOURCE_OBJECT)
						{
							JResourceObject* jRobj = static_cast<JResourceObject*>(nowObject.Get());
							const J_RESOURCE_TYPE resourceType = jRobj->GetResourceType();
							if (resourceType == J_RESOURCE_TYPE::SKELETON)
								RequestOpenPage(JEditorOpenPageEvStruct{ J_EDITOR_PAGE_TYPE::SKELETON_SETTING, Core::GetUserPtr(jRobj) }, true);
							else if (resourceType == J_RESOURCE_TYPE::ANIMATION_CONTROLLER)
								RequestOpenPage(JEditorOpenPageEvStruct{ J_EDITOR_PAGE_TYPE::ANICONT_SETTING, Core::GetUserPtr(jRobj) }, true);
						}
						else
						{
							JDirectory* jDir = static_cast<JDirectory*>(nowObject.Get());
							if (ImGui::GetMouseClickedCount(0) >= 2)
								openNewDirB = std::make_unique<OpenNewDirectoryF::CompletelyBind>(*openNewDirF, Core::GetUserPtr(jDir));
						}
					}
					 
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
					{
						RequestSelectObject({ GetOwnerPageType(), nowObject });
						std::string selectResourceName = JCUtil::WstrToU8Str(nowObject->GetName());
						JImGuiImpl::Text(selectResourceName);
						ImGui::SetDragDropPayload(selectResourceName.c_str(), JEditorPageShareData::GetDragGuidPtr(GetOwnerPageType()), sizeof(int));
						ImGui::EndDragDropSource();
					}

					if (ImGui::BeginDragDropTarget())
					{
						if (!ImGui::IsMouseDragging(0) && objType == J_OBJECT_TYPE::DIRECTORY_OBJECT && JEditorPageShareData::IsEditableSelectedObject(GetOwnerPageType()))
							RequestMoveFile(static_cast<JDirectory*>(nowObject.Get()), static_cast<JObject*>(JEditorPageShareData::GetSelectedObj(GetOwnerPageType()).Get()));
						ImGui::EndDragDropTarget();
					}

					ImGui::SetCursorPos(preCursor);
					if (nowPreviewScene->UseQuadShape())
					{
						//draw icon bg
						JImGuiImpl::DrawRectFilledMultiColor(preWorldCursorPos,
							iconSize,
							iconBgColor + addedColor,
							bgRectDelta,
							true);
					}
					JImGuiImpl::Image(*nowPreviewScene->GetPreviewCamera().Get(), JVector2<float>(btnIconSize, btnIconSize));

					std::wstring name;
					if (objType == J_OBJECT_TYPE::RESOURCE_OBJECT)
						name = static_cast<JResourceObject*>(nowObject.Get())->GetName();
					else if (objType == J_OBJECT_TYPE::DIRECTORY_OBJECT)
						name = static_cast<JDirectory*>(nowObject.Get())->GetName();

					JVector2<float> multilineSize = widgetAlignCal.GetInnerContentsSize();
					const bool isRenameActivaetd = renameHelper->IsActivated() && renameHelper->IsRenameTar(nowObject->GetGuid());

					textAlignCal.Update(JCUtil::WstrToU8Str(name), multilineSize, true);
					widgetAlignCal.SetNextContentsPosition();
					if (isRenameActivaetd)
					{
						renameCursorPos = ImGui::GetCursorPos();
						renameRectSize = multilineSize;
					}
					else
					{
						//draw text bg
						JImGuiImpl::DrawRectFilledMultiColor(ImGui::GetCurrentWindow()->DC.CursorPos,
							multilineSize,
							textBgColor + addedColor,
							bgRectDelta,
							true);
						JImGuiImpl::Text(textAlignCal.MiddleAligned());
					}
				}
			}

			bool canClearMultiSelected = HasMultiSelected() && isMouseClick && !isMouseInContents && !isPressedCtrl && !fileviewPopup->IsOpen();
			if (canClearMultiSelected)
			{
				ClearMultiSelected();
				auto dsEvStruct = std::make_unique<JEditorDeSelectObjectEvStruct>(GetOwnerPageType(), selectedObj->GetGuid());
				AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::DESELECT_OBJECT, JEditorEvent::RegisterEvStruct(std::move(dsEvStruct)));
			}

			JImGuiImpl::SetColorToDefault(ImGuiCol_Header);
			JImGuiImpl::SetColorToDefault(ImGuiCol_HeaderHovered);
			JImGuiImpl::SetColorToDefault(ImGuiCol_HeaderActive);
			if (hasInvaildScene)
				DestroyInvalidPreviewScene();

			if (renameHelper->IsActivated())
			{
				ImGui::SetCursorPos(renameCursorPos);
				renameHelper->UpdateMultiline(renameRectSize, false);
			}
		}
		void JWindowDirectory::ImportFile()
		{
			if (JWindow::Instance().SelectFile(importFilePath, L"please, select resource file"))
			{
				Core::JFileImportHelpData pathData{ importFilePath };
				if (opendDirctory.IsValid())
				{
					if (JResourceImporter::Instance().IsValidFormat(pathData.format))
					{
						std::filesystem::path p{ importFilePath };
						size_t fileSize = std::filesystem::file_size(p);
						if (JWindow::Instance().HasStorageSpace(opendDirctory->GetPath(), fileSize))
						{
							std::vector<JResourceObject*> res = JResourceImporter::Instance().ImportResource(opendDirctory.Get(), pathData);
							auto createImpR = std::make_unique<CreateImportedResourceF::CompletelyBind>(*createImportedResourceF,
								this,
								std::move(res));
							auto evStruct = std::make_unique<JEditorBindFuncEvStruct>(std::move(createImpR), GetOwnerPageType());
							AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::BIND_FUNC, JEditorEvent::RegisterEvStruct(std::move(evStruct)));
						}
					}
					else
						MessageBox(0, L"Is not valid format", 0, 0);
				}
			}
			importFilePath.clear();
		}
		void JWindowDirectory::OpenNewDirectory(Core::JUserPtr<JDirectory> newOpendDirectory)
		{
			if (!newOpendDirectory.IsValid())
				return;

			//window->DC.StateStorage->SetInt(id, is_open);
			ClearPreviewGroup();
			CreateDirectoryPreview(newOpendDirectory.Get(), false);

			if (opendDirctory.IsValid())
				opendDirctory->OCInterface()->CloseDirectory();
			opendDirctory = newOpendDirectory;
			opendDirctory->OCInterface()->OpenDirectory();
			lastUpdateOpenNewDir = true;
		}
		void JWindowDirectory::CreateDirectoryPreview(JDirectory* directory, const bool hasNameMask, const std::wstring& mask)
		{
			const uint existPreviewCount = GetPreviewSceneCount();
			const uint directoryCount = (uint)directory->GetChildernDirctoryCount();
			for (uint i = 0; i < directoryCount; ++i)
			{
				JDirectory* dir = directory->GetChildDirctory(i);
				if (dir == nullptr)
					continue;

				if (hasNameMask && !JCUtil::Contain(dir->GetName(), mask))
					continue;

				if (existPreviewCount > 0)
				{
					bool hasOverlap = false;
					for (uint j = 0; j < existPreviewCount; ++j)
					{
						if (GetPreviewScene(j)->GetJObject()->GetGuid() == dir->GetGuid())
						{
							hasOverlap = true;
							break;
						}
					}
					if(hasOverlap)
						continue;
				}
				CreatePreviewScene(Core::GetUserPtr(dir), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL);
			}

			const uint fileCount = (uint)directory->GetFileCount();
			for (uint i = 0; i < fileCount; ++i)
			{
				JFile* file = directory->GetFile(i);
				if (file == nullptr)
					continue;

				if (hasNameMask && !JCUtil::Contain(file->GetName(), mask))
					continue;

				if (existPreviewCount > 0)
				{
					bool hasOverlap = false;
					for (uint j = 0; j < existPreviewCount; ++j)
					{
						if (GetPreviewScene(j)->GetJObject()->GetGuid() == file->GetResource()->GetGuid())
						{
							hasOverlap = true;
							break;
						}
					}
					if (hasOverlap)
						continue;
				}
				CreatePreviewScene(Core::GetUserPtr(file->GetResource()), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL);
			}
		}
		void JWindowDirectory::CreateAllDirectoryPreview(JDirectory* directory, const bool hasNameMask, const std::wstring& mask)
		{
			const uint fileCount = (uint)directory->GetFileCount();
			for (uint i = 0; i < fileCount; ++i)
			{
				JFile* file = directory->GetFile(i);
				if (file == nullptr)
					continue;

				if (hasNameMask && !JCUtil::Contain(file->GetName(), mask))
					continue;

				CreatePreviewScene(Core::GetUserPtr(file->GetResource()), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL);
			}

			const uint childCount = directory->GetChildernDirctoryCount();
			for (uint i = 0; i < childCount; ++i)
				CreateAllDirectoryPreview(directory->GetChildDirctory(i), hasNameMask, mask);
		}
		void JWindowDirectory::RegisterCreateResourceObjectEv(J_RESOURCE_TYPE shapeType)
		{
			std::unique_ptr<CreateObjectF::CompletelyBind> createB = std::make_unique<CreateObjectF::CompletelyBind>(*createResourceF,
				Core::JUserPtr{ opendDirctory },
				std::move(shapeType));
			auto evStruct = std::make_unique<JEditorBindFuncEvStruct>(std::move(createB), GetOwnerPageType());
			AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::BIND_FUNC, JEditorEvent::RegisterEvStruct(std::move(evStruct)));
		}
		void JWindowDirectory::RegisterCreateDirectoryEv()
		{
			Core::JUserPtr<JDirectory> p;
			if (selectedObj.IsValid() && selectedObj->GetObjectType() == J_OBJECT_TYPE::DIRECTORY_OBJECT)
				p.ConnnectChildUser(selectedObj);
			else
				p = opendDirctory;

			std::unique_ptr<CreateDirectoryF::CompletelyBind> createB = std::make_unique<CreateDirectoryF::CompletelyBind>(*createDirectoryF, std::move(p));
			auto evStruct = std::make_unique<JEditorBindFuncEvStruct>(std::move(createB), GetOwnerPageType());
			AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::BIND_FUNC, JEditorEvent::RegisterEvStruct(std::move(evStruct)));
		}
		void JWindowDirectory::RegisterDestroyResourceObjectEv()
		{
			auto dsEvStruct = std::make_unique<JEditorDeSelectObjectEvStruct>(GetOwnerPageType(), selectedObj->GetGuid());
			AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::DESELECT_OBJECT, JEditorEvent::RegisterEvStruct(std::move(dsEvStruct)));

			if (HasMultiSelected())
			{
				std::unordered_map<size_t, Core::JIdentifier*>& selectedSet = GetSelectedSet();
				for (const auto& data : selectedSet)
				{
					Core::JUserPtr<JObject> user = Core::JUserPtr<JObject>::ConvertChildUser(Core::GetUserPtr(data.second));
					std::unique_ptr<DestroyObjectF::CompletelyBind> destroyB = std::make_unique<DestroyObjectF::CompletelyBind>(*destroyObjectF, std::move(user));
					auto bEvStruct = std::make_unique<JEditorBindFuncEvStruct>(std::move(destroyB), GetOwnerPageType());
					AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::BIND_FUNC, JEditorEvent::RegisterEvStruct(std::move(bEvStruct)));
				}
				ClearMultiSelected();
			}
			else if (selectedObj.IsValid())
			{		 
				std::unique_ptr<DestroyObjectF::CompletelyBind> destroyB = std::make_unique<DestroyObjectF::CompletelyBind>(*destroyObjectF, std::move(selectedObj));
				auto bEvStruct = std::make_unique<JEditorBindFuncEvStruct>(std::move(destroyB), GetOwnerPageType());
				AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::BIND_FUNC, JEditorEvent::RegisterEvStruct(std::move(bEvStruct)));
			}
		}
		void JWindowDirectory::CreateResourceObject(Core::JUserPtr<JDirectory> owner, const J_RESOURCE_TYPE rType)
		{
			if (!owner.IsValid())
				return;

			JResourceObject* resource = nullptr;
			std::string log = "";
			switch (rType)
			{
			case JinEngine::J_RESOURCE_TYPE::MATERIAL:
			{
				resource = JRFI<JMaterial>::Create(Core::JPtrUtil::MakeOwnerPtr<JMaterial::InitData>(owner.Get()));
				log = "Create Material";
				break;
			}
			case JinEngine::J_RESOURCE_TYPE::SCENE:
			{
				resource = JRFI<JScene>::Create(Core::JPtrUtil::MakeOwnerPtr<JScene::InitData>(owner.Get(), J_SCENE_USE_CASE_TYPE::MAIN));
				log = "Create Scene";
				break;
			}
			//case JinEngine::J_RESOURCE_TYPE::SCRIPT:
			//	break;  
			case JinEngine::J_RESOURCE_TYPE::ANIMATION_CONTROLLER:
			{
				resource = JRFI<JAnimationController>::Create(Core::JPtrUtil::MakeOwnerPtr<JAnimationController::InitData>(owner.Get()));
				log = "Create Animation Controller";
				break;
			}
			default:
				break;
			}
			if (resource == nullptr)
				return;

			CreatePreviewScene(Core::GetUserPtr(resource), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL);
			SetModifiedBit(Core::GetUserPtr(resource), true);
			Core::JTransition::Log(log);
		}
		void JWindowDirectory::CreateDirectory(Core::JUserPtr<JDirectory> parent)
		{
			if (!parent.IsValid())
				return;

			JDirectory* newObj = JDFI::Create(*parent.Get());
			CreatePreviewScene(Core::GetUserPtr(newObj), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL);
			SetModifiedBit(Core::GetUserPtr(newObj), true);
			Core::JTransition::Log("Create Directory");
		}
		void JWindowDirectory::DestroyObject(Core::JUserPtr<JObject> obj)
		{
			if (!obj.IsValid())
				return;

			if (obj->GetObjectType() == J_OBJECT_TYPE::RESOURCE_OBJECT)
			{
				SetRemoveBit(obj);
				DestroyPreviewScene(obj); 
				JObject::BeginDestroy(obj.Get());
				Core::JTransition::Log("Destroy Object");
			}
			else if (obj->GetObjectType() == J_OBJECT_TYPE::DIRECTORY_OBJECT)
			{
				JDirectory* parent = static_cast<JDirectory*>(obj.Get())->GetParent();
				if (parent != nullptr)
					OpenNewDirectory(GetUserPtr(parent));
				else
					OpenNewDirectory(root);

				SetRemoveBit(obj);
				DestroyPreviewScene(obj); 
				JObject::BeginDestroy(obj.Get());
				Core::JTransition::Log("Destroy Object");
			}
		}
		void JWindowDirectory::RequestMoveFile(JDirectory* to, JObject* obj)
		{	 
			if (obj->GetObjectType() == J_OBJECT_TYPE::DIRECTORY_OBJECT)
			{
				auto doBind = std::make_unique<MoveFIleF::CompletelyBind>(*moveFileF, std::move(to), std::move(obj));
				auto undoBind = std::make_unique<MoveFIleF::CompletelyBind>(*moveFileF, static_cast<JDirectory*>(obj)->GetParent(), std::move(obj));
				auto evStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorTSetBindFuncEvStruct>
					("Set Directory Parent", GetOwnerPageType(), std::move(doBind), std::move(undoBind)));
				AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::T_BIND_FUNC, evStruct);
			}
			else if (obj->GetObjectType() == J_OBJECT_TYPE::RESOURCE_OBJECT)
			{
				auto doBind = std::make_unique<MoveFIleF::CompletelyBind>(*moveFileF, std::move(to), std::move(obj));
				auto undoBind = std::make_unique<MoveFIleF::CompletelyBind>(*moveFileF, static_cast<JResourceObjectInterface*>(obj)->GetDirectory(), std::move(obj));
				auto evStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorTSetBindFuncEvStruct>
					("Set Resource Directory", GetOwnerPageType(), std::move(doBind), std::move(undoBind)));
				AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::T_BIND_FUNC, evStruct);
			}
		 
		}
		void JWindowDirectory::MoveFile(JDirectory* to, JObject* obj)
		{
			if (obj->GetObjectType() == J_OBJECT_TYPE::DIRECTORY_OBJECT)
			{
				JDirectory* dir = static_cast<JDirectory*>(obj);
				dir->EditorInterface()->SetParent(to);
				DestroyPreviewScene(Core::GetUserPtr(obj));
			}
			else if (obj->GetObjectType() == J_OBJECT_TYPE::RESOURCE_OBJECT)
			{
				JResourceObjectInterface* rObjInterface = static_cast<JResourceObject*>(obj);
				rObjInterface->MoveRFile(to);
				DestroyPreviewScene(Core::GetUserPtr(obj));
			}
		}
		void JWindowDirectory::DoSetFocus()noexcept
		{
			JEditorWindow::DoSetFocus();
		}
		void JWindowDirectory::DoSetUnFocus()noexcept
		{
			JEditorWindow::DoSetUnFocus();
			fileviewPopup->SetOpen(false);
		}
		void JWindowDirectory::DoActivate()noexcept
		{
			JEditorWindow::DoActivate();
			RegisterEventListener(J_EDITOR_EVENT::MOUSE_CLICK);
			if (!opendDirctory.IsValid())
				opendDirctory = root;
			CreateDirectoryPreview(opendDirctory.Get(), false);
		}
		void JWindowDirectory::DoDeActivate()noexcept
		{
			JEditorWindow::DoDeActivate();
			DeRegisterListener();
			ClearPreviewGroup();
			ClearMultiSelected();
		}
		void JWindowDirectory::OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)
		{
			if (senderGuid == GetGuid() || !IsActivated() || !eventStruct->PassDefectInspection())
				return;

			if (eventType == J_EDITOR_EVENT::MOUSE_CLICK)
			{
				if (eventStruct->pageType == GetOwnerPageType())
					fileviewPopup->SetOpen(false);
			}
		}
	}
}