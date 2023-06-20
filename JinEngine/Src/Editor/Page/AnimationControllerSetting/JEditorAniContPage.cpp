#include"JEditorAniContPage.h"   
#include"Window/JAnimationDiagramList.h"
#include"Window/JAnimationParameterList.h"
#include"Window/JAnimationStateView.h"
#include"../JEditorPageShareData.h"
#include"../JEditorAttribute.h"  
#include"../CommonWindow/View/JSceneObserver.h" 
#include"../CommonWindow/Detail/JObjectDetail.h"
#include"../../GuiLibEx/ImGuiEx/JImGuiImpl.h"  
#include"../../Menubar/JEditorMenuBar.h"  
#include "../../Event/JEditorEvent.h"
#include"../../../Core/Identity/JIdenCreator.h"
#include"../../../Core/FSM/AnimationFSM/JAnimationFSMdiagram.h"
#include"../../../Object/Resource/AnimationController/JAnimationController.h"  
#include"../../../Object/Component/JComponentCreator.h"
#include"../../../Object/Resource/JResourceManager.h"    
#include"../../../Object/Resource/Scene/JScene.h"  
#include"../../../Object/Resource/Scene/JSceneManager.h"  
#include"../../../Application/JApplicationProject.h"
#include<vector>

namespace JinEngine
{
	namespace Editor
	{
		JEditorAniContPage::JEditorAniContPage()
			:JEditorPage("AniContSettingPage",
				std::make_unique<JEditorAttribute>(),
				Core::AddSQValueEnum(J_EDITOR_PAGE_SUPPORT_DOCK, J_EDITOR_PAGE_SUPPORT_WINDOW_CLOSING))
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
			J_EDITOR_WINDOW_FLAG dockFlag = Core::AddSQValueEnum(defaultFlag, J_EDITOR_WINDOW_SUPROT_DOCK);
			J_EDITOR_WINDOW_FLAG listFlag = Core::AddSQValueEnum(dockFlag, J_EDITOR_WINDOW_SUPPORT_SELECT, J_EDITOR_WINDOW_SUPPORT_POPUP);

			std::vector<J_OBSERVER_SETTING_TYPE> settingType{};

			diagramList = std::make_unique<JAnimationDiagramList>(windowNames[0], std::move(windowAttributes[0]), GetPageType(), listFlag);
			conditionList = std::make_unique<JAnimationParameterList>(windowNames[1], std::move(windowAttributes[1]), GetPageType(), listFlag);
			stateView = std::make_unique<JAnimationStateView>(windowNames[2], std::move(windowAttributes[2]), GetPageType(), listFlag);
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
			aniCont.Clear();
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

			if (setWndOptionOnce)
			{
				ImGui::SetNextWindowSize(viewport->WorkSize, ImGuiCond_Once);
				ImGui::SetNextWindowPos(viewport->WorkPos, ImGuiCond_Once);
				setWndOptionOnce = false;
			}

			ImGuiDockNodeFlags dockspaceFlag = ImGuiDockNodeFlags_NoWindowMenuButton;
			ImGuiWindowFlags guiWindowFlag = ImGuiWindowFlags_NoNavInputs |
				ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_MenuBar |
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_AlwaysAutoResize;

			EnterPage(guiWindowFlag);
			const JVector2<float> pagePos = ImGui::GetWindowPos();
			const JVector2<float> pageSize = ImGui::GetWindowSize();

			if (HasDockNodeSpace())
				UpdateDockSpace(dockspaceFlag);
			else
			{
				BuildDockNode();
				setWndOptionOnce = true;
			}
			menuBar->Update(true);
			ClosePage();

			UpdateOpenWindow();
			UpdateOpenSimpleWindow();

			JImGuiImpl::PopFont();
			ImGui::PopStyleVar(2);

			UpdateOpenPopupWindow(pagePos, pageSize);
		}
		bool JEditorAniContPage::IsValidOpenRequest(const JUserPtr<Core::JIdentifier>& selectedObj)noexcept
		{
			return StuffAniContData(selectedObj);
		}
		bool JEditorAniContPage::StuffAniContData(const JUserPtr<Core::JIdentifier>& selectedObj)
		{
			if (!selectedObj.IsValid())
				return false;

			JUserPtr<JAnimationController> newAniCont;
			if (!newAniCont.ConnnectChild(selectedObj))
				return false;
			 
			aniCont = newAniCont; 

			const std::wstring sceneName = aniCont->GetName() + L"##_EditorScene";
			JUserPtr<JDirectory> dir = _JResourceManager::Instance().GetDirectory(JApplicationProject::DefaultResourcePath());
			const J_OBJECT_FLAG flag = Core::AddSQValueEnum(OBJECT_FLAG_DO_NOT_SAVE, OBJECT_FLAG_UNEDITABLE);

			JUserPtr<JScene> newScene = JICI::Create<JScene>(sceneName,
				Core::MakeGuid(),
				flag,
				JResourceObject::GetDefaultFormatIndex(),
				dir,
				J_SCENE_USE_CASE_TYPE::THREE_DIMENSIONAL_PREVIEW); 

			if (aniPreviweScene.IsValid())
				JObject::BeginDestroy(aniPreviweScene.Release());
			aniPreviweScene = newScene;

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
				J_EDITOR_EVENT::PUSH_SELECT_OBJECT,
				JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPushSelectObjectEvStruct>(GetPageType(),
					J_EDITOR_WINDOW_TYPE::TEST_WINDOW,
					aniCont->GetDiagramByIndex(0))));

			ResourceEvListener::AddEventListener(*JResourceObject::EvInterface(), GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
		}
		void JEditorAniContPage::DoDeActivate()noexcept
		{
			ResourceEvListener::RemoveListener(*JResourceObject::EvInterface(), GetGuid());
			JEditorPage::DoDeActivate();

			//Test Code
			//JObject::BeginDestroy(aniCont.Get());
			//aniCont.Clear();
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
			ImGuiID dockDiagramList = ImGui::DockBuilderSplitNode(dockStateView, ImGuiDir_Left, 0.25f, nullptr, &dockStateView);
			ImGuiID dockConditionList = ImGui::DockBuilderSplitNode(dockDiagramList, ImGuiDir_Down, 0.5f, nullptr, &dockDiagramList);

			ImGuiID dockAniContDetail = ImGui::DockBuilderSplitNode(dockStateView, ImGuiDir_Right, 0.2f, nullptr, &dockStateView);
			ImGuiID dockAniContObserver = ImGui::DockBuilderSplitNode(dockAniContDetail, ImGuiDir_Down, 0.5f, nullptr, &dockAniContDetail);

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
			std::unique_ptr<JEditorMenuNode> rootNode = std::make_unique<JEditorMenuNode>("Root", true, false, false);

			// root Child 
			std::unique_ptr<JEditorMenuNode> windowNode = std::make_unique<JEditorMenuNode>("Window", false, false, false, nullptr, rootNode.get());

			JEditorMenuNode* windowNodePtr = windowNode.get();
			menuBar = std::make_unique<JEditorMenuBar>(std::move(rootNode), false);
			menuBar->AddNode(std::move(windowNode));
			std::vector<JEditorWindow*> wndVec = GetWindowVec();

			const uint wndCount = (uint)wndVec.size();
			for (uint i = 0; i < wndCount; ++i)
			{
				std::unique_ptr<JEditorMenuNode> newNode = std::make_unique<JEditorMenuNode>(wndVec[i]->GetName(),
					false, true, false,
					wndVec[i]->GetOpenPtr(),
					windowNodePtr);

				auto openBind = std::make_unique<OpenEditorWindowF::CompletelyBind>(*GetOpenEditorWindowFunctorPtr(), *this, wndVec[i]->GetName());
				auto closeBind = std::make_unique<CloseEditorWindowF::CompletelyBind>(*GetCloseEditorWindowFunctorPtr(), *this, wndVec[i]->GetName());

				newNode->RegisterBindHandle(std::move(openBind), std::move(closeBind));
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
					aniCont.Clear();
					if (aniPreviweScene.IsValid())
						JObject::BeginDestroy(aniPreviweScene.Release());
				}
			}
		}
	};
}