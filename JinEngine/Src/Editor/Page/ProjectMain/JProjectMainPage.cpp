#include"JProjectMainPage.h"
#include"Window/JAnimationControllerEditor.h"
#include"Window/JLogViewer.h" 
#include"Window/JWindowDirectory.h"  
#include"../JEditorAttribute.h" 
#include"../JEditorPageShareData.h"
#include"../SimpleWindow/JGraphicOptionSetting.h" 
#include"../../Popup/JEditorPopupWindow.h"
#include"../CommonWindow/Debug/JStringConvertTest.h" 
#include"../CommonWindow/View/JSceneViewer.h"
#include"../CommonWindow/View/JSceneObserver.h"
#include"../CommonWindow/Detail/JObjectDetail.h"
#include"../CommonWindow/Explorer/JObjectExplorer.h"
#include"../CommonWindow/Debug/JGraphicResourceWatcher.h"
#include"../CommonWindow/Debug/JAppElapsedTime.h"
#include"../../Menubar/JEditorMenuBar.h" 
#include"../../GuiLibEx/ImGuiEx/JImGuiImpl.h"  
#include"../../../Object/GameObject/JGameObject.h"
#include"../../../Object/Resource/JResourceManager.h"
#include"../../../Object/Resource/Scene/JScene.h"
#include"../../../Object/Resource/Scene/JSceneManager.h" 
#include"../../../Application/JApplicationVariable.h"
#include"../../../Window/JWindows.h"

namespace JinEngine
{
	namespace Editor
	{
		JProjectMainPage::JProjectMainPage(const bool hasMetadata)
			:JEditorPage("JEngine",
				std::make_unique<JEditorAttribute>(0.0f, 0.0f, 1.0f, 1.0f),
				Core::AddSQValueEnum(J_EDITOR_PAGE_SUPPORT_DOCK)),
			reqInitDockNode(!hasMetadata)
		{  
			storeProjectF = std::make_unique<StoreProjectF::Functor>(&JApplicationProject::RequestStoreProject);
			loadProjectF = std::make_unique<LoadProjectF::Functor>(&JApplicationProject::RequestLoadProject);

			std::vector<WindowInitInfo> openInfo;
			openInfo.emplace_back("Window Directory##JEngine", 0.0f, 0.7f, 0.6f, 0.3f);
			openInfo.emplace_back("Object Explorer##JEngine", 0.6f, 0.0f, 0.2f, 0.6f);
			openInfo.emplace_back("Object Detail", 0.8f, 0.0f, 0.2f, 1.0f);
			openInfo.emplace_back("Scene Observe##JEngine", 0.0f, 0.0f, 0.6f, 0.7f);
			openInfo.emplace_back("Scene Viewer##JEngine", 0.0f, 0.0f, 0.6f, 0.7f);
			openInfo.emplace_back("Log Viewer##JEngine", 0.6f, 0.6f, 0.2f, 0.4f);
			openInfo.emplace_back("Animation Controller Editor##JEngine", 0.4f, 0.4f, 0.4f, 0.4f);
			openInfo.emplace_back("Graphic Resource Watcher##JEngine", 0.6f, 0.4f, 0.0f, 0.0f);
			openInfo.emplace_back("String Convert Test##JEngine", 0.6f, 0.4f, 0.0f, 0.0f);
			openInfo.emplace_back("App Elapsed Time##JEngine", 0.6f, 0.4f, 0.0f, 0.0f);

			windowDirectory = std::make_unique<JWindowDirectory>(openInfo[0].GetName(), openInfo[0].MakeAttribute(), GetPageType());
			objectExplorer = std::make_unique<JObjectExplorer>(openInfo[1].GetName(), openInfo[1].MakeAttribute(), GetPageType());
			objectDetail = std::make_unique<JObjectDetail>(openInfo[2].GetName(), openInfo[2].MakeAttribute(), GetPageType());
			sceneObserver = std::make_unique<JSceneObserver>(openInfo[3].GetName(), openInfo[3].MakeAttribute(), GetPageType());
			sceneViewer = std::make_unique<JSceneViewer>(openInfo[4].GetName(), openInfo[4].MakeAttribute(), GetPageType());
			logViewer = std::make_unique<JLogViewer>(openInfo[5].GetName(), openInfo[5].MakeAttribute(), GetPageType());
			animationControllerEditor = std::make_unique<JAnimationControllerEditor>(openInfo[6].GetName(), openInfo[6].MakeAttribute(), GetPageType());
			graphicResourceWatcher = std::make_unique<JGraphicResourceWatcher>(openInfo[7].GetName(), openInfo[7].MakeAttribute(), GetPageType());
			stringConvertTest = std::make_unique<JStringConvertTest>(openInfo[8].GetName(), openInfo[8].MakeAttribute(), GetPageType());
			appElapseTime = std::make_unique<JAppElapsedTime>(openInfo[9].GetName(), openInfo[9].MakeAttribute(), GetPageType());

			std::vector<JEditorWindow*> windows
			{
				windowDirectory.get(),
				objectExplorer.get(),
				objectDetail.get(),
				sceneObserver.get(),
				sceneViewer.get(),
				logViewer.get(),
				animationControllerEditor.get(),
				graphicResourceWatcher.get(),
				stringConvertTest.get(),
				appElapseTime.get()
			};
			AddWindow(windows);

			closePopup = std::make_unique<JEditorCloseConfirmPopup>();
			std::vector<JEditorPopupWindow*> popupWnds
			{
				closePopup.get()
			};
			AddPopupWindow(popupWnds);

			auto confirmFunc = [](JEditorPage* page)
			{
				page->ClosePopupWindow(J_EDITOR_POPUP_WINDOW_TYPE::CLOSE_CONFIRM);
				JApplicationProject::ConfirmCloseProject();
			};
			auto cancelFunc = [](JEditorPage* page) 
			{
				page->ClosePopupWindow(J_EDITOR_POPUP_WINDOW_TYPE::CLOSE_CONFIRM);
				JApplicationProject::CancelCloseProject();
			};
			
			closePopupConfirmF = std::make_unique<ClosePopupConfirmF::Functor>(confirmFunc);
			closePopupCancelF = std::make_unique<ClosePopupCancelF::Functor>(cancelFunc);
			 
			closePopup->RegisterBind(std::make_unique<ClosePopupConfirmF::CompletelyBind>(*closePopupConfirmF, this),
				nullptr, nullptr,
				std::make_unique<ClosePopupCancelF::CompletelyBind>(*closePopupCancelF, this));

			graphicOptionSetting = std::make_unique<JGraphicOptionSetting>();
			JEditorPageShareData::RegisterPage(GetPageType(), &JProjectMainPage::GetPageFlag, this);
		}
		JProjectMainPage::~JProjectMainPage()
		{
			JEditorPageShareData::UnRegisterPage(GetPageType());
			ClearModifiedInfoStructure();
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
			windowDirectory->Initialize();
			objectExplorer->Initialize(Core::GetUserPtr(JSceneManager::Instance().GetMainScene()->GetRootGameObject()));
			sceneViewer->Initialize(Core::GetUserPtr(JSceneManager::Instance().GetMainScene()));
			sceneObserver->Initialize(Core::GetUserPtr(JSceneManager::Instance().GetMainScene()), L"Editor_ObserverCamera");
			logViewer->Initialize();
			BuildMenuNode();
		}
		void JProjectMainPage::UpdatePage()
		{
			JImGuiImpl::SetFont(J_EDITOR_FONT_TYPE::MEDIUM);
			JImGuiImpl::PushFont();

			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			//ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowPos(viewport->WorkPos);

			ImGuiDockNodeFlags dockspaceFlag = ImGuiDockNodeFlags_None;
			ImGuiWindowFlags windowFlag = ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs |
				ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground |
				ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse;

			//if (!JImGuiImpl::IsFullScreen())
			//	dockspaceFlag &= ~ImGuiDockNodeFlags_PassthruCentralNode;

			EnterPage(windowFlag);
			if (reqInitDockNode)
			{
				BuildDockNode();
				reqInitDockNode = false;
			}
			UpdateDockSpace(dockspaceFlag);
			menuBar->Update(true);
			ClosePage();

			uint currOpWndCount = GetOpenWindowCount();
			for (uint i = 0; i < currOpWndCount; ++i)
				GetOpenWindow(i)->UpdateWindow();

			//PrintOpenWindowState();
			if (graphicOptionSetting->IsOpenViewer())
				graphicOptionSetting->Update();

			JImGuiImpl::PopFont();
			ImGui::PopStyleVar(2); 

			JEditorPopupWindow* openPopup = GetOpenPopupWindow();
			if (openPopup != nullptr)
			{
				openPopup->Update(GetName(),
					u8"종료하기 전 자원관리",
					JVector2<float>(0, 0),
					ImGui::GetMainViewport()->WorkSize);
			}
		}
		bool JProjectMainPage::IsValidOpenRequest(const Core::JUserPtr<JObject>& selectedObj)noexcept
		{
			return true;
		}
		void JProjectMainPage::BuildDockNode()
		{
			ImGui::Begin(windowDirectory->GetName().c_str()); ImGui::End();
			ImGui::Begin(objectExplorer->GetName().c_str()); ImGui::End();
			ImGui::Begin(objectDetail->GetName().c_str()); ImGui::End();
			ImGui::Begin(sceneObserver->GetName().c_str()); ImGui::End();
			ImGui::Begin(sceneViewer->GetName().c_str()); ImGui::End();
			ImGui::Begin(logViewer->GetName().c_str()); ImGui::End();

			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGuiID dockspaceId = ImGui::GetID(GetDockNodeName().c_str());

			ImGui::DockBuilderRemoveNode(dockspaceId);
			ImGui::DockBuilderAddNode(dockspaceId);
			ImGui::DockBuilderSetNodePos(dockspaceId, viewport->WorkPos);
			ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->WorkSize);
			//ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

			ImGuiID dock_main = dockspaceId;
			ImGuiID dockSceneObserver;
			ImGuiID dockObjExplorer;
			ImGuiID dockObjDetail;
			ImGuiID dockObjLogViewer;
			ImGuiID dockWindowDirectory;
			ImGuiID dockSceneViewer;

			//ImGui::DockBuilderAddNode(dockWindowDirectory);
			//ImGui::DockBuilderSetNodePos(dockWindowDirectory, viewport->WorkPos);
			//ImGui::DockBuilderSetNodeSize(dockWindowDirectory, viewport->WorkSize);
			dockWindowDirectory = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, 1, &dock_main, &dock_main);
			dockObjDetail = ImGui::DockBuilderSplitNode(dockWindowDirectory, ImGuiDir_Right, 0.3f, nullptr, &dockWindowDirectory);
			dockObjExplorer = ImGui::DockBuilderSplitNode(dockObjDetail, ImGuiDir_Left, 0.5f, nullptr, &dockObjDetail);
			dockObjLogViewer = ImGui::DockBuilderSplitNode(dockObjExplorer, ImGuiDir_Down, 0.4f, nullptr, &dockObjExplorer);
			dockSceneViewer = ImGui::DockBuilderSplitNode(dockWindowDirectory, ImGuiDir_Up, 0.7f, nullptr, &dockWindowDirectory);
			dockSceneObserver = ImGui::DockBuilderSplitNode(dockSceneViewer, ImGuiDir_Up, 0, &dockSceneViewer, nullptr);

			ImGui::DockBuilderDockWindow(GetName().c_str(), dock_main);
			ImGui::DockBuilderDockWindow(windowDirectory->GetName().c_str(), dockWindowDirectory);
			ImGui::DockBuilderDockWindow(objectExplorer->GetName().c_str(), dockObjExplorer);
			ImGui::DockBuilderDockWindow(objectDetail->GetName().c_str(), dockObjDetail);
			ImGui::DockBuilderDockWindow(sceneViewer->GetName().c_str(), dockSceneViewer);
			ImGui::DockBuilderDockWindow(sceneObserver->GetName().c_str(), dockSceneObserver);
			ImGui::DockBuilderDockWindow(logViewer->GetName().c_str(), dockObjLogViewer);

			ImGui::DockBuilderFinish(dockspaceId);
		}
		void JProjectMainPage::BuildMenuNode()
		{
			std::unique_ptr<JEditorMenuNode> rootNode = std::make_unique<JEditorMenuNode>("Root", true, false);

			// root Child
			std::unique_ptr<JEditorMenuNode> fileNode = std::make_unique<JEditorMenuNode>("JFile", false, false, nullptr, rootNode.get());
			std::unique_ptr<JEditorMenuNode> windowNode = std::make_unique<JEditorMenuNode>("Window", false, false, nullptr, rootNode.get());
			std::unique_ptr<JEditorMenuNode> graphicNode = std::make_unique<JEditorMenuNode>("Graphic", false, false, nullptr, rootNode.get());

			//file Child
			std::unique_ptr<JEditorMenuNode> saveNode = std::make_unique<JEditorMenuNode>("SaveProject", false, true, nullptr, fileNode.get());
			saveNode->RegisterBindHandle(std::make_unique<StoreProjectF::CompletelyBind>(*storeProjectF));
			std::unique_ptr<JEditorMenuNode> loadNode = std::make_unique<JEditorMenuNode>("LoadProject", false, true, nullptr, fileNode.get());
			loadNode->RegisterBindHandle(std::make_unique<LoadProjectF::CompletelyBind>(*loadProjectF));
			std::unique_ptr<JEditorMenuNode> buildNode = std::make_unique<JEditorMenuNode>("BuildProject", false, true, nullptr, fileNode.get());

			//window Child
			std::unique_ptr<JEditorMenuNode> directoryNode = std::make_unique<JEditorMenuNode>(windowDirectory->GetName(),
				false, true,
				windowDirectory->GetOpenPtr(),
				windowNode.get());
			directoryNode->RegisterBindHandle(std::make_unique<OpenEditorWindowF::CompletelyBind>(*GetOpEditorWindowFunctorPtr(), *this, windowDirectory->GetName()));

			std::unique_ptr<JEditorMenuNode> objectExplorerNode = std::make_unique<JEditorMenuNode>(objectExplorer->GetName(),
				false, true,
				objectExplorer->GetOpenPtr(),
				windowNode.get());
			objectExplorerNode->RegisterBindHandle(std::make_unique< OpenEditorWindowF::CompletelyBind>(*GetOpEditorWindowFunctorPtr(), *this, objectExplorer->GetName()));

			std::unique_ptr<JEditorMenuNode> sceneViewerNode = std::make_unique<JEditorMenuNode>(sceneViewer->GetName(),
				false, true,
				sceneViewer->GetOpenPtr(),
				windowNode.get());
			sceneViewerNode->RegisterBindHandle(std::make_unique<OpenEditorWindowF::CompletelyBind>(*GetOpEditorWindowFunctorPtr(), *this, sceneViewer->GetName()));

			/*std::unique_ptr<JEditorMenuNode> objectDetailNode = std::make_unique<JEditorMenuNode>(objectDetail->GetName(),
				false, true,
				objectDetail->GetOpenPtr(),
				windowNode.get());*/
			std::unique_ptr<JEditorMenuNode> logViewerNode = std::make_unique<JEditorMenuNode>(logViewer->GetName(),
				false, true,
				logViewer->GetOpenPtr(),
				windowNode.get());
			logViewerNode->RegisterBindHandle(std::make_unique<OpenEditorWindowF::CompletelyBind>(*GetOpEditorWindowFunctorPtr(), *this, logViewer->GetName()));

			std::unique_ptr<JEditorMenuNode> sceneObserverNode = std::make_unique<JEditorMenuNode>(sceneObserver->GetName(),
				false, true,
				sceneObserver->GetOpenPtr(),
				windowNode.get());
			sceneObserverNode->RegisterBindHandle(std::make_unique<OpenEditorWindowF::CompletelyBind>(*GetOpEditorWindowFunctorPtr(), *this, sceneObserver->GetName()));

			std::unique_ptr<JEditorMenuNode> animationControllderEditorNode = std::make_unique<JEditorMenuNode>(animationControllerEditor->GetName(),
				false, true,
				animationControllerEditor->GetOpenPtr(),
				windowNode.get());
			animationControllderEditorNode->RegisterBindHandle(std::make_unique<OpenEditorWindowF::CompletelyBind>(*GetOpEditorWindowFunctorPtr(), *this, animationControllerEditor->GetName()));

			std::unique_ptr<JEditorMenuNode> graphicResourceWatcherEditorNode = std::make_unique<JEditorMenuNode>(graphicResourceWatcher->GetName(),
				false, true,
				graphicResourceWatcher->GetOpenPtr(),
				windowNode.get());
			graphicResourceWatcherEditorNode->RegisterBindHandle(std::make_unique<OpenEditorWindowF::CompletelyBind>(*GetOpEditorWindowFunctorPtr(), *this, graphicResourceWatcher->GetName()));

			std::unique_ptr<JEditorMenuNode> stringConvertTestEditorNode = std::make_unique<JEditorMenuNode>(stringConvertTest->GetName(),
				false, true,
				stringConvertTest->GetOpenPtr(),
				windowNode.get());
			stringConvertTestEditorNode->RegisterBindHandle(std::make_unique<OpenEditorWindowF::CompletelyBind>(*GetOpEditorWindowFunctorPtr(), *this, stringConvertTest->GetName()));

			std::unique_ptr<JEditorMenuNode> appElapsedTimeNode = std::make_unique<JEditorMenuNode>(appElapseTime->GetName(),
				false, true,
				appElapseTime->GetOpenPtr(),
				windowNode.get());
			appElapsedTimeNode->RegisterBindHandle(std::make_unique<OpenEditorWindowF::CompletelyBind>(*GetOpEditorWindowFunctorPtr(), *this, appElapseTime->GetName()));
			 
			std::unique_ptr<JEditorMenuNode> grapicOptionNode = std::make_unique<JEditorMenuNode>("Graphic Option",
				false, true,
				graphicOptionSetting->GetOpenPtr(),
				graphicNode.get());
			grapicOptionNode->RegisterBindHandle(std::make_unique<OpenSimpleWindowF::CompletelyBind>(*GetOpSimpleWindowFunctorPtr(), graphicOptionSetting->GetOpenPtr()));

			menuBar = std::make_unique<JEditorMenuBar>(std::move(rootNode), true);
			menuBar->AddNode(std::move(fileNode));
			menuBar->AddNode(std::move(windowNode));
			menuBar->AddNode(std::move(graphicNode));
			menuBar->AddNode(std::move(grapicOptionNode));
			menuBar->AddNode(std::move(saveNode));
			menuBar->AddNode(std::move(loadNode));
			menuBar->AddNode(std::move(buildNode));
			menuBar->AddNode(std::move(directoryNode));
			menuBar->AddNode(std::move(objectExplorerNode));
			  
			menuBar->AddNode(std::move(sceneViewerNode));
			menuBar->AddNode(std::move(sceneObserverNode));
			menuBar->AddNode(std::move(logViewerNode));
			menuBar->AddNode(std::move(animationControllderEditorNode));
			menuBar->AddNode(std::move(graphicResourceWatcherEditorNode));
			menuBar->AddNode(std::move(stringConvertTestEditorNode));
			menuBar->AddNode(std::move(appElapsedTimeNode));
		}
	}
}