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
#include"../../../Interface/JEditorObjectCreationInterface.h"
#include"../../../../Utility/JCommonUtility.h"   
#include"../../../../Utility/JMathHelper.h"   
#include"../../../../Core/Guid/JGuidCreator.h" 
#include"../../../../Core/Identity/JIdenCreator.h" 
#include"../../../../Object/Component/Camera/JCamera.h" 
#include"../../../../Object/Directory/JDirectory.h"
#include"../../../../Object/Directory/JDirectoryPrivate.h"
#include"../../../../Object/Directory/JFile.h"
#include"../../../../Object/Resource/Scene/JScene.h"
#include"../../../../Object/Resource/AnimationController/JAnimationController.h"
#include"../../../../Object/Resource/Material/JMaterial.h"  
#include"../../../../Object/Resource/Scene/Preview/JPreviewScene.h" 
#include"../../../../Object/Resource/JResourceManager.h" 
#include"../../../../Object/Resource/JResourceObjectPrivate.h" 
#include"../../../../Object/Resource/JResourceObjectImporter.h" 
#include"../../../../Window/JWindow.h"
#include"../../../../Application/JApplicationProject.h"  
#include"../../../../Utility/JVectorExtend.h"
#include"../../../../../ThirdParty/imgui/imgui.h"
#include<filesystem>


//#include"../../../../Core/File/JFileIOHelper.h"  
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

		class JWindowDirectoryCreationImpl
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
			JWindowDirectoryCreationImpl(RequestRObjCreationEvF::Ptr reqRObjCreationEvPtr,
				RequestDirectoryCreationEvF::Ptr reqDirCreationEvPtr,
				RequestDestructionEvF::Ptr reqDestructionEvPtr)
			{
				reqRObjCreationEvF = std::make_unique<RequestRObjCreationEvF::Functor>(reqRObjCreationEvPtr);
				reqDirCreationEvF = std::make_unique<RequestDirectoryCreationEvF::Functor>(reqDirCreationEvPtr);
				reqDestructionEvF = std::make_unique<RequestDestructionEvF::Functor>(reqDestructionEvPtr);
			}
			~JWindowDirectoryCreationImpl()
			{
				dS.Clear();
			}
		};
		class JWindowDirectorySettingImpl
		{
		public:
			using RenameF = Core::JSFunctorType<void, JWindowDirectory*>;
			using ImportResourceF = Core::JMFunctorType<JWindowDirectory, void>;
			using CreateImportedResourceF = Core::JSFunctorType<void, JWindowDirectory*, std::vector<JUserPtr<JResourceObject>>>;
			using MoveFIleF = Core::JMFunctorType<JWindowDirectory, void, JUserPtr<JDirectory>, JUserPtr<JObject>>;
			using OpenNewDirectoryF = Core::JMFunctorType<JWindowDirectory, void, JUserPtr<JDirectory>>;
		public:
			std::unique_ptr<RenameF::Functor> renameF;
			std::unique_ptr<ImportResourceF::Functor> importResourceF;
			std::unique_ptr<CreateImportedResourceF::Functor> createImportResourceF;
			std::unique_ptr<MoveFIleF::Functor> moveFileF;
			std::unique_ptr<OpenNewDirectoryF::Functor> openNewDirF;
		public:
			std::unique_ptr<OpenNewDirectoryF::CompletelyBind> openNewDirB;
		public:
			void PushOpenNewDirBind(JUserPtr<JDirectory> dir)
			{
				openNewDirB = std::make_unique<OpenNewDirectoryF::CompletelyBind>(*openNewDirF, std::move(dir));
			}
			void ClearOpenNewDirBind()
			{
				openNewDirB.reset();
			}
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

			selectorIconSlidebarId = Core::MakeGuid();
			editorString->AddString(selectorIconSlidebarId, { "Icon Size" , u8"아이콘 크기" });

			InitializeCreationImpl();
			InitializeSettingImpl();
			BuildPopup();
		}
		JWindowDirectory::~JWindowDirectory()
		{
			creationImpl.reset();
			settingImpl.reset();
		}
		void JWindowDirectory::InitializeCreationImpl()
		{
			if (creationImpl != nullptr)
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

				JWindowDirectoryCreationImpl* impl = wndDir->creationImpl.get();
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

				JWindowDirectoryCreationImpl* impl = wndDir->creationImpl.get();
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

				JWindowDirectoryCreationImpl* impl = wndDir->creationImpl.get();
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

			creationImpl = std::make_unique<JWindowDirectoryCreationImpl>(requestCreateRObjLam, requestCreateDirectoryLam, requestDestroyLam);
			creationImpl->resourceObj.GetCreationInterface()->RegisterCreatePostProcess(creationRObjPostProccessLam);
			creationImpl->directory.GetCreationInterface()->RegisterCreatePostProcess(creationDirPostProccessLam);
			creationImpl->destructuion.GetDestruectionInterface()->RegisterDestroyPreProcess(destructionPreProccessLam);

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
				JICI::Create<JDirectory>(parent);
			};

			creationImpl->resourceObj.GetCreationInterface()->RegisterCanCreationF(canCreationRobjLam);
			creationImpl->resourceObj.GetCreationInterface()->RegisterObjectCreationF(creationRobjLam);
			creationImpl->directory.GetCreationInterface()->RegisterCanCreationF(canCreationDirLam);
			creationImpl->directory.GetCreationInterface()->RegisterObjectCreationF(creationDirLam);
		}
		void JWindowDirectory::InitializeSettingImpl()
		{
			if (settingImpl != nullptr)
				return;

			auto createImportedResourceFLam = [](JWindowDirectory* wndDir, std::vector<JUserPtr<JResourceObject>> rVec)
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

			using ImportResourceF = JWindowDirectorySettingImpl::ImportResourceF;
			using CreateImportedResourceF = JWindowDirectorySettingImpl::CreateImportedResourceF;
			using RenameF = JWindowDirectorySettingImpl::RenameF;
			using MoveFIleF = JWindowDirectorySettingImpl::MoveFIleF;
			using OpenNewDirectoryF = JWindowDirectorySettingImpl::OpenNewDirectoryF;

			settingImpl = std::make_unique<JWindowDirectorySettingImpl>();
			settingImpl->importResourceF = std::make_unique<ImportResourceF::Functor>(&JWindowDirectory::ImportFile, this);
			settingImpl->createImportResourceF = std::make_unique<CreateImportedResourceF::Functor>(createImportedResourceFLam);
			settingImpl->renameF = std::make_unique<RenameF::Functor>(renameLam);
			settingImpl->moveFileF = std::make_unique< MoveFIleF::Functor>(&JWindowDirectory::MoveFile, this);
			settingImpl->openNewDirF = std::make_unique<OpenNewDirectoryF::Functor>(&JWindowDirectory::OpenNewDirectory, this);
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

			using RequestRObjCreationEvF = JWindowDirectoryCreationImpl::RequestRObjCreationEvF;
			using RequestDirCreationEvF = JWindowDirectoryCreationImpl::RequestDirectoryCreationEvF;
			using RequestDestructionEvF = JWindowDirectoryCreationImpl::RequestDestructionEvF;

			createMaterialNode->RegisterSelectBind(std::make_unique<RequestRObjCreationEvF::CompletelyBind>(*creationImpl->reqRObjCreationEvF, this, J_RESOURCE_TYPE::MATERIAL));
			createSceneNode->RegisterSelectBind(std::make_unique<RequestRObjCreationEvF::CompletelyBind>(*creationImpl->reqRObjCreationEvF, this, J_RESOURCE_TYPE::SCENE));
			createAnimationControllerNode->RegisterSelectBind(std::make_unique<RequestRObjCreationEvF::CompletelyBind>(*creationImpl->reqRObjCreationEvF, this, J_RESOURCE_TYPE::ANIMATION_CONTROLLER));
			createDirctoryInFileViewNode->RegisterSelectBind(std::make_unique<RequestDirCreationEvF::CompletelyBind>(*creationImpl->reqDirCreationEvF, this));
			destroyNode->RegisterSelectBind(std::make_unique<RequestDestructionEvF::CompletelyBind>(*creationImpl->reqDestructionEvF, this));
			destroyNode->RegisterEnableBind(std::make_unique<JEditorPopupNode::EnableF::CompletelyBind>(*GetPassSelectedAboveOneFunctor(), this));

			using ImportResourceF = JWindowDirectorySettingImpl::ImportResourceF;
			using RenameF = JWindowDirectorySettingImpl::RenameF;

			importNode->RegisterSelectBind(std::make_unique<ImportResourceF::CompletelyBind>(*settingImpl->importResourceF));
			renameFileNode->RegisterSelectBind(std::make_unique<RenameF::CompletelyBind>(*settingImpl->renameF, this));
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
		void JWindowDirectory::Initialize()
		{
			const std::wstring contentPath = JApplicationProject::ContentsPath();
			root = _JResourceManager::Instance().GetDirectory(contentPath);
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
						CreateDirectoryPreview(opendDirctory, false);
					}
					else
					{
						ClearPreviewGroup();
						CreateAllDirectoryPreview(root, true, JCUtil::U8StrToWstr(searchBarHelper->GetInputData()));
					}
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
			DirectoryViewOnScreen(root, canSelect);
			lastUpdateOpenNewDir = false;

			if (settingImpl->openNewDirB != nullptr)
			{
				settingImpl->openNewDirB->InvokeCompletelyBind();
				settingImpl->openNewDirB.reset();
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
			UpdatePopup(PopupSetting(fileviewPopup.get(), editorString.get()));
			JImGuiImpl::EndChildWindow();
			//fileviewPopup->IsMouseInPopup()
		}
		void JWindowDirectory::DirectoryViewOnScreen(const JUserPtr<JDirectory>& directory, const bool canSelect)
		{
			ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_SpanAvailWidth |
				ImGuiTreeNodeFlags_Framed;

			bool isSelected = opendDirctory->GetGuid() == directory->GetGuid();
			if (isSelected && canSelect)
				SetTreeNodeColor(GetSelectedColorFactor());
			if (lastUpdateOpenNewDir && opendDirctory->IsParent(directory.Get()))
				ImGui::SetNextItemOpen(true);

			bool isNodeOpen = JImGuiImpl::TreeNodeEx(JCUtil::WstrToU8Str(directory->GetName()).c_str(), baseFlags);
			auto draggingResult = TryGetDraggingTarget();
			if (draggingResult.IsValid())
			{
				if (draggingResult->GetTypeInfo().IsChildOf<JDirectory>())
					RequestMoveFile(directory, Core::ConnectChildUserPtr<JDirectory>(draggingResult));
				else if (draggingResult->GetTypeInfo().IsChildOf<JResourceObject>())
					RequestMoveFile(directory, Core::ConnectChildUserPtr<JResourceObject>(draggingResult));
			}
			if (isSelected && canSelect)
				SetTreeNodeColor(GetSelectedColorFactor() * -1);
			if (isNodeOpen)
			{
				if (ImGui::IsItemClicked(0) && canSelect && !isSelected)
					settingImpl->PushOpenNewDirBind(directory);

				const uint childDirCount = directory->GetChildernDirctoryCount();
				for (uint i = 0; i < childDirCount; ++i)
				{
					JUserPtr<JDirectory> child = directory->GetChildDirctory(i);
					if (child == nullptr)
						continue;
					DirectoryViewOnScreen(child, canSelect);
				}
				JImGuiImpl::TreePop();
			} 
		}
		void JWindowDirectory::FileViewOnScreen()
		{
			const JVector2<float> windowSize = ImGui::GetWindowSize();
			const JVector2<float> contentsSize = JVector2<float>(btnIconSize, btnIconSize + fileTitleBarSize.y);
			const JVector2<float> padding = JVector2<float>(windowSize.x * 0.015f, windowSize.y * 0.03f);
			const JVector2<float> spacing = windowSize * 0.015f;

			const JVector2<float> innerSize[2] = { CreateVec2(btnIconSize), fileTitleBarSize * 0.8f };
			const JVector2<float> innerPosition[2] = { JVector2<float>(0, 0), fileTitleBarSize * 0.1f };

			JEditorDynamicAlignCalculator<2> widgetAlignCal;
			widgetAlignCal.Update(windowSize, contentsSize, padding, spacing, innerSize, innerPosition, J_EDITOR_INNER_ALGIN_TYPE::COLUMN, ImGui::GetCursorPos());
			JEditorTextAlignCalculator textAlignCal;

			const ImU32 contentsFrameColor = JImGuiImpl::GetUColor(ImGuiCol_Separator);
			const ImU32 contentsBgColor = JImGuiImpl::GetUColor(ImGuiCol_Header);
			const ImU32 iconBgColor = JImGuiImpl::ConvertUColor(JImGuiImpl::GetColor(ImGuiCol_Header) + JVector4<float>(0, 0, 0, -0.05f));
			const ImU32 textBgColor = JImGuiImpl::ConvertUColor(JImGuiImpl::GetColor(ImGuiCol_Header) + JVector4<float>(0.1f, 0.1f, 0.1f, 0.05f));
			const ImU32 bgRectDelta = IM_COL32(25, 25, 25, 0);

			const JVector4<float> preHeaderCol = JImGuiImpl::GetColor(ImGuiCol_Header);
			const JVector4<float> preHeaderHovCol = JImGuiImpl::GetColor(ImGuiCol_HeaderHovered);
			const JVector4<float> preHeaderActCol = JImGuiImpl::GetColor(ImGuiCol_HeaderActive);

			const JVector4<float> sColFactor = IsFocus() ? JImGuiImpl::GetSelectColorFactor() : JImGuiImpl::GetOffFocusSelectedColorFactor();
			SetTreeNodeColor(sColFactor);
			const ImU32 headerCol = JImGuiImpl::GetUColor(ImGuiCol_Header);
			const ImU32 hovCol = JImGuiImpl::GetUColor(ImGuiCol_HeaderHovered);
			const ImU32 hoveredAddedColor = JImGuiImpl::ConvertUColor(JImGuiImpl::GetSelectColorFactor());
			const ImU32 selectedAddedColor = JImGuiImpl::ConvertUColor(sColFactor);

			JImGuiImpl::SetColor(JVector4<float>(0, 0, 0, 0), ImGuiCol_Header);
			JImGuiImpl::SetColor(JVector4<float>(0, 0, 0, 0), ImGuiCol_HeaderHovered);
			JImGuiImpl::SetColor(JVector4<float>(0, 0, 0, 0), ImGuiCol_HeaderActive);

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
					const bool isHorvered = JImGuiImpl::IsMouseInRect(ImGui::GetCurrentWindow()->DC.CursorPos, widgetAlignCal.GetTotalContentsSize());
					if (isHorvered)
					{
						SetHoveredObject(nowObject);
						if (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1))
							SetContentsClick(true);
					}					 

					ImU32 addedColor = IM_COL32(0, 0, 0, 0);
					if (isHorvered)
						addedColor = hoveredAddedColor;
					if (isSelected)
						addedColor = selectedAddedColor;

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
							if (ImGui::GetMouseClickedCount(0) >= 2 && !ImGui::GetIO().KeyCtrl)
								settingImpl->PushOpenNewDirBind(Core::GetUserPtr(jDir));
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
					JImGuiImpl::Image(nowPreviewScene->GetPreviewCamera().Get(), Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, JVector2<float>(btnIconSize, btnIconSize));

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

			JImGuiImpl::SetColor(preHeaderCol, ImGuiCol_Header);
			JImGuiImpl::SetColor(preHeaderHovCol, ImGuiCol_HeaderHovered);
			JImGuiImpl::SetColor(preHeaderActCol, ImGuiCol_HeaderActive);
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
			if (JWindow::SelectFile(importFilePath, L"please, select resource file"))
			{
				Core::JFileImportHelpData pathData{ importFilePath };
				if (opendDirctory.IsValid())
				{
					if (JResourceObjectImporter::Instance().IsValidFormat(pathData.format))
					{
						std::filesystem::path p{ importFilePath };
						size_t fileSize = std::filesystem::file_size(p);
						if (JWindow::HasStorageSpace(opendDirctory->GetPath(), fileSize))
						{
							using CreateImportedResourceF = JWindowDirectorySettingImpl::CreateImportedResourceF;

							std::vector<JUserPtr<JResourceObject>> res = JResourceObjectImporter::Instance().ImportResource(opendDirctory, pathData);
							auto createImpR = std::make_unique<CreateImportedResourceF::CompletelyBind>(*settingImpl->createImportResourceF,
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
			using MoveFileF = JWindowDirectorySettingImpl::MoveFIleF;
			if (obj->GetObjectType() == J_OBJECT_TYPE::DIRECTORY_OBJECT)
			{
				taskName = "Move directory";
				taskDesc = JCUtil::WstrToU8Str(L"object name: " + obj->GetName() + L" " +
					static_cast<JDirectory*>(obj.Get())->GetParent()->GetName() + L" to " +  to->GetName());

				auto doBind = std::make_unique<MoveFileF::CompletelyBind>(*settingImpl->moveFileF, JUserPtr<JDirectory>(to), JUserPtr<JObject>(obj));
				auto undoBind = std::make_unique<MoveFileF::CompletelyBind>(*settingImpl->moveFileF, static_cast<JDirectory*>(obj.Get())->GetParent(), JUserPtr<JObject>(obj));
				auto evStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorTSetBindFuncEvStruct>
					(taskName, taskDesc, GetOwnerPageType(), std::move(doBind), std::move(undoBind)));
				AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::T_BIND_FUNC, evStruct);
			}
			else if (obj->GetObjectType() == J_OBJECT_TYPE::RESOURCE_OBJECT)
			{
				taskName = "Move file";
				taskDesc = JCUtil::WstrToU8Str(L"object name: " + obj->GetName() + L" " +
					static_cast<JResourceObject*>(obj.Get())->GetDirectory()->GetName() + L" to " + to->GetName());
				auto doBind = std::make_unique<MoveFileF::CompletelyBind>(*settingImpl->moveFileF, JUserPtr<JDirectory>(to), JUserPtr<JObject>(obj));
				auto undoBind = std::make_unique<MoveFileF::CompletelyBind>(*settingImpl->moveFileF, static_cast<JResourceObject*>(obj.Get())->GetDirectory(), JUserPtr<JObject>(obj));
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
			JEditorWindow::DoDeActivate();
			DeRegisterListener();
			ClearPreviewGroup();
		}
		void JWindowDirectory::OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)
		{
			JEditorWindow::OnEvent(senderGuid, eventType, eventStruct);
			if (senderGuid == GetGuid())
				return;

			if (eventType == J_EDITOR_EVENT::MOUSE_CLICK)
			{
				if (eventStruct->pageType == GetOwnerPageType())
					fileviewPopup->SetOpen(false);
			}
		}
	}
}