#include"JAnimationStateView.h"  
#include"../../JEditorAttribute.h"
#include"../../JEditorPageShareData.h" 
#include"../../../Event/JEditorEvent.h"
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"  
#include"../../../Popup/JEditorPopupNode.h"
#include"../../../../Core/Reflection/JTypeTemplate.h"
#include"../../../../Core/FSM/JFSMfactory.h"
#include"../../../../Core/FSM/AnimationFSM/JAnimationStateType.h"
#include"../../../../Core/FSM/AnimationFSM/JAnimationFSMdiagram.h"
#include"../../../../Core/FSM/AnimationFSM/JAnimationFSMstate.h"
#include"../../../../Core/FSM/AnimationFSM/JAnimationFSMstateClip.h"
#include"../../../../Core/FSM/AnimationFSM/JAnimationFSMtransition.h"
#include"../../../../Object/Resource/JResourceManager.h"
#include"../../../../Object/Resource/AnimationController/JAnimationController.h"  
#include"../../../../Application/JApplicationVariable.h"

namespace JinEngine
{
	namespace Editor
	{
		namespace Constants
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
				return JApplicationVariable::GetProjectEditorResourcePath() + L"\\AniContViewGraph.txt";
			}
		}

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
			stateGraph->LoadData(Constants::ViewGraphPath());
			stateGraph->UseBeginViewWindow(false);

			uint count = 0;
			auto handle = JResourceManager::Instance().GetResourceVectorHandle<JAnimationController>(count);
			for (uint i = 0; i < count; ++i)
				RegisterViewGraphGroup(static_cast<JAnimationController*>(*(handle + i)));

			//Diagram View Popup
			std::unique_ptr<JEditorPopupNode> stateViewRootNode =
				std::make_unique<JEditorPopupNode>("Animation Controller Editor State View Popup Root", J_EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

			std::unique_ptr<JEditorPopupNode> createNewCilpStateNode =
				std::make_unique<JEditorPopupNode>("Create New State", J_EDITOR_POPUP_NODE_TYPE::LEAF, stateViewRootNode.get());
			editorString->AddString(createNewCilpStateNode->GetNodeId(), { "Create New Animation Clip" , u8"애니메이션 클립 생성" });

			std::unique_ptr<JEditorPopupNode> destroyStateNode =
				std::make_unique<JEditorPopupNode>("Destroy State", J_EDITOR_POPUP_NODE_TYPE::LEAF, stateViewRootNode.get());
			editorString->AddString(destroyStateNode->GetNodeId(), { "Destroy Animation State" , u8"애니메이션 상태 삭제" });

			std::unique_ptr<JEditorPopupNode> addTransitionNode =
				std::make_unique<JEditorPopupNode>("Add Transition", J_EDITOR_POPUP_NODE_TYPE::LEAF, stateViewRootNode.get());
			editorString->AddString(addTransitionNode->GetNodeId(), { "Add Transition" , u8"트랜지션 추가" });

			std::unique_ptr<JEditorPopupNode> destroyTransitionNode =
				std::make_unique<JEditorPopupNode>("Add Transition", J_EDITOR_POPUP_NODE_TYPE::LEAF, stateViewRootNode.get());
			editorString->AddString(destroyTransitionNode->GetNodeId(), { "Destroy Transition" , u8"트랜지션 삭제" });

			regCreateStateEvF = std::make_unique<RegisterEvF::Functor>(&JAnimationStateView::RegisterCreateStateEv, this);
			regDestroyStateEvF = std::make_unique<RegisterEvF::Functor>(&JAnimationStateView::RegisterDestroyStateEv, this); 
			regDestroyTransitionEvF = std::make_unique<RegisterEvF::Functor>(&JAnimationStateView::RegisterDestroyTransitionEv, this);

			createStateF = std::make_unique<CreateStateF::Functor>(&JAnimationStateView::CreateState, this);
			destroyStateF = std::make_unique<CreateStateF::Functor>(&JAnimationStateView::DestroyState, this);
			createTransitionF = std::make_unique<CreateTransitionF::Functor>(&JAnimationStateView::CreateTranstion, this);
			destroyTransitionF = std::make_unique<DestroyTransitionF::Functor>(&JAnimationStateView::DestroyTransition, this);
			undoDestroyF = std::make_unique<UndoDestroyF::Functor>(&JAnimationStateView::UndoDestroy, this);
			
			auto tryConnectStateTransLam = [](JAnimationStateView* stateView)
			{
				using bType = ConnectStateTrasitionF::CompletelyBind;
				if (stateView->stateGraph->IsLastUpdateSeletedNode())
					stateView->stateGraph->SetConnectNodeMode(std::make_unique<bType>(*stateView->connectStateTransF, std::move(stateView)));
			};
			auto connectStateTransLam = [](JAnimationStateView* stateView)
			{
				const size_t fromGuid = stateView->stateGraph->GetConnectFromGuid();
				const size_t toGuid = stateView->stateGraph->GetConnectToGuid();

				stateView->RegisterCreateTransitionEv(stateView->stateGraph->GetConnectFromGuid(),
					stateView->stateGraph->GetConnectToGuid());
			};
			tryConnectStateTransF = std::make_unique<TryConnectStateTransitionF::Functor>(tryConnectStateTransLam);
			connectStateTransF = std::make_unique<ConnectStateTrasitionF::Functor>(connectStateTransLam);

			createNewCilpStateNode->RegisterSelectBind(std::make_unique<RegisterEvF::CompletelyBind>(*regCreateStateEvF));
			destroyStateNode->RegisterSelectBind(std::make_unique<RegisterEvF::CompletelyBind>(*regDestroyStateEvF));
			addTransitionNode->RegisterSelectBind(std::make_unique<TryConnectStateTransitionF::CompletelyBind>(*tryConnectStateTransF, this));
			destroyTransitionNode->RegisterSelectBind(std::make_unique<RegisterEvF::CompletelyBind>(*regDestroyTransitionEvF));

			statePopup = std::make_unique< JEditorPopupMenu>(Constants::StateViewName(GetName()), std::move(stateViewRootNode));
			statePopup->AddPopupNode(std::move(createNewCilpStateNode));
			statePopup->AddPopupNode(std::move(destroyStateNode));
			statePopup->AddPopupNode(std::move(addTransitionNode));
			statePopup->AddPopupNode(std::move(destroyTransitionNode));
		}
		JAnimationStateView::~JAnimationStateView()
		{
			stateGraph->StoreData(Constants::ViewGraphPath());
		}
		J_EDITOR_WINDOW_TYPE JAnimationStateView::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::ANIMATION_STATE_VIEW;
		}
		void JAnimationStateView::Initialize(Core::JUserPtr<JAnimationController> newAnicont)noexcept
		{
			aniCont = newAnicont;
			RegisterViewGraphGroup(aniCont.Get());
		}
		void JAnimationStateView::UpdateWindow()
		{
			EnterWindow(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);
			UpdateDocking();
			if (IsActivated())
			{
				auto selected = JEditorPageShareData::GetSelectedObj(GetOwnerPageType());
				const bool isValidGameObject = selected.IsValid() && selected->GetTypeInfo().IsChildOf<Core::JFSMInterface>();
				if (isValidGameObject)
				{
					if (selected->GetTypeInfo().IsChildOf<Core::JAnimationFSMstate>())
					{
						selectedState.ConnnectChildUser(selected);
						selectedTransition.Clear();
					}
					else if (selected->GetTypeInfo().IsChildOf<Core::JAnimationFSMtransition>())
					{
						selectedTransition.ConnnectChildUser(selected);
						selectedState.Clear();
					}
				}
 
				UpdateMouseClick();
				BuildDiagramView();
			}
			CloseWindow();
		}
		void JAnimationStateView::BuildDiagramView()
		{
			stateGraph->ClearNode();
			stateGraph->SetGridSize(2000);
			if (aniCont.IsValid() && selectedDiagram.IsValid())
			{
				const uint stateCount = selectedDiagram->GetStateCount();
				for (uint i = 0; i < stateCount; ++i)
				{
					Core::JAnimationFSMstate* state = selectedDiagram->GetStateByIndex(i);
					stateGraph->BuildNode(JCUtil::WstrToU8Str(state->GetName()), state->GetGuid(), aniCont->GetGuid());
				}
				for (uint i = 0; i < stateCount; ++i)
				{
					Core::JAnimationFSMstate* state = selectedDiagram->GetStateByIndex(i);
					const size_t fromGuid = state->GetGuid();
					const uint transitionCount = state->GetTransitionCount();
					for (uint j = 0; j < transitionCount; ++j)
						stateGraph->ConnectNode(fromGuid, state->GetTransitionByIndex(j)->GetOutputStateGuid());
				}
				JImGuiImpl::Text(JCUtil::WstrToU8Str(selectedDiagram->GetName()).c_str());
				stateGraph->OnScreen(aniCont->GetGuid());
				if (stateGraph->IsLastUpdateSeletedNode())
				{
					const size_t guid = stateGraph->GetLastUpdateSeletedNodeGuid();
					SetSelecteObject(selectedState, Core::GetUserPtr(selectedDiagram->GetState(guid)));
				}
				else if (stateGraph->IsLastUpdateSeletedEdge())
				{
					size_t fromGuid;
					size_t toGuid;
					stateGraph->GetLastUpdateSelectedEdgeGuid(fromGuid, toGuid);
					Core::JFSMtransition* tPtr = selectedDiagram->GetState(fromGuid)->GetTransitionByOutGuid(toGuid);
					SetSelecteObject(selectedTransition, Core::GetUserPtr<Core::JAnimationFSMtransition>(tPtr));
				}

				if (statePopup->IsOpen())
					statePopup->ExecutePopup(editorString.get());
				statePopup->Update();
			}
			else
				stateGraph->OnScreen();
		}
		void JAnimationStateView::SetSelecteObject(Core::JUserPtr<Core::JIdentifier> preSelected, Core::JUserPtr<Core::JIdentifier> newSelected)
		{
			const bool canSeleted = !preSelected.IsValid() || (newSelected.IsValid() && newSelected->GetGuid() != preSelected->GetGuid());
			if (canSeleted)
				RequestSelectObject(JEditorSelectObjectEvStruct(GetOwnerPageType(), newSelected));
		}
		void JAnimationStateView::RegisterViewGraphGroup(JAnimationController* aniCont)
		{
			auto isValidGroupLam = [](const size_t guid) {return Core::GetUserPtr<JAnimationController>(guid).IsValid(); };
			if (aniCont != nullptr)
				stateGraph->RegisterGroup(aniCont->GetGuid(), isValidGroupLam);
		}
		void JAnimationStateView::RegisterCreateStateEv()
		{
			if (!aniCont.IsValid())
				return;

			using BindT = JAnimationStateView::CreateStateF::Bind;
			size_t guid = Core::MakeGuid();
			auto doBind = std::make_unique<BindT>(*createStateF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, selectedDiagram->GetGuid(), std::move(guid));
			auto undoBind = std::make_unique<BindT>(*destroyStateF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, selectedDiagram->GetGuid(), std::move(guid));

			using CreaetStateTEv = JEditorTCreateBindFuncEvStruct<DataHandleStructure, CreateStateF::Bind, CreateStateF::Bind>;
			auto evStruct = JEditorEvent::RegisterEvStruct(std::make_unique<CreaetStateTEv>
				("Create Animation Fsm State", GetOwnerPageType(), std::move(doBind), std::move(undoBind), fsmdata));
			AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::T_BIND_FUNC, evStruct);
		}
		void JAnimationStateView::RegisterDestroyStateEv()
		{
			if (!aniCont.IsValid() || !selectedState.IsValid())
				return;
			 
			auto doBind = std::make_unique<CreateStateF::Bind>(*createStateF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, selectedDiagram->GetGuid(), selectedState->GetGuid());
			auto undoBind = std::make_unique<UndoDestroyF::Bind>(*undoDestroyF, Core::empty, Core::empty, Core::JUserPtr{ aniCont });

			using DestroyStateTEv = JEditorTCreateBindFuncEvStruct<DataHandleStructure, CreateStateF::Bind, UndoDestroyF::Bind>;
			auto evStruct = JEditorEvent::RegisterEvStruct(std::make_unique<DestroyStateTEv>
				("Destroy Animation Fsm State", GetOwnerPageType(), std::move(doBind), std::move(undoBind), fsmdata));
			AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::T_BIND_FUNC, evStruct);
		}
		void JAnimationStateView::RegisterCreateTransitionEv(const size_t inGuid, const size_t outGuid)
		{
			if (!aniCont.IsValid())
				return;
			 
			size_t guid = Core::MakeGuid();
			auto doBind = std::make_unique<CreateTransitionF::Bind>(*createTransitionF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, selectedDiagram->GetGuid(), std::move(inGuid), std::move(outGuid));
			auto undoBind = std::make_unique<DestroyTransitionF::Bind>(*destroyTransitionF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, selectedDiagram->GetGuid());

			using CreateTransitionTEv = JEditorTCreateBindFuncEvStruct<DataHandleStructure, CreateTransitionF::Bind, DestroyTransitionF::Bind>;
			auto evStruct = JEditorEvent::RegisterEvStruct(std::make_unique<CreateTransitionTEv>
				("Create Animation Fsm State", GetOwnerPageType(), std::move(doBind), std::move(undoBind), fsmdata));
			AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::T_BIND_FUNC, evStruct);
		}
		void JAnimationStateView::RegisterDestroyTransitionEv()
		{
			if (!aniCont.IsValid() || !selectedTransition.IsValid())
				return;

			auto doBind = std::make_unique<DestroyTransitionF::Bind>(*destroyTransitionF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, selectedDiagram->GetGuid());
			auto undoBind = std::make_unique<UndoDestroyF::Bind>(*undoDestroyF, Core::empty, Core::empty, Core::JUserPtr{ aniCont });

			using DestroyTransitionTEv = JEditorTCreateBindFuncEvStruct<DataHandleStructure, DestroyTransitionF::Bind, UndoDestroyF::Bind>;
			auto evStruct = JEditorEvent::RegisterEvStruct(std::make_unique<DestroyTransitionTEv>
				("Destroy Animation Fsm State", GetOwnerPageType(), std::move(doBind), std::move(undoBind), fsmdata));
			AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::T_BIND_FUNC, evStruct);
		}
		void JAnimationStateView::CreateState(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t diagramGuid, const size_t stateGuid)
		{
			if (aniCont.IsValid())
			{
				Core::JOwnerPtr<Core::JIdentifier> owner = dS.Release(dH);
				if (owner.IsValid() && Core::Cast<Core::JAnimationFSMstate>(owner.Get()))
				{
					Core::J_ANIMATION_STATE_TYPE sType = Core::Cast<Core::JAnimationFSMstate>(owner.Get())->GetStateType();
					if (sType == Core::J_ANIMATION_STATE_TYPE::CLIP)
					{
						auto clip = Core::JOwnerPtr<Core::JAnimationFSMstateClip>::ConvertChildUser(std::move(owner));
						auto ptr = clip.Get();
						Core::JIdentifier::AddInstance(std::move(clip));
						JEditorPageShareData::SetSelectObj(GetOwnerPageType(), Core::GetUserPtr(ptr));
					}
					else //미구현
						;//AddInstance(Core::JOwnerPtr<Core::JAnimationBlend>::ConvertChildUser(std::move(owner)));
				}
				else
					aniCont->CreateFSMClip(aniCont->GetDiagram(diagramGuid), stateGuid);
				SetModifiedBit(aniCont, true);
			}
		}
		void JAnimationStateView::DestroyState(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t diagramGuid, const size_t stateGuid)
		{
			if (aniCont.IsValid())
			{
				auto diagramPtr = aniCont->GetDiagram(diagramGuid);
				if (diagramPtr != nullptr)
				{
					auto statePtr = diagramPtr->GetState(stateGuid);
					if (statePtr != nullptr)
					{ 
						if (statePtr->GetStateType() == Core::J_ANIMATION_STATE_TYPE::CLIP)
						{
							Core::JDataHandle newHandle = dS.Add(Core::JIdentifier::ReleaseInstance<Core::JAnimationFSMstateClip>(statePtr->GetGuid()));
							dS.TransitionHandle(newHandle, dH);
						}
						else
						{
							//미구현
							//	Core::JDataHandle newHandle = dataStructure.Add(ReleaseInstance<Core::JFSMparameter>(conditionPtr->GetGuid()));
							//	dataStructure.TransitionHandle(newHandle, dataHandle);
						}
						JEditorDeSelectObjectEvStruct deselectEv{ GetOwnerPageType(), stateGuid };
						NotifyEvent(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::DESELECT_OBJECT, &deselectEv);
					}
					SetModifiedBit(aniCont, true);
				}
			}
		}
		void JAnimationStateView::CreateTranstion(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t diagramGuid, const size_t stateGuid, const size_t outGuid)
		{
			if (aniCont.IsValid())
			{
				auto diagramPtr = aniCont->GetDiagram(diagramGuid);
				Core::JOwnerPtr<Core::JIdentifier> owner = dS.Release(dH);
				if (owner.IsValid() && Core::Cast<Core::JAnimationFSMtransition>(owner.Get()))
					Core::JIdentifier::AddInstance(std::move(owner));
				else
				{
					auto diagram = aniCont->GetDiagram(diagramGuid);
					aniCont->CreateFsmTransition(diagram, diagram->GetState(stateGuid), diagram->GetState(outGuid));
				}
				SetModifiedBit(aniCont, true);
			}
		}
		void JAnimationStateView::DestroyTransition(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t diagramGuid)
		{
			if (aniCont.IsValid())
			{
				auto diagramPtr = aniCont->GetDiagram(diagramGuid);
				if (diagramPtr != nullptr && selectedTransition.IsValid())
				{
					const size_t guid = selectedTransition->GetGuid();
					Core::JDataHandle newHandle = dS.Add(Core::JIdentifier::ReleaseInstance<Core::JAnimationFSMtransition>(guid));
					dS.TransitionHandle(newHandle, dH);

					JEditorDeSelectObjectEvStruct deselectEv{ GetOwnerPageType(), guid };
					NotifyEvent(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::DESELECT_OBJECT, &deselectEv);
					SetModifiedBit(aniCont, true);
				}
			}
		}
		void JAnimationStateView::UndoDestroy(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont)
		{
			auto owner = dS.Release(dH);
			if (owner.IsValid())
			{
				Core::JIdentifier* ptr = owner.Get();
				SetModifiedBit(aniCont, true);
				Core::JIdentifier::AddInstance(std::move(owner));

				JEditorSelectObjectEvStruct selectEv{ GetOwnerPageType(), Core::GetUserPtr(ptr) };
				NotifyEvent(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::SELECT_OBJECT, &selectEv);
			}
		}
		void JAnimationStateView::DoSetClose()noexcept
		{
			aniCont.Clear();
			selectedDiagram.Clear();
			selectedState.Clear();
			selectedTransition.Clear();
		}
		void JAnimationStateView::DoActivate()noexcept
		{
			JEditorWindow::DoActivate();
			std::vector<J_EDITOR_EVENT> enumVec
			{
				J_EDITOR_EVENT::MOUSE_CLICK, J_EDITOR_EVENT::SELECT_OBJECT
			};
			RegisterEventListener(enumVec);
		}
		void JAnimationStateView::DoDeActivate()noexcept
		{
			JEditorWindow::DoDeActivate();
			DeRegisterListener();
		}
		void JAnimationStateView::OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* ev)
		{
			if (senderGuid == GetGuid())
				return;

			if (eventType == J_EDITOR_EVENT::MOUSE_CLICK)
				statePopup->SetOpen(false);
			else if (eventType == J_EDITOR_EVENT::SELECT_OBJECT)
			{
				JEditorSelectObjectEvStruct* evstruct = static_cast<JEditorSelectObjectEvStruct*>(ev);
				if (evstruct->selectObj->GetTypeInfo().IsChildOf<Core::JAnimationFSMdiagram>())
				{
					if (!selectedDiagram.IsValid() || selectedDiagram->GetGuid() != evstruct->selectObj->GetGuid())
					{
						stateGraph->ClearSeletedCash();
						selectedDiagram.ConnnectChildUser(evstruct->selectObj);
					}
				}
			}
		}
	}
}
