#include"JWindowDirectory.h"
#include"../../JEditorPageShareData.h"
#include"../../JEditorAttribute.h"
#include"../../../Gui/JGui.h" 
#include"../../../Gui/JGuiImageInfo.h"
#include"../../../Event/JEditorEvent.h"
#include"../../../String/JEditorStringMap.h"
#include"../../../Popup/JEditorPopupMenu.h"
#include"../../../Popup/JEditorPopupNode.h"  
#include"../../../EditTool/JEditorSearchBar.h"
#include"../../../EditTool/JEditorAddressBar.h"
#include"../../../EditTool/JEditorPageCounter.h"
#include"../../../EditTool/JEditorRenameHelper.h"
#include"../../../EditTool/JEditorTreeStructure.h"
#include"../../../EditTool/JEditorSelectableStructure.h"
#include"../../../EditTool/JEditorMouseDragBox.h"
#include"../../../EditTool/JEditorSliderBar.h"
#include"../../../Align/JEditorAlignCalculator.h"
#include"../../../Interface/JEditorObjectCreationInterface.h"
#include"../../../../Core/Math/JMathHelper.h"
#include"../../../../Core/Math/JVectorExtend.h"
#include"../../../../Core/Guid/JGuidCreator.h" 
#include"../../../../Core/Identity/JIdenCreator.h" 
#include"../../../../Core/Utility/JCommonUtility.h"
#include"../../../../Core/Log/JLogMacro.h"
#include"../../../../Object/JObjectFileIOHelper.h" 
#include"../../../../Object/Component/Camera/JCamera.h" 
#include"../../../../Object/Directory/JDirectory.h"
#include"../../../../Object/Directory/JDirectoryPrivate.h"
#include"../../../../Object/Directory/JFile.h"
#include"../../../../Object/Resource/Scene/JScene.h"
#include"../../../../Object/Resource/AnimationController/JAnimationController.h"
#include"../../../../Object/Resource/Material/JMaterial.h" 
#include"../../../../Object/Resource/Mesh/JMeshGeometry.h" 
#include"../../../../Object/Resource/Scene/Preview/JPreviewScene.h" 
#include"../../../../Object/Resource/Texture/JTexture.h" 
#include"../../../../Object/Resource/JResourceManager.h" 
#include"../../../../Object/Resource/JResourceObjectPrivate.h" 
#include"../../../../Object/Resource/JResourceObjectImporter.h" 
#include"../../../../Window/JWindow.h"
#include"../../../../Application/JApplicationProject.h"   
#include<filesystem>

// #include "../../../../Graphic/GraphicResource/JGraphicResourceInfo.h"

#define DIRECTORY_VIEW_NAME "DirectoryView"
#define FILE_VIEW_NAME "FileView"
#define NEW_DIRECTORY_NAME "New Directory"

namespace JinEngine
{
	class JAnimationController;
	class JMaterial;
	namespace Editor
	{
		namespace
		{
			using DirActInterface = JDirectoryPrivate::ActivationInterface;
			using DirRawInterface = JDirectoryPrivate::RawDirectoryInterface;
			using ResourceFileInterface = JResourceObjectPrivate::FileInterface;
		}
		namespace Private
		{
			static constexpr float selectorIconMaxRate = 0.075f;
			static constexpr float selectorIconMinRate = 0.02f;
			static constexpr float selectorLabelRate = 0.03f;
			static constexpr uint pagePerFile = 50;
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

		class JWindowDirectoryCreationFunctor
		{
		private:
			using ResourceObjectCreationInterface = JEditorCreationRequestor<JEditorObjectCreateInterface<J_RESOURCE_TYPE>>;
			using DirectoryCreationInteface = JEditorCreationRequestor<JEditorObjectCreateInterface<JUserPtr<JDirectory>>>;
			using DestructionInterface = JEditorDestructionRequestor;
		public:
			using ResourceObjectCanCreateF = ResourceObjectCreationInterface::CreateInteface::CanCreateF;
			using ResourceObjectCreateF = ResourceObjectCreationInterface::CreateInteface::ObjectCreateF;
			using DirectoryCanCreateF = DirectoryCreationInteface::CreateInteface::CanCreateF;
			using DirectoryCreateF = DirectoryCreationInteface::CreateInteface::ObjectCreateF;
		public:
			using DataHandleStructure = ResourceObjectCreationInterface::DataHandleStructure;
			using NotifyPtr = ResourceObjectCreationInterface::NotifyPtr;
		public:
			DataHandleStructure dS;
		public:
			ResourceObjectCreationInterface resourceObj;
			DirectoryCreationInteface directory;
			DestructionInterface destructuion;
		public:
			using RequestRObjCreationEvF = Core::JSFunctorType<void, JWindowDirectory*, J_RESOURCE_TYPE>;
			using RequestDirectoryCreationEvF = Core::JSFunctorType<void, JWindowDirectory*>;
			using RequestDestructionEvF = Core::JSFunctorType<void, JWindowDirectory*>;
		public:
			std::unique_ptr<RequestRObjCreationEvF::Functor> reqRObjCreationEvF;
			std::unique_ptr<RequestDirectoryCreationEvF::Functor> reqDirCreationEvF;
			std::unique_ptr<RequestDestructionEvF::Functor> reqDestructionEvF;
		public:
			JWindowDirectoryCreationFunctor(RequestRObjCreationEvF::Ptr reqRObjCreationEvPtr,
				RequestDirectoryCreationEvF::Ptr reqDirCreationEvPtr,
				RequestDestructionEvF::Ptr reqDestructionEvPtr)
			{
				reqRObjCreationEvF = std::make_unique<RequestRObjCreationEvF::Functor>(reqRObjCreationEvPtr);
				reqDirCreationEvF = std::make_unique<RequestDirectoryCreationEvF::Functor>(reqDirCreationEvPtr);
				reqDestructionEvF = std::make_unique<RequestDestructionEvF::Functor>(reqDestructionEvPtr);
			}
			~JWindowDirectoryCreationFunctor()
			{
				dS.Clear();
			}
		};
		class JWindowDirectorySettingFunctor
		{
		public:
			using RenameF = Core::JSFunctorType<void, JWindowDirectory*>;
			using SwitchImportResourceF = Core::JMFunctorType<JWindowDirectory, void>;
			using ImportPostProcessF = Core::JSFunctorType<void, JWindowDirectory*, std::vector<JUserPtr<JResourceObject>>>;
			using MoveFIleF = Core::JMFunctorType<JWindowDirectory, void, JUserPtr<JDirectory>, JUserPtr<JObject>>;
			using OpenNewDirectoryF = Core::JMFunctorType<JWindowDirectory, void, JUserPtr<JDirectory>>;
		public:
			std::unique_ptr<RenameF::Functor> renameF;
			std::unique_ptr<SwitchImportResourceF::Functor> swtichImportResourceF;
			std::unique_ptr<ImportPostProcessF::Functor> importPostProcessF;
			std::unique_ptr<MoveFIleF::Functor> moveFileF;
			std::unique_ptr<OpenNewDirectoryF::Functor> openNewDirF;
		};

		JWindowDirectory::JWindowDirectory(const std::string& name,
			std::unique_ptr<JEditorAttribute> attribute,
			const J_EDITOR_PAGE_TYPE ownerPageType,
			const J_EDITOR_WINDOW_FLAG windowFlag)
			:JEditorWindow(name, std::move(attribute), ownerPageType, windowFlag)
		{
			//JEditorStringMap Init
			editorString = std::make_unique<JEditorStringMap>();
			renameHelper = std::make_unique<JEditorRenameHelper>();
			searchBar = std::make_unique<JEditorSearchBar>(false);
			addressBar = std::make_unique<JEditorAddressBar>(_JResourceManager::Instance().GetProjectContentsDirectory());
			pageCounter = std::make_unique< JEditorPageCounter>();
			treeStrcture = std::make_unique<JEditorTreeStructure>();
			selectableStructure = std::make_unique<JEditorSelectableStructure>();
			mouseBBox = std::make_unique<JEditorMouseDragCanvas>();

			selectorIconSlidebarId = Core::MakeGuid();
			editorString->AddString(selectorIconSlidebarId, { "Icon Size" , u8"아이콘 크기" });

			pageCounter->SetPagePerCount(Private::pagePerFile);
			InitializeCreationImpl();
			InitializeSettingImpl();
			BuildPopup();
		}
		JWindowDirectory::~JWindowDirectory()
		{
			creation.reset();
			setting.reset();
		}
		void JWindowDirectory::InitializeCreationImpl()
		{
			if (creation != nullptr)
				return;

			auto requestCreateRObjLam = [](JWindowDirectory* wndDir, J_RESOURCE_TYPE resourceType)
			{
				if (!wndDir->root.IsValid())
					return;

				JUserPtr<JDirectory> owner = wndDir->addressBar->GetOpendDirectory();
				if (!owner.IsValid())
					owner = wndDir->root;

				JEditorCreationHint creationHint = JEditorCreationHint(wndDir,
					false, false, false, false,
					Core::JTypeInstanceSearchHint(),
					Core::JTypeInstanceSearchHint(owner),
					&JEditorWindow::NotifyEvent);
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification);

				JWindowDirectoryCreationFunctor* impl = wndDir->creation.get();
				impl->resourceObj.RequestCreateObject(impl->dS, false, creationHint, Core::MakeGuid(), requestHint, std::move(resourceType));
			};
			auto requestCreateDirectoryLam = [](JWindowDirectory* wndDir)
			{
				if (!wndDir->root.IsValid())
					return;

				JUserPtr<JDirectory> parent = wndDir->addressBar->GetOpendDirectory();
				if (!parent.IsValid())
					parent = wndDir->root;

				JEditorCreationHint creationHint = JEditorCreationHint(wndDir,
					false, false, false, false,
					Core::JTypeInstanceSearchHint(),
					Core::JTypeInstanceSearchHint(),
					&JEditorWindow::NotifyEvent);
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification);

				JWindowDirectoryCreationFunctor* impl = wndDir->creation.get();
				impl->directory.RequestCreateObject(impl->dS, false, creationHint, Core::MakeGuid(), requestHint, std::move(parent));
			};
			auto requestDestroyLam = [](JWindowDirectory* wndDir)
			{
				std::vector<JUserPtr<Core::JIdentifier>> objVec = wndDir->GetSelectedObjectVec();
				if (objVec.size() == 0)
					return;

				JEditorCreationHint creationHint = JEditorCreationHint(wndDir,
					false, false, false, false,
					Core::JTypeInstanceSearchHint(),
					Core::JTypeInstanceSearchHint(),
					&JEditorWindow::NotifyEvent);
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification);

				JWindowDirectoryCreationFunctor* impl = wndDir->creation.get();
				impl->destructuion.RequestDestroyObject(impl->dS, false, creationHint, objVec, requestHint);
			};
			auto creationRObjPostProccessLam = [](JEditorWindow* wndDir, Core::JIdentifier* resource)
			{
				static_cast<JWindowDirectory*>(wndDir)->PopPreviewScene();
				static_cast<JWindowDirectory*>(wndDir)->CreateDirectoryPreview();
				//static_cast<JWindowDirectory*>(wndDir)->CreatePreviewScene(JUserPtr<JObject>::ConvertChild(Core::GetUserPtr(resource)));
			};
			auto creationDirPostProccessLam = [](JEditorWindow* wndDir, Core::JIdentifier* dir)
			{
				static_cast<JWindowDirectory*>(wndDir)->PopPreviewScene();
				static_cast<JWindowDirectory*>(wndDir)->CreateDirectoryPreview();
				//static_cast<JWindowDirectory*>(wndDir)->CreatePreviewScene(JUserPtr<JObject>::ConvertChild(Core::GetUserPtr(dir)));
			};
			auto destructionPreProccessLam = [](JEditorWindow* wndDir, Core::JIdentifier* resource)
			{
				static_cast<JWindowDirectory*>(wndDir)->DestroyPreviewScene(JUserPtr<JObject>::ConvertChild(Core::GetUserPtr(resource)));
				if (resource->GetTypeInfo().IsChildOf<JResourceObject>())
					ResourceFileInterface::DeleteFile(static_cast<JResourceObject*>(resource));
				else if (resource->GetTypeInfo().IsChildOf<JDirectory>())
					DirRawInterface::DeleteDirectory(Core::GetUserPtr<JDirectory>(resource));
			};

			creation = std::make_unique<JWindowDirectoryCreationFunctor>(requestCreateRObjLam, requestCreateDirectoryLam, requestDestroyLam);
			creation->resourceObj.GetCreationInterface()->RegisterCreatePostProcess(creationRObjPostProccessLam);
			creation->directory.GetCreationInterface()->RegisterCreatePostProcess(creationDirPostProccessLam);
			creation->destructuion.GetDestruectionInterface()->RegisterDestroyPreProcess(destructionPreProccessLam);
			 
			auto canCreationRobjLam = [](const size_t guid, const JEditorCreationHint& creationHint, J_RESOURCE_TYPE rType)
			{
				auto ownerPtr = Core::GetRawPtr(creationHint.ownerDataHint);
				if (ownerPtr == nullptr)
				{
					return false;
				}
				if (ownerPtr->GetTypeInfo().IsChildOf<JDirectory>())
					return true;
				else
					return false;
			};
			auto creationRobjLam = [](const size_t guid, const JEditorCreationHint& creationHint, J_RESOURCE_TYPE rType)
			{
				JUserPtr<JDirectory> ownerDir = Core::ConvertChildUserPtr<JDirectory>(Core::GetUserPtr(creationHint.ownerDataHint));
				switch (rType)
				{
				case JinEngine::J_RESOURCE_TYPE::MATERIAL:
				{
					auto material = JICI::Create<JMaterial>(guid, JResourceObject::GetDefaultFormatIndex(), ownerDir);
					material->SetLight(true);
					material->SetShadow(true);
					break;
				}
				case JinEngine::J_RESOURCE_TYPE::SCENE:
				{
					JICI::Create<JScene>(guid, JResourceObject::GetDefaultFormatIndex(), ownerDir, J_SCENE_USE_CASE_TYPE::MAIN);
					break;
				}
				case JinEngine::J_RESOURCE_TYPE::ANIMATION_CONTROLLER:
				{
					JICI::Create<JAnimationController>(guid, JResourceObject::GetDefaultFormatIndex(), ownerDir);
					break;
				}
				default:
					break;
				}
			};
			auto canCreationDirLam = [](const size_t guid, const JEditorCreationHint& creationHint, JUserPtr<JDirectory> parent)
			{
				return parent.IsValid();
			};
			auto creationDirLam = [](const size_t guid, const JEditorCreationHint& creationHint, JUserPtr<JDirectory> parent)
			{
				JICI::Create<JDirectory>(guid, parent);
			};

			creation->resourceObj.GetCreationInterface()->RegisterCanCreationF(canCreationRobjLam);
			creation->resourceObj.GetCreationInterface()->RegisterObjectCreationF(creationRobjLam);
			creation->directory.GetCreationInterface()->RegisterCanCreationF(canCreationDirLam);
			creation->directory.GetCreationInterface()->RegisterObjectCreationF(creationDirLam);
		}
		void JWindowDirectory::InitializeSettingImpl()
		{
			if (setting != nullptr)
				return;

			auto importedResourcePostProcessFLam = [](JWindowDirectory* wndDir, std::vector<JUserPtr<JResourceObject>> rVec)
			{
				const uint resCount = (uint)rVec.size();
				for (uint i = 0; i < resCount; ++i)
				{
					if (rVec[i] != nullptr)
					{
						wndDir->SetModifiedBit(rVec[i], true);
						wndDir->PopPreviewScene();
					}
				}
				//search bar activate중에는 팝업생성 불가능
				wndDir->CreateDirectoryPreview();
			};
			auto renameLam = [](JWindowDirectory* wndDir)
			{
				auto selected = wndDir->GetFirstSelectedObject();
				wndDir->renameHelper->Activate(selected);
				wndDir->SetModifiedBit(selected, true);
			};

			using SwitchImportResourceF = JWindowDirectorySettingFunctor::SwitchImportResourceF;
			using ImportPostProcessF = JWindowDirectorySettingFunctor::ImportPostProcessF;
			using RenameF = JWindowDirectorySettingFunctor::RenameF;
			using MoveFIleF = JWindowDirectorySettingFunctor::MoveFIleF;
			using OpenNewDirectoryF = JWindowDirectorySettingFunctor::OpenNewDirectoryF;

			setting = std::make_unique<JWindowDirectorySettingFunctor>();
			setting->swtichImportResourceF = std::make_unique<SwitchImportResourceF::Functor>(&JWindowDirectory::SwitchImportSetting, this);
			setting->importPostProcessF = std::make_unique<ImportPostProcessF::Functor>(importedResourcePostProcessFLam);
			setting->renameF = std::make_unique<RenameF::Functor>(renameLam);
			setting->moveFileF = std::make_unique< MoveFIleF::Functor>(&JWindowDirectory::MoveFile, this);
			setting->openNewDirF = std::make_unique<OpenNewDirectoryF::Functor>(&JWindowDirectory::OpenNewDirectory, this);
		}
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

			//createDirectoryInDirectoryViewNode->RegisterSelectBind(std::make_unique<RegisterCreateDEvF::CompletelyBind>(*regCreateDirF));
		//	destoryDirectoryNode->RegisterSelectBind(std::make_unique<RegisterDestroyEvF::CompletelyBind>(*regDestroyObjF));
			//renameDirectoryNode->RegisterSelectBind(std::make_unique<RenameF::CompletelyBind>(*renameF, this));

			using RequestRObjCreationEvF = JWindowDirectoryCreationFunctor::RequestRObjCreationEvF;
			using RequestDirCreationEvF = JWindowDirectoryCreationFunctor::RequestDirectoryCreationEvF;
			using RequestDestructionEvF = JWindowDirectoryCreationFunctor::RequestDestructionEvF;

			createMaterialNode->RegisterSelectBind(std::make_unique<RequestRObjCreationEvF::CompletelyBind>(*creation->reqRObjCreationEvF, this, J_RESOURCE_TYPE::MATERIAL));
			createSceneNode->RegisterSelectBind(std::make_unique<RequestRObjCreationEvF::CompletelyBind>(*creation->reqRObjCreationEvF, this, J_RESOURCE_TYPE::SCENE));
			createAnimationControllerNode->RegisterSelectBind(std::make_unique<RequestRObjCreationEvF::CompletelyBind>(*creation->reqRObjCreationEvF, this, J_RESOURCE_TYPE::ANIMATION_CONTROLLER));
			createDirctoryInFileViewNode->RegisterSelectBind(std::make_unique<RequestDirCreationEvF::CompletelyBind>(*creation->reqDirCreationEvF, this));
			destroyNode->RegisterSelectBind(std::make_unique<RequestDestructionEvF::CompletelyBind>(*creation->reqDestructionEvF, this));
			destroyNode->RegisterEnableBind(std::make_unique<PassPopupConditionFlagF::CompletelyBind>(*GetPassSelectedAboveFlagFunctor(OBJECT_FLAG_UNDESTROYABLE, false), this, OBJECT_FLAG_UNDESTROYABLE, false));

			using SwitchImportResourceF = JWindowDirectorySettingFunctor::SwitchImportResourceF;
			using RenameF = JWindowDirectorySettingFunctor::RenameF;

			importNode->RegisterSelectBind(std::make_unique<SwitchImportResourceF::CompletelyBind>(*setting->swtichImportResourceF));
			renameFileNode->RegisterSelectBind(std::make_unique<RenameF::CompletelyBind>(*setting->renameF, this));
			renameFileNode->RegisterEnableBind(std::make_unique<PassPopupConditionFlagF::CompletelyBind>(*GetPassSelectedOneFlagFunctor(OBJECT_FLAG_UNEDITABLE, false), this, OBJECT_FLAG_UNEDITABLE, false));

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
		void JWindowDirectory::SetImportSetting(const bool value)noexcept
		{
			importData.isActivatedImportWindow = value;
			if (value)
				importData.folder = addressBar->GetOpendDirectory();
			else
			{
				importData.importDesc.clear();
				importData.importRType.clear();
			}
		}
		void JWindowDirectory::SwitchImportSetting()noexcept
		{
			SetImportSetting(!importData.isActivatedImportWindow);
		}
		void JWindowDirectory::Initialize()
		{
			const std::wstring contentPath = JApplicationProject::ContentsPath();
			root = _JResourceManager::Instance().GetDirectory(contentPath);
			//searchBar->RegisterDefaultObject(root);
			OpenNewDirectory(root);
		}
		void JWindowDirectory::UpdateWindow()
		{
			EnterWindow(J_GUI_WINDOW_FLAG_NO_SCROLL_BAR | J_GUI_WINDOW_FLAG_NO_COLLAPSE);
			UpdateDocking();
			if (IsActivated() && addressBar->GetOpendDirectory().IsValid())
			{
				UpdateMouseClick();
				//const float yOffset = JGui::GetWindowBorderSize() + JGui::GetWindowPadding().y;
				//const float xOffset = JGui::GetWindowBorderSize() + JGui::GetWindowPadding().x;
				//JGui::SetCursorPosX(JGui::GetWindowSize().x * 0.21f + xOffset);
				
				if (addressBar->Update() || requestUpdatePreview)
				{
					ClearPreviewGroup();
					CreateDirectoryPreview();
					requestUpdatePreview=  false;
				}
				JGui::SameLine();
				JGui::SetCursorPosX(JGui::GetWindowSize().x * 0.75f);
				JGui::SetNextItemWidth(JGui::GetWindowSize().x * 0.24);
				searchBar->UpdateSearchBar();
				 
				JVector2F nowCursor = JGui::GetCursorPos();
				childWindowHeight = JGui::GetWindowSize().y - nowCursor.y;
				//JGui::DrawRectFrame(nowCursor, JVector2F(JGui::GetWindowSize().x, childWindowHeight), 4, JGui::GetUColor( J_GUI_COLOR::FRAME_BG), true);
				BuildDirectoryView();
				JGui::SameLine();
				BuildFileView();
				ImportSettingOnScreen();
			}
			CloseWindow();
		}
		void JWindowDirectory::BuildDirectoryView()
		{
			//editorString->GetString(selectorIconSlidebarId)
			const JVector2F windowSize = JGui::GetWindowSize();
			const JVector2F viewSize = JVector2F(windowSize.x * 0.2f, childWindowHeight);
			JGui::BeginChildWindow(DIRECTORY_VIEW_NAME, viewSize, true, J_GUI_WINDOW_FLAG_AUTO_RESIZE);

			const bool canSelect = !searchBar->HasInputData();
			treeStrcture->Begin();
			DirectoryViewOnScreen(root, canSelect);
			treeStrcture->End();
			lastUpdateOpenNewDir = false;

			//보류
			//if (directoryViewPopup->IsOpen())
			//	directoryViewPopup->ExecutePopup(editorString.get());
			//directoryViewPopup->Update();
			JGui::EndChildWindow();
		}
		void JWindowDirectory::BuildFileView()
		{
			const JVector2F windowSize = JGui::GetWindowSize();
			const JVector2F viewSize = JVector2F(windowSize.x * 0.8f, childWindowHeight);
			JGui::BeginChildWindow(FILE_VIEW_NAME, viewSize, true, J_GUI_WINDOW_FLAG_AUTO_RESIZE);

			FileViewOnScreen();
			UpdatePopup(PopupSetting(fileviewPopup.get(), editorString.get()));
			JGui::EndChildWindow();
			//fileviewPopup->IsMouseInPopup()
		}
		void JWindowDirectory::DirectoryViewOnScreen(const JUserPtr<JDirectory>& directory, const bool canSelect)
		{
			auto opendDirectory = addressBar->GetOpendDirectory();
			bool isSelected = opendDirectory->GetGuid() == directory->GetGuid();
			if (lastUpdateOpenNewDir && opendDirectory->IsParent(directory.Get()))
				JGui::SetNextItemOpen(true);

			bool isNodeOpen = treeStrcture->DisplayTreeNode(JGui::CreateGuiLabel(directory, GetName()), treeStrcture->GetBaseFlag(), IsFocus(), true, isSelected);
			auto draggingResult = TryGetDraggingTarget();
			for (const auto& data : draggingResult)
			{
				if (!data.IsValid())
					continue;

				if (data->GetTypeInfo().IsChildOf<JDirectory>())
					RequestMoveFile(directory, Core::ConnectChildUserPtr<JDirectory>(data));
				else if (data->GetTypeInfo().IsChildOf<JResourceObject>())
					RequestMoveFile(directory, Core::ConnectChildUserPtr<JResourceObject>(data));
			}
			if (isNodeOpen)
			{
				if (JGui::IsLastItemClicked(Core::J_MOUSE_BUTTON::LEFT) && canSelect && !isSelected)
					RequestOpenNewDirectory(directory);

				const uint childDirCount = directory->GetChildernDirctoryCount();
				for (uint i = 0; i < childDirCount; ++i)
				{
					JUserPtr<JDirectory> child = directory->GetChildDirctory(i);
					if (child == nullptr)
						continue;
					DirectoryViewOnScreen(child, canSelect);
				}
				JGui::TreePop();
			}
		}
		void JWindowDirectory::FileViewOnScreen()
		{  
			//다른page 클릭시 create new preview
			if (pageCounter->GetMaxPageIndex() > 0 && pageCounter->DisplayUI("##PC" + GetName()))
				requestUpdatePreview = true;

			JGui::SameLine();

			const float fontSize = JGui::GetFontSize(); 
			const float textLabelSize = JGui::GetDisplaySize().x * Private::selectorLabelRate;
			iconData.maxSize = JGui::GetDisplaySize().x * Private::selectorIconMaxRate;
			iconData.minSize = JGui::GetAlphabetSize().y + 1.25f;

			iconData.size = JMathHelper::Clamp<float>(iconData.size, iconData.minSize, iconData.maxSize);
			iconData.isDetail = iconData.size >= (JGui::GetDisplaySize().x * Private::selectorIconMinRate);
			fileTitleBarSize = JVector2F(iconData.size, fontSize * (textLabelSize / fontSize) + JGui::GetWindowSize().y * 0.005f);
			 
			JEditorSliderBar sliderBar(iconData.size, iconData.minSize, iconData.maxSize);
			sliderBar.Update("##" + GetName() + "_SizeSlider", 0, true, false);
			iconData.size = sliderBar.GetValue();
			JGui::Separator();

			const JVector2F initCursorPos = JGui::GetCursorPos();
			const JVector2F windowSize = JGui::GetWindowSize();
			const JVector2F contentsSize = iconData.isDetail ? JVector2F(iconData.size, iconData.size + fileTitleBarSize.y) : JVector2F(windowSize.x, iconData.size);
			const JVector2F contentsPadding = iconData.isDetail ? JVector2F(windowSize.x * 0.015f, windowSize.y * 0.03f) : JGui::GetFramePadding();
			const JVector2F contentsSpacing = iconData.isDetail ? windowSize * 0.015f : JGui::GetItemSpacing() * 1.5f;
			const JVector2F alphabetSize = JGui::GetAlphabetSize();

			JVector2F innerSize[2];
			JVector2F innerPosition[2];
			if (iconData.isDetail)
			{
				innerSize[0] = CreateVec2(iconData.size);
				innerSize[1] = fileTitleBarSize * 0.8f;

				innerPosition[0] = JVector2F(0, 0);
				innerPosition[1] = fileTitleBarSize * 0.1f;
			}
			else
			{ 
				innerSize[0] = JVector2F((windowSize.x - iconData.size) * 0.8f, iconData.size);
				innerSize[1] = JVector2F((windowSize.x - iconData.size) * 0.2f, iconData.size);

				innerPosition[0] = JVector2F(0, 0);
				innerPosition[1] = JVector2F(JGui::GetFramePadding().x, 0);
			} 
			 
			const J_EDITOR_INNER_ALGIN_TYPE alignType = iconData.isDetail ? J_EDITOR_INNER_ALGIN_TYPE::COLUMN : J_EDITOR_INNER_ALGIN_TYPE::ROW;
			JEditorDynamicAlignCalculator<2> widgetAlignCal;
			widgetAlignCal.Update(windowSize, contentsSize, contentsPadding, contentsSpacing, innerSize, innerPosition, alignType, JGui::GetCursorPos());
			JEditorTextAlignCalculator textAlignCal;

			const JVector4F contentsFrameColor = JGui::GetColor(J_GUI_COLOR::SEPARATOR);
			const JVector4F contentsBgColor = JGui::GetColor(J_GUI_COLOR::HEADER);
			const JVector4F iconBgColor = JGui::GetColor(J_GUI_COLOR::HEADER) + JVector4F(0, 0, 0, -0.05f);
			const JVector4F textBgColor = JGui::GetColor(J_GUI_COLOR::HEADER) + JVector4F(0.1f, 0.1f, 0.1f, 0.05f);
			const JVector4F bgRectDelta = JVector4F(0.1f, 0.1f, 0.1f, 0);

			//JGui::SetTreeNodeColorSet(sColFactor);
			const JVector4F headerCol = JGui::GetColor(J_GUI_COLOR::HEADER);
			const JVector4F hovCol = JGui::GetColor(J_GUI_COLOR::HEADER_HOVERED);
			 
			if (iconData.isDetail)
			{
				JGui::PushColor(J_GUI_COLOR::HEADER, JVector4F::Zero());
				JGui::PushColor(J_GUI_COLOR::HEADER_HOVERED, JVector4F::Zero());
				JGui::PushColor(J_GUI_COLOR::HEADER_ACTIVE, JVector4F::Zero());
			} 

			auto hoverLam = [](JWindowDirectory* wnd, const JUserPtr<JObject>& nowObject)
			{
				wnd->SetHoveredObject(nowObject);
				if (JGui::AnyMouseClicked(false))
					wnd->SetContentsClick(true);
			};
			auto selectLam = [](JWindowDirectory* wnd, const JUserPtr<JObject>& nowObject)
			{
				wnd->RequestPushSelectObject(nowObject);
				if (nowObject->GetObjectType() == J_OBJECT_TYPE::RESOURCE_OBJECT)
				{
					JResourceObject* jRobj = static_cast<JResourceObject*>(nowObject.Get());
					const J_RESOURCE_TYPE resourceType = jRobj->GetResourceType();
					if (resourceType == J_RESOURCE_TYPE::SKELETON)
						wnd->RequestOpenPage(JEditorOpenPageEvStruct{ J_EDITOR_PAGE_TYPE::SKELETON_SETTING, Core::GetUserPtr(jRobj) }, true);
					else if (resourceType == J_RESOURCE_TYPE::ANIMATION_CONTROLLER)
						wnd->RequestOpenPage(JEditorOpenPageEvStruct{ J_EDITOR_PAGE_TYPE::ANICONT_SETTING, Core::GetUserPtr(jRobj) }, true);
				}
				else
				{
					JDirectory* jDir = static_cast<JDirectory*>(nowObject.Get());
					if (JGui::GetMouseClickedCount(Core::J_MOUSE_BUTTON::LEFT) >= 2 && !JGui::IsKeyDown(Core::J_KEYCODE::CONTROL))
						wnd->RequestOpenNewDirectory(Core::GetUserPtr(jDir));
				} 
			};
			auto draggingLam = [](JWindowDirectory* wnd, const JUserPtr<JObject>& nowObject)
			{
				if (nowObject == nullptr)
					return;

				wnd->TryBeginDragging(Core::GetUserPtr<Core::JIdentifier>(nowObject.Get()));
				auto draggingResult = wnd->TryGetDraggingTarget();
				
				bool isSelectedDir = nowObject->GetTypeInfo().IsChildOf<JDirectory>();
				if (!isSelectedDir)
					return;

				for (const auto& data : draggingResult)
				{
					if (!data.IsValid())
						continue;

					if (data->GetTypeInfo().IsChildOf<JDirectory>())
						wnd->RequestMoveFile(Core::ConnectChildUserPtr<JDirectory>(nowObject), Core::ConnectChildUserPtr<JDirectory>(data));
					else if (data->GetTypeInfo().IsChildOf<JResourceObject>())
						wnd->RequestMoveFile(Core::ConnectChildUserPtr<JDirectory>(nowObject), Core::ConnectChildUserPtr<JResourceObject>(data));
				}

				//if (draggingResult.IsValid() && nowObject->GetObjectType() == J_OBJECT_TYPE::DIRECTORY_OBJECT)
			};

			selectableStructure->Begin(iconData.isDetail, iconData.isDetail, iconData.isDetail, mouseBBox->IsActivated());
			selectableStructure->SetDeltaColor(bgRectDelta);
			selectableStructure->SetFrameThickness(5);

			const J_GUI_SELECTABLE_FLAG_ selectableFlag = J_GUI_SELECTABLE_FLAG_ALLOW_DOUBLE_CLICK | J_GUI_SELECTABLE_FLAG_ALLOW_ITEM_OVERLAP;

			bool hasInvaildScene = false;  
			const uint count = GetPreviewSceneCount();
			for (uint i = 0; i < count; ++i)
			{
				JPreviewScene* nowPreviewScene = GetPreviewScene(i);
				JUserPtr<JObject> nowObject = nowPreviewScene->GetJObject();
				//const J_OBJECT_FLAG flag = nowObject->GetFlag();
				//OBJECT_FLAG_HIDDEN obj들은 애초에 windowDirectory에서 접근할수없는 folder에 존재한다.
				//if ((flag & OBJECT_FLAG_HIDDEN) > 0)
				//	continue;

				if (!searchBar->CanSrcNameOnScreen(nowObject->GetName()))
					continue;

				const J_OBJECT_TYPE objType = nowObject->GetObjectType();
				const bool isValidType = objType == J_OBJECT_TYPE::RESOURCE_OBJECT || objType == J_OBJECT_TYPE::DIRECTORY_OBJECT;
				if (!isValidType)
					continue;

				std::wstring name;
				if (objType == J_OBJECT_TYPE::RESOURCE_OBJECT)
					name = static_cast<JResourceObject*>(nowObject.Get())->GetName();
				else if (objType == J_OBJECT_TYPE::DIRECTORY_OBJECT)
					name = static_cast<JDirectory*>(nowObject.Get())->GetName();

				const bool isSelected = IsSelectedObject(nowObject->GetGuid());
				const bool isRenameActivaetd = renameHelper->IsActivated() && renameHelper->IsRenameTar(nowObject->GetGuid());

				const JVector2F iconSize = widgetAlignCal.GetInnerContentsSize();
				widgetAlignCal.SetNextContentsPosition();
				const bool isHovered = JGui::IsMouseInRect(JGui::GetCursorScreenPos(), widgetAlignCal.GetTotalContentsSize());
				if (isHovered)
					hoverLam(this, nowObject);

				const JVector4F addedColor = JGui::GetSelectableColorFactor(IsFocus(), isSelected, isHovered);
				const std::string label = "##" + std::to_string(nowObject->GetGuid()) + "_Selectable";

				//Has order dependency	
				if (iconData.isDetail)
				{
					JVector2F preWorldCursorPos = JGui::GetCursorScreenPos();
					JVector2F preCursor = JGui::GetCursorPos();

					const bool isClick =  selectableStructure->DisplaySelectable(label,
						selectableFlag,
						IsFocus(),
						true,
						isSelected,
						widgetAlignCal.GetTotalContentsSize(),
						contentsBgColor + addedColor,
						contentsFrameColor + addedColor);
  
					if (isClick)
						selectLam(this, nowObject);
					draggingLam(this, nowObject);

					JGui::SetCursorPos(preCursor);
					if (nowPreviewScene->UseQuadShape())
					{
						//draw icon bg
						JGui::DrawRectFilledMultiColor(preWorldCursorPos,
							iconSize,
							iconBgColor + addedColor,
							bgRectDelta,
							true);
					}
					JGuiImageInfo info(nowPreviewScene->GetPreviewCamera().Get(), Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON);
					JGui::AddRoundedImage(info, JGui::GetCursorScreenPos(), JGui::GetCursorScreenPos() + CreateVec2(iconData.size));
				
					const JVector2F multilineSize = widgetAlignCal.GetInnerContentsSize();			 
					textAlignCal.Update(JCUtil::WstrToU8Str(name), multilineSize, true);
					widgetAlignCal.SetNextContentsPosition();

					if (isRenameActivaetd)
					{
						renameCursorPos = JGui::GetCursorPos();
						renameRectSize = multilineSize;
					}
					else
					{
						//draw text bg
						JGui::DrawRectFilledMultiColor(JGui::GetCursorScreenPos(),
							multilineSize,
							textBgColor + addedColor,
							bgRectDelta,
							true);
						JGui::Text(textAlignCal.MiddleAligned());
					}
				}
				else
				{
					const JVector2F preCursor = JGui::GetCursorPos();
					if (isRenameActivaetd)
					{ 
						renameCursorPos = preCursor + JVector2F(0, (iconData.size - alphabetSize.y) * 0.5f);
						widgetAlignCal.SetNextContentsPosition();
					}
					else
					{
						const bool isClick = selectableStructure->DisplaySelectable(label,
							selectableFlag,
							IsFocus(),
							true,
							isSelected,
							widgetAlignCal.GetTotalContentsSize(),
							contentsBgColor + addedColor,
							contentsFrameColor + addedColor);

						if (isClick)
							selectLam(this, nowObject);
						draggingLam(this, nowObject);
						JGui::SetCursorPos(preCursor + JVector2F(0, (iconData.size - alphabetSize.y) * 0.5f));

						JGui::Text(nowObject->GetName());
						widgetAlignCal.SetNextContentsPosition();

						if (objType == J_OBJECT_TYPE::RESOURCE_OBJECT)
							JGui::Text(JCUtil::ToLowercase(static_cast<JResourceObject*>(nowObject.Get())->GetFormat()));
						else
							JGui::Text("Folder");
					}
				}	 
			}

			selectableStructure->End();
			if (iconData.isDetail)
				JGui::PopColor(3); 
				
			if (hasInvaildScene)
				DestroyInvalidPreviewScene();

			if (renameHelper->IsActivated())
			{
				JGui::SetCursorPos(renameCursorPos);
				if (iconData.isDetail)
					renameHelper->UpdateMultiline(renameRectSize, false);
				else
					renameHelper->Update(false);
			}

			const bool canAcitvateMouseBBox = JGui::IsMouseInCurrentWindow() && 
				JGui::IsMouseDragging(Core::J_MOUSE_BUTTON::LEFT) &&
				!JGui::IsDragDropActivated() &&
				!IsContentsClicked();		 
			if (JGui::IsCurrentWindowFocused(J_GUI_FOCUS_FLAG_CHILD_WINDOW))
			{
				using DragUpdaetIn = JEditorMouseDragCanvas::UpdateIn;
				using DragUpdaetOut = JEditorMouseDragCanvas::UpdateOut;

				std::vector<Core::JBBox2D> bboxVec = selectableStructure->GetLastUpdated();
 
				//local
				JVector2F bboxPadding(0, initCursorPos.y);
				DragUpdaetIn in(canAcitvateMouseBBox, bboxPadding, JVector2F::Zero());
				DragUpdaetOut out;

				//selectableStructure는 mouse bbox가 activate시 count
				if (bboxVec.size() == count)
				{
					in.iden.resize(count);
					for (uint i = 0; i < count; ++i)
						in.iden[i] = JEditorMouseDragCanvas::IdenData(GetPreviewScene(i)->GetJObject(), bboxVec[i]);
					in.selectedMap = GetSelectedObjectMap();
				}  
				
				mouseBBox->UpdateCanvasDrag(in, out);
				if (out.newDeSelectedVec.size() > 0)
				{
					AddEventNotification(*JEditorEvent::EvInterface(),
						GetGuid(),
						J_EDITOR_EVENT::POP_SELECT_OBJECT,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPopSelectObjectEvStruct>(GetOwnerPageType(), out.newDeSelectedVec, JEditorEvStruct::RANGE::ALL)));
				}
				if (out.newSelectedVec.size() > 0)
				{
					AddEventNotification(*JEditorEvent::EvInterface(),
						GetGuid(),
						J_EDITOR_EVENT::PUSH_SELECT_OBJECT,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPushSelectObjectEvStruct>(GetOwnerPageType(), GetWindowType(), out.newSelectedVec, JEditorEvStruct::RANGE::ALL)));
				}
				if (mouseBBox->GetSelectedCount() > 0)
					SetContentsClick(true);
			}
		}
		void JWindowDirectory::ImportSettingOnScreen()
		{
			if (!importData.isActivatedImportWindow)
				return;

			if (importData.isActivatedImportWindow && JApplicationEngine::GetApplicationState() != J_APPLICATION_STATE::EDIT_GAME)
				SwitchImportSetting();

			bool isOpen = importData.isActivatedImportWindow;
			J_GUI_WINDOW_FLAG_ flag = J_GUI_WINDOW_FLAG_NO_DOCKING | J_GUI_WINDOW_FLAG_NO_SAVE;

			if (JGui::BeginWindow("Import Setting", &isOpen, flag))
			{
				static constexpr float textRate = 0.7f;
				const JVector2F windowSize = JGui::GetWindowSize();
				const JVector2F alphabetSize = JGui::GetAlphabetSize();
				const JVector2F textBarSize = JVector2F(windowSize.x * textRate, alphabetSize.y);
				const uint textCount = textBarSize.x / alphabetSize.x;

				JGui::Separator();
				JGui::Text("Folder: ");
				JGui::SameLine();

				std::string folderPath = importData.folder != nullptr ? JCUtil::WstrToU8Str(importData.folder->GetPath()) : "None";
				std::string compressFolderPath = JCUtil::CompressStringPath(folderPath, textCount);

				if (JGui::BeginCombo("##Import FolderPath combo", compressFolderPath))
				{
					static constexpr float comoboTextRate = textRate * 0.9f;
					const JVector2F comboTextSize = JVector2F(windowSize.x * textRate, alphabetSize.y);
					const uint comboTextCount = comboTextSize.x / alphabetSize.x;

					auto dirRawVec = JDirectory::StaticTypeInfo().GetInstanceRawPtrVec();
					for (const auto& data : dirRawVec)
					{
						auto dir = static_cast<JDirectory*>(data);
						if (dir->HasFlag(OBJECT_FLAG_HIDDEN))
							continue;

						if (JGui::Selectable(JCUtil::CompressStringPath(JCUtil::WstrToU8Str(dir->GetPath()), comboTextCount)))
							importData.folder = Core::GetUserPtr(dir);
					}
					JGui::EndCombo();
				}

				JGui::SameLine();
				if (JGui::Button("Search##Directory ImportSettingOnScreen"))
				{
					std::wstring path;
					if (JWindow::SelectDirectory(path, L"please, select resource file"))
					{
						auto dir = _JResourceManager::Instance().GetDirectory(path);
						if (dir != nullptr)
							importData.folder = dir;
					}
				}

				JGui::Separator();
				JGui::Text("File: ");
				JGui::SameLine();
				if (JGui::Button("Search##File ImportSettingOnScreen"))
				{
					std::vector<std::wstring> path;
					if (JWindow::SelectMultiFile(path, L"please, select resource file"))
					{
						importData.importDesc.clear();
						importData.importRType.clear();

						importData.descIndex = 0;
						for (const auto& pData : path)
						{
							Core::JFileImportHelpData pathData{ pData }; 
							size_t fileSizeSum = 0;
							if (!JResourceObjectImporter::Instance().IsValidFormat(pathData.format))
							{
								J_LOG_PRINT_OUTW(L"Is not valid format", pData); 
								continue;
							}

							std::filesystem::path p{ pData };
							size_t fileSize = std::filesystem::file_size(p);
							fileSizeSum += fileSize;

							if (!JWindow::HasStorageSpace(addressBar->GetOpendDirectory()->GetPath(), fileSizeSum))
							{
								J_LOG_PRINT_OUTW(L"Can't find disk space " + std::to_wstring(fileSize) + L"byte", pData); 
								continue;
							}

							std::vector<J_RESOURCE_TYPE> rType = JResourceObjectImporter::Instance().DeterminFileResourceType(pathData);
							for (const auto& rData : rType)
							{
								switch (rData)
								{
								case JinEngine::J_RESOURCE_TYPE::MESH:
								{
									importData.importRType.push_back(rData);
									importData.importDesc.push_back(std::make_unique<JMeshGeometryImportDesc>(pathData));
									break;
								}
								case JinEngine::J_RESOURCE_TYPE::TEXTURE:
								{
									importData.importRType.push_back(rData);
									importData.importDesc.push_back(std::make_unique<JTextureImportDesc>(pathData));
									break;
								}
								case JinEngine::J_RESOURCE_TYPE::ANIMATION_CLIP:
								{
									importData.importRType.push_back(rData);
									importData.importDesc.push_back(std::make_unique<JResourceObjectImportDesc>(pathData));
									break;
								}
								default:
									break;
								}
							}
						}
					}
				}
				if (importData.importDesc.size() > importData.descIndex)
				{
					auto desc = importData.importDesc[importData.descIndex].get();
					JGui::Text(std::to_string(importData.descIndex) + ". " +
						JCUtil::WstrToU8Str(desc->importPathData.fullName) + "(" +
						Core::GetName(importData.importRType[importData.descIndex]) + ")");

					if (desc->GetTypeInfo().IsChildOf<JMeshGeometryImportDesc>())
					{
						auto meshDesc = static_cast<JMeshGeometryImportDesc*>(desc);
						JGui::CheckBox("UseSplit", meshDesc->useSplitMesh);
					}
					else if (desc->GetTypeInfo().IsChildOf<JTextureImportDesc>())
					{
						auto textureDesc = static_cast<JTextureImportDesc*>(desc);
						JGui::CheckBox("UseMipmap", textureDesc->useMipmap);
					}
				}
				int maxIndex = importData.importDesc.size() - 1;
				if (maxIndex < 0)
					maxIndex = 0;
				JGui::SliderInt("##Desc Index Slider ImportSettingOnScreen", &importData.descIndex, 0, maxIndex, J_GUI_SLIDER_FLAG_ALWAYS_CLAMP);

				JVector2F nextCursorPos;
				nextCursorPos.x = windowSize.x - alphabetSize.x * 16 - JGui::GetFramePadding().x * 2;;
				//nextCursorPos.y = windowSize.y - alphabetSize.y * 2 - JGui::GetFramePadding().y * 2;

				JGui::Separator();
				JGui::SetCursorPosX(nextCursorPos.x);

				if (importData.folder == nullptr)
					JGui::PushButtonColorDeActSet();
				if (JGui::Button("Import##File ImportSettingOnScreen") && importData.folder != nullptr)
				{
					std::vector<JUserPtr<JResourceObject>> totalRes;
					for (auto& data : importData.importDesc)
					{
						data->dir = importData.folder;
						std::vector<JUserPtr<JResourceObject>> res = JResourceObjectImporter::Instance().ImportResource(data.get());
						totalRes.insert(totalRes.end(), std::make_move_iterator(res.begin()), std::make_move_iterator(res.end()));
					}
					if (totalRes.size() > 0)
						RequestImportPostProccess(std::move(totalRes));
					isOpen = false;
				}
				if (importData.folder == nullptr)
					JGui::PopButtonColorDeActSet();
				JGui::SameLine();
				if (JGui::Button("Cancel##File ImportSettingOnScreen"))
					isOpen = false;
				JGui::EndWindow();
			}
			if (isOpen != importData.isActivatedImportWindow)
				SwitchImportSetting();
		}
		void JWindowDirectory::OpenNewDirectory(JUserPtr<JDirectory> newOpendDirectory)
		{
			if (!newOpendDirectory.IsValid())
				return;

			auto opendDirectory = addressBar->GetOpendDirectory();
			if (opendDirectory.IsValid() && opendDirectory->GetGuid() == newOpendDirectory->GetGuid())
				return;

			//window->DC.StateStorage->SetInt(id, is_open);
			pageCounter->SetPageIndex(0);
			addressBar->SetOpendDirectory(newOpendDirectory);
			ClearPreviewGroup();
			CreateDirectoryPreview();
			if (opendDirectory.IsValid())
				DirActInterface::CloseDirectory(opendDirectory);
			opendDirectory = newOpendDirectory;
			DirActInterface::OpenDirectory(opendDirectory);
			lastUpdateOpenNewDir = true;
		}
		void JWindowDirectory::CreateDirectoryPreview()
		{
			TryCreateDirectoryPreview(addressBar->GetOpendDirectory(), pageCounter.get(), nullptr, true);
		}
		void JWindowDirectory::CreateAllDirectoryPreview(const JUserPtr<JDirectory>& directory, const bool hasNameMask, const std::wstring& mask)
		{
			const uint fileCount = (uint)directory->GetFileCount();
			for (uint i = 0; i < fileCount; ++i)
			{
				JUserPtr<JFile> file = directory->GetDirectoryFile(i);
				if (file == nullptr)
					continue;

				if (hasNameMask && !JCUtil::Contain(file->GetName(), mask))
					continue;

				CreatePreviewScene(file->TryGetResourceUser(), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL);
			}

			const uint childCount = directory->GetChildernDirctoryCount();
			for (uint i = 0; i < childCount; ++i)
				CreateAllDirectoryPreview(directory->GetChildDirctory(i), hasNameMask, mask);
		}
		void JWindowDirectory::RequestMoveFile(const JUserPtr<JDirectory>& to, const JUserPtr<JObject>& obj)
		{
			if (obj->GetGuid() == to->GetGuid())
				return;

			std::string taskName;
			std::string taskDesc;
			using MoveFileF = JWindowDirectorySettingFunctor::MoveFIleF;
			if (obj->GetObjectType() == J_OBJECT_TYPE::DIRECTORY_OBJECT)
			{  
				taskName = "Move directory";
				taskDesc = JCUtil::WstrToU8Str(L"object name: " + obj->GetName() + L" " +
					static_cast<JDirectory*>(obj.Get())->GetParent()->GetName() + L" to " + to->GetName());

				auto doBind = std::make_unique<MoveFileF::CompletelyBind>(*setting->moveFileF, JUserPtr<JDirectory>(to), JUserPtr<JObject>(obj));
				auto undoBind = std::make_unique<MoveFileF::CompletelyBind>(*setting->moveFileF, static_cast<JDirectory*>(obj.Get())->GetParent(), JUserPtr<JObject>(obj));
				auto evStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorTSetBindFuncEvStruct>
					(taskName, taskDesc, GetOwnerPageType(), std::move(doBind), std::move(undoBind)));
				AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::T_BIND_FUNC, evStruct);
			}
			else if (obj->GetObjectType() == J_OBJECT_TYPE::RESOURCE_OBJECT)
			{
				JUserPtr<JDirectory> fromDir = static_cast<JResourceObject*>(obj.Get())->GetDirectory();
				if (fromDir->GetGuid() == to->GetGuid())
					return;

				taskName = "Move file";
				taskDesc = JCUtil::WstrToU8Str(L"object name: " + obj->GetName() + L" " +
					static_cast<JResourceObject*>(obj.Get())->GetDirectory()->GetName() + L" to " + to->GetName());
				auto doBind = std::make_unique<MoveFileF::CompletelyBind>(*setting->moveFileF, JUserPtr<JDirectory>(to), JUserPtr<JObject>(obj));
				auto undoBind = std::make_unique<MoveFileF::CompletelyBind>(*setting->moveFileF, JUserPtr<JDirectory>(fromDir), JUserPtr<JObject>(obj));
				auto evStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorTSetBindFuncEvStruct>
					(taskName, taskDesc, GetOwnerPageType(), std::move(doBind), std::move(undoBind)));
				AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::T_BIND_FUNC, evStruct);
			}
		}
		void JWindowDirectory::MoveFile(JUserPtr<JDirectory> to, JUserPtr<JObject> obj)
		{
			if (obj->GetObjectType() == J_OBJECT_TYPE::DIRECTORY_OBJECT)
			{
				DirRawInterface::MoveDirectory(Core::ConnectChildUserPtr<JDirectory>(obj), to);
				DestroyPreviewScene(obj);
			}
			else if (obj->GetObjectType() == J_OBJECT_TYPE::RESOURCE_OBJECT)
			{
				ResourceFileInterface::MoveFile(Core::ConnectChildUserPtr<JResourceObject>(obj), to);
				DestroyPreviewScene(obj);
			}
		}
		void JWindowDirectory::RequestOpenNewDirectory(JUserPtr<JDirectory> newOpendDirectory)
		{
			if (newOpendDirectory == nullptr)
				return;

			using OpenNewDirectoryF = JWindowDirectorySettingFunctor::OpenNewDirectoryF;
			auto openNewBindB = std::make_unique<OpenNewDirectoryF::CompletelyBind>(*setting->openNewDirF, std::move(newOpendDirectory));

			auto evStruct = std::make_unique<JEditorBindFuncEvStruct>(std::move(openNewBindB), GetOwnerPageType());
			AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::BIND_FUNC, JEditorEvent::RegisterEvStruct(std::move(evStruct)));
		}
		void JWindowDirectory::RequestImportPostProccess(std::vector<JUserPtr<JResourceObject>>&& rVec)
		{
			using ImportPostProcessF = JWindowDirectorySettingFunctor::ImportPostProcessF;
			auto postImplB = std::make_unique<ImportPostProcessF::CompletelyBind>(*setting->importPostProcessF,
				this,
				std::move(rVec));
			auto evStruct = std::make_unique<JEditorBindFuncEvStruct>(std::move(postImplB), GetOwnerPageType());
			AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::BIND_FUNC, JEditorEvent::RegisterEvStruct(std::move(evStruct)));
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
			if (!addressBar->GetOpendDirectory().IsValid())
				addressBar->SetOpendDirectory(root);
			pageCounter->SetPageIndex(0);
			CreateDirectoryPreview();
		}
		void JWindowDirectory::DoDeActivate()noexcept
		{
			DeRegisterListener();
			ClearPreviewGroup();
			JEditorWindow::DoDeActivate();
		}
		void JWindowDirectory::LoadEditorWindow(JFileIOTool& tool)
		{
			JEditorWindow::LoadEditorWindow(tool);
			RequestOpenNewDirectory(JObjectFileIOHelper::_LoadHasIden<JDirectory>(tool, "OpendDirectory"));
			treeStrcture->LoadData(tool);
		}
		void JWindowDirectory::StoreEditorWindow(JFileIOTool& tool)
		{
			JEditorWindow::StoreEditorWindow(tool);
			JObjectFileIOHelper::_StoreHasIden(tool, addressBar->GetOpendDirectoryRaw(), "OpendDirectory");
			JGuiWindowInfo info;
			JGui::GetWindowInfo(GetName(), info);
			treeStrcture->StoreData(tool, info.windowID);
		}
		void JWindowDirectory::OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)
		{
			JEditorWindow::OnEvent(senderGuid, eventType, eventStruct);
			if (!eventStruct->CanExecuteOtherEv(senderGuid, GetGuid()))
				return;

			if (eventType == J_EDITOR_EVENT::MOUSE_CLICK)
			{
				if (eventStruct->pageType == GetOwnerPageType())
					fileviewPopup->SetOpen(false);
			}
		}
	}
}