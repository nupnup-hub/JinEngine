#include"EditorMainPage.h"
#include"../EditorAttribute.h" 
#include"AnimationControllerEditor.h"
#include"LogViewer.h"
#include"ObjectDetail.h"
#include"ObjectExplorer.h"
#include"WindowDirectory.h" 
#include"../Common/MainSceneEditor.h"
#include"../Common/SceneViewer.h"
#include"../../Utility/EditorUtility.h"
#include"../../Menubar/EditorMenuBar.h"
#include"../../../../Utility/JCommonUtility.h"
#include"../../ImGuiEx/ImGuiManager.h"
#include"../../../../../Lib/imgui/imgui_internal.h"
#include"../../../../Object/Resource/JResourceManager.h"
#include"../Debug/GraphicResourceWatcher.h"
#include"../Debug/StringConvertTest.h"

namespace JinEngine
{
	EditorMainPage::EditorMainPage()
		:EditorWindowPage(std::make_unique<EditorAttribute>("MainPage",
			JCommonUtility::CalculateGuid("MainPage"),
			0.0f, 0.0f, 1.0f, 1.0f, true, true))
	{
		const uint memberWindowCount = 9;
		std::vector<std::string> nameVec
		{
			"WindowDirectory", "ObjectExplorer", "SceneEditor", "ObjectDetail", "LogViewer",
			"SceneMainViewer", "AnimationControllerEditor", "GraphicResourceWatcher", "StringConvertText"
		};
		std::vector<float> initWidthRateVec
		{
			0.6f, 0.2f, 0.6f, 0.2f, 0.2f,
			0.6f, 0.6f, 0.6f, 0.6f
		};
		std::vector<float> initHeightRateVec
		{
			0.4f, 0.8f, 0.6f, 1.0f, 0.2f,
			0.6f, 0.4f, 0.4f, 0.4f
		};
		std::vector<float> initPosXRateVec
		{
			0.0f, 0.6f, 0.0f, 0.8f, 0.6f,
			0.0f, 0.0f, 0.0f, 0.0f
		};
		std::vector<float> initPosYRateVec
		{
			0.6f, 0.0f, 0.0f, 0.0f, 0.8f,
			0.0f, 0.0f, 0.0f, 0.0f
		};
		std::vector<bool> openVec
		{
			true, true, true, true, true,
			true, false, false, false
		};
		std::vector<bool> frontVec
		{
			true, true, true, true, true,
			false, false, false, false
		};
		std::vector<std::unique_ptr< EditorAttribute>> windowAttributes;
		for (uint i = 0; i < memberWindowCount; ++i)
		{
			size_t guid = JCommonUtility::CalculateGuid(nameVec[i]);
			windowAttributes.push_back(std::make_unique<EditorAttribute>(nameVec[i],
				guid,
				initPosXRateVec[i],
				initPosYRateVec[i],
				initWidthRateVec[i],
				initHeightRateVec[i],
				openVec[i],
				frontVec[i]));
		}
		windowDirectory = std::make_unique<WindowDirectory>(std::move(windowAttributes[0]), GetGuid());
		objectExplorer = std::make_unique<ObjectExplorer>(std::move(windowAttributes[1]), GetGuid());
		sceneEditor = std::make_unique<MainSceneEditor>(std::move(windowAttributes[2]), GetGuid());
		objectDetail = std::make_unique<ObjectDetail>(std::move(windowAttributes[3]), GetGuid());
		logViewer = std::make_unique<LogViewer>(std::move(windowAttributes[4]), GetGuid());
		sceneViewer = std::make_unique<SceneViewer>(std::move(windowAttributes[5]), GetGuid());
		animationControllerEditor = std::make_unique<AnimationControllerEditor>(std::move(windowAttributes[6]), GetGuid());
		graphicResourceWatcher = std::make_unique<GraphicResourceWatcher>(std::move(windowAttributes[7]), GetGuid());
		stringConvertTest = std::make_unique<StringConvertTest>(std::move(windowAttributes[8]), GetGuid());

		windows.resize(memberWindowCount);
		windows[0] = windowDirectory.get();
		windows[1] = objectExplorer.get();
		windows[2] = sceneEditor.get();
		windows[3] = objectDetail.get();
		windows[4] = logViewer.get();
		windows[5] = sceneViewer.get();
		windows[6] = animationControllerEditor.get();
		windows[7] = graphicResourceWatcher.get();
		windows[8] = stringConvertTest.get();

		opendWindow.push_back(windows[0]);
		opendWindow.push_back(windows[1]);
		opendWindow.push_back(windows[2]);
		opendWindow.push_back(windows[3]);
		opendWindow.push_back(windows[4]);
		opendWindow.push_back(windows[5]);
	}
	void EditorMainPage::Initialize(EditorUtility* editorUtility, std::vector<EditorWindow*>& allEditorWindows, bool hasImguiTxt)
	{
		allEditorWindows.push_back(windowDirectory.get());
		allEditorWindows.push_back(objectExplorer.get());
		allEditorWindows.push_back(sceneEditor.get());
		allEditorWindows.push_back(objectDetail.get());
		allEditorWindows.push_back(logViewer.get());
		allEditorWindows.push_back(sceneViewer.get());
		allEditorWindows.push_back(animationControllerEditor.get());
		allEditorWindows.push_back(graphicResourceWatcher.get());
		allEditorWindows.push_back(stringConvertTest.get());

		JScene* mainScene = JResourceManager::Instance().GetMainScene();

		windowDirectory->Initialize(editorUtility);
		objectExplorer->Initialize(editorUtility); 
		sceneEditor->Initialize(editorUtility);
		objectDetail->Initialize(editorUtility);
		logViewer->Initialize(editorUtility); 
		animationControllerEditor->Initialize(editorUtility);

		windowDirectory->EnterWindow(editorUtility); windowDirectory->CloseWindow();
		objectExplorer->EnterWindow(editorUtility); objectExplorer->CloseWindow();
		sceneEditor->EnterWindow(editorUtility); sceneEditor->CloseWindow();
		objectDetail->EnterWindow(editorUtility); objectDetail->CloseWindow();
		logViewer->EnterWindow(editorUtility); logViewer->CloseWindow();
		sceneViewer->EnterWindow(editorUtility); sceneViewer->CloseWindow();
		animationControllerEditor->EnterWindow(editorUtility); animationControllerEditor->CloseWindow();
		graphicResourceWatcher->EnterWindow(editorUtility); graphicResourceWatcher->CloseWindow();
		stringConvertTest->EnterWindow(editorUtility); stringConvertTest->CloseWindow();

		//Dock Node
		if (!hasImguiTxt)
		{
			ImGuiWindowFlags groupFlag = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse;
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
			window_flags |= ImGuiWindowFlags_NoBackground;
			ImGui::Begin(GetName().c_str(), 0, window_flags);
			ImGuiID dockspaceId = ImGui::GetID((dockSpaceName).c_str());

			ImGui::DockBuilderRemoveNode(dockspaceId);
			ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_None);
			ImGui::DockBuilderSetNodePos(dockspaceId, ImVec2(0, 0));
			//ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size); 
			//ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

			ImGuiID dock_main = dockspaceId;
			ImGuiID dockSceneEditor = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Up, 0.7f, nullptr, &dock_main);
			ImGuiID dockObjExplorer = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, 0.3f, nullptr, &dockSceneEditor);
			ImGuiID dockObjDetail = ImGui::DockBuilderSplitNode(dockObjExplorer, ImGuiDir_Right, 0.5f, nullptr, &dockObjExplorer);
			ImGuiID dockObjLogViewer = ImGui::DockBuilderSplitNode(dockObjExplorer, ImGuiDir_Down, 0.25f, nullptr, &dockObjExplorer);
			ImGuiID dockWindowDirectory = ImGui::DockBuilderSplitNode(dockSceneEditor, ImGuiDir_Down, 0.3f, nullptr, &dockSceneEditor);
			ImGuiID dockSceneViewer = ImGui::DockBuilderSplitNode(dockSceneEditor, ImGuiDir_Down, 0, &dockSceneEditor, nullptr);

			ImGui::DockBuilderDockWindow(GetName().c_str(), dock_main);
			ImGui::DockBuilderDockWindow(windowDirectory->GetName().c_str(), dockWindowDirectory);
			ImGui::DockBuilderDockWindow(objectExplorer->GetName().c_str(), dockObjExplorer);
			ImGui::DockBuilderDockWindow(sceneEditor->GetName().c_str(), dockSceneEditor);
			ImGui::DockBuilderDockWindow(objectDetail->GetName().c_str(), dockObjDetail);
			ImGui::DockBuilderDockWindow(logViewer->GetName().c_str(), dockObjLogViewer);
			ImGui::DockBuilderDockWindow(sceneViewer->GetName().c_str(), dockSceneViewer);
			ImGui::DockBuilderFinish(dockspaceId);
			ImGui::End();

			windowDirectory->EnterWindow(editorUtility); windowDirectory->CloseWindow();
			objectExplorer->EnterWindow(editorUtility); windowDirectory->CloseWindow();
			sceneEditor->EnterWindow(editorUtility); windowDirectory->CloseWindow();
			objectDetail->EnterWindow(editorUtility); windowDirectory->CloseWindow();
			logViewer->EnterWindow(editorUtility); windowDirectory->CloseWindow();
			sceneViewer->EnterWindow(editorUtility); windowDirectory->CloseWindow();
		}
		//Menu Bar
		{
			// root
			std::unique_ptr<MenuNode> rootNode = std::make_unique<MenuNode>("Root", true, false);

			// root Child
			std::unique_ptr<MenuNode> fileNode = std::make_unique<MenuNode>("JFile", false, false, nullptr, rootNode.get());
			std::unique_ptr<MenuNode> windowNode = std::make_unique<MenuNode>("Window", false, false, nullptr, rootNode.get());
			std::unique_ptr<MenuNode> graphicNode = std::make_unique<MenuNode>("Graphic", false, false, nullptr, rootNode.get());

			//file Child
			std::unique_ptr<MenuNode> saveNode = std::make_unique<MenuNode>("SaveProject", false, false, nullptr, fileNode.get());
			std::unique_ptr<MenuNode> loadNode = std::make_unique<MenuNode>("LoadProject", false, false, nullptr, fileNode.get());
			std::unique_ptr<MenuNode> buildNode = std::make_unique<MenuNode>("BuildProject", false, false, nullptr, fileNode.get());

			//window Child
			std::unique_ptr<MenuNode> directoryNode = std::make_unique<MenuNode>(windowDirectory->GetName(),
				false, true, 
				windowDirectory->GetOpenPtr(),
				windowNode.get());
			std::unique_ptr<MenuNode> objectExplorerNode = std::make_unique<MenuNode>(objectExplorer->GetName(),
				false, true,
				objectExplorer->GetOpenPtr(),
				windowNode.get());
			std::unique_ptr<MenuNode> sceneEditorNode = std::make_unique<MenuNode>(sceneEditor->GetName(),
				false, true, 
				sceneEditor->GetOpenPtr(),
				windowNode.get());
			std::unique_ptr<MenuNode> objectDetailNode = std::make_unique<MenuNode>(objectDetail->GetName(), 
				false, true,
				objectDetail->GetOpenPtr(),
				windowNode.get());
			std::unique_ptr<MenuNode> logViewerNode = std::make_unique<MenuNode>(logViewer->GetName(), 
				false, true,
				logViewer->GetOpenPtr(),
				windowNode.get());
			std::unique_ptr<MenuNode> mainSceneViewerNode = std::make_unique<MenuNode>(sceneViewer->GetName(),
				false, true,
				sceneViewer->GetOpenPtr(),
				windowNode.get());
			std::unique_ptr<MenuNode> animationControllderEditorNode = std::make_unique<MenuNode>(animationControllerEditor->GetName(),
				false, true,
				animationControllerEditor->GetOpenPtr(),
				windowNode.get());
			std::unique_ptr<MenuNode> graphicResourceWatcherEditorNode = std::make_unique<MenuNode>(graphicResourceWatcher->GetName(),
				false, true,
				graphicResourceWatcher->GetOpenPtr(),
				windowNode.get());
			std::unique_ptr<MenuNode> stringConvertTestEditorNode = std::make_unique<MenuNode>(stringConvertTest->GetName(),
				false, true,
				stringConvertTest->GetOpenPtr(),
				windowNode.get());

			editorMenuBar = std::make_unique<EditorMenuBar>();
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
			editorMenuBar->allNode.push_back(std::move(sceneEditorNode));
			editorMenuBar->allNode.push_back(std::move(objectDetailNode));
			editorMenuBar->allNode.push_back(std::move(logViewerNode));
			editorMenuBar->allNode.push_back(std::move(mainSceneViewerNode));
			editorMenuBar->allNode.push_back(std::move(animationControllderEditorNode));
			editorMenuBar->allNode.push_back(std::move(graphicResourceWatcherEditorNode));
			editorMenuBar->allNode.push_back(std::move(stringConvertTestEditorNode));
		}
	}
	void EditorMainPage::UpdatePage(EditorUtility* editorUtility)
	{
		ImGuiDockNodeFlags dockspaceFlag = ImGuiDockNodeFlags_None;
		ImGuiWindowFlags windowFlag = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (ImGuiManager::IsFullScreen())
		{
			windowFlag |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			windowFlag |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
			dockspaceFlag &= ~ImGuiDockNodeFlags_PassthruCentralNode;

		if (dockspaceFlag & ImGuiDockNodeFlags_PassthruCentralNode)
			windowFlag |= ImGuiWindowFlags_NoBackground;

		EnterPage(windowFlag, dockspaceFlag, true);
		uint8 opendWindowCount = (uint8)opendWindow.size();
		for (uint8 i = 0; i < opendWindowCount; ++i)
		{
			opendWindow[i]->EnterWindow(editorUtility);
			if (opendWindow[i]->IsActivated())
				opendWindow[i]->UpdateWindow(editorUtility);
			opendWindow[i]->CloseWindow();
		}
		//PrintOpenWindowState();
		ClosePage(editorUtility);
		UpdateWindowMenuBar(editorUtility);
	}
}
