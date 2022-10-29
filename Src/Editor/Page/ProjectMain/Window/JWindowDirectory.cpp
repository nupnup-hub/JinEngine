#include"JWindowDirectory.h"
#include"../../JEditorPageShareData.h"
#include"../../JEditorAttribute.h"

#include"../../../Event/JEditorEvent.h"
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h" 
#include"../../../String/JEditorString.h"
#include"../../../Popup/JEditorPopup.h"
#include"../../../Popup/JEditorPopupNode.h" 
#include"../../../Utility/JEditorWidgetPosCalculator.h"
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

namespace JinEngine
{
	class JAnimationController;
	class JMaterial;
	namespace Editor
	{
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
			selectorIconSlidebarId = Core::MakeGuid();
			editorString->AddString(selectorIconSlidebarId, { "Icon Size" , u8"아이콘 크기" });

			//Popup Init
			std::unique_ptr<JEditorPopupNode> fileViewPopupRootNode =
				std::make_unique<JEditorPopupNode>("Window JDirectory FileView Popup Root", J_EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

			std::unique_ptr<JEditorPopupNode> createResourceNode =
				std::make_unique<JEditorPopupNode>("CreateResource", J_EDITOR_POPUP_NODE_TYPE::INTERNAL, fileViewPopupRootNode.get());
			editorString->AddString(createResourceNode->GetNodeId(), { "Create Resource" , u8"자원 생성" });

			std::unique_ptr<JEditorPopupNode> createMaterialNode =
				std::make_unique<JEditorPopupNode>("JMaterial", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, createResourceNode.get());
			editorString->AddString(createMaterialNode->GetNodeId(), { "JMaterial" , u8"머테리얼" });

			std::unique_ptr<JEditorPopupNode> createSceneNode =
				std::make_unique<JEditorPopupNode>("JScene", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, createResourceNode.get());
			editorString->AddString(createSceneNode->GetNodeId(), { "JScene" , u8"씬" });

			std::unique_ptr<JEditorPopupNode> createAnimationControllerNode =
				std::make_unique<JEditorPopupNode>("AnimationContorller", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, createResourceNode.get());
			editorString->AddString(createAnimationControllerNode->GetNodeId(), { "AnimationContorller" , u8"애니메이션 컨트롤러" });

			std::unique_ptr<JEditorPopupNode> createDirctoryNode =
				std::make_unique<JEditorPopupNode>("CreateDirctory", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, fileViewPopupRootNode.get());
			editorString->AddString(createDirctoryNode->GetNodeId(), { "Create Dirctory" , u8"폴더 생성" });

			std::unique_ptr<JEditorPopupNode> destroyNode =
				std::make_unique<JEditorPopupNode>("Destroy", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, fileViewPopupRootNode.get());
			editorString->AddString(destroyNode->GetNodeId(), { "Destroy" , u8"삭제" });

			std::unique_ptr<JEditorPopupNode> importNode =
				std::make_unique<JEditorPopupNode>("ImportResource", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, fileViewPopupRootNode.get());
			editorString->AddString(importNode->GetNodeId(), { "Import Resource" , u8"자원 불러오기" });

			//CreatePreviewScene(static_cast<JResourceObject*>(res), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL_RESOURCE);
			auto createMatLam = [](JWindowDirectory& wind, Core::JUserPtr<JDirectory> dir) -> void
			{
				if (!dir.IsValid())
					return;
				JObject* newObj = JRFI<JMaterial>::Create(Core::JPtrUtil::MakeOwnerPtr<JMaterial::InitData>(dir.Get()));
				wind.CreatePreviewScene(Core::GetUserPtr<JMaterial>(newObj), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL_RESOURCE);
				Core::JTransition::Log("Create Material");
			};
			auto createSceneLam = [](JWindowDirectory& wind, Core::JUserPtr<JDirectory> dir) -> void
			{
				if (!dir.IsValid())
					return;
				JObject* newObj = JRFI<JScene>::Create(Core::JPtrUtil::MakeOwnerPtr<JScene::InitData>(dir.Get()));
				wind.CreatePreviewScene(Core::GetUserPtr<JScene>(newObj), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL_RESOURCE);
				Core::JTransition::Log("Create Scene");
			};
			auto createAnimationCtrlLam = [](JWindowDirectory& wind, Core::JUserPtr<JDirectory> dir) -> void
			{
				if (!dir.IsValid())
					return;
				JObject* newObj = JRFI<JAnimationController>::Create(Core::JPtrUtil::MakeOwnerPtr<JAnimationController::InitData>(dir.Get()));
				wind.CreatePreviewScene(Core::GetUserPtr<JAnimationController>(newObj), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL_RESOURCE);
				Core::JTransition::Log("Create Animation Controller");
			};
			auto createDirLam = [](JWindowDirectory& wind, Core::JUserPtr<JDirectory> dir) { JDFI::Create(*dir.Get()); 	Core::JTransition::Log("Create Directory"); };

			createResourceFuncMap.emplace(createMaterialNode->GetNodeId(), std::make_unique<CreateObjectF>(createMatLam));
			createResourceFuncMap.emplace(createSceneNode->GetNodeId(), std::make_unique<CreateObjectF>(createSceneLam));
			createResourceFuncMap.emplace(createAnimationControllerNode->GetNodeId(), std::make_unique<CreateObjectF>(createAnimationCtrlLam));
			createResourceFuncMap.emplace(createDirctoryNode->GetNodeId(), std::make_unique<CreateObjectF>(createDirLam));

			openNewDirFunctor = std::make_unique<OpenNewDirectoryF>(createDirLam);

			auto destroyResourceLam = [](JWindowDirectory& wind, Core::JUserPtr<JObject> jObj)
			{
				if (!jObj.IsValid())
					return;

				if (jObj->GetObjectType() == J_OBJECT_TYPE::RESOURCE_OBJECT)
				{ 
					wind.DestroyPreviewScene(jObj);
					jObj->BeginDestroy();
					Core::JTransition::Log("Destroy Object");
				}
				else if (jObj->GetObjectType() == J_OBJECT_TYPE::DIRECTORY_OBJECT)
				{
					JDirectory* parent = static_cast<JDirectory*>(jObj.Get())->GetParent();
					if (parent != nullptr)
						wind.OpenNewDirectory(parent);
					else
						wind.OpenNewDirectory(wind.root.Get());

					wind.DestroyPreviewScene(jObj);
					jObj->BeginDestroy();
					Core::JTransition::Log("Destroy Object");
				}
			};
			destroyResourceFuncMap.emplace(destroyNode->GetNodeId(), std::make_unique<DestroyObjectF>(destroyResourceLam));

			auto importResourceLam = [](JWindowDirectory& wind)
			{
				wind.actImport = !wind.actImport;
			};
			importResourceT = std::tuple(importNode->GetNodeId(), std::make_unique<ImportResourceF>(importResourceLam));

			fileviewPopup = std::make_unique<JEditorPopup>("Window JDirectory FileView Popup", std::move(fileViewPopupRootNode));
			fileviewPopup->AddPopupNode(std::move(createResourceNode));
			fileviewPopup->AddPopupNode(std::move(createMaterialNode));
			fileviewPopup->AddPopupNode(std::move(createSceneNode));
			fileviewPopup->AddPopupNode(std::move(createAnimationControllerNode));
			fileviewPopup->AddPopupNode(std::move(destroyNode));
			fileviewPopup->AddPopupNode(std::move(createDirctoryNode));
			fileviewPopup->AddPopupNode(std::move(importNode));

			editorPositionCal = std::make_unique<JEditorWidgetPosCalculator>();

			auto oepnNewDirectoryLam = [](JWindowDirectory& windowDir, Core::JUserPtr<JDirectory> newDir)
			{
				if (newDir.IsValid())
					windowDir.OpenNewDirectory(newDir.Get());
			};
		}
		JWindowDirectory::~JWindowDirectory() {}
		J_EDITOR_WINDOW_TYPE JWindowDirectory::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::WINDOW_DIRECTORY;
		}
		void JWindowDirectory::Initialize()
		{
			const std::wstring contentPath = JApplicationVariable::GetProjectContentPath();
			root = Core::GetUserPtr(JResourceManager::Instance().GetDirectory(contentPath));
			OpenNewDirectory(root.Get());
		}
		void JWindowDirectory::UpdateWindow()
		{
			EnterWindow(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
			UpdateDocking();
			if (IsActivated() && opendDirctory.IsValid())
			{
				UpdateMouseClick();
				btnIconMaxSize = JImGuiImpl::GetDisplaySize().x * selectorIconMaxRate;
				btnIconMinSize = JImGuiImpl::GetDisplaySize().x * selectorIconMinRate;
				btnIconSize = JMathHelper::Clamp<float>(btnIconSize, btnIconMinSize, btnIconMaxSize);

				float preFramePaddingY = ImGui::GetStyle().FramePadding.y;
				ImGui::GetStyle().FramePadding.y = 0;
			  
				ImGui::SetCursorPosX(JImGuiImpl::GetSliderPosX());
				ImGui::SetNextItemWidth(JImGuiImpl::GetSliderWidth());
				 
				JImGuiImpl::SliderFloat("##WindowDirectory_SizeSlider", &btnIconSize, btnIconMinSize, btnIconMaxSize, "", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoInput);
				if (ImGui::IsItemActive() || ImGui::IsItemHovered())
					ImGui::SetTooltip("%.1f", btnIconSize);

				ImGui::GetStyle().FramePadding.y = preFramePaddingY;

				BuildDirectoryView();
				if (actImport)
					ImportFile();
			}
			CloseWindow();
		}
		void JWindowDirectory::BuildDirectoryView()
		{
			//editorString->GetString(selectorIconSlidebarId)
			ImVec2 windowSize = ImGui::GetWindowSize();

			JImGuiImpl::BeginChildWindow(directoryViewName.c_str(),
				JVector2<float>(windowSize.x * 0.2f, 0),
				true,
				ImGuiWindowFlags_AlwaysAutoResize);

			JDirectory* clickedDir = DirectoryViewOnScreen(root.Get());
			SetTreeNodeDefaultColor();

			if (clickedDir != nullptr && clickedDir->GetGuid() != opendDirctory->GetGuid())
			{
				OpenNewDirectory(clickedDir);
				Core::JTransition::Log(JCUtil::WstrToU8Str(opendDirctory->GetName() + L" Open Directory"));
			} 
			JImGuiImpl::EndChildWindow();
			ImGui::SameLine();
			BuildFileView();
		}
		void JWindowDirectory::BuildFileView()
		{
			ImVec2 windowSize = JImGuiImpl::GetWindowSize();
			JImGuiImpl::BeginChildWindow(fileViewName.c_str(),
				JVector2<float>(windowSize.x * 0.8f, 0),
				true,
				ImGuiWindowFlags_None);

			FileViewOnScreen();
			if (openNewDirBinder != nullptr)
			{
				(*openNewDirBinder)();
				openNewDirBinder.reset();
			}

			if (fileviewPopup->IsOpen())
			{
				J_EDITOR_POPUP_NODE_RES res;
				size_t clickedPopupGuid = 0;
				fileviewPopup->ExecutePopup(editorString.get(), res, clickedPopupGuid);
				if (res == J_EDITOR_POPUP_NODE_RES::CLICK_SLELECT_NODE)
				{
					auto createFunc = createResourceFuncMap.find(clickedPopupGuid);
					if (createFunc != createResourceFuncMap.end())
					{
						std::unique_ptr<CreateObjectB> createB = std::make_unique<CreateObjectB>(*createFunc->second, *this, Core::JUserPtr<JDirectory>(opendDirctory));
						auto evStruct = std::make_unique<JEditorBindFuncEvStruct>(std::move(createB), GetOwnerPageType());
						AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::BIND_FUNC, JEditorEvent::RegisterEvStruct(std::move(evStruct)));
					}
					auto destroyFunc = destroyResourceFuncMap.find(clickedPopupGuid);
					if (destroyFunc != destroyResourceFuncMap.end())
					{
						Core::JUserPtr<JObject> selectedObj = JEditorPageShareData::GetSelectedObj(GetOwnerPageType());
						if (selectedObj.IsValid())
						{
							std::unique_ptr<DestroyObjectB> destroyB = std::make_unique<DestroyObjectB>(*destroyFunc->second, *this, std::move(selectedObj));;
							auto evStruct = std::make_unique<JEditorBindFuncEvStruct>(std::move(destroyB), GetOwnerPageType());
							AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::BIND_FUNC, JEditorEvent::RegisterEvStruct(std::move(evStruct)));
						}
					}
					if (clickedPopupGuid == std::get<0>(importResourceT))
						std::get<1>(importResourceT)->Invoke(*this);
					fileviewPopup->SetOpen(false);
				}
			}

			fileviewPopup->Update(); 
			ImGui::EndChild();
		}
		JDirectory* JWindowDirectory::DirectoryViewOnScreen(JDirectory* directory)
		{
			ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Framed;
			bool isSelected = opendDirctory->GetGuid() == directory->GetGuid();
			if (isSelected)
				SetTreeNodeSelectColor();

			JDirectory* clickedDir = nullptr;
			if (JImGuiImpl::TreeNodeEx(JCUtil::WstrToU8Str(directory->GetName()).c_str(), baseFlags))
			{
				if (isSelected)
					SetTreeNodeDefaultColor();

				if (ImGui::IsItemClicked())
					clickedDir = directory;

				const uint childDirCount = directory->GetChildernDirctoryCount();
				for (uint i = 0; i < childDirCount; ++i)
				{
					JDirectory* child = directory->GetChildDirctory(i);
					if (child == nullptr)
						continue;
					JDirectory* res = DirectoryViewOnScreen(child);
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
			Core::JUserPtr<JObject> selectedObj = JEditorPageShareData::GetSelectedObj(GetOwnerPageType());

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
						SetButtonSelectColor();
					}

					const J_OBJECT_TYPE objType = nowObject->GetObjectType();
					if (objType == J_OBJECT_TYPE::RESOURCE_OBJECT)
						ResourceFileViewOnScreen(nowPreviewScene, static_cast<JResourceObject*>(nowObject.Get()));
					else if (objType == J_OBJECT_TYPE::DIRECTORY_OBJECT)
						DirectoryFileViewOnScreen(nowPreviewScene, static_cast<JDirectory*>(nowObject.Get()));
					else
						continue;

					if (isSelected)
						SetButtonDefaultColor();

					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
					{
						RequestSelectObject({ GetOwnerPageType(), nowObject });
						JImGuiImpl::SetMouseDrag(true);
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
			Core::JUserPtr<JObject> selectedObj = JEditorPageShareData::GetSelectedObj(GetOwnerPageType());
			if (selectedObj.IsValid() && selectedObj->GetGuid() == jRobj->GetGuid())
			{
				isSelected = true;
				SetButtonSelectColor();
			}
			ImGui::SetCursorPos(ImVec2(editorPositionCal->GetPositionX(), editorPositionCal->GetPositionY()));

			if (JImGuiImpl::ImageButton(*nowPreviewScene->GetPreviewCamera().Get(), JVector2<float>(btnIconSize, btnIconSize)))
			{
				const J_RESOURCE_TYPE resourceType = jRobj->GetResourceType();
				if (resourceType == J_RESOURCE_TYPE::SKELETON)
					RequestOpenPage(JEditorOpenPageEvStruct{ J_EDITOR_PAGE_TYPE::SKELETON_SETTING, Core::GetUserPtr(jRobj) });
				else
					RequestSelectObject(JEditorSelectObjectEvStruct{ GetOwnerPageType(), Core::GetUserPtr(jRobj) });
			}
			if (isSelected)
				SetButtonDefaultColor();
		}
		void JWindowDirectory::DirectoryFileViewOnScreen(JPreviewScene* nowPreviewScene, JDirectory* jDir)
		{
			bool isSelected = false;
			Core::JUserPtr<JObject> selectedObj = JEditorPageShareData::GetSelectedObj(GetOwnerPageType());
			if (selectedObj->GetGuid() == jDir->GetGuid())
			{
				isSelected = true;
				SetButtonSelectColor();
			}

			ImGui::SetCursorPos(ImVec2(editorPositionCal->GetPositionX(), editorPositionCal->GetPositionY()));
			if (JImGuiImpl::ImageButton(*nowPreviewScene->GetPreviewCamera().Get(), JVector2<float>(btnIconSize, btnIconSize)))
			{
				openNewDirBinder = std::make_unique<OpenNewDirectoryB>(*openNewDirFunctor, *this, Core::GetUserPtr(jDir));
			}
			if (isSelected)
				SetButtonDefaultColor();
		}
		void JWindowDirectory::ImportFile()
		{
			if (JWindow::Instance().SelectFile(importFilePath, L"please, select resource file"))
			{
				Core::JFileImportPathData pathData{ importFilePath };
				if (opendDirctory.IsValid())
				{
					if (JResourceImporter::Instance().IsValidFormat(pathData.format))
					{
						std::filesystem::path p{ importFilePath };
						size_t fileSize = std::filesystem::file_size(p);
						if (JWindow::Instance().HasStorageSpace(opendDirctory->GetPath(), fileSize))
						{
							std::vector<JResourceObject*> res = JResourceImporter::Instance().ImportResource(opendDirctory.Get(), pathData);
							const uint resCount = (uint)res.size();
							for (uint i = 0; i < resCount; ++i)
							{
								if (res[i] != nullptr)
									CreatePreviewScene(Core::GetUserPtr(res[i]), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL_RESOURCE);
							}
						}
					}
					else
						MessageBox(0, L"Is not valid format", 0, 0);
				}
			}
			actImport = false;
		}
		void JWindowDirectory::OpenNewDirectory(JDirectory* newOpendDirectory)
		{
			if (newOpendDirectory == nullptr)
				return;

			const uint fileCount = (uint)newOpendDirectory->GetFileCount();
			ClearPreviewGroup();

			for (uint i = 0; i < fileCount; ++i)
			{
				JFile* file = newOpendDirectory->GetFile(i);
				if (file == nullptr)
					continue;

				CreatePreviewScene(Core::GetUserPtr(file->GetResource()), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL_RESOURCE);
			}

			if (opendDirctory.IsValid())
				opendDirctory->OCInterface()->CloseDirectory();
			opendDirctory = Core::GetUserPtr(newOpendDirectory);
			opendDirctory->OCInterface()->OpenDirectory();
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
			actImport = false;
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