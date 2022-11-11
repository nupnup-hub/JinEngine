#include"JProjectMainPage.h"
//#include"Window/JAnimationControllerEditor.h"
//#include"Window/JLogViewer.h" 
//#include"Window/JWindowDirectory.h"  
#include"../JEditorAttribute.h" 
#include"../JEditorPageShareData.h"
//#include"../CommonWindow/View/JSceneViewer.h"
//#include"../CommonWindow/View/JSceneObserver.h"
//#include"../CommonWindow/Explorer/JObjectExplorer.h"
//#include"../CommonWindow/Debug/JGraphicResourceWatcher.h"
//#include"../CommonWindow/Debug/JStringConvertTest.h"
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
		JProjectMainPage::JProjectMainPage(bool hasMetadata)
			:JEditorPage("JEngine",
				std::make_unique<JEditorAttribute>(0.0f, 0.0f, 1.0f, 1.0f, false, false),
				Core::AddSQValueEnum(J_EDITOR_PAGE_SUPPORT_DOCK)),
			reqInitDockNode(!hasMetadata)
		{ 
			auto storeProjectLam = []() { JApplicationProject::RequestStoreProject(); };
			auto loadProjectLam = []() { JApplicationProject::RequestLoadProject(); };
			storeProjectF = std::make_unique<StoreProjectF::Functor>(storeProjectLam);
			loadProjectF = std::make_unique<LoadProjectF::Functor>(loadProjectLam);

			std::vector<WindowInitInfo> openInfo;
			openInfo.emplace_back("Window Directory##JEngine", 0.0f, 0.7f, 0.6f, 0.3f, true, true);
			openInfo.emplace_back("Object Explorer##JEngine", 0.6f, 0.0f, 0.2f, 0.6f, true, true);
			openInfo.emplace_back("Object Detail", 0.8f, 0.0f, 0.2f, 1.0f, true, true);
			openInfo.emplace_back("Scene Observe##JEngine", 0.0f, 0.0f, 0.6f, 0.7f, true, true);
			openInfo.emplace_back("Scene Viewer##JEngine", 0.0f, 0.0f, 0.6f, 0.7f, true, false);
			openInfo.emplace_back("Log Viewer##JEngine", 0.6f, 0.6f, 0.2f, 0.4f, true, true);
			openInfo.emplace_back("Animation Controller Editor##JEngine", 0.4f, 0.4f, 0.4f, 0.4f, false, false);
			openInfo.emplace_back("Graphic Resource Watcher##JEngine", 0.6f, 0.4f, 0.0f, 0.0f, false, false);
			openInfo.emplace_back("String Convert Test##JEngine", 0.6f, 0.4f, 0.0f, 0.0f, false, false);
			openInfo.emplace_back("App Elapsed Time##JEngine", 0.6f, 0.4f, 0.0f, 0.0f, false, false);

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

			windows.resize(openInfo.size());
			windows[0] = windowDirectory.get();
			windows[1] = objectExplorer.get();
			windows[2] = objectDetail.get();
			windows[3] = sceneObserver.get();
			windows[4] = sceneViewer.get();
			windows[5] = logViewer.get();
			windows[6] = animationControllerEditor.get();
			windows[7] = graphicResourceWatcher.get();
			windows[8] = stringConvertTest.get();
			windows[9] = appElapseTime.get();

			if (!hasMetadata)
			{
				opendWindow.push_back(windows[0]);
				opendWindow.push_back(windows[1]);
				opendWindow.push_back(windows[2]);
				opendWindow.push_back(windows[3]);
				opendWindow.push_back(windows[4]);
				opendWindow.push_back(windows[5]);
			}
			JEditorPageShareData::RegisterPage(GetPageType(), pageFlag);
		}
		JProjectMainPage::~JProjectMainPage()
		{
			JEditorPageShareData::UnRegisterPage(GetPageType());
		}
		J_EDITOR_PAGE_TYPE JProjectMainPage::GetPageType()const noexcept
		{
			return J_EDITOR_PAGE_TYPE::PROJECT_MAIN;
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
			UpdateWindowMenuBar();
			ClosePage();

			uint8 opendWindowCount = (uint8)opendWindow.size();
			for (uint8 i = 0; i < opendWindowCount; ++i)
				opendWindow[i]->UpdateWindow();
			//PrintOpenWindowState();

			JImGuiImpl::PopFont();
			ImGui::PopStyleVar(2);
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
			std::unique_ptr<JMenuNode> rootNode = std::make_unique<JMenuNode>("Root", true, false);

			// root Child
			std::unique_ptr<JMenuNode> fileNode = std::make_unique<JMenuNode>("JFile", false, false, nullptr, rootNode.get());
			std::unique_ptr<JMenuNode> windowNode = std::make_unique<JMenuNode>("Window", false, false, nullptr, rootNode.get());
			std::unique_ptr<JMenuNode> graphicNode = std::make_unique<JMenuNode>("Graphic", false, false, nullptr, rootNode.get());

			//file Child
			std::unique_ptr<JMenuNode> saveNode = std::make_unique<JMenuNode>("SaveProject", false, true, nullptr, fileNode.get());
			saveNode->RegisterBind(std::make_unique<StoreProjectF::CompletelyBind>(*storeProjectF));
			std::unique_ptr<JMenuNode> loadNode = std::make_unique<JMenuNode>("LoadProject", false, true, nullptr, fileNode.get());
			loadNode->RegisterBind(std::make_unique<LoadProjectF::CompletelyBind>(*loadProjectF));
			std::unique_ptr<JMenuNode> buildNode = std::make_unique<JMenuNode>("BuildProject", false, true, nullptr, fileNode.get());

			//window Child
			std::unique_ptr<JMenuNode> directoryNode = std::make_unique<JMenuNode>(windowDirectory->GetName(),
				false, true,
				windowDirectory->GetOpenPtr(),
				windowNode.get());
			directoryNode->RegisterBind(std::make_unique<OpenWindowF::CompletelyBind>(*GetFunctorPtr(), *this, windowDirectory->GetName()));

			std::unique_ptr<JMenuNode> objectExplorerNode = std::make_unique<JMenuNode>(objectExplorer->GetName(),
				false, true,
				objectExplorer->GetOpenPtr(),
				windowNode.get());
			objectExplorerNode->RegisterBind(std::make_unique< OpenWindowF::CompletelyBind>(*GetFunctorPtr(), *this, objectExplorer->GetName()));

			std::unique_ptr<JMenuNode> sceneViewerNode = std::make_unique<JMenuNode>(sceneViewer->GetName(),
				false, true,
				sceneViewer->GetOpenPtr(),
				windowNode.get());
			sceneViewerNode->RegisterBind(std::make_unique<OpenWindowF::CompletelyBind>(*GetFunctorPtr(), *this, sceneViewer->GetName()));

			/*std::unique_ptr<JMenuNode> objectDetailNode = std::make_unique<JMenuNode>(objectDetail->GetName(),
				false, true,
				objectDetail->GetOpenPtr(),
				windowNode.get());*/
			std::unique_ptr<JMenuNode> logViewerNode = std::make_unique<JMenuNode>(logViewer->GetName(),
				false, true,
				logViewer->GetOpenPtr(),
				windowNode.get());
			logViewerNode->RegisterBind(std::make_unique<OpenWindowF::CompletelyBind>(*GetFunctorPtr(), *this, logViewer->GetName()));

			std::unique_ptr<JMenuNode> sceneObserverNode = std::make_unique<JMenuNode>(sceneObserver->GetName(),
				false, true,
				sceneObserver->GetOpenPtr(),
				windowNode.get());
			sceneObserverNode->RegisterBind(std::make_unique<OpenWindowF::CompletelyBind>(*GetFunctorPtr(), *this, sceneObserver->GetName()));

			std::unique_ptr<JMenuNode> animationControllderEditorNode = std::make_unique<JMenuNode>(animationControllerEditor->GetName(),
				false, true,
				animationControllerEditor->GetOpenPtr(),
				windowNode.get());
			animationControllderEditorNode->RegisterBind(std::make_unique<OpenWindowF::CompletelyBind>(*GetFunctorPtr(), *this, animationControllerEditor->GetName()));

			std::unique_ptr<JMenuNode> graphicResourceWatcherEditorNode = std::make_unique<JMenuNode>(graphicResourceWatcher->GetName(),
				false, true,
				graphicResourceWatcher->GetOpenPtr(),
				windowNode.get());
			graphicResourceWatcherEditorNode->RegisterBind(std::make_unique<OpenWindowF::CompletelyBind>(*GetFunctorPtr(), *this, graphicResourceWatcher->GetName()));

			std::unique_ptr<JMenuNode> stringConvertTestEditorNode = std::make_unique<JMenuNode>(stringConvertTest->GetName(),
				false, true,
				stringConvertTest->GetOpenPtr(),
				windowNode.get());
			stringConvertTestEditorNode->RegisterBind(std::make_unique<OpenWindowF::CompletelyBind>(*GetFunctorPtr(), *this, stringConvertTest->GetName()));

			std::unique_ptr<JMenuNode> appElapsedTimeNode = std::make_unique<JMenuNode>(appElapseTime->GetName(),
				false, true,
				appElapseTime->GetOpenPtr(),
				windowNode.get());
			appElapsedTimeNode->RegisterBind(std::make_unique<OpenWindowF::CompletelyBind>(*GetFunctorPtr(), *this, appElapseTime->GetName()));

			editorMenuBar = std::make_unique<JEditorMenuBar>();
			editorMenuBar->rootNode = rootNode.get();
			editorMenuBar->allNode.push_back(std::move(rootNode));
			editorMenuBar->allNode.push_back(std::move(fileNode));
			editorMenuBar->allNode.push_back(std::move(windowNode));
			editorMenuBar->allNode.push_back(std::move(graphicNode));

			editorMenuBar->allNode.push_back(std::move(saveNode));
			editorMenuBar->allNode.push_back(std::move(loadNode));
			editorMenuBar->allNode.push_back(std::move(buildNode));

			editorMenuBar->allNode.push_back(std::move(directoryNode));
			editorMenuBar->allNode.push_back(std::move(objectExplorerNode));
			editorMenuBar->allNode.push_back(std::move(sceneViewerNode));
			editorMenuBar->allNode.push_back(std::move(sceneObserverNode));
			//editorMenuBar->allNode.push_back(std::move(objectDetailNode));
			editorMenuBar->allNode.push_back(std::move(logViewerNode));
			editorMenuBar->allNode.push_back(std::move(animationControllderEditorNode));
			editorMenuBar->allNode.push_back(std::move(graphicResourceWatcherEditorNode));
			editorMenuBar->allNode.push_back(std::move(stringConvertTestEditorNode));
			editorMenuBar->allNode.push_back(std::move(appElapsedTimeNode));
		}
	}
}