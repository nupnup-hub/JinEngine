#include"JWindowDirectory.h"
#include"../../JEditorPageShareData.h"
#include"../../JEditorAttribute.h"
#include"../../../Gui/JGui.h" 
#include"../../../Gui/JGuiImageInfo.h"
#include"../../../Event/JEditorEvent.h"
#include"../../../String/JEditorStringMap.h"
#include"../../../Popup/JEditorPopupMenu.h"
#include"../../../Popup/JEditorPopupNode.h"  
#include"../../../EditTool/JEditorSearchBarHelper.h"
#include"../../../EditTool/JEditorRenameHelper.h"
#include"../../../EditTool/JEditorTreeStructure.h"
#include"../../../Align/JEditorAlignCalculator.h"
#include"../../../Interface/JEditorObjectCreationInterface.h"
#include"../../../../Core/Math/JMathHelper.h"
#include"../../../../Core/Math/JVectorExtend.h"
#include"../../../../Core/Guid/JGuidCreator.h" 
#include"../../../../Core/Identity/JIdenCreator.h" 
#include"../../../../Core/Utility/JCommonUtility.h"
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
			searchBarHelper = std::make_unique<JEditorSearchBarHelper>(false);
			treeStrcture = std::make_unique<JEditorTreeStructure>();

			selectorIconSlidebarId = Core::MakeGuid();
			editorString->AddString(selectorIconSlidebarId, { "Icon Size" , u8"아이콘 크기" });

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

				JUserPtr<JDirectory> owner = wndDir->opendDirctory;
				if (!owner.IsValid())
					owner = wndDir->root;

				JEditorCreationHint creationHint = JEditorCreationHint(wndDir,
					false, false, false, false,
					Core::JTypeInstanceSearchHint(),
					Core::JTypeInstanceSearchHint(owner),
					&JEditorWindow::NotifyEvent);
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification, wndDir->GetClearTaskFunctor());

				JWindowDirectoryCreationFunctor* impl = wndDir->creation.get();
				impl->resourceObj.RequestCreateObject(impl->dS, false, creationHint, Core::MakeGuid(), requestHint, std::move(resourceType));
			};
			auto requestCreateDirectoryLam = [](JWindowDirectory* wndDir)
			{
				if (!wndDir->root.IsValid())
					return;

				JUserPtr<JDirectory> parent = wndDir->opendDirctory;
				if (!parent.IsValid())
					parent = wndDir->root;

				JEditorCreationHint creationHint = JEditorCreationHint(wndDir,
					false, false, false, false,
					Core::JTypeInstanceSearchHint(),
					Core::JTypeInstanceSearchHint(),
					&JEditorWindow::NotifyEvent);
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification, wndDir->GetClearTaskFunctor());

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
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification, wndDir->GetClearTaskFunctor());

				JWindowDirectoryCreationFunctor* impl = wndDir->creation.get();
				impl->destructuion.RequestDestroyObject(impl->dS, false, creationHint, objVec, requestHint);
			};
			auto creationRObjPostProccessLam = [](JEditorWindow* wndDir, Core::JIdentifier* resource)
			{
				static_cast<JWindowDirectory*>(wndDir)->CreatePreviewScene(JUserPtr<JObject>::ConvertChild(Core::GetUserPtr(resource)));
			};
			auto creationDirPostProccessLam = [](JEditorWindow* wndDir, Core::JIdentifier* dir)
			{
				static_cast<JWindowDirectory*>(wndDir)->CreatePreviewScene(JUserPtr<JObject>::ConvertChild(Core::GetUserPtr(dir)));
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
					JICI::Create<JMaterial>(guid, JResourceObject::GetDefaultFormatIndex(), ownerDir);
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
						wndDir->SetModifiedBit(rVec[i], true);
				}
				//search bar activate중에는 팝업생성 불가능
				wndDir->CreateDirectoryPreview(wndDir->opendDirctory, false);
			};
			auto renameLam = [](JWindowDirectory* wndDir)
			{
				wndDir->renameHelper->Activate(wndDir->GetHoveredObject());
				wndDir->SetModifiedBit(wndDir->GetHoveredObject(), true);
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
			destroyNode->RegisterEnableBind(std::make_unique<JEditorPopupNode::EnableF::CompletelyBind>(*GetPassSelectedAboveOneFunctor(), this));

			using SwitchImportResourceF = JWindowDirectorySettingFunctor::SwitchImportResourceF;
			using RenameF = JWindowDirectorySettingFunctor::RenameF;

			importNode->RegisterSelectBind(std::make_unique<SwitchImportResourceF::CompletelyBind>(*setting->swtichImportResourceF));
			renameFileNode->RegisterSelectBind(std::make_unique<RenameF::CompletelyBind>(*setting->renameF, this));
			renameFileNode->RegisterEnableBind(std::make_unique<JEditorPopupNode::EnableF::CompletelyBind>(*GetPassSelectedOneFunctor(), this));

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
				importData.folder = opendDirctory;
			else if (!value)
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
			//searchBarHelper->RegisterDefaultObject(root);
			OpenNewDirectory(root);
		}
		void JWindowDirectory::UpdateWindow()
		{
			EnterWindow(J_GUI_WINDOW_FLAG_NO_SCROLL_BAR | J_GUI_WINDOW_FLAG_NO_COLLAPSE);
			UpdateDocking();
			if (IsActivated() && opendDirctory.IsValid())
			{
				UpdateMouseClick();  
				const float yOffset = JGui::GetWindowBorderSize() + JGui::GetWindowPadding().y;
				const float xOffset = JGui::GetWindowBorderSize() + JGui::GetWindowPadding().x; 
				JGui::SetCursorPosX(JGui::GetWindowSize().x * 0.21f + xOffset);
				searchBarHelper->UpdateSearchBar();
				const bool hasData = searchBarHelper->HasInputData();
				const bool isUpdated = searchBarHelper->IsUpdateInputData();
				if (isUpdated)
				{
					if (!hasData)
					{
						ClearPreviewGroup();
						CreateDirectoryPreview(opendDirctory, false);
					}
					else
					{
						ClearPreviewGroup();
						CreateAllDirectoryPreview(root, true, JCUtil::U8StrToWstr(searchBarHelper->GetInputData()));
					}
				}

				JGui::SameLine();
				const float fontSize = JGui::GetFontSize();
				btnIconMaxSize = JGui::GetDisplaySize().x * selectorIconMaxRate;
				btnIconMinSize = JGui::GetDisplaySize().x * selectorIconMinRate;
				btnIconSize = JMathHelper::Clamp<float>(btnIconSize, btnIconMinSize, btnIconMaxSize);
				fileTitleBarSize = JVector2<float>(btnIconSize, fontSize * (btnIconMinSize / fontSize) + JGui::GetWindowSize().y * 0.005f);
				 
				//JGui::PushStyle(J_GUI_STYLE::FRAME_PADDING, JVector2F(JGui::GetFramePadding().x, 0));
				//JGui::SetCursorPosX(JGui::GetWindowSize().x - JGui::GetSliderWidth() - xOffset);
				JGui::SetNextItemWidth(JGui::GetRestWindowContentsSize().x);
				JGui::SliderFloat("##" + GetName() + "_SizeSlider", &btnIconSize, btnIconMinSize, btnIconMaxSize, 1, J_GUI_SLIDER_FLAG_ALWAYS_CLAMP | J_GUI_SLIDER_FLAG_NO_INPUT);
				if (JGui::IsLastItemActivated() || JGui::IsLastItemHovered())
					JGui::Tooltip(btnIconSize, 3);
				 
				JVector2<float> nowCursor = JGui::GetCursorPos();
				childWindowHeight = JGui::GetWindowSize().y - nowCursor.y;
				//JGui::DrawRectFrame(nowCursor, JVector2<float>(JGui::GetWindowSize().x, childWindowHeight), 4, JGui::GetUColor( J_GUI_COLOR::FRAME_BG), true);
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
			const JVector2<float> windowSize = JGui::GetWindowSize();
			const JVector2<float> viewSize = JVector2<float>(windowSize.x * 0.2f, childWindowHeight);
			JGui::BeginChildWindow(Private::directoryViewName.c_str(), viewSize, true, J_GUI_WINDOW_FLAG_AUTO_RESIZE);

			const bool canSelect = !searchBarHelper->HasInputData();
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
			const JVector2<float> windowSize = JGui::GetWindowSize();
			const JVector2<float> viewSize = JVector2<float>(windowSize.x * 0.8f, childWindowHeight);
			JGui::BeginChildWindow(Private::fileViewName.c_str(), viewSize, true, J_GUI_WINDOW_FLAG_AUTO_RESIZE);

			FileViewOnScreen();
			UpdatePopup(PopupSetting(fileviewPopup.get(), editorString.get()));
			JGui::EndChildWindow();
			//fileviewPopup->IsMouseInPopup()
		}
		void JWindowDirectory::DirectoryViewOnScreen(const JUserPtr<JDirectory>& directory, const bool canSelect)
		{
			bool isSelected = opendDirctory->GetGuid() == directory->GetGuid();
			if (lastUpdateOpenNewDir && opendDirctory->IsParent(directory.Get()))
				JGui::SetNextItemOpen(true);
  
			bool isNodeOpen = treeStrcture->DisplayTreeNode(JGui::CreateGuiLabel(directory, GetName()), treeStrcture->GetBaseFlag(), IsFocus(), true, isSelected);
			auto draggingResult = TryGetDraggingTarget();
			if (draggingResult.IsValid())
			{
				if (draggingResult->GetTypeInfo().IsChildOf<JDirectory>())
					RequestMoveFile(directory, Core::ConnectChildUserPtr<JDirectory>(draggingResult));
				else if (draggingResult->GetTypeInfo().IsChildOf<JResourceObject>())
					RequestMoveFile(directory, Core::ConnectChildUserPtr<JResourceObject>(draggingResult));
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
			const JVector2<float> windowSize = JGui::GetWindowSize();
			const JVector2<float> contentsSize = JVector2<float>(btnIconSize, btnIconSize + fileTitleBarSize.y);
			const JVector2<float> padding = JVector2<float>(windowSize.x * 0.015f, windowSize.y * 0.03f);
			const JVector2<float> spacing = windowSize * 0.015f;

			const JVector2<float> innerSize[2] = { CreateVec2(btnIconSize), fileTitleBarSize * 0.8f };
			const JVector2<float> innerPosition[2] = { JVector2<float>(0, 0), fileTitleBarSize * 0.1f };

			JEditorDynamicAlignCalculator<2> widgetAlignCal;
			widgetAlignCal.Update(windowSize, contentsSize, padding, spacing, innerSize, innerPosition, J_EDITOR_INNER_ALGIN_TYPE::COLUMN, JGui::GetCursorPos());
			JEditorTextAlignCalculator textAlignCal;

			const JVector4F contentsFrameColor = JGui::GetColor(J_GUI_COLOR::SEPARATOR);
			const JVector4F contentsBgColor = JGui::GetColor(J_GUI_COLOR::HEADER);
			const JVector4F iconBgColor = JGui::GetColor(J_GUI_COLOR::HEADER) + JVector4F(0, 0, 0, -0.05f);
			const JVector4F textBgColor = JGui::GetColor(J_GUI_COLOR::HEADER) + JVector4F(0.1f, 0.1f, 0.1f, 0.05f);
			const JVector4F bgRectDelta = JVector4F(0.1f, 0.1f, 0.1f, 0);
			 
			//JGui::SetTreeNodeColorSet(sColFactor);
			const JVector4F headerCol = JGui::GetColor(J_GUI_COLOR::HEADER);
			const JVector4F hovCol = JGui::GetColor(J_GUI_COLOR::HEADER_HOVERED); 

			JGui::PushColor(J_GUI_COLOR::HEADER, JVector4F::Zero());
			JGui::PushColor(J_GUI_COLOR::HEADER_HOVERED, JVector4F::Zero());
			JGui::PushColor(J_GUI_COLOR::HEADER_ACTIVE, JVector4F::Zero());
			 
			bool hasInvaildScene = false;
			const uint count = GetPreviewSceneCount();
			for (uint i = 0; i < count; ++i)
			{
				JPreviewScene* nowPreviewScene = GetPreviewScene(i);
				if (nowPreviewScene != nullptr)
				{
					JUserPtr<JObject> nowObject = nowPreviewScene->GetJObject();
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

					bool isSelected = IsSelectedObject(nowObject->GetGuid());
					//Has order dependency
					const JVector2<float> iconSize = widgetAlignCal.GetInnerContentsSize();
					widgetAlignCal.SetNextContentsPosition();
					const bool isHovered = JGui::IsMouseInRect(JGui::GetCursorScreenPos(), widgetAlignCal.GetTotalContentsSize());
					if (isHovered)
					{
						SetHoveredObject(nowObject);
						if (JGui::AnyMouseClicked(false))
							SetContentsClick(true);
					}

					const JVector4F addedColor = JGui::GetSelectableColorFactor(IsFocus(), isSelected, isHovered);  
					//draw bg frame rect
					JGui::DrawRectFrame(JGui::GetCursorScreenPos(),
						widgetAlignCal.GetTotalContentsSize(),
						contentsFrameColor + addedColor,
						5,
						true);

					//draw bg rect
					JGui::DrawRectFilledMultiColor(JGui::GetCursorScreenPos(),
						widgetAlignCal.GetTotalContentsSize(),
						contentsBgColor + addedColor,
						bgRectDelta,
						true);

					JVector2<float> preWorldCursorPos = JGui::GetCursorScreenPos();
					JVector2<float> preCursor = JGui::GetCursorPos();			 
					const std::string unqName = "##" + std::to_string(nowObject->GetGuid()) + "_Selectable";
					if (JGui::Selectable(unqName, false, J_GUI_SELECTABLE_FLAG_ALLOW_DOUBLE_CLICK | J_GUI_SELECTABLE_FLAG_ALLOW_ITEM_OVERLAP, JVector2<float>(btnIconSize, btnIconSize + fileTitleBarSize.y)))
					{
						RequestPushSelectObject(nowObject);
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
							if (JGui::GetMouseClickedCount(Core::J_MOUSE_BUTTON::LEFT) >= 2 && !JGui::IsKeyDown(Core::J_KEYCODE::CONTROL))
								RequestOpenNewDirectory(Core::GetUserPtr(jDir));
						}
					}

					TryBeginDragging(Core::GetUserPtr<Core::JIdentifier>(nowObject.Get()));
					auto draggingResult = TryGetDraggingTarget();
					if (draggingResult.IsValid() && objType == J_OBJECT_TYPE::DIRECTORY_OBJECT)
					{
						if (draggingResult->GetTypeInfo().IsChildOf<JDirectory>())
							RequestMoveFile(Core::ConnectChildUserPtr<JDirectory>(nowObject), Core::ConnectChildUserPtr<JDirectory>(draggingResult));
						else if (draggingResult->GetTypeInfo().IsChildOf<JResourceObject>())
							RequestMoveFile(Core::ConnectChildUserPtr<JDirectory>(nowObject), Core::ConnectChildUserPtr<JResourceObject>(draggingResult));
					}

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
					JGui::AddRoundedImage(info, JGui::GetCursorScreenPos(), JGui::GetCursorScreenPos() + CreateVec2(btnIconSize));

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
			}
			 
			JGui::PopColor(3);
			if (hasInvaildScene)
				DestroyInvalidPreviewScene();

			if (renameHelper->IsActivated())
			{
				JGui::SetCursorPos(renameCursorPos);
				renameHelper->UpdateMultiline(renameRectSize, false);
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
						if(dir != nullptr)
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
						importData.descIndex = 0;
						for (const auto& data : path)
						{
							Core::JFileImportHelpData pathData{ data };
							size_t fileSizeSum = 0;
							if (!JResourceObjectImporter::Instance().IsValidFormat(pathData.format))
							{
								MessageBox(0, data.c_str(), L"Is not valid format", 0);
								continue;
							}

							std::filesystem::path p{ data };
							size_t fileSize = std::filesystem::file_size(p);
							fileSizeSum += fileSize;

							if (!JWindow::HasStorageSpace(opendDirctory->GetPath(), fileSizeSum))
							{
								MessageBox(0, data.c_str(), (L"Can't find disk space " + std::to_wstring(fileSize) + L"byte").c_str(), 0);
								continue;
							}

							std::vector<J_RESOURCE_TYPE> rType = JResourceObjectImporter::Instance().DeterminFileResourceType(pathData);
							for (const auto& data : rType)
							{
								switch (data)
								{
								case JinEngine::J_RESOURCE_TYPE::MESH:
								{
									importData.importRType.push_back(data);
									importData.importDesc.push_back(std::make_unique<JMeshGeometryImportDesc>(pathData));
									break;
								}
								case JinEngine::J_RESOURCE_TYPE::TEXTURE:
								{
									importData.importRType.push_back(data);
									importData.importDesc.push_back(std::make_unique<JTextureImportDesc>(pathData));
									break;
								}
								case JinEngine::J_RESOURCE_TYPE::ANIMATION_CLIP:
								{
									importData.importRType.push_back(data);
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
					JGui::Text(std::to_string(importData.descIndex) +". " + 
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

			if (opendDirctory.IsValid() && opendDirctory->GetGuid() == newOpendDirectory->GetGuid())
				return;
			 
			//window->DC.StateStorage->SetInt(id, is_open);
			ClearPreviewGroup();
			CreateDirectoryPreview(newOpendDirectory, false);
			if (opendDirctory.IsValid())
				DirActInterface::CloseDirectory(opendDirctory);
			opendDirctory = newOpendDirectory;
			DirActInterface::OpenDirectory(opendDirctory);
			lastUpdateOpenNewDir = true; 
		}
		void JWindowDirectory::CreateDirectoryPreview(const JUserPtr<JDirectory>& directory, const bool hasNameMask, const std::wstring& mask)
		{
			const uint existPreviewCount = GetPreviewSceneCount();
			const uint directoryCount = (uint)directory->GetChildernDirctoryCount();
			for (uint i = 0; i < directoryCount; ++i)
			{
				JUserPtr<JDirectory> dir = directory->GetChildDirctory(i);
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
					if (hasOverlap)
						continue;
				}
				CreatePreviewScene(dir, J_PREVIEW_DIMENSION::TWO_DIMENTIONAL);
			}

			const uint fileCount = (uint)directory->GetFileCount();
			for (uint i = 0; i < fileCount; ++i)
			{
				JUserPtr<JFile> file = directory->GetDirectoryFile(i);
				if (file == nullptr)
					continue;

				if (hasNameMask && !JCUtil::Contain(file->GetName(), mask))
					continue;

				if (existPreviewCount > 0)
				{
					bool hasOverlap = false;
					for (uint j = 0; j < existPreviewCount; ++j)
					{
						if (GetPreviewScene(j)->GetJObject()->GetGuid() == file->GetResourceGuid())
						{
							hasOverlap = true;
							break;
						}
					}
					if (hasOverlap)
						continue;
				}
				auto preview = CreatePreviewScene(file->TryGetResourceUser(), J_PREVIEW_DIMENSION::TWO_DIMENTIONAL);
			}
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
				taskName = "Move file";
				taskDesc = JCUtil::WstrToU8Str(L"object name: " + obj->GetName() + L" " +
					static_cast<JResourceObject*>(obj.Get())->GetDirectory()->GetName() + L" to " + to->GetName());
				auto doBind = std::make_unique<MoveFileF::CompletelyBind>(*setting->moveFileF, JUserPtr<JDirectory>(to), JUserPtr<JObject>(obj));
				auto undoBind = std::make_unique<MoveFileF::CompletelyBind>(*setting->moveFileF, static_cast<JResourceObject*>(obj.Get())->GetDirectory(), JUserPtr<JObject>(obj));
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
			if (!opendDirctory.IsValid())
				opendDirctory = root;
			CreateDirectoryPreview(opendDirctory, false);
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
			JObjectFileIOHelper::_StoreHasIden(tool, opendDirctory.Get(), "OpendDirectory");
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