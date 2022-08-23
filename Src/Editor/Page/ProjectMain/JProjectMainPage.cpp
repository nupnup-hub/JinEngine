#include"JProjectMainPage.h"
#include"../JEditorAttribute.h" 
#include"Window/JAnimationControllerEditor.h"
#include"Window/JLogViewer.h"
#include"Window/JObjectDetail.h"
#include"Window/JObjectExplorer.h"
#include"Window/JWindowDirectory.h" 
#include"../Common/JMainSceneEditor.h"
#include"../Common/JSceneViewer.h"
#include"../../Utility/J.h"
#include"../../Menubar/JEditorMenuBar.h"
#include"../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../Utility/JCommonUtility.h"
#include"../../../Object/Resource/JResourceManager.h"

#include"../Debug/JGraphicResourceWatcher.h"
#include"../Debug/JStringConvertTest.h"

#include"../../../../Lib/imgui/imgui_internal.h"

namespace JinEngine
{
	namespace Editor
	{
		JProjectMainPage::JProjectMainPage(bool hasMetadata)
			:JEditorPage(std::make_unique<JEditorAttribute>("MainPage",
				JCommonUtility::CalculateGuid("MainPage"),
				0.0f, 0.0f, 1.0f, 1.0f, true, true))
		{
			const uint memberWindowCount = 9;
			std::vector<std::string> nameVec
			{
				"JWindowDirectory", "JObjectExplorer", "JSceneEditor", "JObjectDetail", "JLogViewer",
				"SceneMainViewer", "JAnimationControllerEditor", "JGraphicResourceWatcher", "StringConvertText"
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
			std::vector<std::unique_ptr< JEditorAttribute>> windowAttributes;
			for (uint i = 0; i < memberWindowCount; ++i)
			{
				size_t guid = JCommonUtility::CalculateGuid(nameVec[i]);
				windowAttributes.push_back(std::make_unique<JEditorAttribute>(nameVec[i],
					guid,
					initPosXRateVec[i],
					initPosYRateVec[i],
					initWidthRateVec[i],
					initHeightRateVec[i],
					openVec[i],
					frontVec[i]));
			}
			windowDirectory = std::make_unique<JWindowDirectory>(std::move(windowAttributes[0]), GetGuid());
			objectExplorer = std::make_unique<JObjectExplorer>(std::move(windowAttributes[1]), GetGuid());
			sceneEditor = std::make_unique<JMainSceneEditor>(std::move(windowAttributes[2]), GetGuid());
			objectDetail = std::make_unique<JObjectDetail>(std::move(windowAttributes[3]), GetGuid());
			logViewer = std::make_unique<JLogViewer>(std::move(windowAttributes[4]), GetGuid());
			sceneViewer = std::make_unique<JSceneViewer>(std::move(windowAttributes[5]), GetGuid());
			animationControllerEditor = std::make_unique<JAnimationControllerEditor>(std::move(windowAttributes[6]), GetGuid());
			graphicResourceWatcher = std::make_unique<JGraphicResourceWatcher>(std::move(windowAttributes[7]), GetGuid());
			stringConvertTest = std::make_unique<JStringConvertTest>(std::move(windowAttributes[8]), GetGuid());

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

			if (!hasMetadata)
			{
				opendWindow.push_back(windows[0]);
				opendWindow.push_back(windows[1]);
				opendWindow.push_back(windows[2]);
				opendWindow.push_back(windows[3]);
				opendWindow.push_back(windows[4]);
				opendWindow.push_back(windows[5]);
			}
		}
		void JProjectMainPage::Initialize(bool hasImguiTxt)
		{
			windowDirectory->Initialize();
			objectExplorer->Initialize();
			sceneEditor->Initialize();
			objectDetail->Initialize();
			logViewer->Initialize();
			animationControllerEditor->Initialize();

			windowDirectory->EnterWindow(); windowDirectory->CloseWindow();
			objectExplorer->EnterWindow(); objectExplorer->CloseWindow();
			sceneEditor->EnterWindow(); sceneEditor->CloseWindow();
			objectDetail->EnterWindow(); objectDetail->CloseWindow();
			logViewer->EnterWindow(); logViewer->CloseWindow();
			sceneViewer->EnterWindow(); sceneViewer->CloseWindow();
			animationControllerEditor->EnterWindow(); animationControllerEditor->CloseWindow();
			graphicResourceWatcher->EnterWindow(); graphicResourceWatcher->CloseWindow();
			stringConvertTest->EnterWindow(); stringConvertTest->CloseWindow();

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

				windowDirectory->EnterWindow(); windowDirectory->CloseWindow();
				objectExplorer->EnterWindow(); windowDirectory->CloseWindow();
				sceneEditor->EnterWindow(); windowDirectory->CloseWindow();
				objectDetail->EnterWindow(); windowDirectory->CloseWindow();
				logViewer->EnterWindow(); windowDirectory->CloseWindow();
				sceneViewer->EnterWindow(); windowDirectory->CloseWindow();
			}
			//Menu Bar
			{
				// root
				std::unique_ptr<JMenuNode> rootNode = std::make_unique<JMenuNode>("Root", true, false);

				// root Child
				std::unique_ptr<JMenuNode> fileNode = std::make_unique<JMenuNode>("JFile", false, false, nullptr, rootNode.get());
				std::unique_ptr<JMenuNode> windowNode = std::make_unique<JMenuNode>("Window", false, false, nullptr, rootNode.get());
				std::unique_ptr<JMenuNode> graphicNode = std::make_unique<JMenuNode>("Graphic", false, false, nullptr, rootNode.get());

				//file Child
				std::unique_ptr<JMenuNode> saveNode = std::make_unique<JMenuNode>("SaveProject", false, false, nullptr, fileNode.get());
				std::unique_ptr<JMenuNode> loadNode = std::make_unique<JMenuNode>("LoadProject", false, false, nullptr, fileNode.get());
				std::unique_ptr<JMenuNode> buildNode = std::make_unique<JMenuNode>("BuildProject", false, false, nullptr, fileNode.get());

				//window Child
				std::unique_ptr<JMenuNode> directoryNode = std::make_unique<JMenuNode>(windowDirectory->GetName(),
					false, true,
					windowDirectory->GetOpenPtr(),
					windowNode.get());
				std::unique_ptr<JMenuNode> objectExplorerNode = std::make_unique<JMenuNode>(objectExplorer->GetName(),
					false, true,
					objectExplorer->GetOpenPtr(),
					windowNode.get());
				std::unique_ptr<JMenuNode> sceneEditorNode = std::make_unique<JMenuNode>(sceneEditor->GetName(),
					false, true,
					sceneEditor->GetOpenPtr(),
					windowNode.get());
				std::unique_ptr<JMenuNode> objectDetailNode = std::make_unique<JMenuNode>(objectDetail->GetName(),
					false, true,
					objectDetail->GetOpenPtr(),
					windowNode.get());
				std::unique_ptr<JMenuNode> logViewerNode = std::make_unique<JMenuNode>(logViewer->GetName(),
					false, true,
					logViewer->GetOpenPtr(),
					windowNode.get());
				std::unique_ptr<JMenuNode> mainSceneViewerNode = std::make_unique<JMenuNode>(sceneViewer->GetName(),
					false, true,
					sceneViewer->GetOpenPtr(),
					windowNode.get());
				std::unique_ptr<JMenuNode> animationControllderEditorNode = std::make_unique<JMenuNode>(animationControllerEditor->GetName(),
					false, true,
					animationControllerEditor->GetOpenPtr(),
					windowNode.get());
				std::unique_ptr<JMenuNode> graphicResourceWatcherEditorNode = std::make_unique<JMenuNode>(graphicResourceWatcher->GetName(),
					false, true,
					graphicResourceWatcher->GetOpenPtr(),
					windowNode.get());
				std::unique_ptr<JMenuNode> stringConvertTestEditorNode = std::make_unique<JMenuNode>(stringConvertTest->GetName(),
					false, true,
					stringConvertTest->GetOpenPtr(),
					windowNode.get());

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
				editorMenuBar->allNode.push_back(std::move(sceneEditorNode));
				editorMenuBar->allNode.push_back(std::move(objectDetailNode));
				editorMenuBar->allNode.push_back(std::move(logViewerNode));
				editorMenuBar->allNode.push_back(std::move(mainSceneViewerNode));
				editorMenuBar->allNode.push_back(std::move(animationControllderEditorNode));
				editorMenuBar->allNode.push_back(std::move(graphicResourceWatcherEditorNode));
				editorMenuBar->allNode.push_back(std::move(stringConvertTestEditorNode));
			}
		}
		void JProjectMainPage::UpdatePage(J* )
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
				opendWindow[i]->EnterWindow();
				if (opendWindow[i]->IsActivated())
					opendWindow[i]->UpdateWindow();
				opendWindow[i]->CloseWindow();
			}
			//PrintOpenWindowState();
			ClosePage();
			UpdateWindowMenuBar();
		}
	}
}