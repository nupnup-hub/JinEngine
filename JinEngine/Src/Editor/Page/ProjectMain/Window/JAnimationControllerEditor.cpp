#include"JAnimationControllerEditor.h"
#include"../../JEditorAttribute.h"
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h" 
#include"../../../String/JEditorString.h"
#include"../../../Popup/JEditorPopup.h"
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
#include"../../../../Core/FSM/JFSMcondition.h"
#include"../../../../Core/Reflection/JTypeTemplate.h"
#include"../../../../Core/Guid/GuidCreator.h"
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

		JAnimationControllerEditor::JAnimationControllerEditor(const std::string& name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE ownerPageType)
			:JEditorWindow(name, std::move(attribute), ownerPageType),
			diagramListName("DiagramList##" + GetName()),
			conditionListName("ConditionList##" + GetName()),
			diagramViewName("DiagramView##" + GetName())
		{
			editorString = std::make_unique<JEditorString>();
			RegisterDiagramFunc();
			RegisterConditionFunc();
			RegisterStateFunc();
			inputBuff = std::make_unique<JEditorInputBuffHelper>(JImGuiImpl::GetTextBuffRange());
		}
		JAnimationControllerEditor::~JAnimationControllerEditor() {};

		J_EDITOR_WINDOW_TYPE JAnimationControllerEditor::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::ANIMATION_CONTROLLER_EDITOR;
		}
		void JAnimationControllerEditor::RegisterDiagramFunc()
		{
			//Diagram List Popup
			std::unique_ptr<JEditorPopupNode> diagramListRootNode =
				std::make_unique<JEditorPopupNode>("Animation Controller Editor Diagram List Popup Root", J_EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

			std::unique_ptr<JEditorPopupNode> createNewDiagramNode =
				std::make_unique<JEditorPopupNode>("Create New Diagram", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, diagramListRootNode.get());
			editorString->AddString(createNewDiagramNode->GetNodeId(), { "Create New Diagram" , u8"애니메이션 다이어그램 생성" });

			std::unique_ptr<JEditorPopupNode> eraseDiagramNode =
				std::make_unique<JEditorPopupNode>("Destroy Diagram", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, diagramListRootNode.get());
			editorString->AddString(eraseDiagramNode->GetNodeId(), { "Destroy Diagram" , u8"애니메이션 다이어그램 삭제" });

			auto createDiagramLam = [](DataHandleStructure& dataStructure,
				Core::JDataHandle& dataHandle,
				Core::JUserPtr<JAnimationController> aniCont,
				const size_t guid)
			{
				if (aniCont.IsValid())
				{
					Core::JOwnerPtr<Core::JIdentifier> owner = dataStructure.Release(dataHandle); 
					if (owner.IsValid() && Core::Cast<Core::JAnimationFSMdiagram>(owner.Get()))
						Core::JIdentifier::AddInstance(Core::JOwnerPtr<Core::JAnimationFSMdiagram>::ConvertChildType(std::move(owner)));
					else
						Core::JFDFI<Core::JAnimationFSMdiagram>::Create(Core::JPtrUtil::MakeOwnerPtr<Core::JFSMdiagram::InitData>(guid, aniCont));
				}
			};

			auto destroyDiagramLam = [](DataHandleStructure& dataStructure,
				Core::JDataHandle& dataHandle,
				Core::JUserPtr<JAnimationController> aniCont,
				const size_t guid)
			{
				if (aniCont.IsValid())
				{
					auto diagramPtr = aniCont->GetDiagram(guid);
					if (diagramPtr != nullptr)
					{ 
						Core::JDataHandle newHandle = dataStructure.Add(Core::JIdentifier::ReleaseInstance<Core::JAnimationFSMdiagram>(diagramPtr->GetGuid()));
						dataStructure.TransitionHandle(newHandle, dataHandle);
					}
				}
			};

			createDiagramT = std::tuple(createNewDiagramNode->GetNodeId(), std::make_unique<CreateDiagramFunctor>(createDiagramLam));
			destroyDiagramT = std::tuple(eraseDiagramNode->GetNodeId(), std::make_unique<CreateDiagramFunctor>(destroyDiagramLam));

			diagramListPopup = std::make_unique<JEditorPopup>(diagramListName, std::move(diagramListRootNode));
			diagramListPopup->AddPopupNode(std::move(createNewDiagramNode));
			diagramListPopup->AddPopupNode(std::move(eraseDiagramNode));
		}
		void JAnimationControllerEditor::RegisterConditionFunc()
		{
			//Condition List Popup
			std::unique_ptr<JEditorPopupNode> conditionListRootNode =
				std::make_unique<JEditorPopupNode>("Animation Controller Editor Condition List Popup Root", J_EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

			std::unique_ptr<JEditorPopupNode> createNewConditionNode =
				std::make_unique<JEditorPopupNode>("Create New Condition", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, conditionListRootNode.get());
			editorString->AddString(createNewConditionNode->GetNodeId(), { "Create New Condition" , u8"애니메이션 패러미터 생성" });

			std::unique_ptr<JEditorPopupNode> destroyConditionNode =
				std::make_unique<JEditorPopupNode>("Destroy Condition", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, conditionListRootNode.get());
			editorString->AddString(destroyConditionNode->GetNodeId(), { "Destroy Condition" , u8"애니메이션 패러미터 삭제" });

			auto createConditionLam = [](DataHandleStructure& dataStructure,
				Core::JDataHandle& dataHandle,
				Core::JUserPtr<JAnimationController> aniCont,
				const size_t guid)
			{
				if (aniCont.IsValid())
				{
					Core::JOwnerPtr<Core::JIdentifier> owner = dataStructure.Release(dataHandle);
					if (owner.IsValid() && Core::Cast<Core::JFSMcondition>(owner.Get()))
						Core::JIdentifier::AddInstance(Core::JOwnerPtr<Core::JFSMcondition>::ConvertChildType(std::move(owner)));
					else
						Core::JFCFI<Core::JFSMcondition>::Create(Core::JPtrUtil::MakeOwnerPtr<Core::JFSMcondition::InitData>(guid, aniCont.Get()));
				}
			};

			auto destroyConditionLam = [](DataHandleStructure& dataStructure,
				Core::JDataHandle& dataHandle,
				Core::JUserPtr<JAnimationController> aniCont,
				const size_t guid)
			{
				if (aniCont.IsValid())
				{
					auto conditionPtr = aniCont->GetCondition(guid);
					if (conditionPtr != nullptr)
					{
						Core::JDataHandle newHandle = dataStructure.Add(Core::JIdentifier::ReleaseInstance<Core::JFSMcondition>(conditionPtr->GetGuid()));
						dataStructure.TransitionHandle(newHandle, dataHandle);
					}
				}
			};

			createConditionT = std::tuple(createNewConditionNode->GetNodeId(), std::make_unique<CreateConditionFunctor>(createConditionLam));
			destroyConditionT = std::tuple(destroyConditionNode->GetNodeId(), std::make_unique<CreateConditionFunctor>(destroyConditionLam));

			conditionListPopup = std::make_unique<JEditorPopup>(conditionListName, std::move(conditionListRootNode));
			conditionListPopup->AddPopupNode(std::move(createNewConditionNode));
			conditionListPopup->AddPopupNode(std::move(destroyConditionNode));

			auto setConditionTypeLam = [](const Core::J_FSMCONDITION_VALUE_TYPE vType,
				Core::JUserPtr<JAnimationController> aniCont,
				const size_t guid)
			{
				aniCont->GetCondition(guid)->SetValueType(vType);
			};
			setConditionTypeF = std::make_unique<SetConditionTypeFunctor>(setConditionTypeLam);
			auto setConditionNameLam = [](const std::string value,
				Core::JUserPtr<JAnimationController> aniCont,
				size_t guid)
			{
				aniCont->GetCondition(guid)->SetName(JCUtil::U8StrToWstr(value));			 
			};
			auto setConditionBoolLam = [](const bool value,
				Core::JUserPtr<JAnimationController> aniCont,
				size_t guid)
			{
				aniCont->GetCondition(guid)->SetValue(value);
			};
			auto setConditionIntLam = [](const int value,
				Core::JUserPtr<JAnimationController> aniCont,
				size_t guid)
			{
				aniCont->GetCondition(guid)->SetValue(value);
			};
			auto setConditionFloatLam = [](const float value,
				Core::JUserPtr<JAnimationController> aniCont,
				size_t guid)
			{
				aniCont->GetCondition(guid)->SetValue(value);
			};

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
				std::make_unique<JEditorPopupNode>("Create New State", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, diagramViewRootNode.get());
			editorString->AddString(createNewCilpStateNode->GetNodeId(), { "Create New Animation Clip" , u8"애니메이션 클립 생성" });

			std::unique_ptr<JEditorPopupNode> destroyStateNode =
				std::make_unique<JEditorPopupNode>("Destroy State", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, diagramViewRootNode.get());
			editorString->AddString(destroyStateNode->GetNodeId(), { "Destroy Animation State" , u8"애니메이션 상태 삭제" });

			auto createStateLam = [](DataHandleStructure& dataStructure,
				Core::JDataHandle& dataHandle,
				Core::JUserPtr<JAnimationController> aniCont,
				const size_t diagramGuid,
				const size_t stateGuid)
			{
				if (aniCont.IsValid())
				{
					Core::JOwnerPtr<Core::JIdentifier> owner = dataStructure.Release(dataHandle);
					if (owner.IsValid() && Core::Cast<Core::JAnimationFSMstate>(owner.Get()))
					{
						Core::J_ANIMATION_STATE_TYPE sType = Core::Cast<Core::JAnimationFSMstate>(owner.Get())->GetStateType();
						if (sType == Core::J_ANIMATION_STATE_TYPE::CLIP)
							Core::JIdentifier::AddInstance(Core::JOwnerPtr<Core::JAnimationFSMstateClip>::ConvertChildType(std::move(owner)));
						else //미구현
							;//AddInstance(Core::JOwnerPtr<Core::JAnimationBlend>::ConvertChildType(std::move(owner)));
					}
					else
						Core::JFSFI<Core::JAnimationFSMstate>::Create(Core::JPtrUtil::MakeOwnerPtr<Core::JFSMstate::InitData>
							(stateGuid, Core::GetUserPtr<Core::JAnimationFSMdiagram>(diagramGuid)));
				}
			};

			auto destroyStateLam = [](DataHandleStructure& dataStructure,
				Core::JDataHandle& dataHandle,
				Core::JUserPtr<JAnimationController> aniCont,
				const size_t diagramGuid,
				const size_t stateGuid)
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
								Core::JDataHandle newHandle = dataStructure.Add(Core::JIdentifier::ReleaseInstance<Core::JAnimationFSMstateClip>(statePtr->GetGuid()));
								dataStructure.TransitionHandle(newHandle, dataHandle);
							}
							else
							{//미구현
							//	Core::JDataHandle newHandle = dataStructure.Add(ReleaseInstance<Core::JFSMcondition>(conditionPtr->GetGuid()));
							//	dataStructure.TransitionHandle(newHandle, dataHandle);
							}
						}
					}
				}
			};

			createStateT = std::tuple(createNewCilpStateNode->GetNodeId(), std::make_unique<CreateStateFunctor>(createStateLam));
			destroyStateT = std::tuple(destroyStateNode->GetNodeId(), std::make_unique<CreateStateFunctor>(destroyStateLam));

			diagramViewPopup = std::make_unique< JEditorPopup>(diagramViewName, std::move(diagramViewRootNode));
			diagramViewPopup->AddPopupNode(std::move(createNewCilpStateNode));
			diagramViewPopup->AddPopupNode(std::move(destroyStateNode));
		}
		void JAnimationControllerEditor::UpdateWindow()
		{
			EnterWindow(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
			UpdateDocking();
			if (IsActivated() && aniCont.IsValid())
			{ 
				UpdateMouseClick(); 
				float oriSize = ImGui::GetStyle().ChildBorderSize;
				ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 2);
				float preCursorPosY = ImGui::GetCursorPosY();
				BuildDiagramList();
				BuildConditionList();

				JImGuiImpl::SetColor(ImVec4(0, 0, 0, 0), ImGuiCol_Header);
				JImGuiImpl::SetColor(ImVec4(0, 0, 0, 0), ImGuiCol_HeaderHovered);
				JImGuiImpl::SetColor(ImVec4(0, 0, 0, 0), ImGuiCol_HeaderActive);
				BuildDiagramView(preCursorPosY);
				JImGuiImpl::SetColorToDefault(ImGuiCol_Header);
				JImGuiImpl::SetColorToDefault(ImGuiCol_HeaderHovered);
				JImGuiImpl::SetColorToDefault(ImGuiCol_HeaderActive);

				ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, oriSize);
				preMousePosX = ImGui::GetMousePos().x;
				preMousePosY = ImGui::GetMousePos().y;

				ClearCash();
			}
			CloseWindow();		 
		}
		void JAnimationControllerEditor::BuildDiagramList()
		{
			ImVec2 windowSize = JImGuiImpl::GetWindowSize();
			JImGuiImpl::BeginChildWindow("DiagramList##JAnimationControllerEditor", JVector2<float>(windowSize.x * 0.2f, windowSize.y * 0.4f), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);
			JImGuiImpl::Text("DiagramList");

			const std::vector<Core::JAnimationFSMdiagram*>& diagramVec = aniCont->GetDiagramVec();
			const uint diagramCount = (uint)diagramVec.size();

			ImGuiTableFlags flag = ImGuiTableFlags_Borders | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
			if (JImGuiImpl::BeginTable("##DiagramList_Table_AnimationControllerEditor", 1, flag))
			{
				JImGuiImpl::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
				JImGuiImpl::TableHeadersRow();
				for (uint i = 0; i < diagramCount; ++i)
				{
					JImGuiImpl::TableNextRow();
					JImGuiImpl::TableSetColumnIndex(0);

					if (JImGuiImpl::Selectable(JCUtil::WstrToU8Str(diagramVec[i]->GetName()), &diagramListSelectable[i]))
					{
						if (diagramIndex != i)
						{
							ClearSelectableBuff(diagramListSelectable);
							diagramListSelectable[i] = true;
							diagramIndex = i;
							conditionIndex = invalidIndex;
							stateIndex = invalidIndex;
						}
					}
				}
				JImGuiImpl::EndTable();
				BuildDiagramListPopup();
			}

			if (JImGuiImpl::IsMouseInRect(JImGuiImpl::GetGuiWindowPos(), JImGuiImpl::GetGuiWindowSize()))
			{
				if (JImGuiImpl::IsRightMouseClicked())
				{
					diagramListPopup->SetOpen(!diagramListPopup->IsOpen());
					conditionListPopup->SetOpen(false);
					diagramViewPopup->SetOpen(false);
				}
				else if (JImGuiImpl::IsLeftMouseClicked() && diagramListPopup->IsOpen() && !diagramListPopup->IsMouseInPopup())
					CloseAllPopup();
			}

			JImGuiImpl::EndChildWindow();
		}
		void JAnimationControllerEditor::BuildDiagramListPopup()
		{
			if (diagramListPopup->IsOpen())
			{
				J_EDITOR_POPUP_NODE_RES res;
				size_t menuGuid;
				diagramListPopup->ExecutePopup(editorString.get(), res, menuGuid);
				if (res == J_EDITOR_POPUP_NODE_RES::CLICK_SLELECT_NODE)
				{
					Core::JAnimationFSMdiagram* diagram = aniCont->GetDiagramByIndex(diagramIndex);
					if (aniCont->CanCreateDiagram())
					{							
						if (std::get<0>(createDiagramT) == menuGuid)
						{ 
							auto createF = &*(std::get<1>(createDiagramT));
							auto destroyF = &*(std::get<1>(destroyDiagramT));
			 
							size_t guid = Core::MakeGuid(); 
							auto cUptr = std::make_unique<CreateDiagramBind>(*createF, Core::empty, Core::empty, Core::JUserPtr{aniCont}, std::move(guid));
							auto dUptr = std::make_unique<CreateDiagramBind>(*destroyF, Core::empty, Core::empty, Core::JUserPtr{aniCont}, std::move(guid));
							 
							using CreationTask = Core::JTransitionCreationTask<DataHandleStructure, CreateDiagramBind, CreateDiagramBind>;
							Core::JTransition::Execute(std::make_unique<CreationTask>("Create Diagram", std::move(cUptr), std::move(dUptr), fsmdata));						  
						}
						else if (diagram && std::get<0>(destroyDiagramT) == menuGuid)
						{
							auto createF = &*(std::get<1>(createDiagramT));
							auto destroyF = &*(std::get<1>(destroyDiagramT));
							 
							auto cUptr = std::make_unique<CreateDiagramBind>(*createF, Core::empty, Core::empty, Core::JUserPtr{aniCont}, diagram->GetGuid());
							auto dUptr = std::make_unique<CreateDiagramBind>(*destroyF, Core::empty, Core::empty, Core::JUserPtr{aniCont}, diagram->GetGuid());

							using CreationTask = Core::JTransitionCreationTask<DataHandleStructure, CreateDiagramBind, CreateDiagramBind>;
							Core::JTransition::Execute(std::make_unique<CreationTask>("Destroy Diagram", std::move(dUptr), std::move(cUptr), fsmdata));
						}
					}
					CloseAllPopup();
				}
			}
		}
		void JAnimationControllerEditor::BuildConditionList()
		{
			ImVec2 windowSize = ImGui::GetWindowSize();
			JImGuiImpl::BeginChildWindow("Condition##JAnimationControllerEditor", JVector2<float>(windowSize.x * 0.2f, windowSize.y * 0.6f), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);
			JImGuiImpl::Text("Condition");

			ImGuiTableFlags flag = ImGuiTableFlags_BordersV |
				ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ContextMenuInBody;

			const uint conditionCount = aniCont->GetConditionCount();
			if (JImGuiImpl::BeginTable("##ConditionList_Table_AnimationControllerEditor", 4, flag))
			{
				JImGuiImpl::TableSetupColumn("", ImGuiTableColumnFlags_DefaultHide, 0.005f);
				JImGuiImpl::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.445f);
				JImGuiImpl::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 0.275f);
				JImGuiImpl::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.275f);
				JImGuiImpl::TableHeadersRow();
				for (uint i = 0; i < conditionCount; ++i)
				{
					Core::JFSMcondition* nowCondition = aniCont->GetConditionByIndex(i);
					std::string name = JCUtil::WstrToU8Str(nowCondition->GetName());

					JImGuiImpl::TableNextRow();
					JImGuiImpl::TableSetColumnIndex(0);
					if (JImGuiImpl::Selectable(("##ConditionName_Selectable" + name).c_str(), &conditionListSelectable[i], ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap))
					{
						if (conditionIndex != i)
						{
							ClearSelectableBuff(conditionListSelectable);
							conditionListSelectable[i] = true;
							conditionIndex = i;
							diagramIndex = invalidIndex;
							stateIndex = invalidIndex;
						}
						inputBuff->SetBuff(name);
					}
					JImGuiImpl::TableSetColumnIndex(1);
					ImGui::PushItemWidth(-FLT_MIN);
					if (conditionListSelectable[i])
					{
						const ImVec2 itemPos = ImGui::GetCursorPos() + ImGui::GetWindowPos();

						if (JImGuiImpl::InputTextSet(GetName(),
							inputBuff.get(),
							ImGuiInputTextFlags_EnterReturnsTrue,
							*setConditionNameF,
							aniCont,
							nowCondition->GetGuid()))
							conditionListSelectable[i] = false;

						const ImVec2 itemSize = ImGui::GetItemRectSize();
						if (JImGuiImpl::IsRightMouseClicked() || JImGuiImpl::IsLeftMouseClicked())
						{
							if (!JImGuiImpl::IsMouseInRect(itemPos, itemSize))
								conditionListSelectable[i] = false;
						}
					}
					else
						JImGuiImpl::Text(name.c_str());

					Core::J_FSMCONDITION_VALUE_TYPE valueType = nowCondition->GetValueType();
					JImGuiImpl::TableSetColumnIndex(2);
					ImGui::PushItemWidth(-FLT_MIN);

					JImGuiImpl::ComoboSet(GetName(), valueType, *setConditionTypeF, aniCont, nowCondition->GetGuid());

					JImGuiImpl::TableSetColumnIndex(3);
					ImGui::PushItemWidth(-FLT_MIN);
					if (valueType == Core::J_FSMCONDITION_VALUE_TYPE::BOOL)
						JImGuiImpl::CheckBoxSet(GetName(), (bool)nowCondition->GetValue(), *setConditionBoolF, aniCont, nowCondition->GetGuid());
					else if (valueType == Core::J_FSMCONDITION_VALUE_TYPE::INT)
						JImGuiImpl::InputIntSet(GetName(), (int)nowCondition->GetValue(), *setConditionIntF, aniCont, nowCondition->GetGuid());
					else if (valueType == Core::J_FSMCONDITION_VALUE_TYPE::FLOAT)
						JImGuiImpl::InputFloatSet(GetName(), (float)nowCondition->GetValue(), *setConditionFloatF, aniCont, nowCondition->GetGuid());
				}
				JImGuiImpl::EndTable();
				BuildConditionListPopup();
			}
			if (JImGuiImpl::IsMouseInRect())
			{
				if (JImGuiImpl::IsRightMouseClicked())
				{
					diagramListPopup->SetOpen(false);
					conditionListPopup->SetOpen(!conditionListPopup->IsOpen());
					diagramViewPopup->SetOpen(false);
				}
				else if (JImGuiImpl::IsLeftMouseClicked() && conditionListPopup->IsOpen() && !conditionListPopup->IsMouseInPopup())
					CloseAllPopup();
			}
			ImGui::EndChild();
		}
		void JAnimationControllerEditor::BuildConditionListPopup()
		{
			if (conditionListPopup->IsOpen())
			{
				J_EDITOR_POPUP_NODE_RES res;
				size_t menuGuid;
				conditionListPopup->ExecutePopup(editorString.get(), res, menuGuid);
				if (res == J_EDITOR_POPUP_NODE_RES::CLICK_SLELECT_NODE)
				{
					Core::JFSMcondition* cond = aniCont->GetConditionByIndex(conditionIndex);
					if (aniCont->CanCreateCondition())
					{ 
						//using CreateBind = Core::
						if (std::get<0>(createConditionT) == menuGuid)
						{ 
							auto createF = &*(std::get<1>(createConditionT));
							auto destroyF = &*(std::get<1>(destroyConditionT));

							size_t guid = Core::MakeGuid();
							auto cUptr = std::make_unique<CreateConditionBind>(*createF, Core::empty, Core::empty, Core::JUserPtr{aniCont}, std::move(guid));
							auto dUptr = std::make_unique<CreateConditionBind>(*destroyF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, std::move(guid));

							using CreationTask = Core::JTransitionCreationTask<DataHandleStructure, CreateConditionBind, CreateConditionBind>;
							Core::JTransition::Execute(std::make_unique<CreationTask>("Create Condition", std::move(cUptr), std::move(dUptr), fsmdata));
						}
						else if (cond && std::get<0>(destroyConditionT) == menuGuid)
						{
							auto createF = &*(std::get<1>(createConditionT));
							auto destroyF = &*(std::get<1>(destroyConditionT));
							 
							auto cUptr = std::make_unique<CreateConditionBind>(*createF, Core::empty, Core::empty, Core::JUserPtr{aniCont}, cond->GetGuid());
							auto dUptr = std::make_unique<CreateConditionBind>(*destroyF, Core::empty, Core::empty, Core::JUserPtr{aniCont}, cond->GetGuid());
							 
							using CreationTask = Core::JTransitionCreationTask<DataHandleStructure, CreateConditionBind, CreateConditionBind>;
							Core::JTransition::Execute(std::make_unique<CreationTask>("Destroy Condition", std::move(dUptr), std::move(cUptr), fsmdata));
						}
						CloseAllPopup();
					}
				}
			}
		}
		void JAnimationControllerEditor::BuildDiagramView(float cursorPosY)
		{
			ImVec2 windowSize = ImGui::GetWindowSize();
			ImGui::SetCursorPos(ImVec2(windowSize.x * 0.2f + (ImGui::GetStyle().WindowPadding.x * 2) + (ImGui::GetStyle().WindowBorderSize * 2), cursorPosY));
			ImGui::BeginChild("Diagram##JAnimationControllerEditor", ImVec2(windowSize.x * 0.8f, windowSize.y), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::Text(JCUtil::WstrToU8Str(aniCont->GetDiagramName(diagramIndex)).c_str());
 
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			DiagramViewFuncData funcData;
			funcData.windowPosX = ImGui::GetWindowPos().x;
			funcData.windowPosY = ImGui::GetWindowPos().y;
			funcData.windowWidth = ImGui::GetWindowWidth();
			funcData.windowHeight = ImGui::GetWindowHeight();

			ImVec2 shapeSize = ImVec2(stateShapeWidth, stateShapeHeight);
			ImVec2 shapeOffset = ImVec2(funcData.windowWidth * 0.5f, funcData.windowHeight * 0.5f);
			ImVec2 windowPos = ImGui::GetWindowPos();
			ImVec2 mousePos = ImGui::GetMousePos();

			ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
			ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
			ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);
			 
			Core::JAnimationFSMdiagram* diagram = aniCont->GetDiagram(diagramIndex);
			const uint stateCount = diagram->GetStateCount();

			for (uint i = 0; i < stateCount; ++i)
			{
				Core::JAnimationFSMstate* nowState = diagram->GetState(i);
				JVector2<float> stateCoord = nowState->GetPos();
				ImVec2 cursorPos = ImVec2(stateCoord.x, stateCoord.y) + shapeOffset;
				bool isSelect = true;
				ImGui::SetCursorPos(cursorPos);
				if (ImGui::Selectable(JCUtil::WstrToU8Str(nowState->GetName()).c_str(), &diagramViewSelectable[i], ImGuiSelectableFlags_SelectOnClick, ImVec2(stateShapeWidth, stateShapeHeight)))
				{
					if (nowState->GetGuid() != diagram->GetState(stateIndex)->GetGuid())
					{
						ClearSelectableBuff(diagramViewSelectable);
						diagramViewSelectable[i] = true;
						stateIndex = i; 
						conditionIndex = invalidIndex;
					} 
				}

				ImVec2 pMin = ImVec2(stateCoord.x, stateCoord.y) + (windowPos + shapeOffset);
				ImVec2 pMax = ImVec2(stateCoord.x, stateCoord.y) + (windowPos + (shapeOffset + shapeSize));

				if (diagramViewSelectable[i])
				{
					ImVec2 framePMin = pMin - ImVec2(frameThickness, frameThickness);
					ImVec2 framePMax = pMax + ImVec2(frameThickness, frameThickness);
					drawList->AddRect(framePMin, framePMax, ImGui::GetColorU32(ImVec4(0.2f, 0.2f, 0.75f, 0.7f)), 0.5f, 0, frameThickness + 0.5f);
				} 
				drawList->AddRectFilledMultiColor(pMin, pMax,
					ImGui::GetColorU32(ImVec4(0.25f, 0.25f, 0.75f, 0.7f)),
					ImGui::GetColorU32(ImVec4(0.25f, 0.75f, 0.75f, 0.7f)),
					ImGui::GetColorU32(ImVec4(0.25f, 0.75f, 0.25f, 0.7f)),
					ImGui::GetColorU32(ImVec4(0.25f, 0.75f, 0.75f, 0.7f)));

				if (diagramViewSelectable[i] && ImGui::IsMouseDragging(0))
				{
					nowState->SetPos({ stateCoord.x + (mousePos.x - preMousePosX),
						stateCoord.y + (mousePos.y - preMousePosY) });
				}

				//미구현
				/*std::vector<Core::JAnimationFSMtransition*>& transitionIter = stateVec[i]->GetTransitionVector();
				const uint transtionCount = (uint)transitionIter.size();
				for (uint j = 0; j < transtionCount; ++j)
				{
					Core::JAnimationFSMtransition* nowTransition = *(transitionIter + j);
				}*/
			}
			BuildDiagramViewPopup();
			if (JImGuiImpl::IsMouseInRect())
			{
				if (JImGuiImpl::IsRightMouseClicked())
				{
					diagramListPopup->SetOpen(false);
					conditionListPopup->SetOpen(false);
					diagramViewPopup->SetOpen(!diagramViewPopup->IsOpen());
				}
				else if (JImGuiImpl::IsLeftMouseClicked() && diagramViewPopup->IsOpen() && !diagramViewPopup->IsMouseInPopup())
					CloseAllPopup();
			}
			ImGui::EndChild();
		}
		void JAnimationControllerEditor::BuildDiagramViewPopup()
		{
			if (diagramViewPopup->IsOpen())
			{
				J_EDITOR_POPUP_NODE_RES res;
				size_t menuGuid;
				diagramViewPopup->ExecutePopup(editorString.get(), res, menuGuid);
				if (res == J_EDITOR_POPUP_NODE_RES::CLICK_SLELECT_NODE)
				{
					auto diagram = aniCont->GetDiagram(diagramIndex);	
					auto state = diagram != nullptr ? diagram->GetState(stateIndex) : nullptr;
					if (aniCont->CanCreateState(diagramIndex))
					{ 
						if (std::get<0>(createStateT) == menuGuid)
						{ 			
							auto createF = &*(std::get<1>(createStateT));
							auto destroyF = &*(std::get<1>(destroyStateT));

							size_t guid = Core::MakeGuid();
							auto cUptr = std::make_unique<CreateStateBind>(*createF, Core::empty, Core::empty, Core::JUserPtr{aniCont}, diagram->GetGuid(), std::move(guid));
							auto dUptr = std::make_unique<CreateStateBind>(*destroyF, Core::empty, Core::empty, Core::JUserPtr{aniCont}, diagram->GetGuid(), std::move(guid));

							using CreationTask = Core::JTransitionCreationTask<DataHandleStructure, CreateStateBind, CreateStateBind>;
							Core::JTransition::Execute(std::make_unique<CreationTask>("Create State", std::move(cUptr), std::move(dUptr), fsmdata));
						}
						else if (state && std::get<0>(destroyStateT) == menuGuid)
						{						 
							auto createF = &*(std::get<1>(createStateT));
							auto destroyF = &*(std::get<1>(destroyStateT));
							 
							auto cUptr = std::make_unique<CreateStateBind>(*createF, Core::empty, Core::empty, Core::JUserPtr{aniCont}, diagram->GetGuid(), state->GetGuid());
							auto dUptr = std::make_unique<CreateStateBind>(*destroyF, Core::empty, Core::empty, Core::JUserPtr{aniCont}, diagram->GetGuid(), state->GetGuid());

							using CreationTask = Core::JTransitionCreationTask<DataHandleStructure, CreateStateBind, CreateStateBind>;
							Core::JTransition::Execute(std::make_unique<CreationTask>("Destry State", std::move(dUptr), std::move(cUptr), fsmdata));
						} 
					}
					CloseAllPopup();
				}
			}
		}
		void JAnimationControllerEditor::CloseAllPopup()noexcept
		{
			diagramListPopup->SetOpen(false);
			conditionListPopup->SetOpen(false);
			diagramViewPopup->SetOpen(false);
		}
		void JAnimationControllerEditor::ClearSelectableBuff(bool* selectableBuf)noexcept
		{
			for (uint i = 0; i < selectableBufLength; ++i)
				selectableBuf[i] = false;
		}
		void JAnimationControllerEditor::ClearCash()noexcept
		{
			aniCont.Clear();
			ClearSelectableBuff(diagramListSelectable);
			ClearSelectableBuff(conditionListSelectable);
			ClearSelectableBuff(diagramViewSelectable);
			diagramIndex = invalidIndex;
			conditionIndex = invalidIndex;
			stateIndex = invalidIndex;
		}
		void JAnimationControllerEditor::DoActivate()noexcept
		{
			JEditorWindow::Activate();
			std::vector<J_EDITOR_EVENT> enumVec
			{
				J_EDITOR_EVENT::MOUSE_CLICK, J_EDITOR_EVENT::SELECT_OBJECT, J_EDITOR_EVENT::DESELECT_OBJECT
			};
			RegisterEventListener(enumVec);

			preMousePosX = ImGui::GetMousePos().x;
			preMousePosY = ImGui::GetMousePos().y;
		}
		void JAnimationControllerEditor::DoDeActivate()noexcept
		{
			JEditorWindow::DeActivate();
			DeRegisterListener();
			ClearCash();
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
				if (evstruct->pageType == GetOwnerPageType() && evstruct->selectObj->GetObjectType() == J_OBJECT_TYPE::RESOURCE_OBJECT)
				{
					JResourceObject* rObject = static_cast<JResourceObject*>(evstruct->selectObj.Get());
					if (rObject->GetResourceType() == J_RESOURCE_TYPE::ANIMATION_CONTROLLER)
					{
						ClearCash();
						aniCont.ConnnectBaseUser(evstruct->selectObj);
						if (aniCont->GetDiagramCount() > 0)
						{
							diagramIndex = 0;
							diagramListSelectable[0] = true;
						}
					}
				}
			}
			else if (eventType == J_EDITOR_EVENT::DESELECT_OBJECT)
			{
				JEditorDeSelectObjectEvStruct* evstruct = static_cast<JEditorDeSelectObjectEvStruct*>(eventStruct);
				if (evstruct->pageType == GetOwnerPageType())
					ClearCash();
			}
		}
	}
}