#include"JEditorAniContPage.h"   
#include"Window/JAnimationDiagramList.h"
#include"Window/JAnimationConditionList.h"
#include"Window/JAnimationStateView.h"
#include"../JEditorPageShareData.h"
#include"../JEditorAttribute.h"  
#include"../CommonWindow/View/JSceneObserver.h" 
#include"../CommonWindow/Detail/JObjectDetail.h"
#include"../../GuiLibEx/ImGuiEx/JImGuiImpl.h"  
#include"../../Menubar/JEditorMenuBar.h"  
#include "../../Event/JEditorEvent.h"
#include"../../../Core/FSM/AnimationFSM/JAnimationFSMdiagram.h"
#include"../../../Object/Resource/AnimationController/JAnimationController.h"  
#include"../../../Object/Component/JComponentFactoryUtility.h"
#include"../../../Object/Resource/JResourceManager.h"   
#include"../../../Object/Resource/JResourceObjectFactory.h" 
#include"../../../Object/Resource/Scene/JScene.h"  
#include"../../../Object/Resource/Scene/JSceneManager.h"  
#include"../../../Application/JApplicationVariable.h"
#include<vector>

namespace JinEngine
{
	namespace Editor
	{
		JEditorAniContPage::JEditorAniContPage(bool hasMetadata)
			:JEditorPage("AniContSettingPage",
				std::make_unique<JEditorAttribute>(),
				Core::AddSQValueEnum(J_EDITOR_PAGE_SUPPORT_DOCK , J_EDITOR_PAGE_SUPPORT_WINDOW_CLOSING)),
			reqInit(!hasMetadata)
		{
			constexpr uint memberWindowCount = 5;
			std::vector<std::string> windowNames
			{
				"DiagramList##AniContSettingPage",
				"ConditionList##AniContSettingPage",
				"StateView##AniContSettingPage",
				"AniContObserver##AniContSettingPage",
				"AniContDetail##AniContSettingPage"
			};
			std::vector<std::unique_ptr<JEditorAttribute>> windowAttributes(memberWindowCount);
			for (uint i = 0; i < memberWindowCount; ++i)
				windowAttributes[i] = std::make_unique<JEditorAttribute>();

			J_EDITOR_WINDOW_FLAG defaultFlag = J_EDITOR_WINDOW_SUPPORT_WINDOW_CLOSING;
			J_EDITOR_WINDOW_FLAG dockFlag = Core::AddSQValueEnum(J_EDITOR_WINDOW_SUPROT_DOCK, defaultFlag);
 
			std::vector<J_OBSERVER_SETTING_TYPE> settingType{};

			diagramList = std::make_unique<JAnimationDiagramList>(windowNames[0], std::move(windowAttributes[0]), GetPageType(), dockFlag);
			conditionList = std::make_unique<JAnimationConditionList>(windowNames[1], std::move(windowAttributes[1]), GetPageType(), dockFlag);
			stateView = std::make_unique<JAnimationStateView>(windowNames[2], std::move(windowAttributes[2]), GetPageType(), dockFlag);
			aniContObserver = std::make_unique<JSceneObserver>(windowNames[3], std::move(windowAttributes[3]), GetPageType(), dockFlag, settingType);
			aniContDetail = std::make_unique<JObjectDetail>(windowNames[4], std::move(windowAttributes[4]), GetPageType(), dockFlag);

			std::vector<JEditorWindow*> windows
			{
				diagramList.get(),
				conditionList.get(),
				stateView.get(),
				aniContObserver.get(),
				aniContDetail.get()
			};
			AddWindow(windows);
		}
		JEditorAniContPage::~JEditorAniContPage()
		{
			if (aniCont.IsValid())
				CallOffResourceReference(aniCont.Release());
		}
		J_EDITOR_PAGE_TYPE JEditorAniContPage::GetPageType()const noexcept
		{
			return J_EDITOR_PAGE_TYPE::ANICONT_SETTING;
		}
		void JEditorAniContPage::SetInitWindow()
		{
			uint currOpWndCount = GetOpenWindowCount();
			for (uint i = 0; i < currOpWndCount; ++i)
				CloseWindow(GetOpenWindow(i));

			OpenWindow(diagramList.get());
			OpenWindow(conditionList.get());
			OpenWindow(stateView.get());
			OpenWindow(aniContObserver.get());
			OpenWindow(aniContDetail.get());

			currOpWndCount = GetOpenWindowCount();
			for (uint i = 0; i < currOpWndCount; ++i)
				GetOpenWindow(i)->SetLastActivated(true);
		}
		void JEditorAniContPage::Initialize()
		{
			JEditorPage::Initialize();
			BuildMenuNode();
		}
		void JEditorAniContPage::UpdatePage()
		{
			JImGuiImpl::SetFont(J_EDITOR_FONT_TYPE::MEDIUM);
			JImGuiImpl::PushFont();

			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

			if (reqInit)
			{
				//ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, viewport->WorkSize.y + ImGui::GetFrameHeight()), ImGuiCond_Once);
				//ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y - ImGui::GetFrameHeight()), ImGuiCond_Once);
			}

			ImGuiDockNodeFlags dockspaceFlag = ImGuiDockNodeFlags_NoWindowMenuButton;
			ImGuiWindowFlags guiWindowFlag = ImGuiWindowFlags_NoNavInputs |
				ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_MenuBar |
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_AlwaysAutoResize;

			EnterPage(guiWindowFlag); 
			if (reqInit)
				BuildDockNode();
			UpdateDockSpace(dockspaceFlag);
			menuBar->Update(true);
			ClosePage();

			uint currOpWndCount = GetOpenWindowCount();
			for (uint i = 0; i < currOpWndCount; ++i)
				GetOpenWindow(i)->UpdateWindow();
			JImGuiImpl::PopFont();
			ImGui::PopStyleVar(2);

			if (reqInit)
				reqInit = false;
		}
		bool JEditorAniContPage::IsValidOpenRequest(const Core::JUserPtr<Core::JIdentifier>& selectedObj)noexcept
		{
			return StuffAniContData(selectedObj);
		}
		bool JEditorAniContPage::StuffAniContData(const Core::JUserPtr<Core::JIdentifier>& selectedObj)
		{
			if (!selectedObj.IsValid())
				return false;

			Core::JUserPtr<JAnimationController> newAniCont;
			if (!newAniCont.ConnnectChildUser(selectedObj))
				return false;

			if (aniCont.IsValid())
				CallOffResourceReference(aniCont.Release());
			aniCont = newAniCont;
			if (aniCont.IsValid())
				CallOnResourceReference(aniCont.Get());

			const std::wstring sceneName = aniCont->GetName() + L"##_EditorScene";
			JDirectory* dir = JResourceManager::Instance().GetDirectory(JApplicationVariable::GetProjectDefaultResourcePath());
			const J_OBJECT_FLAG flag = Core::AddSQValueEnum(OBJECT_FLAG_DO_NOT_SAVE, OBJECT_FLAG_UNEDITABLE);

			JScene* newScene = JRFI<JScene>::Create(Core::JPtrUtil::MakeOwnerPtr<JScene::InitData>(sceneName,
				Core::MakeGuid(),
				flag,
				dir,
				J_SCENE_USE_CASE_TYPE::THREE_DIMENSIONAL_PREVIEW));
			JSceneManager::Instance().TryOpenScene(newScene, false);

			if (aniPreviweScene.IsValid())
				JObject::BeginDestroy(aniPreviweScene.Release());
			aniPreviweScene = Core::GetUserPtr(newScene);

			diagramList->Initialize(aniCont);
			conditionList->Initialize(aniCont);
			stateView->Initialize(aniCont);
			aniContObserver->Initialize(aniPreviweScene, L"AniCont Preview Cam");
			return true;
		}
		void JEditorAniContPage::DoSetClose()noexcept 
		{
			JEditorPage::DoSetClose();
			if (aniPreviweScene.IsValid())
				JObject::BeginDestroy(aniPreviweScene.Release());
		}
		void JEditorAniContPage::DoActivate()noexcept
		{ 
			JEditorPage::DoActivate(); 
			JEditor::AddEventNotification(*JEditorEvent::EvInterface(), 
				GetGuid(), 
				J_EDITOR_EVENT::SELECT_OBJECT,
				JEditorEvent::RegisterEvStruct(std::make_unique<JEditorSelectObjectEvStruct>(GetPageType(),
					Core::GetUserPtr(aniCont->GetDiagramByIndex(0)))));

			JResourceUserInterface::AddEventListener(*JResourceManager::Instance().EvInterface(), GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
		}
		void JEditorAniContPage::DoDeActivate()noexcept
		{
			JResourceUserInterface::RemoveListener(*JResourceManager::Instance().EvInterface(), GetGuid());
			JEditorPage::DoDeActivate();
		}
		void JEditorAniContPage::StorePage(std::wofstream& stream)
		{
			JEditorPage::StorePage(stream);
		}
		void JEditorAniContPage::LoadPage(std::wifstream& stream)
		{
			JEditorPage::LoadPage(stream);
		}
		void JEditorAniContPage::BuildDockNode()
		{			
			//ImGui::Begin(GetName().c_str()); ImGui::End();
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGuiID dockspaceId = ImGui::GetID(GetDockNodeName().c_str());

			ImGui::Begin(diagramList->GetName().c_str()); ImGui::End();
			ImGui::Begin(conditionList->GetName().c_str()); ImGui::End();
			ImGui::Begin(stateView->GetName().c_str()); ImGui::End();
			ImGui::Begin(aniContObserver->GetName().c_str()); ImGui::End();
			ImGui::Begin(aniContDetail->GetName().c_str()); ImGui::End();

			ImGui::DockBuilderRemoveNode(dockspaceId);
			ImGui::DockBuilderAddNode(dockspaceId);
			ImGui::DockBuilderSetNodePos(dockspaceId, viewport->WorkPos);
			ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->WorkSize);

			ImGuiID dock_main = dockspaceId;
			ImGuiID dockStateView = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Up, 1.0f, &dock_main, &dock_main);
			ImGuiID dockDiagramList= ImGui::DockBuilderSplitNode(dockStateView, ImGuiDir_Left, 0.25f, nullptr, &dockStateView);
			ImGuiID dockConditionList = ImGui::DockBuilderSplitNode(dockDiagramList, ImGuiDir_Down, 0.5f, nullptr, &dockDiagramList);
			 
			ImGuiID dockAniContDetail = ImGui::DockBuilderSplitNode(dockStateView, ImGuiDir_Right, 0.2f, nullptr, &dockStateView);
			ImGuiID dockAniContObserver= ImGui::DockBuilderSplitNode(dockAniContDetail, ImGuiDir_Down, 0.5f, nullptr, &dockAniContDetail);

			ImGui::DockBuilderDockWindow(GetName().c_str(), dock_main);
			ImGui::DockBuilderDockWindow(diagramList->GetName().c_str(), dockDiagramList);
			ImGui::DockBuilderDockWindow(conditionList->GetName().c_str(), dockConditionList);
			ImGui::DockBuilderDockWindow(stateView->GetName().c_str(), dockStateView);
			ImGui::DockBuilderDockWindow(aniContObserver->GetName().c_str(), dockAniContObserver);
			ImGui::DockBuilderDockWindow(aniContDetail->GetName().c_str(), dockAniContDetail);
			ImGui::DockBuilderFinish(dockspaceId);
		}
		void JEditorAniContPage::BuildMenuNode()
		{
			std::unique_ptr<JEditorMenuNode> rootNode = std::make_unique<JEditorMenuNode>("Root", true, false);

			// root Child 
			std::unique_ptr<JEditorMenuNode> windowNode = std::make_unique<JEditorMenuNode>("Window", false, false, nullptr, rootNode.get());

			JEditorMenuNode* windowNodePtr = windowNode.get();
			menuBar = std::make_unique<JEditorMenuBar>(std::move(rootNode), false);
			menuBar->AddNode(std::move(windowNode));
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
		void JEditorAniContPage::OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
		{
			if (iden == GetGuid())
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
			{
				if (aniCont.IsValid() && jRobj->GetGuid() == aniCont->GetGuid())
				{ 
					CallOffResourceReference(aniCont.Release());
					if(aniPreviweScene.IsValid())
						JObject::BeginDestroy(aniPreviweScene.Release());
				}
			}
		}
	};
}
