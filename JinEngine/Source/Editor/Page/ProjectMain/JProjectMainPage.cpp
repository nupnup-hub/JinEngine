#include"JProjectMainPage.h" 
#include"Window/JLogViewer.h" 
#include"Window/JWindowDirectory.h"  
#include"Window/JGraphicOptionSetting.h" 
#include"Window/JWindowStateViewer.h" 
#include"../JEditorAttribute.h" 
#include"../JEditorPageShareData.h"
#include"../CommonWindow/Debug/JStringConvertTest.h" 
#include"../CommonWindow/View/JSceneViewer.h"
#include"../CommonWindow/View/JSceneObserver.h"
#include"../CommonWindow/Detail/JObjectDetail.h"
#include"../CommonWindow/Explorer/JObjectExplorer.h"
#include"../CommonWindow/Debug/JGraphicResourceWatcher.h"
#include"../CommonWindow/Debug/JApplicationWatcher.h"
#include"../../Align/JEditorAlignCalculator.h"
#include"../../Popup/JEditorPopupWindow.h"
#include"../../Event/JEditorEvent.h"
#include"../../Menubar/JEditorMenuBar.h" 
#include"../../Gui/JGui.h"  
#include"../../../Object/GameObject/JGameObject.h"
#include"../../../Object/Directory/JDirectory.h"
#include"../../../Object/Directory/JFile.h"
#include"../../../Object/Resource/JResourceManager.h"
#include"../../../Object/Resource/Scene/JScene.h"
#include"../../../Object/Resource/Scene/JSceneManager.h"  
#include"../../../Application/JApplicationProjectPrivate.h"
#include"../../../Core/Math/JVectorExtend.h" 

namespace JinEngine
{
	namespace Editor
	{
		JProjectMainPage::JProjectMainPage(std::unique_ptr<JEditorProjectInterface>&& newProjInterface)
			:JEditorPage("JProjectMainPage",
				std::make_unique<JEditorAttribute>(),
				Core::AddSQValueEnum(J_EDITOR_PAGE_IS_MAIN_PAGE, J_EDITOR_PAGE_SUPPORT_DOCK)),
			projInterface(std::move(newProjInterface))
		{   
			std::vector<WindowInitInfo> openInfo;
			openInfo.emplace_back("Window Directory##JProjectMainPage");
			openInfo.emplace_back("Object Explorer##JProjectMainPage");
			openInfo.emplace_back("Object Detail##JProjectMainPage");
			openInfo.emplace_back("Scene Observe##JProjectMainPage");
			openInfo.emplace_back("Scene Viewer##JProjectMainPage");
			openInfo.emplace_back("Log Viewer##JProjectMainPage");
			openInfo.emplace_back("Graphic Resource Watcher##JProjectMainPage");
			openInfo.emplace_back("String Convert Test##JProjectMainPage");
			openInfo.emplace_back("Application Watcher##JProjectMainPage");
			openInfo.emplace_back("Graphic Option##JProjectMainPage");
			openInfo.emplace_back("Window State##JProjectMainPage");

			J_EDITOR_WINDOW_FLAG defaultFlag = J_EDITOR_WINDOW_SUPPORT_WINDOW_CLOSING;
			J_EDITOR_WINDOW_FLAG noneDockFlag = Core::AddSQValueEnum(defaultFlag, J_EDITOR_WINDOW_SUPPORT_MAXIMIZE);
			J_EDITOR_WINDOW_FLAG dockFlag = Core::AddSQValueEnum(defaultFlag, J_EDITOR_WINDOW_SUPROT_DOCK);

			J_EDITOR_WINDOW_FLAG wndDirFlag = Core::AddSQValueEnum(dockFlag, J_EDITOR_WINDOW_SUPPORT_POPUP, J_EDITOR_WINDOW_SUPPORT_SELECT);
			J_EDITOR_WINDOW_FLAG objExplorerFlag = Core::AddSQValueEnum(dockFlag, J_EDITOR_WINDOW_SUPPORT_POPUP, J_EDITOR_WINDOW_SUPPORT_SELECT, J_EDITOR_WINDOW_LISTEN_OTHER_WINDOW_SELECT);
			J_EDITOR_WINDOW_FLAG sceneObserverFlag = Core::AddSQValueEnum(dockFlag, J_EDITOR_WINDOW_SUPPORT_POPUP, J_EDITOR_WINDOW_SUPPORT_SELECT, J_EDITOR_WINDOW_LISTEN_OTHER_WINDOW_SELECT,J_EDITOR_WINDOW_SUPPORT_MAXIMIZE );
			J_EDITOR_WINDOW_FLAG sceneViewerFlag = Core::AddSQValueEnum(dockFlag, J_EDITOR_WINDOW_SUPPORT_MAXIMIZE);
 
			const size_t explorerGuid = JEditorWindow::CalculateGuid(openInfo[1].GetName());
			const size_t observerGuid = JEditorWindow::CalculateGuid(openInfo[3].GetName());
			using GuidVec = std::vector<size_t>;

			windowDirectory = std::make_unique<JWindowDirectory>(openInfo[0].GetName(), openInfo[0].MakeAttribute(), GetPageType(), wndDirFlag);
			objectExplorer = std::make_unique<JObjectExplorer>(openInfo[1].GetName(), openInfo[1].MakeAttribute(), GetPageType(), objExplorerFlag, GuidVec{ observerGuid });
			objectDetail = std::make_unique<JObjectDetail>(openInfo[2].GetName(), openInfo[2].MakeAttribute(), GetPageType(), dockFlag);
			sceneObserver = std::make_unique<JSceneObserver>(openInfo[3].GetName(), openInfo[3].MakeAttribute(), GetPageType(), sceneObserverFlag, Constants::GetAllObserverSetting(), GuidVec{ explorerGuid });
			sceneViewer = std::make_unique<JSceneViewer>(openInfo[4].GetName(), openInfo[4].MakeAttribute(), GetPageType(), sceneViewerFlag);
			logViewer = std::make_unique<JLogViewer>(openInfo[5].GetName(), openInfo[5].MakeAttribute(), GetPageType(), dockFlag);
			graphicResourceWatcher = std::make_unique<JGraphicResourceWatcher>(openInfo[6].GetName(), openInfo[6].MakeAttribute(), GetPageType(), noneDockFlag);
			stringConvertTest = std::make_unique<JStringConvertTest>(openInfo[7].GetName(), openInfo[7].MakeAttribute(), GetPageType(), noneDockFlag);
			appWatcher = std::make_unique<JApplicationWatcher>(openInfo[8].GetName(), openInfo[8].MakeAttribute(), GetPageType(), noneDockFlag);
			graphicOptionSetting = std::make_unique<JGraphicOptionSetting>(openInfo[9].GetName(), openInfo[9].MakeAttribute(), GetPageType(), noneDockFlag);
			wndStateViewer = std::make_unique<JWindowStateViewer>(openInfo[10].GetName(), openInfo[10].MakeAttribute(), GetPageType(), noneDockFlag);
 
			sceneObserver->SetScenePlayProccess(std::make_unique<JSceneObserver::BeginScenePlayF::Functor>(&JProjectMainPage::BeginScenePlay, this),
				std::make_unique<JSceneObserver::BeginScenePlayF::Functor>(&JProjectMainPage::EndScenePlay, this));

			std::vector<JEditorWindow*> windows
			{
				windowDirectory.get(),
				objectExplorer.get(),
				objectDetail.get(),
				sceneObserver.get(),
				sceneViewer.get(),
				logViewer.get(), 
				graphicResourceWatcher.get(),
				stringConvertTest.get(),
				appWatcher.get(),
				graphicOptionSetting.get(),
				wndStateViewer.get()
			};
			AddWindow(windows);

			closePopup = std::make_unique<JEditorCloseConfirmPopup>();
			std::vector<JEditorPopupWindow*> popupWnds
			{
				closePopup.get()
			};
			AddPopupWindow(popupWnds);

			auto openFunc = [](JProjectMainPage* mainPage)
			{
				bool hasMoified = false;
				auto modVec = mainPage->GetModifiedObjectInfoVec();
				for (const auto& data : modVec)
				{
					if (data->isModified)
					{
						hasMoified = true;
						break;
					}
				}			
				if (!hasMoified)
				{
					mainPage->RequestCloseConfirmPopup(false);					
				}
			};
			auto confirmFunc = [](JProjectMainPage* mainPage){mainPage->RequestCloseConfirmPopup(false);};
			auto cancelFunc = [](JProjectMainPage* mainPage){mainPage->RequestCloseConfirmPopup(true);};
			auto conetnsFunc = [](JProjectMainPage* mainPage)
			{
				constexpr uint columnCount = 4;
				constexpr uint textCount = 3;
				const float fontSize = JGui::GetFontSize();
				const JVector2<float> wndSize = JGui::GetWindowSize();
				const JVector2<float> listSize = JVector2<float>(wndSize.x * 0.85f, wndSize.y * 0.65f);
				const JVector2<float> padding = listSize * 0.002f;
				const JVector2<float> contentsSize = JVector2<float>(listSize.x - padding.x * 2, fontSize * 2); 
				const JVector2<float> innerSize[columnCount] =
				{
					JVector2<float>{contentsSize.x * 0.25f,contentsSize.y },
					JVector2<float>{contentsSize.x * 0.375f,contentsSize.y },
					JVector2<float>{contentsSize.x * 0.275f,contentsSize.y },
					JVector2<float>{contentsSize.x * 0.1f,contentsSize.y }
				};
				const std::string label[columnCount] =
				{
					"Name",
					"Path",
					"Type",
					"Store"
				};

				JGui::SetCursorPosX(wndSize.x * 0.075f); 
				JGui::BeginListBox("##ModifiedResource_ProjectMainPage", listSize);
				JGui::Separator();

				JEditorDynamicAlignCalculator<4> alignCal;
				alignCal.Update(listSize, contentsSize, padding, CreateVec2(0), innerSize, J_EDITOR_INNER_ALGIN_TYPE::ROW, JGui::GetCursorPos());

				for (uint i = 0; i < columnCount; ++i)
				{
					alignCal.SetNextContentsPosition();
					JGui::Text(label[i]);
				}

				JEditorTextAlignCalculator textCal;
				auto modVec = mainPage->GetModifiedObjectInfoVec();
				for (auto& data : modVec)
				{
					std::string strArr[textCount]{ "","","" };
					JUserPtr<Core::JIdentifier> obj = Core::GetUserPtr<Core::JIdentifier>(data->typeGuid, data->objectGuid);
					if (obj == nullptr)
						continue;

					const bool isResource = obj->GetTypeInfo().IsChildOf(JResourceObject::StaticTypeInfo());
					const bool isDir = !isResource ? obj->GetTypeInfo().IsChildOf(JDirectory::StaticTypeInfo()) : false;
					if (!isResource && !isDir)
						continue;

					if (obj != nullptr)
					{
						strArr[0] = JCUtil::WstrToU8Str(obj->GetName());	
						if (isResource)
							strArr[1] = JCUtil::WstrToU8Str(static_cast<JResourceObject*>(obj.Get())->GetPath());
						else
							strArr[1] = JCUtil::WstrToU8Str(static_cast<JDirectory*>(obj.Get())->GetPath());
						strArr[2] = obj->GetTypeInfo().NameWithOutModifier();
					}
					else
					{
						JUserPtr<JFile> file = JDirectory::SearchFile(data->objectGuid);
						if(file == nullptr)
							continue;

						strArr[0] = JCUtil::WstrToU8Str(file->GetName());
						strArr[1] = JCUtil::WstrToU8Str(file->GetPath());
						strArr[2] = file->GetResourceTypeInfo().NameWithOutModifier();
					}
					 
					for (uint j = 0; j < textCount; ++j)
					{
						textCal.Update(strArr[j], alignCal.GetInnerContentsSize(), true);
						alignCal.SetNextContentsPosition();
						JGui::Text(textCal.LeftAligned());
					}
					alignCal.SetNextContentsPosition();
					JGui::CheckBox("##ModifiedResource_CheckBox" + strArr[0], data->canStore);
				}
				JGui::EndListBox();
			};

			closePopupOpenF = std::make_unique<ClosePopupOpenF::Functor>(openFunc);
			closePopupConfirmF = std::make_unique<ClosePopupConfirmF::Functor>(confirmFunc);
			closePopupCancelF = std::make_unique<ClosePopupCancelF::Functor>(cancelFunc);
			closePopupContetnsF = std::make_unique<ClosePopupContentsF::Functor>(conetnsFunc);

			closePopup->RegisterBind(J_EDITOR_POPUP_WINDOW_FUNC_TYPE::OPEN_POPUP,
				std::make_unique<ClosePopupOpenF::CompletelyBind>(*closePopupOpenF, this));
			closePopup->RegisterBind(J_EDITOR_POPUP_WINDOW_FUNC_TYPE::CONFIRM, 
				std::make_unique<ClosePopupConfirmF::CompletelyBind>(*closePopupConfirmF, this));
			closePopup->RegisterBind(J_EDITOR_POPUP_WINDOW_FUNC_TYPE::CANCEL,
				std::make_unique<ClosePopupCancelF::CompletelyBind>(*closePopupCancelF, this));
			closePopup->RegisterBind(J_EDITOR_POPUP_WINDOW_FUNC_TYPE::CONTENTS,
				std::make_unique<ClosePopupContentsF::CompletelyBind>(*closePopupContetnsF, this));
			closePopup->SetDesc(u8"변경된 자원");

		}
		JProjectMainPage::~JProjectMainPage()
		{
			//(*storeProjectF)();
			//ClearModifiedInfoStructure(); 
		}
		J_EDITOR_PAGE_TYPE JProjectMainPage::GetPageType()const noexcept
		{
			return J_EDITOR_PAGE_TYPE::PROJECT_MAIN;
		}
		void JProjectMainPage::SetInitWindow()
		{
			uint currOpWndCount = GetOpenWindowCount();
			for (uint i = 0; i < currOpWndCount; ++i)
				CloseWindow(GetOpenWindow(i));
			 
			OpenWindow(windowDirectory.get());
			OpenWindow(objectExplorer.get());
			OpenWindow(objectDetail.get());
			OpenWindow(sceneObserver.get());
			OpenWindow(sceneViewer.get());
			OpenWindow(logViewer.get());

			currOpWndCount = GetOpenWindowCount();
			for (uint i = 0; i < currOpWndCount; ++i)
				GetOpenWindow(i)->SetLastActivated(true);
		}
		void JProjectMainPage::Initialize()
		{ 
			JEditorPage::Initialize();
			windowDirectory->Initialize();
			objectExplorer->Initialize(_JSceneManager::Instance().GetFirstScene()->GetRootGameObject());
			sceneViewer->Initialize(_JSceneManager::Instance().GetFirstScene());
			sceneObserver->Initialize(_JSceneManager::Instance().GetFirstScene(), L"Editor_ObserverCamera");
			logViewer->Initialize();
			wndStateViewer->Initialize(GetWindowVec());
			BuildMenuNode();
		}
		void JProjectMainPage::UpdatePage()
		{
			JGui::SetNextWindowSize(JGui::GetMainWorkSize());
			JGui::SetNextWindowPos(JGui::GetMainWorkPos());
			 
			J_GUI_DOCK_NODE_FLAG_ dockspaceFlag = J_GUI_DOCK_NODE_FLAG_NO_WINDOW_MENU_BUTTON;  
			J_GUI_WINDOW_FLAG_ guiWindowFlag = J_GUI_WINDOW_FLAG_NO_NAV_INPUT |
				J_GUI_WINDOW_FLAG_NO_BRING_TO_FRONT_ON_FOCUS | J_GUI_WINDOW_FLAG_NO_BACKGROUND |
				J_GUI_WINDOW_FLAG_NO_TITLE_BAR | J_GUI_WINDOW_FLAG_NO_SCROLL_BAR | J_GUI_WINDOW_FLAG_NO_COLLAPSE;

			//if (!JGui::IsFullScreen())
			//	dockspaceFlag &= ~JGuiDockNodeFlags_PassthruCentralNode;

			EnterPage(guiWindowFlag);
			JGui::SetFont(J_GUI_FONT_TYPE::MEDIUM);
			JGui::PushFont();
			if (HasDockNodeSpace())
				UpdateDockSpace(dockspaceFlag);
			else
				BuildDockNode();
			menuBar->Update(true);
			ClosePage();

			UpdateOpenWindow(); 
			JGui::PopFont();  
		}
		bool JProjectMainPage::IsValidOpenRequest(const JUserPtr<Core::JIdentifier>& selectedObj)noexcept
		{
			return true;
		}
		void JProjectMainPage::RequestCloseConfirmPopup(const bool isCancel)
		{			 
			AddEventNotification(*JEditorEvent::EvInterface(),
				GetGuid(),
				J_EDITOR_EVENT::CLOSE_POPUP_WINDOW,
				JEditorEvent::RegisterEvStruct(std::make_unique<JEditorClosePopupWindowEvStruct>(J_EDITOR_POPUP_WINDOW_TYPE::CLOSE_CONFIRM, GetPageType())));

			auto confirmLam = [](JProjectMainPage* page) {(*page->projInterface->confirmCloseProjectF)(); };
			auto cancelLam = [](JProjectMainPage* page) {(*page->projInterface->cancelCloseProjectF)(); };
			using CloseF = Core::JSFunctorType<void, JProjectMainPage*>;
			std::unique_ptr<CloseF::CompletelyBind> closePopupB;
			if(isCancel)
				closePopupB = std::make_unique<CloseF::CompletelyBind>(std::make_unique<CloseF::Functor>(cancelLam), this);
			else
				closePopupB = std::make_unique<CloseF::CompletelyBind>(std::make_unique<CloseF::Functor>(confirmLam), this);
			
			closePopup->RegisterBind(J_EDITOR_POPUP_WINDOW_FUNC_TYPE::CLOSE_POPUP, std::move(closePopupB));
		}
		void JProjectMainPage::BuildDockNode()
		{
			static constexpr int dockCount = 7;		//page(1) + window(6) 
			std::vector<std::unique_ptr<JGuiDockBuildNode>> dockVec(dockCount);
			dockVec[0] = JGuiDockBuildNode::CreateRootNode(GetName(), GetDockNodeName());
			dockVec[1] = JGuiDockBuildNode::CreateNode(windowDirectory->GetName(), 1, 0, J_GUI_CARDINAL_DIR::DOWN, 0.7f);
			dockVec[2] = JGuiDockBuildNode::CreateNode(objectDetail->GetName(), 2, 0, J_GUI_CARDINAL_DIR::RIGHT, 0.3f);
			dockVec[3] = JGuiDockBuildNode::CreateNode(objectExplorer->GetName(), 3, 2, J_GUI_CARDINAL_DIR::LEFT, 0.5f);
			dockVec[4] = JGuiDockBuildNode::CreateNode(logViewer->GetName(), 4, 3, J_GUI_CARDINAL_DIR::DOWN, 0.4f);
			dockVec[5] = JGuiDockBuildNode::CreateNode(sceneViewer->GetName(), 5, 1, J_GUI_CARDINAL_DIR::UP, 0.7f);
			dockVec[6] = JGuiDockBuildNode::CreateNode(sceneObserver->GetName(), 6, 5, J_GUI_CARDINAL_DIR::NONE, 0);
			JGui::BuildDockHirechary(dockVec); 
		}
		void JProjectMainPage::BuildMenuNode()
		{
			std::unique_ptr<JEditorMenuNode> rootNode = std::make_unique<JEditorMenuNode>("Root", true, false, false);

			// root Child
			std::unique_ptr<JEditorMenuNode> fileNode = std::make_unique<JEditorMenuNode>("JFile", false, false, false, nullptr, rootNode.get());
			std::unique_ptr<JEditorMenuNode> windowNode = std::make_unique<JEditorMenuNode>("Window", false, false, false, nullptr, rootNode.get());
			std::unique_ptr<JEditorMenuNode> graphicNode = std::make_unique<JEditorMenuNode>("Graphic", false, false, false, nullptr, rootNode.get());

			//file Child
			std::unique_ptr<JEditorMenuNode> saveNode = std::make_unique<JEditorMenuNode>("SaveProject", false, true, false, nullptr, fileNode.get());
			saveNode->RegisterBindHandle(std::make_unique<JEditorProjectInterface::StoreProjectF::CompletelyBind>(*projInterface->storeProjectF));
			std::unique_ptr<JEditorMenuNode> loadNode = std::make_unique<JEditorMenuNode>("LoadProject", false, true, false, nullptr, fileNode.get());
			loadNode->RegisterBindHandle(std::make_unique<JEditorProjectInterface::LoadOtherProjectF::CompletelyBind>(*projInterface->loadOtherProjectF));
			std::unique_ptr<JEditorMenuNode> buildNode = std::make_unique<JEditorMenuNode>("BuildProject", false, true, false, nullptr, fileNode.get());
		
			JEditorMenuNode* windowNodePtr = windowNode.get();
			JEditorMenuNode* graphicNodePtr = graphicNode.get();
			menuBar = std::make_unique<JEditorMenuBar>(std::move(rootNode), true);
			menuBar->AddNode(std::move(fileNode));
			menuBar->AddNode(std::move(windowNode));
			menuBar->AddNode(std::move(graphicNode));
			menuBar->AddNode(std::move(saveNode));
			menuBar->AddNode(std::move(loadNode));
			menuBar->AddNode(std::move(buildNode)); 

			std::vector<JEditorWindow*> wndVec = GetWindowVec();	
			for (const auto& data : wndVec)
			{
				std::unique_ptr<JEditorMenuNode> newNode = std::make_unique<JEditorMenuNode>(data->GetName(),
					false, true, false,
					data->GetOpenPtr(),
					windowNodePtr);

				auto openBind = std::make_unique<OpenEditorWindowF::CompletelyBind>(GetRequestOpenEditorWindowFunctorPtr(), *this, data->GetName());
				auto closeBind = std::make_unique<CloseEditorWindowF::CompletelyBind>(GetRequestCloseEditorWindowFunctorPtr(), *this, data->GetName());
				newNode->RegisterBindHandle(std::move(openBind), std::move(closeBind));
				menuBar->AddNode(std::move(newNode));
			}

			std::unique_ptr<JEditorMenuNode> grapicOptionNode = std::make_unique<JEditorMenuNode>("Graphic Option",
				false, true, false,
				graphicOptionSetting->GetOpenPtr(),
				graphicNodePtr);
   
			auto openBind = std::make_unique<OpenEditorWindowF::CompletelyBind>(GetRequestOpenEditorWindowFunctorPtr(), *this, graphicOptionSetting->GetName());
			auto closeBind = std::make_unique<CloseEditorWindowF::CompletelyBind>(GetRequestCloseEditorWindowFunctorPtr(), *this, graphicOptionSetting->GetName());
			grapicOptionNode->RegisterBindHandle(std::move(openBind), std::move(closeBind));

			menuBar->AddNode(std::move(grapicOptionNode)); 
		}
		void JProjectMainPage::BeginScenePlay()
		{
			(*projInterface->storeProjectF)();
		}
		void JProjectMainPage::EndScenePlay()
		{
			JModifedObjectInterface::ClearModifiedInfoStructure();
			(*projInterface->reLoadProjectF)();
		}
	}
}