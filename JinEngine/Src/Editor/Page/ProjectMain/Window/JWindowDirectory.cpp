#include"JWindowDirectory.h"
#include"../../JEditorPageShareData.h"
#include"../../JEditorAttribute.h"

#include"../../../Event/JEditorEvent.h"
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h" 
#include"../../../String/JEditorString.h"
#include"../../../Popup/JEditorPopupMenu.h"
#include"../../../Popup/JEditorPopupNode.h" 
#include"../../../Utility/JEditorWidgetPosCalculator.h"
#include"../../../Utility/JEditorSearchBarHelper.h"
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
		JWindowDirectory::JWindowDirectory(const std::string& name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE ownerPageType)
			:JEditorWindow(name, std::move(attribute), ownerPageType)
		{
			//JEditorString Init
			editorString = std::make_unique<JEditorString>();
			editorPositionCal = std::make_unique<JEditorWidgetPosCalculator>();
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
					{
						wndDir->CreatePreviewScene(Core::GetUserPtr(rVec[i]), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL);
						wndDir->SetModifiedBit(Core::GetUserPtr(rVec[i]), true);
					}
				}
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
				selectedObj = JEditorPageShareData::GetSelectedObj(GetOwnerPageType());
				UpdateMouseClick();
				 
				ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.7f);
				searchBarHelper->UpdateSearchBar(GetName(), false);
				const bool hasData = searchBarHelper->HasInputData();
				const bool isUpdated = searchBarHelper->IsUpdateInputData();
				if (isUpdated)
				{
					if (!hasData)
					{
						ClearPreviewGroup();
						CreateOpendDirectoryPreview(opendDirctory.Get(), false);
					}
					else
					{
						ClearPreviewGroup();
						CreateAllDirectoryPreview(root.Get(), true, JCUtil::U8StrToWstr(searchBarHelper->GetInputData()));
					}
				}
					 
				childWindowHeight = ImGui::GetWindowSize().y - ImGui::GetCursorPosY() - ImGui::GetFrameHeight() * 1.2f; 
				BuildDirectoryView();
				float preFramePaddingY = ImGui::GetStyle().FramePadding.y;
				ImGui::GetStyle().FramePadding.y = 0;

				ImGui::SetCursorPosX(JImGuiImpl::GetSliderPosX());
				ImGui::SetNextItemWidth(JImGuiImpl::GetSliderWidth());
				 
				btnIconMaxSize = JImGuiImpl::GetDisplaySize().x * selectorIconMaxRate;
				btnIconMinSize = JImGuiImpl::GetDisplaySize().x * selectorIconMinRate;
				btnIconSize = JMathHelper::Clamp<float>(btnIconSize, btnIconMinSize, btnIconMaxSize);
				JImGuiImpl::SliderFloat("##" + GetName() + "_SizeSlider", &btnIconSize, btnIconMinSize, btnIconMaxSize, "", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoInput);
				if (ImGui::IsItemActive() || ImGui::IsItemHovered())
					ImGui::SetTooltip("%.1f", btnIconSize);
				ImGui::GetStyle().FramePadding.y = preFramePaddingY;
			}
			CloseWindow();
		}
		void JWindowDirectory::BuildDirectoryView()
		{
			//editorString->GetString(selectorIconSlidebarId)
			ImVec2 windowSize = ImGui::GetWindowSize(); 
			JImGuiImpl::BeginChildWindow(Constants::directoryViewName.c_str(),
				JVector2<float>(windowSize.x * 0.2f, childWindowHeight),
				true,
				ImGuiWindowFlags_AlwaysAutoResize);

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
			ImGui::SameLine();
			BuildFileView();
		}
		void JWindowDirectory::BuildFileView()
		{
			ImVec2 windowSize = ImGui::GetWindowSize(); 
			JImGuiImpl::BeginChildWindow(Constants::fileViewName.c_str(),
				JVector2<float>(windowSize.x * 0.8f, childWindowHeight),
				true,
				ImGuiWindowFlags_AlwaysAutoResize);

			FileViewOnScreen();
			if (fileviewPopup->IsOpen())
				fileviewPopup->ExecutePopup(editorString.get());

			fileviewPopup->Update();
			JImGuiImpl::EndChildWindow();
		}
		JDirectory* JWindowDirectory::DirectoryViewOnScreen(JDirectory* directory, const bool canSelect)
		{
			ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_SpanAvailWidth |
				ImGuiTreeNodeFlags_Framed;

			bool isSelected = opendDirctory->GetGuid() == directory->GetGuid();
			if (isSelected && canSelect)
				SetTreeNodeColor(JImGuiImpl::GetTreeDeepFactor());
			JDirectory* clickedDir = nullptr;
			if (lastUpdateOpenNewDir && opendDirctory->IsParent(directory))
				ImGui::SetNextItemOpen(true);
 
			bool isNodeOpen = JImGuiImpl::TreeNodeEx(JCUtil::WstrToU8Str(directory->GetName()).c_str(), baseFlags);
			if (isSelected && canSelect)
				SetTreeNodeColor(-JImGuiImpl::GetTreeDeepFactor());
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
			editorPositionCal->Update(ImGui::GetWindowWidth() * 0.75f, ImGui::GetWindowHeight(), btnIconSize, btnIconSize);
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

					bool isSelected = false;
					if (selectedObj.IsValid() && selectedObj->GetGuid() == nowObject->GetGuid())
					{
						isSelected = true;
						SetButtonColor(JImGuiImpl::GetButtonDeepFactor());
					}

					const J_OBJECT_TYPE objType = nowObject->GetObjectType();
					if (objType == J_OBJECT_TYPE::RESOURCE_OBJECT)
						ResourceFileViewOnScreen(nowPreviewScene, static_cast<JResourceObject*>(nowObject.Get()));
					else if (objType == J_OBJECT_TYPE::DIRECTORY_OBJECT)
						DirectoryFileViewOnScreen(nowPreviewScene, static_cast<JDirectory*>(nowObject.Get()));
					else
						continue;

					if (isSelected)
						SetButtonColor(-JImGuiImpl::GetButtonDeepFactor());

					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
					{
						RequestSelectObject({ GetOwnerPageType(), nowObject });
						std::string selectResourceName = JCUtil::WstrToU8Str(nowObject->GetName());
						JImGuiImpl::Text(selectResourceName);
						ImGui::SetDragDropPayload(selectResourceName.c_str(), JEditorPageShareData::GetDragGuidPtr(GetOwnerPageType()), sizeof(int));
						ImGui::EndDragDropSource();
					}

					std::wstring name;
					if (objType == J_OBJECT_TYPE::RESOURCE_OBJECT)
						name = static_cast<JResourceObject*>(nowObject.Get())->GetFullName();
					else if (objType == J_OBJECT_TYPE::DIRECTORY_OBJECT)
						name = static_cast<JDirectory*>(nowObject.Get())->GetName();

					editorPositionCal->TitleText(name);
					editorPositionCal->Next();
				}
			}

			if (hasInvaildScene)
				DestroyInvalidPreviewScene();
			//ImGuiManager::SetColorToDefault(ImGuiCol_Header);
		}
		void JWindowDirectory::ResourceFileViewOnScreen(JPreviewScene* nowPreviewScene, JResourceObject* jRobj)
		{
			bool isSelected = false;
			if (selectedObj.IsValid() && selectedObj->GetGuid() == jRobj->GetGuid())
			{ 
				isSelected = true;
				SetButtonColor(JImGuiImpl::GetButtonDeepFactor());
			}
			ImGui::SetCursorPos(ImVec2(editorPositionCal->GetPositionX(), editorPositionCal->GetPositionY()));

			if (JImGuiImpl::ImageButton(*nowPreviewScene->GetPreviewCamera().Get(), JVector2<float>(btnIconSize, btnIconSize)))
			{
				const J_RESOURCE_TYPE resourceType = jRobj->GetResourceType();
				if (resourceType == J_RESOURCE_TYPE::SKELETON)
				{
					AddEventNotification(*JEditorEvent::EvInterface(),
						GetGuid(),
						J_EDITOR_EVENT::OPEN_PAGE,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorOpenPageEvStruct>(J_EDITOR_PAGE_TYPE::SKELETON_SETTING, Core::GetUserPtr(jRobj))));
					AddEventNotification(*JEditorEvent::EvInterface(),
						GetGuid(),
						J_EDITOR_EVENT::ACTIVATE_PAGE,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorActPageEvStruct>(J_EDITOR_PAGE_TYPE::SKELETON_SETTING)));
				}
				else
					RequestSelectObject(JEditorSelectObjectEvStruct{ GetOwnerPageType(), Core::GetUserPtr(jRobj) });
			}
			if (isSelected)
				SetButtonColor(-JImGuiImpl::GetButtonDeepFactor());
		}
		void JWindowDirectory::DirectoryFileViewOnScreen(JPreviewScene* nowPreviewScene, JDirectory* jDir)
		{
			bool isSelected = false;
			if (selectedObj.IsValid() && selectedObj->GetGuid() == jDir->GetGuid())
			{
				isSelected = true;
				SetButtonColor(JImGuiImpl::GetButtonDeepFactor());
			}
			//bool isDoubleClick = false;
			///if (ImGui::GetMouseClickedCount(0) >= 2)
			//	isDoubleClick = true;
			 
			ImGui::SetCursorPos(ImVec2(editorPositionCal->GetPositionX(), editorPositionCal->GetPositionY()));
			ImVec2 preCursor = ImGui::GetCursorPos();
			const std::string unqName ="##"+ JCUtil::WstrToU8Str(nowPreviewScene->GetJObject()->GetName()) + GetName() + "_Selectable";
			if (JImGuiImpl::Selectable(unqName, false, ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_AllowItemOverlap, JVector2<float>(btnIconSize, btnIconSize)))
			{
				if (ImGui::GetMouseClickedCount(0) >= 2)
					openNewDirB = std::make_unique<OpenNewDirectoryF::CompletelyBind>(*openNewDirF, Core::GetUserPtr(jDir));
			}
			ImGui::SetCursorPos(preCursor);
			JImGuiImpl::Image(*nowPreviewScene->GetPreviewCamera().Get(), JVector2<float>(btnIconSize, btnIconSize));
			if (isSelected)
				SetButtonColor(-JImGuiImpl::GetButtonDeepFactor());
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
			CreateOpendDirectoryPreview(newOpendDirectory.Get(), false);
			
			if (opendDirctory.IsValid())
				opendDirctory->OCInterface()->CloseDirectory();
			opendDirctory = newOpendDirectory;
			opendDirctory->OCInterface()->OpenDirectory();
			lastUpdateOpenNewDir = true;
		}
		void JWindowDirectory::CreateOpendDirectoryPreview(JDirectory* directory, const bool hasNameMask, const std::wstring& mask)
		{
			const uint directoryCount = (uint)directory->GetChildernDirctoryCount();
			for (uint i = 0; i < directoryCount; ++i)
			{
				JDirectory* dir = directory->GetChildDirctory(i);
				if (dir == nullptr)
					continue;

				if (hasNameMask && !JCUtil::Contain(dir->GetName(), mask))
					continue;

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
				Core::JUserPtr{ opendDirctory},
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
			if (selectedObj.IsValid())
			{
				std::unique_ptr<DestroyObjectF::CompletelyBind> destroyB = std::make_unique<DestroyObjectF::CompletelyBind>(*destroyObjectF, std::move(selectedObj));
				auto evStruct = std::make_unique<JEditorBindFuncEvStruct>(std::move(destroyB), GetOwnerPageType());
				AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::BIND_FUNC, JEditorEvent::RegisterEvStruct(std::move(evStruct)));
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
				RemoveModifiedInfo(obj);
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

				RemoveModifiedInfo(obj);
				DestroyPreviewScene(obj);
				JObject::BeginDestroy(obj.Get());
				Core::JTransition::Log("Destroy Object");
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
		}
		void JWindowDirectory::DoDeActivate()noexcept
		{
			JEditorWindow::DoDeActivate();
			DeRegisterListener();
			ClearPreviewGroup(); 
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