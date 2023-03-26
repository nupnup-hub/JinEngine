#include"JProjectMainPage.h" 
#include"Window/JLogViewer.h" 
#include"Window/JWindowDirectory.h"  
#include"../JEditorAttribute.h" 
#include"../JEditorPageShareData.h"
#include"../SimpleWindow/JGraphicOptionSetting.h" 
#include"../CommonWindow/Debug/JStringConvertTest.h" 
#include"../CommonWindow/View/JSceneViewer.h"
#include"../CommonWindow/View/JSceneObserver.h"
#include"../CommonWindow/Detail/JObjectDetail.h"
#include"../CommonWindow/Explorer/JObjectExplorer.h"
#include"../CommonWindow/Debug/JGraphicResourceWatcher.h"
#include"../CommonWindow/Debug/JAppElapsedTime.h"
#include"../../Align/JEditorAlignCalculator.h"
#include"../../Popup/JEditorPopupWindow.h"
#include"../../Event/JEditorEvent.h"
#include"../../Menubar/JEditorMenuBar.h" 
#include"../../GuiLibEx/ImGuiEx/JImGuiImpl.h"  
#include"../../../Object/GameObject/JGameObject.h"
#include"../../../Object/Directory/JDirectory.h"
#include"../../../Object/Resource/JResourceManager.h"
#include"../../../Object/Resource/Scene/JScene.h"
#include"../../../Object/Resource/Scene/JSceneManager.h" 
#include"../../../Application/JApplicationVariable.h"
#include"../../../Window/JWindows.h"

namespace JinEngine
{
	namespace Editor
	{
		JProjectMainPage::JProjectMainPage()
			:JEditorPage("JEngine#MainPage",
				std::make_unique<JEditorAttribute>(),
				Core::AddSQValueEnum(J_EDITOR_PAGE_SUPPORT_DOCK))
		{  
			storeProjectF = std::make_unique<StoreProjectF::Functor>(&JApplicationProject::StoreProject);
			loadProjectF = std::make_unique<LoadProjectF::Functor>(&JApplicationProject::LoadProject);

			std::vector<WindowInitInfo> openInfo;
			openInfo.emplace_back("Window Directory##JEngine");
			openInfo.emplace_back("Object Explorer##JEngine");
			openInfo.emplace_back("Object Detail");
			openInfo.emplace_back("Scene Observe##JEngine");
			openInfo.emplace_back("Scene Viewer##JEngine");
			openInfo.emplace_back("Log Viewer##JEngine");
			openInfo.emplace_back("Graphic Resource Watcher##JEngine");
			openInfo.emplace_back("String Convert Test##JEngine");
			openInfo.emplace_back("App Elapsed Time##JEngine");
			 
			J_EDITOR_WINDOW_FLAG defaultFlag = J_EDITOR_WINDOW_SUPPORT_WINDOW_CLOSING;
			J_EDITOR_WINDOW_FLAG dockFlag = Core::AddSQValueEnum(defaultFlag, J_EDITOR_WINDOW_SUPROT_DOCK);
			J_EDITOR_WINDOW_FLAG canSelectFlag = Core::AddSQValueEnum(dockFlag, J_EDITOR_WINDOW_SELECT);
			J_EDITOR_WINDOW_FLAG selectGObjWindowFlag = Core::AddSQValueEnum(canSelectFlag, J_EDITOR_WINDOW_LISTEN_OTHER_WINDOW_SELECT);

			windowDirectory = std::make_unique<JWindowDirectory>(openInfo[0].GetName(), openInfo[0].MakeAttribute(), GetPageType(), Core::AddSQValueEnum(J_EDITOR_WINDOW_SUPPORT_POPUP, canSelectFlag));
			objectExplorer = std::make_unique<JObjectExplorer>(openInfo[1].GetName(), openInfo[1].MakeAttribute(), GetPageType(), Core::AddSQValueEnum(J_EDITOR_WINDOW_SUPPORT_POPUP, selectGObjWindowFlag));
			objectDetail = std::make_unique<JObjectDetail>(openInfo[2].GetName(), openInfo[2].MakeAttribute(), GetPageType(), dockFlag);
			sceneObserver = std::make_unique<JSceneObserver>(openInfo[3].GetName(), openInfo[3].MakeAttribute(), GetPageType(), Core::AddSQValueEnum(J_EDITOR_WINDOW_SUPPORT_POPUP, selectGObjWindowFlag), Constants::GetAllObserverSetting());
			sceneViewer = std::make_unique<JSceneViewer>(openInfo[4].GetName(), openInfo[4].MakeAttribute(), GetPageType(), dockFlag);
			logViewer = std::make_unique<JLogViewer>(openInfo[5].GetName(), openInfo[5].MakeAttribute(), GetPageType(), dockFlag);
			graphicResourceWatcher = std::make_unique<JGraphicResourceWatcher>(openInfo[6].GetName(), openInfo[6].MakeAttribute(), GetPageType(), defaultFlag);
			stringConvertTest = std::make_unique<JStringConvertTest>(openInfo[7].GetName(), openInfo[7].MakeAttribute(), GetPageType(), defaultFlag);
			appElapseTime = std::make_unique<JAppElapsedTime>(openInfo[8].GetName(), openInfo[8].MakeAttribute(), GetPageType(), defaultFlag);

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
				appElapseTime.get()
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
			auto confirmFunc = [](JProjectMainPage* page){page->RequestCloseConfirmPopup(false);};
			auto cancelFunc = [](JProjectMainPage* page){page->RequestCloseConfirmPopup(true);};
			auto conetnsFunc = [](JProjectMainPage* mainPage)
			{
				constexpr uint columnCount = 4;
				constexpr uint textCount = 3;
				const float fontSize = ImGui::GetCurrentContext()->FontSize;
				const JVector2<float> wndSize = ImGui::GetWindowSize();
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

				ImGui::SetCursorPosX(wndSize.x * 0.075f); 
				JImGuiImpl::BeginListBox("##ModifiedResource_ProjectMainPage", listSize);
				ImGui::Separator();

				JEditorDynamicAlignCalculator<4> alignCal;
				alignCal.Update(listSize, contentsSize, padding, 0, innerSize, J_EDITOR_INNER_ALGIN_TYPE::ROW, ImGui::GetCursorPos());

				for (uint i = 0; i < columnCount; ++i)
				{
					alignCal.SetNextContentsPosition();
					JImGuiImpl::Text(label[i]);
				}

				JEditorTextAlignCalculator textCal;
				auto modVec = mainPage->GetModifiedObjectInfoVec();
				for (auto& data : modVec)
				{
					std::string strArr[textCount]{ "","","" };
					if (data->isRemoved)
					{
						strArr[0] = JCUtil::WstrToU8Str(data->lastObjName);
						strArr[1] = JCUtil::WstrToU8Str(data->lastObjPath);
						strArr[2] = Core::JReflectionInfo::Instance().GetTypeInfo(data->typeGuid)->NameWithOutPrefix();
					}
					else
					{
						Core::JIdentifier* obj = Core::GetRawPtr(data->typeGuid, data->objectGuid);
						strArr[0] = JCUtil::WstrToU8Str(obj->GetName());
						strArr[1] = JCUtil::WstrToU8Str(static_cast<JResourceObject*>(obj)->GetPath());
						strArr[2] = obj->GetTypeInfo().NameWithOutPrefix();
					}  
					 
					for (uint j = 0; j < textCount; ++j)
					{
						textCal.Update(strArr[j], alignCal.GetInnerContentsSize(), true);
						alignCal.SetNextContentsPosition();
						JImGuiImpl::Text(textCal.LeftAligned());
					}
					alignCal.SetNextContentsPosition();
					JImGuiImpl::CheckBox("##ModifiedResource_CheckBox" + strArr[0], data->isStore);
				}
				JImGuiImpl::EndListBox();
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

			graphicOptionSetting = std::make_unique<JGraphicOptionSetting>();	 
		}
		JProjectMainPage::~JProjectMainPage()
		{
			(*storeProjectF)();
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
			JEditorPage::Initialize();
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
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowPos(viewport->WorkPos);

			ImGuiDockNodeFlags dockspaceFlag = ImGuiDockNodeFlags_NoWindowMenuButton;
			ImGuiWindowFlags guiWindowFlag = ImGuiWindowFlags_NoInputs |
				ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground |
				ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse;

			//if (!JImGuiImpl::IsFullScreen())
			//	dockspaceFlag &= ~ImGuiDockNodeFlags_PassthruCentralNode;

			EnterPage(guiWindowFlag);
			if (HasDockNodeSpace())
				UpdateDockSpace(dockspaceFlag);
			else
				BuildDockNode();
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
				openPopup->Update(GetName(), JVector2<float>(0, 0), ImGui::GetMainViewport()->WorkSize);
		}
		bool JProjectMainPage::IsValidOpenRequest(const Core::JUserPtr<Core::JIdentifier>& selectedObj)noexcept
		{
			return true;
		}
		void JProjectMainPage::RequestCloseConfirmPopup(const bool isCancel)
		{			 
			AddEventNotification(*JEditorEvent::EvInterface(),
				GetGuid(),
				J_EDITOR_EVENT::CLOSE_POPUP_WINDOW,
				JEditorEvent::RegisterEvStruct(std::make_unique<JEditorClosePopupWindowEvStruct>(J_EDITOR_POPUP_WINDOW_TYPE::CLOSE_CONFIRM, GetPageType())));

			auto confirmLam = []() {JApplicationProject::ConfirmCloseProject(); };
			auto cancelLam = []() {JApplicationProject::CancelCloseProject(); };
			using CloseF = Core::JSFunctorType<void>;
			std::unique_ptr<CloseF::CompletelyBind> closePopupB;
			if(isCancel)
				closePopupB = std::make_unique<CloseF::CompletelyBind>(std::make_unique<CloseF::Functor>(cancelLam));
			else
				closePopupB = std::make_unique<CloseF::CompletelyBind>(std::make_unique<CloseF::Functor>(confirmLam));
			
			closePopup->RegisterBind(J_EDITOR_POPUP_WINDOW_FUNC_TYPE::CLOSE_POPUP, std::move(closePopupB));
		}
		void JProjectMainPage::BuildDockNode()
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGuiID dockspaceId = ImGui::GetID(GetDockNodeName().c_str()); 

			ImGui::Begin(windowDirectory->GetName().c_str()); ImGui::End();
			ImGui::Begin(objectExplorer->GetName().c_str()); ImGui::End();
			ImGui::Begin(objectDetail->GetName().c_str()); ImGui::End();
			ImGui::Begin(sceneObserver->GetName().c_str()); ImGui::End();
			ImGui::Begin(sceneViewer->GetName().c_str()); ImGui::End();
			ImGui::Begin(logViewer->GetName().c_str()); ImGui::End(); 

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
			std::unique_ptr<JEditorMenuNode> grapicOptionNode = std::make_unique<JEditorMenuNode>("Graphic Option",
				false, true,
				graphicOptionSetting->GetOpenPtr(),
				graphicNode.get());
			grapicOptionNode->RegisterBindHandle(std::make_unique<OpenSimpleWindowF::CompletelyBind>(*GetOpSimpleWindowFunctorPtr(), graphicOptionSetting->GetOpenPtr()));

			JEditorMenuNode* windowNodePtr = windowNode.get();
			menuBar = std::make_unique<JEditorMenuBar>(std::move(rootNode), true);
			menuBar->AddNode(std::move(fileNode));
			menuBar->AddNode(std::move(windowNode));
			menuBar->AddNode(std::move(graphicNode));
			menuBar->AddNode(std::move(grapicOptionNode));
			menuBar->AddNode(std::move(saveNode));
			menuBar->AddNode(std::move(loadNode));
			menuBar->AddNode(std::move(buildNode)); 
			std::vector<JEditorWindow*> wndVec = GetWindowVec();
			
			const uint wndCount = (uint)wndVec.size();
			for (uint i = 0; i < wndCount; ++i)
			{
				std::unique_ptr<JEditorMenuNode> newNode = std::make_unique<JEditorMenuNode>(wndVec[i]->GetName(),
					false, true,
					wndVec[i]->GetOpenPtr(),
					windowNodePtr);
				newNode->RegisterBindHandle(std::make_unique<OpenEditorWindowF::CompletelyBind>(*GetOpEditorWindowFunctorPtr(), *this, wndVec[i]->GetName()));
				menuBar->AddNode(std::move(newNode));
			}
		}
	}
}