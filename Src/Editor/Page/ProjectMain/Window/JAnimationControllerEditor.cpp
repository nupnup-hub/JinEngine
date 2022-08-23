#include"JAnimationControllerEditor.h"
#include"../../JEditorAttribute.h"
#include"../../../Transition/JEditorTransition.h"
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h" 
#include"../../../String/JEditorString.h"
#include"../../../Popup/JEditorPopup.h"
#include"../../../Popup/JEditorPopupNode.h"
#include"../../../Diagram/JEditorDiagramNode.h" 

#include"../../../../Object/Resource/AnimationController/JAnimationController.h"
#include"../../../../Object/Resource/JResourceManager.h"
#include"../../../../Object/Component/Animator/JAnimator.h"
#include"../../../../Object/GameObject/JGameObject.h"
#include"../../../../Core/FSM/AnimationFSM/JAnimationFSMdiagram.h"
#include"../../../../Core/FSM/AnimationFSM/JAnimationFSMstate.h"
#include"../../../../Core/FSM/JFSMcondition.h"
#include"../../../../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Editor
	{
		struct JAnimationControllerEditor::DiagramViewFuncData
		{
		public:
			float windowPosX;
			float windowPosY;
			float windowWidth;
			float windowHeight;
		};

		static JAnimationController* controller = nullptr;
		static Core::JAnimationFSMdiagram* selectedDiagram = nullptr;
		static Core::JAnimationFSMstate* selectedState = nullptr;
		static Core::JFSMcondition* selectedCondition = nullptr;

		JAnimationControllerEditor::JAnimationControllerEditor(std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE ownerPageType)
			:JEditorWindow(std::move(attribute), ownerPageType),
			diagramListName("DiagramList##" + GetName()),
			conditionListName("ConditionList##" + GetName()),
			diagramViewName("DiagramView##" + GetName())
		{
			editorString = std::make_unique<JEditorString>();

			//Diagram List Popup
			std::unique_ptr<JEditorPopupNode> diagramListRootNode =
				std::make_unique<JEditorPopupNode>("Animation Controller Editor Diagram List Popup Root", J_EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

			std::unique_ptr<JEditorPopupNode> createNewDiagramNode =
				std::make_unique<JEditorPopupNode>("Create New Diagram", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, diagramListRootNode.get());
			editorString->AddString(createNewDiagramNode->GetNodeId(), { "Create New Diagram" , u8"애니메이션 다이어그램 생성" });

			std::unique_ptr<JEditorPopupNode> eraseDiagramNode =
				std::make_unique<JEditorPopupNode>("Destroy Diagram", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, diagramListRootNode.get());
			editorString->AddString(eraseDiagramNode->GetNodeId(), { "Destroy Diagram" , u8"애니메이션 다이어그램 삭제" });

			DiagramCreateFunctor createDiagramFunc{ &JAnimationControllerEditor::CreateNewDiagram };
			DiagramDestroyFunctor destroyDiagramFunc{ &JAnimationControllerEditor::DestroyDiagram };

			createDiagramTuple = std::make_unique<DiagramCreateTuple>(createNewDiagramNode->GetNodeId(), createDiagramFunc);
			destroyDiagramTuple = std::make_unique<DiagramDestroyTuple>(createNewDiagramNode->GetNodeId(), destroyDiagramFunc);

			diagramListPopup = std::make_unique<JEditorPopup>(diagramListName, std::move(diagramListRootNode));
			diagramListPopup->AddPopupNode(std::move(createNewDiagramNode));
			diagramListPopup->AddPopupNode(std::move(eraseDiagramNode));

			//Condition List Popup
			std::unique_ptr<JEditorPopupNode> conditionListRootNode =
				std::make_unique<JEditorPopupNode>("Animation Controller Editor Condition List Popup Root", J_EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

			std::unique_ptr<JEditorPopupNode> createNewConditionNode =
				std::make_unique<JEditorPopupNode>("Create New Condition", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, conditionListRootNode.get());
			editorString->AddString(createNewConditionNode->GetNodeId(), { "Create New Condition" , u8"애니메이션 패러미터 생성" });

			std::unique_ptr<JEditorPopupNode> destroyConditionNode =
				std::make_unique<JEditorPopupNode>("Destroy Condition", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, conditionListRootNode.get());
			editorString->AddString(destroyConditionNode->GetNodeId(), { "Destroy Condition" , u8"애니메이션 패러미터 삭제" });

			ConditionCreateFunctor createConditionFunc{ &JAnimationControllerEditor::CreateNewCondition };
			ConditionDestroyFunctor destroyConditionFunc{ &JAnimationControllerEditor::DestroyCondition };

			createConditionTuple = std::make_unique<ConditionCreateTuple>(createNewConditionNode->GetNodeId(), createConditionFunc);
			destroyConditionTuple = std::make_unique<ConditionDestroyTuple>(destroyConditionNode->GetNodeId(), destroyConditionFunc);

			conditionListPopup = std::make_unique<JEditorPopup>(conditionListName, std::move(conditionListRootNode));
			conditionListPopup->AddPopupNode(std::move(createNewConditionNode));
			conditionListPopup->AddPopupNode(std::move(destroyConditionNode));

			//Diagram View Popup
			std::unique_ptr<JEditorPopupNode> diagramViewRootNode =
				std::make_unique<JEditorPopupNode>("Animation Controller Editor Diagram View Popup Root", J_EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

			std::unique_ptr<JEditorPopupNode> createNewStateNode =
				std::make_unique<JEditorPopupNode>("Create New State", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, diagramViewRootNode.get());
			editorString->AddString(createNewStateNode->GetNodeId(), { "Create New State" , u8"애니메이션 상태 생성" });

			std::unique_ptr<JEditorPopupNode> destroyStateNode =
				std::make_unique<JEditorPopupNode>("Destroy State", J_EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, diagramViewRootNode.get());
			editorString->AddString(destroyStateNode->GetNodeId(), { "Destroy State" , u8"애니메이션 상태 삭제" });

			DiagramViewCreateFunctor createStateFunc{ &JAnimationControllerEditor::CreateNewState };
			DiagramViewDestroyFunctor destroyStateFunc{ &JAnimationControllerEditor::DestroyState };

			createDiagramViewTuple = std::make_unique<DiagramViewCreateTuple>(createNewStateNode->GetNodeId(), createStateFunc);
			destroyDiagramViewTuple = std::make_unique<DiagramViewDestroyTuple>(destroyStateNode->GetNodeId(), destroyStateFunc);

			diagramViewPopup = std::make_unique< JEditorPopup>(diagramViewName, std::move(diagramViewRootNode));
			diagramViewPopup->AddPopupNode(std::move(createNewStateNode));
			diagramViewPopup->AddPopupNode(std::move(destroyStateNode));

			nameBuf.resize(nameMaxLength);

			auto setBooleanLam = [](Core::JFSMcondition& condition, bool value) {condition.SetValue(value); };
			auto setIntLam = [](Core::JFSMcondition& condition, int value) {condition.SetValue(value); };
			auto setFloatLam = [](Core::JFSMcondition& condition, float value) {condition.SetValue(value); };

			void(*setBPtr)(Core::JFSMcondition & condition, bool value) = setBooleanLam;
			void(*setIPtr)(Core::JFSMcondition & condition, int value) = setIntLam;
			void(*setFPtr)(Core::JFSMcondition & condition, float value) = setFloatLam;

			setBooleanFunctor = std::make_unique< SetBooleanConditionFunctor>(setBPtr);
			setIntFunctor = std::make_unique< SetIntConditionFunctor>(setIPtr);
			setFloatFunctor = std::make_unique< SetFloatConditionFunctor>(setFPtr);
		}
		JAnimationControllerEditor::~JAnimationControllerEditor() {};

		void JAnimationControllerEditor::Initialize()
		{


		}
		void JAnimationControllerEditor::UpdateWindow()
		{
			JEditorWindow::UpdateWindow();

			controller = JResourceManager::Instance().GetResource<JAnimationController>(controllerGuid);
			if (controller != nullptr)
			{
				selectedDiagram = isSelectedDiagram ? controller->EditorInterface()->GetDiagram(selectedDiagramGuid) : nullptr;
				selectedState = isSelectedState ? controller->EditorInterface()->GetState(selectedDiagramGuid, selectedStateGuid) : nullptr;
				selectedCondition = isSelectedCondition ? controller->EditorInterface()->GetCondition(selectedConditionGuid) : nullptr;
			}

			float oriSize = ImGui::GetStyle().ChildBorderSize;
			ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 2);
			float preCursorPosY = ImGui::GetCursorPosY();
			BuildDiagramList();
			BuildConditionList();

			JImGuiImpl::SetColor(ImVec4(0, 0, 0, 0), ImGuiCol_Header);
			JImGuiImpl::SetColor(ImVec4(0, 0, 0, 0), ImGuiCol_HeaderHovered);
			JImGuiImpl::SetColor(ImVec4(0, 0, 0, 0), ImGuiCol_HeaderActive);
			BuildAnimationDiagramView(preCursorPosY);
			JImGuiImpl::SetColorToDefault(ImGuiCol_Header);
			JImGuiImpl::SetColorToDefault(ImGuiCol_HeaderHovered);
			JImGuiImpl::SetColorToDefault(ImGuiCol_HeaderActive);

			ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, oriSize);
			preMousePosX = ImGui::GetMousePos().x;
			preMousePosY = ImGui::GetMousePos().y;

			ClearCash();
		}
		void JAnimationControllerEditor::BuildDiagramList()
		{
			ImVec2 windowSize = JImGuiImpl::GetWindowSize();
			JImGuiImpl::BeginChildWindow("DiagramList##JAnimationControllerEditor", JVector2<float>(windowSize.x * 0.2f, windowSize.y * 0.4f), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);
			JImGuiImpl::Text("DiagramList");

			if (controller != nullptr)
			{
				std::vector<Core::JAnimationFSMdiagram*> diagramVec = controller->EditorInterface()->GetDiagramVec();
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

						if (JImGuiImpl::Selectable(diagramVec[i]->GetName().c_str(), &diagramListSelectable[i]))
						{
							if (selectedDiagram == nullptr || diagramVec[i]->GetGuid() != selectedDiagram->GetGuid())
							{
								for (uint j = 0; j < diagramCount; ++j)
									diagramListSelectable[j] = false;
								diagramListSelectable[i] = true;
								selectedDiagramGuid = diagramVec[i]->GetGuid();
								isSelectedDiagram = true;
							}
							isSelectedCondition = false;
							isSelectedState = false;
						}
					}
					JImGuiImpl::EndTable();
					BuildDiagramListPopup();
				}

				if (JImGuiImpl::IsMouseInWindow(JImGuiImpl::GetGuiWindowPos(), JImGuiImpl::GetGuiWindowSize()))
				{
					if (JImGuiImpl::IsRightMouseClick())
					{
						diagramListPopup->SetOpen(!diagramListPopup->IsOpen());
						conditionListPopup->SetOpen(false);
						diagramViewPopup->SetOpen(false);
					}
					else if (JImGuiImpl::IsLeftMouseClick() && diagramListPopup->IsOpen() && !diagramListPopup->IsMouseInPopup())
						CloseAllPopup();
				}
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
					if (std::get<0>(*createDiagramTuple) == menuGuid)
					{
						DiagramCreateFunctor& doFunc = std::get<1>(*createDiagramTuple);
						DiagramDestroyFunctor& undoFunc = std::get<1>(*destroyDiagramTuple);

						auto doBind = DiagramCreateBinder(doFunc, *selectedController);
						auto undoBind = DiagramDestroyBinder(undoFunc, *selectedController);
						JEditorTransition::Execute(JEditorTask{ doBind, "Create Diagram" }, JEditorTask{ undoBind, "Destroy Diagram" });
					}
					else if (std::get<0>(*destroyDiagramTuple) == menuGuid)
					{
						DiagramCreateFunctor& doFunc = std::get<1>(*destroyDiagramTuple);
						DiagramDestroyFunctor& undoFunc = std::get<1>(*createDiagramTuple);

						auto doBind = DiagramDestroyBinder(doFunc, *selectedController);
						auto undoBind = DiagramCreateBinder(undoFunc, *selectedController);
						JEditorTransition::Execute(JEditorTask{ doBind, "Destroy Diagram" }, JEditorTask{ undoBind, "Create Diagram" });
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
			if (selectedController != nullptr)
			{
				const uint conditionCount = selectedController->GetConditionCount();
				if (JImGuiImpl::BeginTable("##ConditionList_Table_AnimationControllerEditor", 4, flag))
				{
					JImGuiImpl::TableSetupColumn("", ImGuiTableColumnFlags_DefaultHide, 0.005f);
					JImGuiImpl::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.445f);
					JImGuiImpl::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 0.275f);
					JImGuiImpl::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.275f);
					JImGuiImpl::TableHeadersRow();
					for (uint i = 0; i < conditionCount; ++i)
					{
						Core::JFSMcondition* nowCondition = selectedController->GetCondition(i);
						std::string name = nowCondition->GetName();

						JImGuiImpl::TableNextRow();
						JImGuiImpl::TableSetColumnIndex(0);
						if (JImGuiImpl::Selectable(("##ConditionName_Selectable" + name).c_str(), &conditionListSelectable[i], ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap))
						{
							if (selectedCondition == nullptr || selectedCondition->GetGuid() != nowCondition->GetGuid())
							{
								for (uint j = 0; j < conditionCount; ++j)
									conditionListSelectable[j] = false;
								conditionListSelectable[i] = true;
								selectedCondition = nowCondition;
							}
							selectedState = nullptr;
							nameBuf.clear();
							nameBuf.resize(nameMaxLength);

							for (uint j = 0; j < name.size(); ++j)
								nameBuf[j] = name[j];
						}
						JImGuiImpl::TableSetColumnIndex(1);
						ImGui::PushItemWidth(-FLT_MIN);
						if (conditionListSelectable[i])
						{
							if (JImGuiImpl::InputText(("##ConditionName" + name).c_str(), &nameBuf[0], nameMaxLength, ImGuiInputTextFlags_EnterReturnsTrue))
							{
								const std::string newName = JCommonUtility::EraseEmptySpace(nameBuf);
								if (newName != name)
									selectedController->SetConditionName(nowCondition->GetName(), newName);
							}
						}
						else
							JImGuiImpl::Text(name.c_str());

						Core::J_FSMCONDITION_VALUE_TYPE valueType = nowCondition->GetValueType();
						JImGuiImpl::TableSetColumnIndex(2);
						ImGui::PushItemWidth(-FLT_MIN);
						if (JImGuiImpl::BeginCombo(("##SelectConditionValueType" + name).c_str(), Core::JFsmConditionValueType::ConditionEnumToString(valueType).c_str()))
						{
							for (uint j = 0; j < Core::JFsmConditionValueType::count; ++j)
							{
								Core::J_FSMCONDITION_VALUE_TYPE nowType = (Core::J_FSMCONDITION_VALUE_TYPE)((int)Core::J_FSMCONDITION_VALUE_TYPE::BOOL + j);
								std::string valueStr = Core::JFsmConditionValueType::ConditionEnumToString(nowType) + "##FSMconditionValue";
								if (JImGuiImpl::Selectable(valueStr.c_str()))
									selectedController->SetConditionValueType(nowCondition->GetName(), nowType);
							}
							JImGuiImpl::EndCombo();
						}

						JImGuiImpl::TableSetColumnIndex(3);
						ImGui::PushItemWidth(-FLT_MIN);
						if (valueType == Core::J_FSMCONDITION_VALUE_TYPE::BOOL)
						{
							bool nowValue = (bool)nowCondition->GetValue();
							bool preValue = nowValue;
							if (JImGuiImpl::CheckBox(("##ConditionBoolean" + name).c_str(), nowValue))
							{
								auto doBind = SetBooleanConditionBinder(*setBooleanFunctor, *nowCondition, std::move(nowValue));

								nowCondition->SetValue((float)nowValue);
							}
						}
						else if (valueType == Core::J_FSMCONDITION_VALUE_TYPE::INT)
						{
							int nowValue = (int)nowCondition->GetValue();
							if (JImGuiImpl::InputInt(("##ConditionInt" + name).c_str(), &nowValue))
							{
								if (nowValue != (int)nowCondition->GetValue())
								{
									nowCondition->SetValue((float)nowValue);
								}
							}
						}
						else if (valueType == Core::J_FSMCONDITION_VALUE_TYPE::FLOAT)
						{
							float nowValue = nowCondition->GetValue();
							if (JImGuiImpl::InputFloat(("##ConditionFloat" + name).c_str(), &nowValue))
							{
								if (nowValue != nowCondition->GetValue())
								{
									nowCondition->SetValue(nowValue);
								}
							}
						}
					}
					JImGuiImpl::EndTable();
				}

				if (conditionListPopup->IsOpen())
				{
					J_EDITOR_POPUP_NODE_RES res;
					size_t menuGuid;
					conditionListPopup->ExecutePopup(editorString.get(), res, menuGuid);
					if (res == J_EDITOR_POPUP_NODE_RES::CLICK_SLELECT_NODE)
					{
						auto func = conditionListPopupFunc.find(menuGuid);
						if (func != conditionListPopupFunc.end())
							func->second.Invoke();

						CloseAllPopup();
					}
				}
				if (ImGui::IsMouseInWindow(ImGui::GetWindowPos(), ImGui::GetWindowSize()))
				{
					if (JImGuiImpl::IsRightMouseClick())
					{
						diagramListPopup->SetOpen(false);
						conditionListPopup->SetOpen(!conditionListPopup->IsOpen());
						diagramViewPopup->SetOpen(false);
					}
					else if (JImGuiImpl::IsLeftMouseClick() && conditionListPopup->IsOpen() && !conditionListPopup->IsMouseInPopup())
						CloseAllPopup();
				}
			}
			ImGui::EndChild();
		}
		void JAnimationControllerEditor::BuildConditionListPopup()
		{

		}
		void JAnimationControllerEditor::BuildAnimationDiagramView(float cursorPosY)
		{
			ImVec2 windowSize = ImGui::GetWindowSize();
			ImGui::SetCursorPos(ImVec2(windowSize.x * 0.2f + (ImGui::GetStyle().WindowPadding.x * 2) + (ImGui::GetStyle().WindowBorderSize * 2), cursorPosY));
			ImGui::BeginChild("Diagram##JAnimationControllerEditor", ImVec2(windowSize.x * 0.8f, windowSize.y), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);
			if (selectedController != nullptr)
			{
				ImGui::Text(selectedController->GetAnimationDiagramName(diagramIndex).c_str());
				uint stateCount;
				std::vector<Core::JAnimationFSMstate*>::const_iterator stateIter;
				selectedController->GetAnimationFSMstateVectorHandle(diagramIndex, stateCount, stateIter);
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

				for (uint i = 0; i < stateCount; ++i)
				{
					Core::JAnimationFSMstate* nowState = *(stateIter + i);
					JEditorDiagramNode* nowStateCoord = nowState->GetEditorDiagramNode();

					ImVec2 cursorPos = ImVec2(nowStateCoord->GetPosX(), nowStateCoord->GetPosY()) + shapeOffset;
					bool isSelect = true;
					ImGui::SetCursorPos(cursorPos);
					if (ImGui::Selectable(nowState->GetName().c_str(), &diagramViewSelectable[i], ImGuiSelectableFlags_SelectOnClick, ImVec2(stateShapeWidth, stateShapeHeight)))
					{
						if (selectedState == nullptr || selectedState->GetGuid() != nowState->GetGuid())
						{
							for (uint j = 0; j < stateCount; ++j)
								diagramViewSelectable[j] = false;
							diagramViewSelectable[i] = true;
							selectedState = nowState;
						}
						selectedCondition = nullptr;
					}

					ImVec2 pMin = ImVec2(nowStateCoord->GetPosX(), nowStateCoord->GetPosY()) + (windowPos + shapeOffset);
					ImVec2 pMax = ImVec2(nowStateCoord->GetPosX(), nowStateCoord->GetPosY()) + (windowPos + (shapeOffset + shapeSize));

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
						nowStateCoord->SetPos(nowStateCoord->GetPosX() + (mousePos.x - preMousePosX),
							nowStateCoord->GetPosY() + (mousePos.y - preMousePosY));
					}

					uint transtionCount;
					std::vector<Core::JAnimationFSMtransition*>::const_iterator transitionIter = nowState->GetTransitionVectorHandle(transtionCount);

					for (uint j = 0; j < transtionCount; ++j)
					{
						Core::JAnimationFSMtransition* nowTransition = *(transitionIter + j);
					}
				}
				if (diagramViewPopup->IsOpen())
				{
					J_EDITOR_POPUP_NODE_RES res;
					size_t menuGuid;
					diagramViewPopup->ExecutePopup(editorString.get(), res, menuGuid);
					if (res == J_EDITOR_POPUP_NODE_RES::CLICK_SLELECT_NODE)
					{
						auto func = diagramViewPopupFunc.find(menuGuid);
						if (func != diagramViewPopupFunc.end())
							func->second.Invoke(funcData);

						CloseAllPopup();
					}
				}
				if (ImGui::IsMouseInWindow(ImGui::GetWindowPos(), ImGui::GetWindowSize()))
				{
					if (editorUtility->rightMouseClick)
					{
						diagramListPopup->SetOpen(false);
						conditionListPopup->SetOpen(false);
						diagramViewPopup->SetOpen(!diagramViewPopup->IsOpen());
					}
					else if (editorUtility->leftMouseClick && diagramViewPopup->IsOpen() && !diagramViewPopup->IsMouseInPopup())
						CloseAllPopup();
				}
			}
			ImGui::EndChild();
		}
		void JAnimationControllerEditor::CloseAllPopup()noexcept
		{
			diagramListPopup->SetOpen(false);
			conditionListPopup->SetOpen(false);
			diagramViewPopup->SetOpen(false);
		}
		void JAnimationControllerEditor::ClearSelectableBuff()noexcept
		{
			for (uint i = 0; i < selectableBufLength; ++i)
			{
				diagramListSelectable[i] = false;
				conditionListSelectable[i] = false;
				diagramViewSelectable[i] = false;
			}
		}
		void JAnimationControllerEditor::CreateNewDiagram(const size_t controllerGuid, const std::string name, const size_t guid)noexcept
		{
			JAnimationController* controller = JResourceManager::Instance().GetResource<JAnimationController>(controllerGuid);
			if(controller != nullptr)
				controller->CreateAnimationDiagram(name, guid);
		}
		void JAnimationControllerEditor::CreateNewCondition(const size_t controllerGuid, const std::string name, const size_t guid)noexcept
		{
			JAnimationController* controller = JResourceManager::Instance().GetResource<JAnimationController>(controllerGuid);
			if (controller != nullptr)
				controller->CreateCondition(name, guid);
		}
		void JAnimationControllerEditor::CreateNewState(const size_t controllerGuid, const std::string name, const size_t guid, const DiagramViewFuncData funcData)noexcept
		{
			JAnimationController* controller = JResourceManager::Instance().GetResource<JAnimationController>(controllerGuid);
			if (controller != nullptr)
			{
				ImVec2 windowPos = ImVec2(funcData.windowPosX, funcData.windowPosY);
				ImVec2 mousePos = ImGui::GetMousePos();
				ImVec2 shapeSize = ImVec2(stateShapeWidth, stateShapeHeight) * 0.5f;
				ImVec2 shapeOffset = ImVec2(funcData.windowWidth * 0.5f, funcData.windowHeight * 0.5f);
				ImVec2 newStateCoord = mousePos - windowPos - shapeOffset - shapeSize;
			}
			controller.CreateAnimationClip(diagramIndex, newStateCoord.x, newStateCoord.y);
		}
		void JAnimationControllerEditor::DestroyDiagram(const size_t controllerGuid, const size_t guid)noexcept
		{
			JAnimationController* controller = JResourceManager::Instance().GetResource<JAnimationController>(controllerGuid);
			if (controller != nullptr)
			{
				controller->DestroyAnimationDiagram(guid);
				for (uint i = 0; i < selectableBufLength; ++i)
					diagramListSelectable[i] = false;
			}		 
		}
		void JAnimationControllerEditor::DestroyCondition(const size_t controllerGuid, const size_t guid)noexcept
		{
			JAnimationController* controller = JResourceManager::Instance().GetResource<JAnimationController>(controllerGuid);
			if (controller != nullptr)
			{
				controller->DestroyCondition(guid);
				for (uint i = 0; i < selectableBufLength; ++i)
					conditionListSelectable[i] = false;
			}
		}
		void JAnimationControllerEditor::DestroyState(const size_t controllerGuid, const size_t guid)noexcept
		{
			controller.DestroyAnimationState(diagramIndex, aniState.GetName());
			for (uint i = 0; i < selectableBufLength; ++i)
				diagramViewSelectable[i] = false;
		}
		void JAnimationControllerEditor::ClearCash()noexcept
		{
			controller = nullptr;
			selectedDiagram = nullptr;
			selectedState = nullptr;
			selectedCondition = nullptr;

			isSelectedDiagram = false;
			isSelectedState = false;
			isSelectedCondition = false;
		}
		void JAnimationControllerEditor::Activate()noexcept
		{
			JEditor::Activate();
			std::vector<J_EDITOR_EVENT> enumVec
			{
				J_EDITOR_EVENT::MOUSE_CLICK, J_EDITOR_EVENT::SELECT_OBJECT, J_EDITOR_EVENT::DESELECT_OBJECT
			};

			RegisterEventListener(enumVec);

			preMousePosX = ImGui::GetMousePos().x;
			preMousePosY = ImGui::GetMousePos().y;
		}
		void JAnimationControllerEditor::DeActivate()noexcept
		{
			JEditor::DeActivate();
			DeRegisterListener();
			ClearCash();
		}
		void JAnimationControllerEditor::OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEventStruct* eventStruct)
		{
			if (senderGuid == GetGuid())
				return;

			if (eventType == J_EDITOR_EVENT::MOUSE_CLICK)
				CloseAllPopup();
			else if (eventType == J_EDITOR_EVENT::SELECT_GAMEOBJECT)
			{
				JEditorSelectGameObjectEvStruct* selectGameObjEvStruct = static_cast<JEditorSelectGameObjectEvStruct*>(eventStruct);
				JAnimator* animators = selectGameObjEvStruct->gameObject->GetComponentWithParent<JAnimator>();
				if (animators != nullptr)
				{
					if (selectedController == nullptr || selectedController->GetGuid() !=
						animators->GetAnimatorController()->GetGuid())
					{
						ClearSelectableBuff();
						selectedController = animators->GetAnimatorController();
						diagramIndex = 0;
						if (selectedController->GetAnimationDiagramCount() > 0)
							diagramListSelectable[0] = true;

						selectedState = nullptr;
						selectedCondition = nullptr;
					}
				}
			}
			else if (eventType == J_EDITOR_EVENT::SELECT_RESOURCE)
			{
				JEditorSelectResourceEvStruct* selectResourceEvStruct = static_cast<JEditorSelectResourceEvStruct*>(eventStruct);
				if (selectResourceEvStruct->rObj->GetResourceType() == J_RESOURCE_TYPE::ANIMATION_CONTROLLER)
				{
					selectedController = static_cast<JAnimationController*>(selectResourceEvStruct->rObj);
					diagramIndex = 0;
					if (selectedController->GetAnimationDiagramCount() > 0)
						diagramListSelectable[0] = true;

					selectedState = nullptr;
					selectedCondition = nullptr;
				}
			}
			else if (eventType == J_EDITOR_EVENT::DESELECT_RESOURCE)
			{
				JEditorSelectResourceEvStruct* selectResourceEvStruct = static_cast<JEditorSelectResourceEvStruct*>(eventStruct);
				if (selectResourceEvStruct->rObj->GetResourceType() == J_RESOURCE_TYPE::ANIMATION_CONTROLLER)
				{
					if (selectedController != nullptr && selectedController->GetGuid() == selectResourceEvStruct->rObj->GetGuid())
					{
						selectedController = nullptr;
						diagramIndex = 0;
						selectedState = nullptr;
						selectedCondition = nullptr;
						CloseAllPopup();
					}
				}
			}
		}
	}
}