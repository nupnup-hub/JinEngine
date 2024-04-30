#include"JAnimationStateView.h"  
#include"../../JEditorAttribute.h"
#include"../../JEditorPageShareData.h" 
#include"../../../Event/JEditorEvent.h"
#include"../../../Popup/JEditorPopupNode.h"
#include"../../../Gui/JGui.h"  
#include"../../../Interface/JEditorObjectCreationInterface.h"
#include"../../../../Core/Reflection/JTypeTemplate.h" 
#include"../../../../Object/Resource/AnimationController/FSM/JAnimationStateType.h"
#include"../../../../Object/Resource/AnimationController/FSM/JAnimationFSMdiagram.h"
#include"../../../../Object/Resource/AnimationController/FSM/JAnimationFSMstate.h"
#include"../../../../Object/Resource/AnimationController/FSM/JAnimationFSMstateClip.h"
#include"../../../../Object/Resource/AnimationController/FSM/JAnimationFSMtransition.h"
#include"../../../../Object/Resource/AnimationController/JAnimationController.h"  
#include"../../../../Object/Resource/JResourceManager.h"
#include"../../../../Application/JApplicationProject.h"

namespace JinEngine
{
	namespace Editor
	{
		namespace Private
		{
			static std::string DockNodeName(const std::string& uniqueLabel)noexcept
			{
				return uniqueLabel + "_DockNode";
			}
			static std::string StateViewName(const std::string& uniqueLabel)noexcept
			{
				return "StateView##" + uniqueLabel;
			}
			static std::wstring ViewGraphPath()noexcept
			{
				return JApplicationProject::EditoConfigPath() + L"\\AniContViewGraph.txt";
			}
		}

		class JAnimationStateViewCreationFunctor
		{
		private:
			using ClipCreationInterface = JEditorCreationRequestor<JEditorObjectCreateInterface<>>;
			using TransitionCreationInteface = JEditorCreationRequestor<JEditorObjectCreateInterface<size_t, size_t>>;
			using DestructionInterface = JEditorDestructionRequestor;
		public:
			using ClipCanCreateF = ClipCreationInterface::CreateInteface::CanCreateF;
			using ClipCreateF = ClipCreationInterface::CreateInteface::ObjectCreateF;
			using TransitionCanCreateF = TransitionCreationInteface::CreateInteface::CanCreateF;
			using TransitionCreateF = TransitionCreationInteface::CreateInteface::ObjectCreateF;
		public:
			using DataHandleStructure = ClipCreationInterface::DataHandleStructure;
			using NotifyPtr = ClipCreationInterface::NotifyPtr;
		public:
			DataHandleStructure dS;
		public:
			ClipCreationInterface clip;
			TransitionCreationInteface transition;
			DestructionInterface destructuion;
		public:
			using RequestEvF = Core::JSFunctorType<void, JAnimationStateView*>;
			using TryConnectStateTransitionF = Core::JSFunctorType<void, JAnimationStateView*>;
			using ConnectStateTrasitionF = Core::JSFunctorType<void, JAnimationStateView*>;
		public:
			std::unique_ptr<RequestEvF::Functor> reqCreateStateEvF;
			std::unique_ptr<TryConnectStateTransitionF::Functor> tryConnectStateTransF;
			std::unique_ptr<ConnectStateTrasitionF::Functor> connectStateTransF;
			std::unique_ptr<RequestEvF::Functor> reqDestroyEvF;
		public:
			JAnimationStateViewCreationFunctor(RequestEvF::Ptr reqCreateStateEvPtr,
				TryConnectStateTransitionF::Ptr tryConnectStateTransPtr,
				ConnectStateTrasitionF::Ptr connectStateTransPtr,
				RequestEvF::Ptr reqDestroyEvPtr)
			{
				reqCreateStateEvF = std::make_unique<RequestEvF::Functor>(reqCreateStateEvPtr);
				tryConnectStateTransF = std::make_unique<TryConnectStateTransitionF::Functor>(tryConnectStateTransPtr);
				connectStateTransF = std::make_unique<ConnectStateTrasitionF::Functor>(connectStateTransPtr);

				reqDestroyEvF = std::make_unique<RequestEvF::Functor>(reqDestroyEvPtr);
			}
			~JAnimationStateViewCreationFunctor()
			{
				dS.Clear();
			}
		};

		JAnimationStateView::JAnimationStateView(const std::string& name,
			std::unique_ptr<JEditorAttribute> attribute,
			const J_EDITOR_PAGE_TYPE ownerPageType,
			const J_EDITOR_WINDOW_FLAG windowFlag)
			:JEditorWindow(name, std::move(attribute), ownerPageType, windowFlag)
		{
			editorString = std::make_unique<JEditorStringMap>();
			stateGraph = std::make_unique<JEditorGraphView>();
			stateGraph->GetGrid()->SetMinZoomRate(50);
			stateGraph->GetGrid()->SetMaxZoomRate(50);
			stateGraph->LoadData(Private::ViewGraphPath());
			stateGraph->UseBeginViewWindow(false);

			uint count = 0;
			 
			auto rawVec = JAnimationController::StaticTypeInfo().GetInstanceRawPtrVec();
			for (const auto& data : rawVec)
				RegisterViewGraphGroup(static_cast<JAnimationController*>(data));

			InitializeCreationImpl();
			//Diagram View Popup
			std::unique_ptr<JEditorPopupNode> stateViewRootNode =
				std::make_unique<JEditorPopupNode>("Animation Controller Editor State View Popup Root", J_EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

			std::unique_ptr<JEditorPopupNode> createNewCilpStateNode =
				std::make_unique<JEditorPopupNode>("Create New State", J_EDITOR_POPUP_NODE_TYPE::LEAF, stateViewRootNode.get());
			editorString->AddString(createNewCilpStateNode->GetNodeId(), { "Create New Animation Clip" , u8"애니메이션 클립 생성" });

			std::unique_ptr<JEditorPopupNode> createTransitionNode =
				std::make_unique<JEditorPopupNode>("Create Transition", J_EDITOR_POPUP_NODE_TYPE::LEAF, stateViewRootNode.get());
			editorString->AddString(createTransitionNode->GetNodeId(), { "Create Transition" , u8"트랜지션 추가" });

			std::unique_ptr<JEditorPopupNode> destroyNode =
				std::make_unique<JEditorPopupNode>("Destroy", J_EDITOR_POPUP_NODE_TYPE::LEAF, stateViewRootNode.get());
			editorString->AddString(destroyNode->GetNodeId(), { "Destroy" , u8"삭제" });

			//createNewCilpStateNode->RegisterEnableBind(std::make_unique<PassPopupConditionF::CompletelyBind>(*GetPassSelectedOneFunctor(), this));

			using RequestEvF = JAnimationStateViewCreationFunctor::RequestEvF;
			using TryConnectStateTransitionF = JAnimationStateViewCreationFunctor::TryConnectStateTransitionF;

			createNewCilpStateNode->RegisterSelectBind(std::make_unique<RequestEvF::CompletelyBind>(*creation->reqCreateStateEvF, this));
			createTransitionNode->RegisterSelectBind(std::make_unique<TryConnectStateTransitionF::CompletelyBind>(*creation->tryConnectStateTransF, this));
			createTransitionNode->RegisterEnableBind(std::make_unique<PassPopupConditionF::CompletelyBind>(*GetPassSelectedOneFunctor(), this));
			destroyNode->RegisterSelectBind(std::make_unique<RequestEvF::CompletelyBind>(*creation->reqDestroyEvF, this));
			destroyNode->RegisterEnableBind(std::make_unique<PassPopupConditionF::CompletelyBind>(*GetPassSelectedAboveOneFunctor(), this));

			statePopup = std::make_unique<JEditorPopupMenu>(Private::StateViewName(GetName()), std::move(stateViewRootNode));
			statePopup->AddPopupNode(std::move(createNewCilpStateNode));
			statePopup->AddPopupNode(std::move(createTransitionNode));
			statePopup->AddPopupNode(std::move(destroyNode));
		}
		JAnimationStateView::~JAnimationStateView()
		{
			stateGraph->StoreData(Private::ViewGraphPath());
			creation.reset();
		}
		void JAnimationStateView::InitializeCreationImpl()
		{
			if (creation != nullptr)
				return;

			auto requestCreateStateLam = [](JAnimationStateView* stateView)
			{
				if (!stateView->aniCont.IsValid() || !stateView->selectedDiagram.IsValid())
					return;

				JEditorCreationHint creationHint = JEditorCreationHint(stateView,
					true, false, false, true,
					Core::JTypeInstanceSearchHint(stateView->aniCont),
					Core::JTypeInstanceSearchHint(stateView->selectedDiagram),
					&JEditorWindow::NotifyEvent);
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification);

				JAnimationStateViewCreationFunctor* impl = stateView->creation.get();
				impl->clip.RequestCreateObject(impl->dS, true, creationHint, Core::MakeGuid(), requestHint);
			};
			auto tryConnectStateTransLam = [](JAnimationStateView* stateView)
			{
				using bType = JAnimationStateViewCreationFunctor::ConnectStateTrasitionF::CompletelyBind;
				auto selectedVec = stateView->GetSelectedObjectVec();
				if (stateView->aniCont.IsValid() && selectedVec[0].IsValid() &&
					selectedVec[0]->GetTypeInfo().IsChildOf<JAnimationFSMstate>())
				{
					stateView->stateGraph->SetConnectNodeMode(selectedVec[0]->GetGuid(),
						std::make_unique<bType>(*stateView->creation->connectStateTransF, std::move(stateView)));
				}
			};
			auto connectStateTransLam = [](JAnimationStateView* stateView)
			{
				if (!stateView->aniCont.IsValid() || !stateView->selectedDiagram.IsValid())
					return;

				size_t fromGuid = stateView->stateGraph->GetConnectFromGuid();
				size_t toGuid = stateView->stateGraph->GetConnectToGuid();

				JEditorCreationHint creationHint = JEditorCreationHint(stateView,
					true, false, false, true,
					Core::JTypeInstanceSearchHint(stateView->aniCont),
					Core::JTypeInstanceSearchHint(stateView->selectedDiagram),
					&JEditorWindow::NotifyEvent);
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification);

				JAnimationStateViewCreationFunctor* impl = stateView->creation.get();
				impl->transition.RequestCreateObject(impl->dS, true, creationHint, Core::MakeGuid(), requestHint, std::move(fromGuid), std::move(toGuid));
			};
			auto requestDestroyLam = [](JAnimationStateView* stateView)
			{
				if (!stateView->aniCont.IsValid() || !stateView->selectedDiagram.IsValid())
					return;

				std::vector<JUserPtr<Core::JIdentifier>> objVec = stateView->GetSelectedObjectVec();
				if (objVec.size() == 0)
					return;

				JEditorCreationHint creationHint = JEditorCreationHint(stateView,
					true, false, false, true,
					Core::JTypeInstanceSearchHint(stateView->aniCont),
					Core::JTypeInstanceSearchHint(stateView->selectedDiagram),
					&JEditorWindow::NotifyEvent);
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification);

				JAnimationStateViewCreationFunctor* impl = stateView->creation.get();
				impl->destructuion.RequestDestroyObject(impl->dS, true, creationHint, objVec, requestHint);
			};

			creation = std::make_unique<JAnimationStateViewCreationFunctor>(requestCreateStateLam,
				tryConnectStateTransLam, connectStateTransLam, requestDestroyLam);

			auto canCreateClipLam = [](const size_t guid, const JEditorCreationHint& creationHint)
			{
				auto openSelectedPtr = Core::GetRawPtr(creationHint.openDataHint);
				auto ownerPtr = Core::GetRawPtr(creationHint.ownerDataHint);
				if (openSelectedPtr == nullptr || ownerPtr == nullptr)
					return false;

				if (openSelectedPtr->GetTypeInfo().IsChildOf<JAnimationController>() && ownerPtr->GetTypeInfo().IsChildOf<JAnimationFSMdiagram>())
					return static_cast<JAnimationController*>(openSelectedPtr)->CanCreateState(static_cast<JAnimationFSMdiagram*>(ownerPtr));
				else
					return false;
			};
			auto creatClipLam = [](const size_t guid, const JEditorCreationHint& creationHint)
			{
				JAnimationController* aniCont = static_cast<JAnimationController*>(Core::GetRawPtr(creationHint.openDataHint));
				JUserPtr<JAnimationFSMdiagram> ownerDiagaram = Core::GetUserPtr<JAnimationFSMdiagram>(creationHint.ownerDataHint);
				aniCont->CreateFSMclip(ownerDiagaram, guid);
			};

			auto canCreateTransitionLam = [](const size_t guid, const JEditorCreationHint& creationHint, const size_t fromStateGuid, const size_t toStateGuid)
			{
				auto openSelectedPtr = Core::GetRawPtr(creationHint.openDataHint);
				auto ownerPtr = Core::GetRawPtr(creationHint.ownerDataHint);
				if (openSelectedPtr == nullptr || ownerPtr == nullptr)
					return false;

				auto fromStatePtr = Core::SearchRawPtr(JAnimationFSMstate::StaticTypeInfo(), fromStateGuid);
				auto toStatePtr = Core::SearchRawPtr(JAnimationFSMstate::StaticTypeInfo(), toStateGuid);
				if (fromStatePtr != nullptr && toStatePtr != nullptr && openSelectedPtr->GetTypeInfo().IsChildOf<JAnimationController>() && ownerPtr->GetTypeInfo().IsChildOf<JAnimationFSMdiagram>())
					return true;
				else
					return false;
			};
			auto createTransitionLam = [](const size_t guid, const JEditorCreationHint& creationHint, const size_t fromStateGuid, const size_t toStateGuid)
			{
				JAnimationController* aniCont = static_cast<JAnimationController*>(Core::GetRawPtr(creationHint.openDataHint));
				JUserPtr<JAnimationFSMdiagram> ownerDiagaram = Core::GetUserPtr<JAnimationFSMdiagram>(creationHint.ownerDataHint);
				JUserPtr<JAnimationFSMstate> fromState = Core::SearchUserPtr<JAnimationFSMstate>(JAnimationFSMstate::StaticTypeInfo(), fromStateGuid);
				JUserPtr<JAnimationFSMstate> toState = Core::SearchUserPtr<JAnimationFSMstate>(JAnimationFSMstate::StaticTypeInfo(), toStateGuid);
				aniCont->CreateFsmtransition(ownerDiagaram, fromState, toState, guid);
			};

			creation->clip.GetCreationInterface()->RegisterCanCreationF(canCreateClipLam);
			creation->clip.GetCreationInterface()->RegisterObjectCreationF(creatClipLam);
			creation->transition.GetCreationInterface()->RegisterCanCreationF(canCreateTransitionLam);
			creation->transition.GetCreationInterface()->RegisterObjectCreationF(createTransitionLam);
		}
		J_EDITOR_WINDOW_TYPE JAnimationStateView::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::ANIMATION_STATE_VIEW;
		}
		void JAnimationStateView::SetAnimationController(const JUserPtr<JAnimationController>& newAniCont)
		{
			aniCont = newAniCont;
			selectedDiagram = aniCont->GetDiagramByIndex(0);
			RegisterViewGraphGroup(aniCont.Get());
		}
		void JAnimationStateView::SetSelecteObject(JUserPtr<Core::JIdentifier> newSelected)
		{
			RequestPushSelectObject(newSelected);
			SetContentsClick(true);
		} 
		void JAnimationStateView::UpdateWindow()
		{ 
			const bool hoveredGraphObj = stateGraph->IsLastUpdateHoveredNode() || stateGraph->IsLastUpdateSeletedEdge();
			J_GUI_WINDOW_FLAG_ flag = J_GUI_WINDOW_FLAG_NO_SCROLL_BAR | J_GUI_WINDOW_FLAG_NO_COLLAPSE;
			if (hoveredGraphObj)
				flag |= J_GUI_WINDOW_FLAG_NO_MOVE;

			EnterWindow(flag);
			UpdateDocking();
			if (IsActivated())
			{
				UpdateMouseClick();
				BuildDiagramView();
			}
			CloseWindow();
		}
		void JAnimationStateView::BuildDiagramView()
		{
			bool isHoveredContents = false;
			stateGraph->ClearNode();
			stateGraph->SetGridSize(2000);
			if (aniCont.IsValid() && selectedDiagram == nullptr)
				selectedDiagram = aniCont->GetDiagramByIndex(0);

			if (aniCont.IsValid() && selectedDiagram.IsValid())
			{
				const uint stateCount = selectedDiagram->GetStateCount();
				for (uint i = 0; i < stateCount; ++i)
				{
					JUserPtr<JAnimationFSMstate> state = selectedDiagram->GetStateByIndex(i);
					stateGraph->BuildNode(JCUtil::WstrToU8Str(state->GetName()),
						state->GetGuid(),
						aniCont->GetGuid(),
						IsSelectedObject(state->GetGuid()));
				}
				for (uint i = 0; i < stateCount; ++i)
				{
					JUserPtr<JAnimationFSMstate> state = selectedDiagram->GetStateByIndex(i);
					const size_t fromGuid = state->GetGuid();
					const uint transitionCount = state->GetTransitionCount();
					for (uint j = 0; j < transitionCount; ++j)
					{
						JUserPtr<JAnimationFSMtransition> trans = state->GetTransitionByIndex(j);
						stateGraph->ConnectNode(fromGuid,
							trans->GetOutputStateGuid(),
							IsSelectedObject(trans->GetGuid()));
					}
				}
				JGui::Text(JCUtil::WstrToU8Str(selectedDiagram->GetName()));
				stateGraph->OnScreen(aniCont->GetGuid());
				if (stateGraph->IsLastUpdateHoveredNode())
				{
					isHoveredContents = true;
					SetHoveredObject(selectedDiagram->GetState(stateGraph->GetLastUpdateHoveredNodeGuid()));
				}
				else if(stateGraph->IsLastUpdateHoveredEdge())
				{
					isHoveredContents = true;
					size_t fromGuid;
					size_t toGuid;
					stateGraph->GetLastUpdateHoveredEdgeGuid(fromGuid, toGuid); 
					SetHoveredObject(selectedDiagram->GetState(fromGuid)->GetTransitionByOutGuid(toGuid));
				}
				if (stateGraph->IsLastUpdateSeletedNode())
				{
					const size_t guid = stateGraph->GetLastUpdateSeletedNodeGuid();
					SetSelecteObject(selectedDiagram->GetState(guid));
				}
				else if (stateGraph->IsLastUpdateSeletedEdge())
				{
					size_t fromGuid;
					size_t toGuid;
					stateGraph->GetLastUpdateSelectedEdgeGuid(fromGuid, toGuid);
					JUserPtr<Core::JFSMtransition> tUser = selectedDiagram->GetState(fromGuid)->GetTransitionByOutGuid(toGuid);
					SetSelecteObject(Core::ConvertChildUserPtr<JAnimationFSMtransition>(std::move(tUser)));
				} 
				if(isHoveredContents && JGui::AnyMouseClicked(false))
					SetContentsClick(true);
				 
				UpdatePopup(PopupSetting(statePopup.get(), editorString.get()));
			}
			else
				stateGraph->OnScreen();
		}
		void JAnimationStateView::RegisterViewGraphGroup(JAnimationController* aniCont)noexcept
		{
			auto isValidGroupLam = [](const size_t guid) {return Core::GetUserPtr<JAnimationController>(guid).IsValid(); };
			if (aniCont != nullptr)
				stateGraph->RegisterGroup(aniCont->GetGuid(), isValidGroupLam);
		}
		void JAnimationStateView::DoSetClose()noexcept
		{
			aniCont.Clear();
			selectedDiagram.Clear();
			JEditorWindow::DoSetClose();
		}
		void JAnimationStateView::DoActivate()noexcept
		{
			JEditorWindow::DoActivate();
			std::vector<J_EDITOR_EVENT> enumVec
			{
				J_EDITOR_EVENT::MOUSE_CLICK, J_EDITOR_EVENT::PUSH_SELECT_OBJECT
			};
			RegisterEventListener(enumVec);
		}
		void JAnimationStateView::DoDeActivate()noexcept
		{
			DeRegisterListener();
			JEditorWindow::DoDeActivate();
		}
		void JAnimationStateView::OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStructure)
		{
			JEditorWindow::OnEvent(senderGuid, eventType, eventStructure);
			if (!eventStructure->CanExecuteOtherEv(senderGuid, GetGuid()))
				return;
 
			if (eventType == J_EDITOR_EVENT::MOUSE_CLICK)
				statePopup->SetOpen(false);
			else if (eventType == J_EDITOR_EVENT::PUSH_SELECT_OBJECT && eventStructure->pageType == GetOwnerPageType())
			{
				JEditorPushSelectObjectEvStruct* evstruct = static_cast<JEditorPushSelectObjectEvStruct*>(eventStructure);
				JUserPtr< Core::JIdentifier> diagram = evstruct->GetFirstMatchedTypeObject(JAnimationFSMdiagram::StaticTypeInfo());
				if (diagram.IsValid())
				{
					if (!selectedDiagram.IsValid() || selectedDiagram->GetGuid() != diagram->GetGuid())
						selectedDiagram.ConnnectChild(diagram);
				}
			}
		}
	}
}
