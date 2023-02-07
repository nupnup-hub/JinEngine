#include"JAnimationControllerEditor.h"
#include"../../JEditorAttribute.h"
#include"../../JEditorPageShareData.h"
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h" 
#include"../../../String/JEditorStringMap.h"
#include"../../../Helpers/JEditorInputBuffHelper.h"
#include"../../../EditTool/JEditorViewStructure.h"
#include"../../../Popup/JEditorPopupMenu.h"
#include"../../../Popup/JEditorPopupNode.h"
#include"../../../Diagram/JEditorDiagramNode.h" 

#include"../../../../Object/Resource/AnimationController/JAnimationController.h"
#include"../../../../Object/Resource/JResourceManager.h"
#include"../../../../Object/Component/Animator/JAnimator.h"
#include"../../../../Object/GameObject/JGameObject.h"
#include"../../../../Core/FSM/JFSMfactory.h"
#include"../../../../Core/FSM/AnimationFSM/JAnimationFSMdiagram.h"
#include"../../../../Core/FSM/AnimationFSM/JAnimationFSMstate.h"
#include"../../../../Core/FSM/AnimationFSM/JAnimationFSMstateClip.h"
#include"../../../../Core/FSM/AnimationFSM/JAnimationFSMtransition.h"
#include"../../../../Core/FSM/JFSMcondition.h"
#include"../../../../Core/Reflection/JTypeTemplate.h"
#include"../../../../Core/Guid/GuidCreator.h"
#include"../../../../Application/JApplicationVariable.h"
#include"../../../../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Editor
	{
		struct DiagramViewFuncData
		{
		public:
			float windowPosX;
			float windowPosY;
			float windowWidth;
			float windowHeight;
		};
		namespace Constants
		{
			static constexpr uint stateShapeWidth = 100;
			static constexpr uint stateShapeHeight = 150;
			static constexpr uint frameThickness = 3;

			static JVector2<float> StateShapeSize()noexcept
			{
				return JVector2<float>(stateShapeWidth, stateShapeHeight);
			}
			static JVector2<float> FrameThicknessVec()noexcept
			{
				return JVector2<float>(frameThickness, frameThickness);
			}
			static std::string DockNodeName(const std::string& uniqueLabel)noexcept
			{
				return uniqueLabel + "_DockNode";
			}
			static std::string DiagramListName(const std::string& uniqueLabel)noexcept
			{
				return "DiagramList##" + uniqueLabel;
			}
			static std::string ConditionListName(const std::string& uniqueLabel)noexcept
			{
				return "ConditionList##" + uniqueLabel;
			}
			static std::string DiagramViewName(const std::string& uniqueLabel)noexcept
			{
				return "DiagramView##" + uniqueLabel;
			}
			static std::wstring ViewGraphPath()noexcept
			{
				return JApplicationVariable::GetProjectEditorResourcePath() + L"\\AniContViewGraph.txt";
			}
		}

		JAnimationControllerEditor::JAnimationControllerEditor(const std::string& name,
			std::unique_ptr<JEditorAttribute> attribute,
			const J_EDITOR_PAGE_TYPE ownerPageType,
			const J_EDITOR_WINDOW_FLAG windowFlag,
			const bool hasMetadata)
			:JEditorWindow(name, std::move(attribute), ownerPageType, windowFlag), reqInitDockNode(!hasMetadata)
		{
			editorString = std::make_unique<JEditorStringMap>();
			inputBuff = std::make_unique<JEditorInputBuffHelper>(JImGuiImpl::GetTextBuffRange());
			stateGraph = std::make_unique<JEditorGraphView>();
			stateGraph->GetGrid()->SetMinZoomRate(50);
			stateGraph->GetGrid()->SetMaxZoomRate(50);
			RegisterDiagramFunc();
			RegisterConditionFunc();
			RegisterStateFunc();

			stateGraph->LoadData(Constants::ViewGraphPath());

			uint count = 0;
			auto handle = JResourceManager::Instance().GetResourceVectorHandle<JAnimationController>(count);
			for (uint i = 0; i < count; ++i)
				RegisterViewGraphGroup(static_cast<JAnimationController*>(*(handle + i)));
		}
		JAnimationControllerEditor::~JAnimationControllerEditor()
		{
			SetAnimationController(Core::JUserPtr< JObject>{});
			stateGraph->StoreData(Constants::ViewGraphPath());
		};

		J_EDITOR_WINDOW_TYPE JAnimationControllerEditor::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::ANIMATION_CONTROLLER_EDITOR;
		}
		void JAnimationControllerEditor::SetAnimationController(Core::JUserPtr<JObject> newAniCont)
		{
			bool isValid = newAniCont.IsValid() &&
				newAniCont->GetObjectType() == J_OBJECT_TYPE::RESOURCE_OBJECT &&
				static_cast<JResourceObject*>(newAniCont.Get())->GetResourceType() == J_RESOURCE_TYPE::ANIMATION_CONTROLLER;

			if (isValid)
			{
				// SetValid AniCont
				ClearSelectedIndex();
				CallOffResourceReference(aniCont.Get());
				aniCont.ConnnectChildUser(newAniCont);
				CallOnResourceReference(aniCont.Get());
				if (aniCont->GetDiagramCount() > 0)
					selectedDiagramIndex = 0;

				if (!stateGraph->HasGroupData(aniCont->GetGuid()))
					RegisterViewGraphGroup(aniCont.Get());
			}
			else if (!newAniCont.IsValid() && HasAnimationController())
			{
				// Set Nullptr
				CallOffResourceReference(aniCont.Get());
				aniCont.Clear();
			}
		}
		bool JAnimationControllerEditor::HasAnimationController()const noexcept
		{
			return aniCont.IsValid();
		}
		void JAnimationControllerEditor::RegisterDiagramFunc()
		{
			//Diagram List Popup
			std::unique_ptr<JEditorPopupNode> diagramListRootNode =
				std::make_unique<JEditorPopupNode>("Animation Controller Editor Diagram List Popup Root", J_EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

			std::unique_ptr<JEditorPopupNode> createNewDiagramNode =
				std::make_unique<JEditorPopupNode>("Create New Diagram", J_EDITOR_POPUP_NODE_TYPE::LEAF, diagramListRootNode.get());
			editorString->AddString(createNewDiagramNode->GetNodeId(), { "Create New Diagram" , u8"애니메이션 다이어그램 생성" });

			std::unique_ptr<JEditorPopupNode> destroyDigamraNode =
				std::make_unique<JEditorPopupNode>("Destroy Diagram", J_EDITOR_POPUP_NODE_TYPE::LEAF, diagramListRootNode.get());
			editorString->AddString(destroyDigamraNode->GetNodeId(), { "Destroy Diagram" , u8"애니메이션 다이어그램 삭제" });

			regCreateDiagramEvF = std::make_unique<RegisterEvF::Functor>(&JAnimationControllerEditor::RegisterCreateDiagramEv, this);
			regDestroyDiagramEvF = std::make_unique<RegisterEvF::Functor>(&JAnimationControllerEditor::RegisterDestroyDiagramEv, this);
			createDiagramF = std::make_unique<DiagramHandleFunctor>(&JAnimationControllerEditor::CreateDiagram, this);
			destroyDiagramF = std::make_unique<DiagramHandleFunctor>(&JAnimationControllerEditor::DestroyDiagram, this);

			createNewDiagramNode->RegisterSelectBind(std::make_unique< RegisterEvF::CompletelyBind>(*regCreateDiagramEvF));
			destroyDigamraNode->RegisterSelectBind(std::make_unique< RegisterEvF::CompletelyBind>(*regDestroyDiagramEvF));

			diagramListPopup = std::make_unique<JEditorPopupMenu>(Constants::DiagramListName(GetName()), std::move(diagramListRootNode));
			diagramListPopup->AddPopupNode(std::move(createNewDiagramNode));
			diagramListPopup->AddPopupNode(std::move(destroyDigamraNode));
		}
		void JAnimationControllerEditor::RegisterConditionFunc()
		{
			//Condition List Popup
			std::unique_ptr<JEditorPopupNode> conditionListRootNode =
				std::make_unique<JEditorPopupNode>("Animation Controller Editor Condition List Popup Root", J_EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

			std::unique_ptr<JEditorPopupNode> createNewConditionNode =
				std::make_unique<JEditorPopupNode>("Create New Condition", J_EDITOR_POPUP_NODE_TYPE::LEAF, conditionListRootNode.get());
			editorString->AddString(createNewConditionNode->GetNodeId(), { "Create New Condition" , u8"애니메이션 패러미터 생성" });

			std::unique_ptr<JEditorPopupNode> destroyConditionNode =
				std::make_unique<JEditorPopupNode>("Destroy Condition", J_EDITOR_POPUP_NODE_TYPE::LEAF, conditionListRootNode.get());
			editorString->AddString(destroyConditionNode->GetNodeId(), { "Destroy Condition" , u8"애니메이션 패러미터 삭제" });

			regCreateConditionEvF = std::make_unique<RegisterEvF::Functor>(&JAnimationControllerEditor::RegisterCreateConditionEv, this);
			regDestroyConditionEvF = std::make_unique<RegisterEvF::Functor>(&JAnimationControllerEditor::RegisterDestroyConditionEv, this);
			createConditionF = std::make_unique<ConditionHandleFunctor>(&JAnimationControllerEditor::CreateCondition, this);
			destroyConditionF = std::make_unique<ConditionHandleFunctor>(&JAnimationControllerEditor::DestroyCondition, this);

			createNewConditionNode->RegisterSelectBind(std::make_unique<RegisterEvF::CompletelyBind>(*regCreateConditionEvF));
			destroyConditionNode->RegisterSelectBind(std::make_unique<RegisterEvF::CompletelyBind>(*regDestroyConditionEvF));

			conditionListPopup = std::make_unique<JEditorPopupMenu>(Constants::ConditionListName(GetName()), std::move(conditionListRootNode));
			conditionListPopup->AddPopupNode(std::move(createNewConditionNode));
			conditionListPopup->AddPopupNode(std::move(destroyConditionNode));

			auto setConditionTypeLam = [](const Core::J_FSMCONDITION_VALUE_TYPE vType, AniContUserPtr aniCont, size_t guid)
			{
				aniCont->GetCondition(guid)->SetValueType(vType);
			};
			auto setConditionNameLam = [](const std::string value, AniContUserPtr aniCont, size_t guid)
			{
				aniCont->GetCondition(guid)->SetName(JCUtil::U8StrToWstr(value));
			};
			auto setConditionBoolLam = [](const bool value, AniContUserPtr aniCont, size_t guid)
			{
				aniCont->GetCondition(guid)->SetValue(value);
			};
			auto setConditionIntLam = [](const int value, AniContUserPtr aniCont, size_t guid)
			{
				aniCont->GetCondition(guid)->SetValue(value);
			};
			auto setConditionFloatLam = [](const float value, AniContUserPtr aniCont, size_t guid)
			{
				aniCont->GetCondition(guid)->SetValue(value);
			};

			setConditionTypeF = std::make_unique<SetConditionTypeFunctor>(setConditionTypeLam);
			setConditionNameF = std::make_unique<SetConditionNameFunctor>(setConditionNameLam);
			setConditionBoolF = std::make_unique< SetConditionBooleanValueFunctor>(setConditionBoolLam);
			setConditionIntF = std::make_unique< SetConditionIntValueFunctor>(setConditionIntLam);
			setConditionFloatF = std::make_unique< SetConditionFloatValueFunctor>(setConditionFloatLam);
		}
		void JAnimationControllerEditor::RegisterStateFunc()
		{
			//Diagram View Popup
			std::unique_ptr<JEditorPopupNode> diagramViewRootNode =
				std::make_unique<JEditorPopupNode>("Animation Controller Editor Diagram View Popup Root", J_EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

			std::unique_ptr<JEditorPopupNode> createNewCilpStateNode =
				std::make_unique<JEditorPopupNode>("Create New State", J_EDITOR_POPUP_NODE_TYPE::LEAF, diagramViewRootNode.get());
			editorString->AddString(createNewCilpStateNode->GetNodeId(), { "Create New Animation Clip" , u8"애니메이션 클립 생성" });

			std::unique_ptr<JEditorPopupNode> destroyStateNode =
				std::make_unique<JEditorPopupNode>("Destroy State", J_EDITOR_POPUP_NODE_TYPE::LEAF, diagramViewRootNode.get());
			editorString->AddString(destroyStateNode->GetNodeId(), { "Destroy Animation State" , u8"애니메이션 상태 삭제" });

			std::unique_ptr<JEditorPopupNode> addTransitionNode =
				std::make_unique<JEditorPopupNode>("Add Transition", J_EDITOR_POPUP_NODE_TYPE::LEAF, diagramViewRootNode.get());
			editorString->AddString(addTransitionNode->GetNodeId(), { "Add Transition" , u8"트랜지션 추가" });

			regCreateStateEvF = std::make_unique<RegisterEvF::Functor>(&JAnimationControllerEditor::RegisterCreateStateEv, this);
			regDestroyStateEvF = std::make_unique<RegisterEvF::Functor>(&JAnimationControllerEditor::RegisterDestroyStateEv, this);
			createStateF = std::make_unique<StateHandleFunctor>(&JAnimationControllerEditor::CreateState, this);
			destroyStateF = std::make_unique<StateHandleFunctor>(&JAnimationControllerEditor::DestroyState, this);

			createNewCilpStateNode->RegisterSelectBind(std::make_unique<RegisterEvF::CompletelyBind>(*regCreateStateEvF));
			destroyStateNode->RegisterSelectBind(std::make_unique<RegisterEvF::CompletelyBind>(*regDestroyStateEvF));
			 
			auto tryConnectStateTransLam = [](JAnimationControllerEditor* aniContEdit) 
			{ 
				using bType = ConnectStateTrasitionF::CompletelyBind; 
				aniContEdit->stateGraph->SetConnectNodeMode(std::make_unique<bType>(*aniContEdit->connectStateTransF, std::move(aniContEdit)));
			};
			auto connectStateTransLam = [](JAnimationControllerEditor* aniContEdit)
			{
				const size_t fromGuid = aniContEdit->stateGraph->GetConnectFromGuid();
				const size_t toGuid = aniContEdit->stateGraph->GetConnectToGuid();

				if (aniContEdit->selectedDiagramIndex == -1)
					return;

				Core::JAnimationFSMdiagram* diagram = aniContEdit->aniCont->GetDiagramByIndex(aniContEdit->selectedDiagramIndex);
				diagram->GetState(fromGuid)->AddTransition(diagram->GetState(toGuid));
			};

			tryConnectStateTransF = std::make_unique<TryConnectStateTransitionF::Functor>(tryConnectStateTransLam);
			connectStateTransF = std::make_unique<ConnectStateTrasitionF::Functor>(connectStateTransLam);
			
			addTransitionNode->RegisterSelectBind(std::make_unique<TryConnectStateTransitionF::CompletelyBind>(*tryConnectStateTransF, this));

			diagramViewPopup = std::make_unique< JEditorPopupMenu>(Constants::DiagramViewName(GetName()), std::move(diagramViewRootNode));
			diagramViewPopup->AddPopupNode(std::move(createNewCilpStateNode));
			diagramViewPopup->AddPopupNode(std::move(destroyStateNode));
			diagramViewPopup->AddPopupNode(std::move(addTransitionNode));
		}
		void JAnimationControllerEditor::RegisterViewGraphGroup(JAnimationController* aniCont)
		{
			auto isValidGroupLam = [](const size_t guid) {return Core::GetUserPtr<JAnimationController>(guid).IsValid(); };
			if (aniCont != nullptr)
				stateGraph->RegisterGroup(aniCont->GetGuid(), isValidGroupLam);
		}
		void JAnimationControllerEditor::UpdateWindow()
		{
			ImGuiWindowFlags guiWindowFlag = ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoNavInputs |
				ImGuiWindowFlags_NoNavFocus |
				ImGuiWindowFlags_NoBackground;

 

			EnterWindow(guiWindowFlag);
			UpdateDocking();
			if (IsActivated())
			{
				if (reqInitDockNode)
				{
					BuildDockNode();
					reqInitDockNode = false;
				}
				ImGuiID dockspace_id = ImGui::GetID(Constants::DockNodeName(GetName()).c_str());
				ImGui::DockSpace(dockspace_id);
			}
			CloseWindow();
			if (IsActivated())
			{
				UpdateMouseClick();
				BuildDiagramList();
				BuildConditionList();

				//JImGuiImpl::SetColor(ImVec4(0, 0, 0, 0), ImGuiCol_Header);
				//JImGuiImpl::SetColor(ImVec4(0, 0, 0, 0), ImGuiCol_HeaderHovered);
				//JImGuiImpl::SetColor(ImVec4(0, 0, 0, 0), ImGuiCol_HeaderActive);
				BuildDiagramView();
				//JImGuiImpl::SetColorToDefault(ImGuiCol_Header);
				//JImGuiImpl::SetColorToDefault(ImGuiCol_HeaderHovered);
				//JImGuiImpl::SetColorToDefault(ImGuiCol_HeaderActive);

				preMousePosX = ImGui::GetMousePos().x;
				preMousePosY = ImGui::GetMousePos().y;
			}
		}
		void JAnimationControllerEditor::BuildDiagramList()
		{
			ImGuiWindowFlags guiWindowFlag = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;
			JImGuiImpl::BeginWindow(Constants::DiagramListName(GetName()), nullptr, guiWindowFlag);
			JImGuiImpl::Text("DiagramList");

			ImGuiTableFlags flag = ImGuiTableFlags_Borders | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
			if (HasAnimationController() && JImGuiImpl::BeginTable("##DiagramListTable" + GetName(), 1, flag))
			{
				JImGuiImpl::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
				JImGuiImpl::TableHeadersRow();

				const std::vector<Core::JAnimationFSMdiagram*>& diagramVec = aniCont->GetDiagramVec();
				const uint diagramCount = (uint)diagramVec.size();
				for (uint i = 0; i < diagramCount; ++i)
				{
					JImGuiImpl::TableNextRow();
					JImGuiImpl::TableSetColumnIndex(0);

					const bool isSelect = i == selectedDiagramIndex;
					if (JImGuiImpl::Selectable(JCUtil::WstrToU8Str(diagramVec[i]->GetName()), &isSelect))
					{
						if (!isSelect)
							selectedDiagramIndex = i;
					}
				}
				JImGuiImpl::EndTable();
			}
			if (HasAnimationController())
			{
				if (diagramListPopup->IsOpen())
					diagramListPopup->ExecutePopup(editorString.get());
				diagramListPopup->Update();
			}
			JImGuiImpl::EndWindow();
		}
		void JAnimationControllerEditor::BuildConditionList()
		{
			ImGuiWindowFlags guiWindowFlag = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;
			JImGuiImpl::BeginWindow(Constants::ConditionListName(GetName()), nullptr, guiWindowFlag);
			JImGuiImpl::Text("Condition");

			ImGuiTableFlags flag = ImGuiTableFlags_BordersV |
				ImGuiTableFlags_BordersOuterH |
				ImGuiTableFlags_RowBg |
				ImGuiTableFlags_ContextMenuInBody |
				ImGuiTableFlags_Resizable;

			if (HasAnimationController() && JImGuiImpl::BeginTable("##ConditionList_Table" + GetName(), 4, flag))
			{
				JImGuiImpl::TableSetupColumn("", ImGuiTableColumnFlags_DefaultHide, 0.005f);
				JImGuiImpl::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.445f);
				JImGuiImpl::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 0.275f);
				JImGuiImpl::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.275f);
				JImGuiImpl::TableHeadersRow();
				const uint conditionCount = aniCont->GetConditionCount();
				for (uint i = 0; i < conditionCount; ++i)
				{
					Core::JFSMcondition* nowCondition = aniCont->GetConditionByIndex(i);
					std::string name = JCUtil::WstrToU8Str(nowCondition->GetName());

					JImGuiImpl::TableNextRow();
					JImGuiImpl::TableSetColumnIndex(0);

					const bool isSelect = i == selectedConditionIndex;
					if (JImGuiImpl::Selectable(("##ConditionName_Selectable" + name).c_str(), &isSelect, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap))
					{
						if (!isSelect)
							selectedConditionIndex = i;
						inputBuff->SetBuff(name);
					}
					JImGuiImpl::TableSetColumnIndex(1);
					ImGui::PushItemWidth(-FLT_MIN);
					if (isSelect)
					{
						/*
												const JVector2<float> itemPos = ImGui::GetCursorPos() + ImGui::GetWindowPos();

						if (JImGuiImpl::InputTextSetT(GetName(),
							inputBuff.get(),
							"New name...",
							ImGuiInputTextFlags_EnterReturnsTrue,
							*setConditionNameF,
							aniCont,
							nowCondition->GetGuid()))
							conditionListSelectable[i] = false;

						const JVector2<float> itemSize = ImGui::GetItemRectSize();
						if (JImGuiImpl::IsRightMouseClicked() || JImGuiImpl::IsLeftMouseClicked())
						{
							if (!JImGuiImpl::IsMouseInRect(itemPos, itemSize))
								conditionListSelectable[i] = false;
						}
						*/
					}
					else
						JImGuiImpl::Text(name.c_str());

					Core::J_FSMCONDITION_VALUE_TYPE valueType = nowCondition->GetValueType();
					JImGuiImpl::TableSetColumnIndex(2);
					ImGui::PushItemWidth(-FLT_MIN);

					JImGuiImpl::ComoboEnumSetT(GetName(), valueType, *setConditionTypeF, aniCont, nowCondition->GetGuid());

					JImGuiImpl::TableSetColumnIndex(3);
					ImGui::PushItemWidth(-FLT_MIN);
					if (valueType == Core::J_FSMCONDITION_VALUE_TYPE::BOOL)
						JImGuiImpl::CheckBoxSetT(GetName(), (bool)nowCondition->GetValue(), *setConditionBoolF, aniCont, nowCondition->GetGuid());
					else if (valueType == Core::J_FSMCONDITION_VALUE_TYPE::INT)
						JImGuiImpl::InputIntSetT(GetName(), (int)nowCondition->GetValue(), *setConditionIntF, aniCont, nowCondition->GetGuid());
					else if (valueType == Core::J_FSMCONDITION_VALUE_TYPE::FLOAT)
						JImGuiImpl::InputFloatSetT(GetName(), (float)nowCondition->GetValue(), *setConditionFloatF, aniCont, nowCondition->GetGuid());
				}
				JImGuiImpl::EndTable();
			}
			if (HasAnimationController())
			{
				if (conditionListPopup->IsOpen())
					conditionListPopup->ExecutePopup(editorString.get());
				conditionListPopup->Update();
			}
			JImGuiImpl::EndWindow();
		}
		void JAnimationControllerEditor::BuildDiagramView()
		{
			stateGraph->Clear();
			stateGraph->SetGridSize(2000);
			ImGuiWindowFlags guiWindowFlag = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;
			if (stateGraph->BeginView(Constants::DiagramViewName(GetName()), nullptr, guiWindowFlag))
			{
				if (HasAnimationController())
				{
					Core::JAnimationFSMdiagram* diagram = aniCont->GetDiagramByIndex(selectedDiagramIndex);
					const uint stateCount = diagram->GetStateCount();
					for (uint i = 0; i < stateCount; ++i)
					{ 
						Core::JAnimationFSMstate* state = diagram->GetStateByIndex(i);
						stateGraph->BuildNode(JCUtil::WstrToU8Str(state->GetName()), state->GetGuid(), aniCont->GetGuid());
					}
					for (uint i = 0; i < stateCount; ++i)
					{
						Core::JAnimationFSMstate* state = diagram->GetStateByIndex(i);
						const size_t fromGuid = state->GetGuid();
						const uint transitionCount = state->GetTransitionCount();
						for (uint j = 0; j < transitionCount; ++j)
							stateGraph->ConnectNode(fromGuid, state->GetTransition(j)->GetOutputStateGuid());
					}
					JImGuiImpl::Text(JCUtil::WstrToU8Str(aniCont->GetDiagramByIndex(selectedDiagramIndex)->GetName()).c_str());
					stateGraph->OnScreen(aniCont->GetGuid());
					if (diagramViewPopup->IsOpen())
						diagramViewPopup->ExecutePopup(editorString.get());
					diagramViewPopup->Update();
				}
				else
					stateGraph->OnScreen();
				stateGraph->EndView();
			}
			//미구현
			/*std::vector<Core::JAnimationFSMtransition*>& transitionIter = stateVec[i]->GetTransitionVector();
			const uint transtionCount = (uint)transitionIter.size();
			for (uint j = 0; j < transtionCount; ++j)
			{
				Core::JAnimationFSMtransition* nowTransition = *(transitionIter + j);
			}*/
		}
		void JAnimationControllerEditor::BuildDockNode()
		{
			//ImGui::Begin(GetName().c_str()); ImGui::End();
			ImGui::Begin(Constants::DiagramListName(GetName()).c_str()); ImGui::End();
			ImGui::Begin(Constants::ConditionListName(GetName()).c_str()); ImGui::End();
			ImGui::Begin(Constants::DiagramViewName(GetName()).c_str()); ImGui::End();

			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGuiID dockspaceId = ImGui::GetID(Constants::DockNodeName(GetName()).c_str());

			ImGui::DockBuilderRemoveNode(dockspaceId);
			ImGui::DockBuilderAddNode(dockspaceId);
			//ImGui::DockBuilderSetNodePos(dockspaceId, viewport->WorkPos);
			//ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->WorkSize);
			//ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

			ImGuiID dock_main = dockspaceId;
			ImGuiID dockDigramList;
			ImGuiID dockConditionList;
			ImGuiID dockDiagramView;

			//ImGui::DockBuilderAddNode(dockWindowDirectory);
			//ImGui::DockBuilderSetNodePos(dockWindowDirectory, viewport->WorkPos);
			//ImGui::DockBuilderSetNodeSize(dockWindowDirectory, viewport->WorkSize);
			dockDigramList = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Left, 1, &dock_main, &dock_main);
			dockDiagramView = ImGui::DockBuilderSplitNode(dockDigramList, ImGuiDir_Right, 0.6f, nullptr, &dockDigramList);
			dockConditionList = ImGui::DockBuilderSplitNode(dockDigramList, ImGuiDir_Down, 0.5f, nullptr, &dockDigramList);

			ImGui::DockBuilderDockWindow(GetName().c_str(), dock_main);
			ImGui::DockBuilderDockWindow(Constants::DiagramListName(GetName()).c_str(), dockDigramList);
			ImGui::DockBuilderDockWindow(Constants::ConditionListName(GetName()).c_str(), dockConditionList);
			ImGui::DockBuilderDockWindow(Constants::DiagramViewName(GetName()).c_str(), dockDiagramView);

			ImGui::DockBuilderFinish(dockspaceId);
		}
		void JAnimationControllerEditor::CloseAllPopup()noexcept
		{
			diagramListPopup->SetOpen(false);
			conditionListPopup->SetOpen(false);
			diagramViewPopup->SetOpen(false);
		}
		void JAnimationControllerEditor::ClearSelectedIndex()noexcept
		{
			selectedDiagramIndex = invalidIndex;
			selectedConditionIndex = invalidIndex;
			selectedStateIndex = invalidIndex;
		}
		void JAnimationControllerEditor::RegisterCreateDiagramEv()
		{
			using BindT = JAnimationControllerEditor::DiagramHandleBind;
			size_t guid = Core::MakeGuid();
			auto cUptr = std::make_unique<BindT>(*createDiagramF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, std::move(guid));
			auto dUptr = std::make_unique<BindT>(*destroyDiagramF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, std::move(guid));

			using CreationTask = Core::JTransitionCreationTask<DataHandleStructure, BindT, BindT>;
			Core::JTransition::Execute(std::make_unique<CreationTask>("Create Diagram", std::move(cUptr), std::move(dUptr), fsmdata));
		}
		void JAnimationControllerEditor::RegisterDestroyDiagramEv()
		{
			using BindT = JAnimationControllerEditor::DiagramHandleBind;
			Core::JAnimationFSMdiagram* diagram = aniCont->GetDiagramByIndex(selectedDiagramIndex);
			auto cUptr = std::make_unique<BindT>(*createDiagramF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, diagram->GetGuid());
			auto dUptr = std::make_unique<BindT>(*destroyDiagramF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, diagram->GetGuid());

			using DestroyTask = Core::JTransitionCreationTask<DataHandleStructure, BindT, BindT>;
			Core::JTransition::Execute(std::make_unique<DestroyTask>("Destroy Diagram", std::move(dUptr), std::move(cUptr), fsmdata));
		}
		void JAnimationControllerEditor::RegisterCreateConditionEv()
		{
			using BindT = JAnimationControllerEditor::ConditionHandleBind;
			size_t guid = Core::MakeGuid();
			auto cUptr = std::make_unique<BindT>(*createConditionF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, std::move(guid));
			auto dUptr = std::make_unique<BindT>(*destroyConditionF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, std::move(guid));

			using CreationTask = Core::JTransitionCreationTask<DataHandleStructure, BindT, BindT>;
			Core::JTransition::Execute(std::make_unique<CreationTask>("Create Condition", std::move(cUptr), std::move(dUptr), fsmdata));
		}
		void JAnimationControllerEditor::RegisterDestroyConditionEv()
		{
			Core::JFSMcondition* cond = aniCont->GetConditionByIndex(selectedConditionIndex);
			if (!cond)
				return;

			using BindT = JAnimationControllerEditor::ConditionHandleBind;
			size_t guid = Core::MakeGuid();
			auto cUptr = std::make_unique<BindT>(*createConditionF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, cond->GetGuid());
			auto dUptr = std::make_unique<BindT>(*destroyConditionF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, cond->GetGuid());

			using DestroyTask = Core::JTransitionCreationTask<DataHandleStructure, BindT, BindT>;
			Core::JTransition::Execute(std::make_unique<DestroyTask>("Destroy Condition", std::move(dUptr), std::move(cUptr), fsmdata));
		}
		void JAnimationControllerEditor::RegisterCreateStateEv()
		{
			auto diagram = aniCont->GetDiagramByIndex(selectedDiagramIndex);
			if (!aniCont->CanCreateState(diagram))
				return;

			using BindT = JAnimationControllerEditor::StateHandleBind;
			size_t guid = Core::MakeGuid();
			auto cUptr = std::make_unique<BindT>(*createStateF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, diagram->GetGuid(), std::move(guid));
			auto dUptr = std::make_unique<BindT>(*destroyStateF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, diagram->GetGuid(), std::move(guid));

			using CreationTask = Core::JTransitionCreationTask<DataHandleStructure, BindT, BindT>;
			Core::JTransition::Execute(std::make_unique<CreationTask>("Create State", std::move(cUptr), std::move(dUptr), fsmdata));
		}
		void JAnimationControllerEditor::RegisterDestroyStateEv()
		{
			auto diagram = aniCont->GetDiagramByIndex(selectedDiagramIndex);
			auto state = diagram != nullptr ? diagram->GetStateByIndex(selectedStateIndex) : nullptr;
			if (!state)
				return;

			using BindT = JAnimationControllerEditor::StateHandleBind;
			auto cUptr = std::make_unique<BindT>(*createStateF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, diagram->GetGuid(), state->GetGuid());
			auto dUptr = std::make_unique<BindT>(*destroyStateF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, diagram->GetGuid(), state->GetGuid());

			using DestroyTask = Core::JTransitionCreationTask<DataHandleStructure, BindT, BindT>;
			Core::JTransition::Execute(std::make_unique<DestroyTask>("Destroy State", std::move(dUptr), std::move(cUptr), fsmdata));
		}
		void JAnimationControllerEditor::CreateDiagram(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t guid)
		{
			if (aniCont.IsValid())
			{
				Core::JOwnerPtr<Core::JIdentifier> owner = dS.Release(dH);
				if (owner.IsValid() && Core::Cast<Core::JAnimationFSMdiagram>(owner.Get()))
					Core::JIdentifier::AddInstance(Core::JOwnerPtr<Core::JAnimationFSMdiagram>::ConvertChildUser(std::move(owner)));
				else
					aniCont->CreateFSMDiagram(guid);
				SetModifiedBit(aniCont, true);
			}
		}
		void JAnimationControllerEditor::DestroyDiagram(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t guid)
		{
			if (aniCont.IsValid())
			{
				auto diagramPtr = aniCont->GetDiagram(guid);
				if (diagramPtr != nullptr)
				{
					Core::JDataHandle newHandle = dS.Add(Core::JIdentifier::ReleaseInstance<Core::JAnimationFSMdiagram>(diagramPtr->GetGuid()));
					dS.TransitionHandle(newHandle, dH);
					SetModifiedBit(aniCont, true);
				}
			}
		}
		void JAnimationControllerEditor::CreateCondition(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t guid)
		{
			if (aniCont.IsValid())
			{
				Core::JOwnerPtr<Core::JIdentifier> owner = dS.Release(dH);
				if (owner.IsValid() && Core::Cast<Core::JFSMcondition>(owner.Get()))
					Core::JIdentifier::AddInstance(Core::JOwnerPtr<Core::JFSMcondition>::ConvertChildUser(std::move(owner)));
				else
					aniCont->CreateFSMCondition(guid);
				SetModifiedBit(aniCont, true);
			}
		}
		void JAnimationControllerEditor::DestroyCondition(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t guid)
		{
			if (aniCont.IsValid())
			{
				auto conditionPtr = aniCont->GetCondition(guid);
				if (conditionPtr != nullptr)
				{
					Core::JDataHandle newHandle = dS.Add(Core::JIdentifier::ReleaseInstance<Core::JFSMcondition>(conditionPtr->GetGuid()));
					dS.TransitionHandle(newHandle, dH);
					SetModifiedBit(aniCont, true);
				}
			}
		}
		void JAnimationControllerEditor::CreateState(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t diagramGuid, const size_t stateGuid)
		{
			if (aniCont.IsValid())
			{
				Core::JOwnerPtr<Core::JIdentifier> owner = dS.Release(dH);
				if (owner.IsValid() && Core::Cast<Core::JAnimationFSMstate>(owner.Get()))
				{
					Core::J_ANIMATION_STATE_TYPE sType = Core::Cast<Core::JAnimationFSMstate>(owner.Get())->GetStateType();
					if (sType == Core::J_ANIMATION_STATE_TYPE::CLIP)
						Core::JIdentifier::AddInstance(Core::JOwnerPtr<Core::JAnimationFSMstateClip>::ConvertChildUser(std::move(owner)));
					else //미구현
						;//AddInstance(Core::JOwnerPtr<Core::JAnimationBlend>::ConvertChildUser(std::move(owner)));
				}
				else
					aniCont->CreateFSMClip(stateGuid, aniCont->GetDiagram(diagramGuid));				 
				SetModifiedBit(aniCont, true);
			}
		}
		void JAnimationControllerEditor::DestroyState(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t diagramGuid, const size_t stateGuid)
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
							//	Core::JDataHandle newHandle = dataStructure.Add(ReleaseInstance<Core::JFSMcondition>(conditionPtr->GetGuid()));
							//	dataStructure.TransitionHandle(newHandle, dataHandle);
						}
					}
					SetModifiedBit(aniCont, true);
				}
			}
		}
		void JAnimationControllerEditor::DoActivate()noexcept
		{
			JEditorWindow::DoActivate();
			std::vector<J_EDITOR_EVENT> enumVec
			{
				J_EDITOR_EVENT::MOUSE_CLICK, J_EDITOR_EVENT::SELECT_OBJECT
			};
			RegisterEventListener(enumVec);
			JResourceUserInterface::AddEventListener(*JResourceManager::Instance().EvInterface(), GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);

			preMousePosX = ImGui::GetMousePos().x;
			preMousePosY = ImGui::GetMousePos().y;

			SetAnimationController(JEditorPageShareData::GetSelectedObj(GetOwnerPageType()));
		}
		void JAnimationControllerEditor::DoDeActivate()noexcept
		{
			JEditorWindow::DoDeActivate();
			JResourceUserInterface::RemoveListener(*JResourceManager::Instance().EvInterface(), GetGuid());
			DeRegisterListener();
		}
		void JAnimationControllerEditor::OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)
		{
			if (senderGuid == GetGuid())
				return;

			if (eventType == J_EDITOR_EVENT::MOUSE_CLICK)
				CloseAllPopup();
			else if (eventType == J_EDITOR_EVENT::SELECT_OBJECT)
			{
				JEditorSelectObjectEvStruct* evstruct = static_cast<JEditorSelectObjectEvStruct*>(eventStruct);
				SetAnimationController(evstruct->selectObj);
			}
		}
		void JAnimationControllerEditor::OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
		{
			if (iden == GetGuid())
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
			{
				if (aniCont.IsValid() && jRobj->GetGuid() == aniCont->GetGuid())
					SetAnimationController(Core::JUserPtr<JObject>{});
			}
		}
	}
}