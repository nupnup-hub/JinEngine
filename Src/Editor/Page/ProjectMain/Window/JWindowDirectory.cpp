#include"JWindowDirectory.h"
#include"../../JEditorAttribute.h"

#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../Transition/JEditorTransition.h"
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
#include"../../../../Object/Resource/Model/JModel.h"
#include"../../../../Object/Resource/JResourceObjectFactory.h"
#include"../../../../Object/Resource/Scene/Preview/JPreviewScene.h"
#include"../../../../Object/Resource/Scene/Preview/JPreviewSceneGroup.h"
#include"../../../../Object/Resource/JResourceManager.h" 
#include"../../../../Application/JApplicationVariable.h"  
#include"../../../../../Lib/imgui/imgui.h"

namespace JinEngine
{
	class JAnimationController;
	class JMaterial;
	namespace Editor
	{
		static int DirectoryLevelFinder(const std::string& path, std::wstring& copyTarget)
		{
			std::wstring copy = JCommonUtility::U8StringToWstring(path);
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
		JWindowDirectory::JWindowDirectory(std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE ownerPageType)
			:JEditorWindow(std::move(attribute), ownerPageType), 
			openNewDirByGuidFunctor(&JWindowDirectory::OpenNewDirectoryByGuid)
		{
			//JEditorString Init
			editorString = std::make_unique<JEditorString>();
			selectorIconSlidebarId = Core::MakeGuid();
			editorString->AddString(selectorIconSlidebarId, { "Icon Size" , u8"아이콘 크기 조절" });

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

			std::unique_ptr<JEditorPopupNode> destroyNode =
				std::make_unique<JEditorPopupNode>("Destroy", J_EDITOR_POPUP_NODE_TYPE::INTERNAL, fileViewPopupRootNode.get());
			editorString->AddString(destroyNode->GetNodeId(), { "Destroy" , u8"삭제" });

			std::unique_ptr<JEditorPopupNode> createDirctory =
				std::make_unique<JEditorPopupNode>("CreateDirctory", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, fileViewPopupRootNode.get());
			editorString->AddString(createDirctory->GetNodeId(), { "Create Dirctory" , u8"폴더 생성" });

			//CreatePreviewScene(static_cast<JResourceObject*>(res), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL_RESOURCE);
			auto createMatLam = [](JWindowDirectory& wind, JDirectory& dir)
			{
				wind.CreatePreviewScene(JRFI<JMaterial>::Create(dir), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL_RESOURCE);
			};
			auto createSceneLam = [](JWindowDirectory& wind, JDirectory& dir)
			{
				wind.CreatePreviewScene(JRFI<JScene>::Create(dir), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL_RESOURCE);
			};
			auto createAnimationCtrlLam = [](JWindowDirectory& wind, JDirectory& dir)
			{
				wind.CreatePreviewScene(JRFI<JAnimationController>::Create(dir), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL_RESOURCE);
			};
			auto createDirLam = [](JWindowDirectory& wind, JDirectory& dir) { JDFI::Create(dir); };

			createResourceFuncMap.emplace(createMaterialNode->GetNodeId(), CreateObjectFunctor(createMatLam));
			createResourceFuncMap.emplace(createSceneNode->GetNodeId(), CreateObjectFunctor(createSceneLam));
			createResourceFuncMap.emplace(createAnimationControllerNode->GetNodeId(), CreateObjectFunctor(createAnimationCtrlLam));
			createResourceFuncMap.emplace(createDirctory->GetNodeId(), CreateObjectFunctor(createDirLam));

			auto destroyResourceLam = [](JWindowDirectory& wind, JObject& jObj)
			{
				if (jObj.GetObjectType() == J_OBJECT_TYPE::DIRECTORY_OBJECT)
				{
					JDirectory* jDobj = static_cast<JDirectory*>(&jObj);
					wind.DestroyPreviewScene(jDobj);
					jDobj->BeginDestroy();
				}
				else if (jObj.GetObjectType() == J_OBJECT_TYPE::RESOURCE_OBJECT)
				{
					JResourceObject* jRobj = static_cast<JResourceObject*>(&jObj);
					wind.DestroyPreviewScene(jRobj);
					jRobj->BeginDestroy();
				}
			};

			destroyResourceFuncMap.emplace(destroyNode, DestroyObjectFunctor(destroyResourceLam));

			fileviewPopup = std::make_unique<JEditorPopup>("Window JDirectory FileView Popup", std::move(fileViewPopupRootNode));
			fileviewPopup->AddPopupNode(std::move(createResourceNode));
			fileviewPopup->AddPopupNode(std::move(createMaterialNode));
			fileviewPopup->AddPopupNode(std::move(createSceneNode));
			fileviewPopup->AddPopupNode(std::move(createAnimationControllerNode));
			fileviewPopup->AddPopupNode(std::move(destroyNode));
			fileviewPopup->AddPopupNode(std::move(createDirctory));

			editorPositionCal = std::make_unique<JEditorWidgetPosCalculator>();

			auto destroyRecentLam = [](JWindowDirectory& wind, JDirectory& dir)
			{
				JFile* jFile = dir.GetRecentFile();
				JResourceObject* jRobj = jFile->GetResource();
				wind.DestroyPreviewScene(jRobj);
				jRobj->BeginDestroy();
			};
			destroyRecentFunc = std::make_unique< DestroyRecentFunctor>(destroyRecentLam);

			auto oepnNewDirectoryLam = [](JWindowDirectory& windowDir, const size_t guid)
			{
				JObject* newDir = Core::JReflectionInfo::Instance().GetTypeInfo(JDirectory::TypeName())->GetInstance(guid);
				if (newDir != nullptr)
					windowDir.OpenNewDirectory(static_cast<JDirectory*>(newDir));
			};

			openNewDirectoryFunc = std::make_unique<OpenNewDirectoryFunctor>(oepnNewDirectoryLam);
		}
		JWindowDirectory::~JWindowDirectory() {}
		void JWindowDirectory::Initialize()
		{
			const std::string contentPath = JApplicationVariable::GetProjectContentPath();
			root = JResourceManager::Instance().GetDirectory(contentPath);
			OpenNewDirectory(root);
		}
		void JWindowDirectory::UpdateWindow()
		{
			JEditorWindow::UpdateWindow();
			btnIconMaxSize = JImGuiImpl::GetDisplaySize().x * selectorIconMaxRate;
			btnIconMinSize = JImGuiImpl::GetDisplaySize().x * selectorIconMinRate;
			btnIconSize = JMathHelper::Clamp<float>(btnIconSize, btnIconMinSize, btnIconMaxSize);
			BuildDirectoryView();
		}
		J_EDITOR_WINDOW_TYPE JWindowDirectory::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::WINDOW_DIRECTORY;
		}
		void JWindowDirectory::BuildDirectoryView()
		{
			ImVec2 windowSize = ImGui::GetWindowSize();
			JImGuiImpl::Text(editorString->GetString(selectorIconSlidebarId));
			ImGui::SameLine();

			ImGui::SetNextItemWidth(JImGuiImpl::GetSliderWidth());
			JImGuiImpl::SliderFloat("##WindowDirectory_SizeSlider", &btnIconSize, btnIconMinSize, btnIconMaxSize, "%.1f", ImGuiSliderFlags_AlwaysClamp);

			JImGuiImpl::BeginChildWindow(directoryViewName.c_str(),
				JVector2<float>(windowSize.x * 0.2f, windowSize.y),
				true,
				ImGuiWindowFlags_AlwaysAutoResize);

			JDirectory* preOpend = opendDirctory;
			DirectoryViewOnScreen(root);
			SetTreeNodeDefaultColor();

			if (preOpend != opendDirctory)
			{
				size_t doGuid = opendDirctory->GetGuid();
				size_t undoGuid = preOpend->GetGuid();

				auto doBinder = Core::JBindHandle(openNewDirByGuidFunctor, std::move(doGuid));
				auto undoBinder = Core::JBindHandle(openNewDirByGuidFunctor, std::move(undoGuid));

				JEditorTransition::Execute(JEditorTask{ doBinder, "Open Directory" }, JEditorTask{ undoBinder, "Cancel Open Directory" });
			}

			JImGuiImpl::EndChildWindow();
			ImGui::SameLine();
			BuildFileView();
		}
		void JWindowDirectory::DirectoryViewOnScreen(JDirectory* directory)
		{
			bool isSelected = false;
			if (opendDirctory != nullptr && opendDirctory->GetGuid() == directory->GetGuid())
			{
				isSelected = true;
				SetTreeNodeSelectColor();
			}

			if (JImGuiImpl::TreeNodeEx(directory->GetName().c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Selected))
			{
				if (isSelected)
					SetTreeNodeDefaultColor();

				if (ImGui::IsItemClicked())
					opendDirctory = directory;

				const uint childDirCount = directory->GetChildernDirctoryCount();
				for (uint i = 0; i < childDirCount; ++i)
				{
					JDirectory* child = directory->GetChildDirctory(i);
					if (child == nullptr)
						continue;
					DirectoryViewOnScreen(child);
				}
				JImGuiImpl::TreePop();
			}
		}
		void JWindowDirectory::BuildFileView()
		{
			ImVec2 windowSize = JImGuiImpl::GetWindowSize();
			JImGuiImpl::BeginChildWindow(fileViewName.c_str(),
				JVector2<float>(windowSize.x * 0.8f, windowSize.y),
				true,
				ImGuiWindowFlags_None);

			FileViewOnScreen();

			if (fileviewPopup->IsOpen())
			{
				J_EDITOR_POPUP_NODE_RES res;
				size_t clickedPopupGuid;
				fileviewPopup->ExecutePopup(editorString.get(), res, clickedPopupGuid);
				if (res == J_EDITOR_POPUP_NODE_RES::CLICK_SLELECT_NODE)
				{
					auto createFunc = createResourceFuncMap.find(clickedPopupGuid);
					if (createFunc != createResourceFuncMap.end())
					{
						//destroyRecentFunc
						auto doBinder = CreateObjectBinder(createFunc->second, *this, *opendDirctory);
						auto undoBinder = DestroyRecentBinder(*destroyRecentFunc, *this, *opendDirctory);
						JEditorTransition::Execute(JEditorTask{ doBinder, "Create Object" }, JEditorTask{ undoBinder, "Destroy Object" });
					}
					fileviewPopup->SetOpen(false);
				}
			}
			if (JImGuiImpl::IsMouseInWindow(JImGuiImpl::GetGuiWindowPos(), JImGuiImpl::GetGuiWindowSize()))
			{
				if (JImGuiImpl::IsLeftMouseClick())
					fileviewPopup->SetOpen(!fileviewPopup->IsOpen());
				else if (JImGuiImpl::IsRightMouseClick() && fileviewPopup->IsOpen() && !fileviewPopup->IsMouseInPopup())
					fileviewPopup->SetOpen(false);
			}
			ImGui::EndChild();
			if (openNewDirectoryBinder != nullptr)
			{
				openNewDirectoryBinder->InvokeCompletelyBind();
				openNewDirectoryBinder.reset();
				openNewDirectoryBinder = nullptr;
			}
		}
		void JWindowDirectory::FileViewOnScreen()
		{
			ImVec2 nowCursor = ImGui::GetCursorPos();
			ImVec2 itemSpacing = ImGui::GetStyle().ItemSpacing;
			float frameBorder = ImGui::GetStyle().FrameBorderSize;
			editorPositionCal->Update(ImGui::GetWindowWidth() * 0.75f, ImGui::GetWindowHeight(),
				btnIconSize, btnIconSize,
				nowCursor.x, nowCursor.y,
				itemSpacing.x, itemSpacing.y,
				frameBorder,
				JImGuiImpl::GetTextSize().x, JImGuiImpl::GetTextSize().y);

			//ImGuiManager::SetColor(ImVec4(1,1,1,0.1f), ImGuiCol_Header);   
			const uint count = GetPreviewSceneCount();
			for (uint i = 0; i < count; ++i)
			{
				JPreviewScene* nowPreviewScene = GetPreviewScene(i);
				if (nowPreviewScene != nullptr)
				{
					JObject* nowObject = nowPreviewScene->GetJObject();
					const J_OBJECT_FLAG flag = nowObject->GetFlag();
					if ((flag & OBJECT_FLAG_HIDDEN) > 0)
						continue;

					const J_OBJECT_TYPE objType = nowObject->GetObjectType();
					if (objType == J_OBJECT_TYPE::RESOURCE_OBJECT)
						ResourceFileViewOnScreen(nowPreviewScene, static_cast<JResourceObject*>(nowObject));
					else if (objType == J_OBJECT_TYPE::DIRECTORY_OBJECT)
						DirectoryFileViewOnScreen(nowPreviewScene, static_cast<JDirectory*>(nowObject));
					else
						continue;

					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
					{
						JImGuiImpl::SetSelectedObj(GetOwnerPageType(), nowObject);
						JImGuiImpl::SetMouseDrag(true);
						int noUse = 0;
						std::string selectResourceName = nowObject->GetName();
						JImGuiImpl::Text(selectResourceName);
						ImGui::SetDragDropPayload(selectResourceName.c_str(), &noUse, sizeof(int));
						ImGui::EndDragDropSource();
					}

					ImGui::SetCursorPos(ImVec2(editorPositionCal->GetTextPositionX(), editorPositionCal->GetTextPositionY(0)));
					int maxTextCount = editorPositionCal->GetSameLineMaxTextCount();
					int lineCount = 0;

					std::string name;
					if (objType == J_OBJECT_TYPE::RESOURCE_OBJECT)
						name = static_cast<JResourceObject*>(nowObject)->GetFullName();
					else if (objType == J_OBJECT_TYPE::DIRECTORY_OBJECT)
						name = static_cast<JDirectory*>(nowObject)->GetName();

					std::string subName;
					while (name.size() > maxTextCount)
					{
						++lineCount;
						subName = name.substr(maxTextCount, name.size());
						JImGuiImpl::Text(name.substr(0, maxTextCount));
						ImGui::SetCursorPos(ImVec2(editorPositionCal->GetTextPositionX(), editorPositionCal->GetTextPositionY(lineCount)));
						name = subName;
					}
					JImGuiImpl::Text(name);
					editorPositionCal->Next();
				}
			}
			//ImGuiManager::SetColorToDefault(ImGuiCol_Header);
		}
		void JWindowDirectory::ResourceFileViewOnScreen(JPreviewScene* nowPreviewScene, JResourceObject* jRobj)
		{
			bool isSelected = false;
			JObject* selectedObj = JImGuiImpl::GetSelectedObj(GetOwnerPageType());
			if (selectedObj != nullptr && selectedObj->GetGuid() == jRobj->GetGuid())
			{
				isSelected = true;
				SetButtonSelectColor();
			}
			ImGui::SetCursorPos(ImVec2(editorPositionCal->GetPositionX(), editorPositionCal->GetPositionY()));
			if (JImGuiImpl::ImageButton(*nowPreviewScene->GetPreviewCamera(), JVector2<float>(btnIconSize, btnIconSize)))
			{
				if (isSelected)
					SetButtonDefaultColor();

				const J_RESOURCE_TYPE resourceType = jRobj->GetResourceType();
				if (resourceType != J_RESOURCE_TYPE::NONE)
					RequestSelectObject(JEditorSelectObjectEvStruct{ *jRobj, GetOwnerPageType() });
				if (resourceType == J_RESOURCE_TYPE::SKELETON)
				{ 
					using FindMatchSkeleton = Core::JStaticCallable<bool, JResourceObject&, const size_t&>;
					auto findMatchSkeletonLam = [](JResourceObject& rObj, const size_t& skeletonAssetGuid) -> bool
					{
						if (static_cast<JModel*>(&rObj)->GetSkeletonAsset()->GetGuid() == skeletonAssetGuid)
							return true;
						else
							return false;
					};

					JSkeletonAsset* jSkeletonAsset = static_cast<JSkeletonAsset*>(jRobj);
					if (jSkeletonAsset->HasValidModelGuid())
					{
						RequestOpenPage(JEditorOpenPageEvStruct{ JModel::TypeName(), 
							jSkeletonAsset->GetOwnerModelGuid(),
							J_EDITOR_PAGE_TYPE::SKELETON_SETTING });
					}				 
				}
			}
			if (isSelected)
				SetButtonDefaultColor();
		}
		void JWindowDirectory::DirectoryFileViewOnScreen(JPreviewScene* nowPreviewScene, JDirectory* jDir)
		{ 
			ImGui::SetCursorPos(ImVec2(editorPositionCal->GetPositionX(), editorPositionCal->GetPositionY()));
			if (JImGuiImpl::ImageButton(*nowPreviewScene->GetPreviewCamera(), JVector2<float>(btnIconSize, btnIconSize)))
			{ 
				size_t dirGuid = jDir->GetGuid();
				openNewDirectoryBinder = std::make_unique<OpenNewDirectoryBinder>(this, std::move(dirGuid));
			}
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

				CreatePreviewScene(file->GetResource(), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL_RESOURCE);
			}

			if (opendDirctory != nullptr)
				opendDirctory->OCInterface()->CloseDirectory();
			opendDirctory = newOpendDirectory;
			opendDirctory->OCInterface()->OpenDirectory();
		}
		void JWindowDirectory::OpenNewDirectoryByGuid(const size_t guid)
		{
			OpenNewDirectory(JResourceManager::Instance().GetDirectory(guid));
		}
		void JWindowDirectory::DoSetFocus()noexcept
		{
			JEditor::DoSetFocus();
		}
		void JWindowDirectory::DoOffFocus()noexcept
		{
			JEditor::DoOffFocus();
			fileviewPopup->SetOpen(false);
		}
		void JWindowDirectory::DoActivate()noexcept
		{
			RegisterEventListener(J_EDITOR_EVENT::MOUSE_CLICK);
		}
		void JWindowDirectory::DoDeActivate()noexcept
		{
			DeRegisterListener();
			ClearPreviewGroup();
		}
		void JWindowDirectory::OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEventStruct* eventStruct)
		{
			if (senderGuid == GetGuid() || !IsActivated() || !eventStruct->PassDefectInspection())
				return;
			if (eventType == J_EDITOR_EVENT::MOUSE_CLICK)
				fileviewPopup->SetOpen(false);
		}
	}
}