#include"AnimationControllerEditor.h"
#include"../EditorAttribute.h"
#include"../../ImGuiEx/ImGuiManager.h" 
#include"../../String/EditorString.h"
#include"../../Popup/EditorPopup.h"
#include"../../Popup/EditorPopupNode.h"
#include"../../Diagram/EditorDiagramNode.h"
#include"../../Utility/EditorUtility.h" 
#include"../../Event/EditorEventStruct.h"

#include"../../../../Object/Resource/AnimationController/JAnimationController.h"
#include"../../../../Object/Component/Animator/JAnimator.h"
#include"../../../../Object/GameObject/JGameObject.h"
#include"../../../../Core/FSM/AnimationFSM/JAnimationFSMdiagram.h"
#include"../../../../Core/FSM/AnimationFSM/JAnimationFSMstate.h"
#include"../../../../Core/FSM/JFSMcondition.h"
#include"../../../../Utility/JCommonUtility.h"

namespace JinEngine
{
	AnimationControllerEditor::AnimationControllerEditor(std::unique_ptr<EditorAttribute> attribute, const size_t ownerPageGuid)
		:EditorWindow(std::move(attribute), ownerPageGuid),
		diagramListName("DiagramList##" + GetName()),
		parameterListName("ParameterList##" + GetName()),
		diagramViewName("DiagramView##" + GetName())
	{
		editorString = std::make_unique<EditorString>();

		//Diagram List Popup
		std::unique_ptr<EditorPopupNode> diagramListRootNode =
			std::make_unique<EditorPopupNode>("Animation Controller Editor Diagram List Popup Root", EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

		std::unique_ptr<EditorPopupNode> createNewDiagramNode =
			std::make_unique<EditorPopupNode>("Create New Diagram", EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, diagramListRootNode.get());
		editorString->AddString(createNewDiagramNode->GetNodeId(), { "Create New Diagram" , u8"애니메이션 다이어그램 생성" });

		std::unique_ptr<EditorPopupNode> eraseDiagramNode =
			std::make_unique<EditorPopupNode>("Erase Diagram", EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, diagramListRootNode.get());
		editorString->AddString(eraseDiagramNode->GetNodeId(), { "Erase Diagram" , u8"애니메이션 다이어그램 삭제" });

		JDelegate<void(EditorUtility*)> createDiagramFunc;
		createDiagramFunc.connect(this, &AnimationControllerEditor::CreateNewDiagram);

		JDelegate<void(EditorUtility*)> eraseDiagramFunc;
		eraseDiagramFunc.connect(this, &AnimationControllerEditor::EraseDiagram);

		diagramListPopupFunc.emplace(createNewDiagramNode->GetNodeId(), createDiagramFunc);
		diagramListPopupFunc.emplace(eraseDiagramNode->GetNodeId(), eraseDiagramFunc);

		diagramListPopup = std::make_unique<EditorPopup>(diagramListName, std::move(diagramListRootNode));

		diagramListPopup->AddPopupNode(std::move(createNewDiagramNode));
		diagramListPopup->AddPopupNode(std::move(eraseDiagramNode));

		//Parameter List Popup
		std::unique_ptr<EditorPopupNode> parameterListRootNode =
			std::make_unique<EditorPopupNode>("Animation Controller Editor Parameter List Popup Root", EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

		std::unique_ptr<EditorPopupNode> createNewParameterNode =
			std::make_unique<EditorPopupNode>("Create New Parameter", EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, parameterListRootNode.get());
		editorString->AddString(createNewParameterNode->GetNodeId(), { "Create New Paramete" , u8"애니메이션 패러미터 생성" });

		std::unique_ptr<EditorPopupNode> eraseParameterNode =
			std::make_unique<EditorPopupNode>("Erase Parameter", EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, parameterListRootNode.get());
		editorString->AddString(eraseParameterNode->GetNodeId(), { "Erase Parameter" , u8"애니메이션 패러미터 삭제" });

		JDelegate<void(EditorUtility*)> createParameterFunc;
		createParameterFunc.connect(this, &AnimationControllerEditor::CreateNewParameter);

		JDelegate<void(EditorUtility*)> eraseParameterFunc;
		eraseParameterFunc.connect(this, &AnimationControllerEditor::EraseParameter);

		parameterListPopupFunc.emplace(createNewParameterNode->GetNodeId(), createParameterFunc);
		parameterListPopupFunc.emplace(eraseParameterNode->GetNodeId(), eraseParameterFunc);

		parameterListPopup = std::make_unique<EditorPopup>(parameterListName, std::move(parameterListRootNode));
		parameterListPopup->AddPopupNode(std::move(createNewParameterNode));
		parameterListPopup->AddPopupNode(std::move(eraseParameterNode));

		//Diagram View Popup
		std::unique_ptr<EditorPopupNode> diagramViewRootNode =
			std::make_unique<EditorPopupNode>("Animation Controller Editor Diagram View Popup Root", EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

		std::unique_ptr<EditorPopupNode> createNewStateNode =
			std::make_unique<EditorPopupNode>("Create New State", EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, diagramViewRootNode.get());
		editorString->AddString(createNewStateNode->GetNodeId(), { "Create New State" , u8"애니메이션 상태 생성" });

		std::unique_ptr<EditorPopupNode> eraseStateNode =
			std::make_unique<EditorPopupNode>("Erase State", EDITOR_POPUP_NODE_TYPE::LEAF_SELECT, diagramViewRootNode.get());
		editorString->AddString(eraseStateNode->GetNodeId(), { "Erase State" , u8"애니메이션 상태 삭제" });

		JDelegate<void(EditorUtility*, DiagramViewFuncData&)> createStateFunc;
		createStateFunc.connect(this, &AnimationControllerEditor::CreateNewState);

		JDelegate<void(EditorUtility*, DiagramViewFuncData&)> eraseStateFunc;
		eraseStateFunc.connect(this, &AnimationControllerEditor::EraseState);

		diagramViewPopupFunc.emplace(createNewStateNode->GetNodeId(), createStateFunc);
		diagramViewPopupFunc.emplace(eraseStateNode->GetNodeId(), eraseStateFunc);

		diagramViewPopup = std::make_unique< EditorPopup>(diagramViewName, std::move(diagramViewRootNode));
		diagramViewPopup->AddPopupNode(std::move(createNewStateNode));
		diagramViewPopup->AddPopupNode(std::move(eraseStateNode));

		nameBuf.resize(nameMaxLength);
	}
	AnimationControllerEditor::~AnimationControllerEditor() {};

	void AnimationControllerEditor::Initialize(EditorUtility* editorUtility)
	{


	}
	void AnimationControllerEditor::UpdateWindow(EditorUtility* editorUtility)
	{
		EditorWindow::UpdateWindow(editorUtility);

		float oriSize = ImGui::GetStyle().ChildBorderSize;
		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 2);

		float preCursorPosY = ImGui::GetCursorPosY();
		BuildDiagramListView(editorUtility);
		BuildParameterView(editorUtility);

		ImGuiManager::SetColor(ImVec4(0, 0, 0, 0), ImGuiCol_Header);
		ImGuiManager::SetColor(ImVec4(0, 0, 0, 0), ImGuiCol_HeaderHovered);
		ImGuiManager::SetColor(ImVec4(0, 0, 0, 0), ImGuiCol_HeaderActive);
		BuildAnimationDiagramView(editorUtility, preCursorPosY);
		ImGuiManager::ReturnDefaultColor(ImGuiCol_Header);
		ImGuiManager::ReturnDefaultColor(ImGuiCol_HeaderHovered);
		ImGuiManager::ReturnDefaultColor(ImGuiCol_HeaderActive);

		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, oriSize);
		preMousePosX = ImGui::GetMousePos().x;
		preMousePosY = ImGui::GetMousePos().y;
	}
	bool AnimationControllerEditor::Activate(EditorUtility* editorUtility)
	{
		if (Editor::Activate(editorUtility))
		{
			std::vector<EDITOR_EVENT> enumVec
			{
				EDITOR_EVENT::MOUSE_CLICK,EDITOR_EVENT::SELECT_GAMEOBJECT,EDITOR_EVENT::SELECT_RESOURCE, EDITOR_EVENT::DESELECT_RESOURCE
			};

			this->AddEventListener(*editorUtility->EvInterface(), GetGuid(), enumVec);

			preMousePosX = ImGui::GetMousePos().x;
			preMousePosY = ImGui::GetMousePos().y;
			return true;
		}
		else
			return false;
	}
	bool AnimationControllerEditor::DeActivate(EditorUtility* editorUtility)
	{
		if (Editor::DeActivate(editorUtility))
		{
			this->EraseListener(*editorUtility->EvInterface(), GetGuid());
			selectedController = nullptr;
			diagramIndex = 0;
			selectedState = nullptr;
			selectedCondition = nullptr;
			return true;
		}
		else
			return false;
	}
	void AnimationControllerEditor::BuildDiagramListView(EditorUtility* editorUtility)
	{
		ImVec2 windowSize = ImGui::GetWindowSize();
		ImGui::BeginChild("DiagramList##AnimationControllerEditor", ImVec2(windowSize.x * 0.2f, windowSize.y * 0.4f), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("DiagramList");

		if (selectedController != nullptr)
		{
			const uint diagramCount = selectedController->GetAnimationDiagramCount();
			ImGuiTableFlags flag = ImGuiTableFlags_Borders | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
			if (ImGui::BeginTable("##DiagramList_Table_AnimationControllerEditor", 1, flag))
			{
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableHeadersRow();
				for (uint i = 0; i < diagramCount; ++i)
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);

					if (ImGui::Selectable(selectedController->GetAnimationDiagramName(i).c_str(), &diagramListSelectable[i]))
					{
						if (diagramIndex != i)
						{
							for (uint j = 0; j < diagramCount; ++j)
								diagramListSelectable[j] = false;
							diagramListSelectable[i] = true;
							diagramIndex = i;
						}
						selectedCondition = nullptr;
						selectedState = nullptr;
					}
				}
				ImGui::EndTable();
			}
			if (diagramListPopup->IsOpen())
			{
				EDITOR_POPUP_NODE_RES res;
				size_t menuGuid;
				diagramListPopup->ExecutePopup(editorString.get(), res, menuGuid);
				if (res == EDITOR_POPUP_NODE_RES::CLICK_SLELECT_NODE)
				{
					auto func = diagramListPopupFunc.find(menuGuid);
					if (func != diagramListPopupFunc.end())
						func->second.Invoke(editorUtility);

					CloseAllPopup();
				}
			}

			if (ImGui::IsMouseInWindow(ImGui::GetWindowPos(), ImGui::GetWindowSize()))
			{
				if (editorUtility->rightMouseClick)
				{
					diagramListPopup->SetOpen(!diagramListPopup->IsOpen());
					parameterListPopup->SetOpen(false);
					diagramViewPopup->SetOpen(false);
				}
				else if (editorUtility->leftMouseClick && diagramListPopup->IsOpen() && !diagramListPopup->IsMouseInPopup())
					CloseAllPopup();
			}
		}
		ImGui::EndChild();
	}
	void AnimationControllerEditor::BuildParameterView(EditorUtility* editorUtility)
	{
		ImVec2 windowSize = ImGui::GetWindowSize();
		ImGui::BeginChild("Parameter##AnimationControllerEditor", ImVec2(windowSize.x * 0.2f, windowSize.y * 0.6f), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Parameter");

		ImGuiTableFlags flag = ImGuiTableFlags_BordersV |
			ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ContextMenuInBody;
		if (selectedController != nullptr)
		{
			const uint conditionCount = selectedController->GetConditionCount();	 
			if (ImGui::BeginTable("##ParameterList_Table_AnimationControllerEditor", 4, flag))
			{
				ImGui::TableSetupColumn("", ImGuiTableColumnFlags_DefaultHide, 0.005f);
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.445f);
				ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 0.275f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.275f);
				ImGui::TableHeadersRow();
				for (uint i = 0; i < conditionCount; ++i)
				{
					Core::JFSMcondition* nowCondition = selectedController->GetCondition(i);
					std::string name = nowCondition->GetName();

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					if (ImGui::Selectable(("##ConditionName_Selectable" + name).c_str(), &parameterListSelectable[i], ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap))
					{
						if (selectedCondition == nullptr || selectedCondition->GetId() != nowCondition->GetId())
						{
							for (uint j = 0; j < conditionCount; ++j)
								parameterListSelectable[j] = false;
							parameterListSelectable[i] = true;
							selectedCondition = nowCondition;
						}
						selectedState = nullptr;
						nameBuf.clear();
						nameBuf.resize(nameMaxLength);

						for (uint j = 0; j < name.size(); ++j)
							nameBuf[j] = name[j];
					}
					ImGui::TableSetColumnIndex(1);
					ImGui::PushItemWidth(-FLT_MIN);
					if (parameterListSelectable[i])
					{
						if (ImGui::InputText(("##ConditionName" + name).c_str(), &nameBuf[0], nameMaxLength, ImGuiInputTextFlags_EnterReturnsTrue))
						{
							const std::string newName = JCommonUtility::EraseEmptySpace(nameBuf);
							if (newName != name)
								selectedController->SetConditionName(nowCondition->GetName(), newName);
						}
					}
					else
						ImGui::Text(name.c_str());

					Core::J_FSMCONDITION_VALUE_TYPE valueType = nowCondition->GetValueType();
					ImGui::TableSetColumnIndex(2);
					ImGui::PushItemWidth(-FLT_MIN);
					if (ImGui::BeginCombo(("##SelectConditionValueType" + name).c_str(), Core::JFsmConditionValueType::ConditionEnumToString(valueType).c_str()))
					{
						for (uint j = 0; j < Core::JFsmConditionValueType::count; ++j)
						{
							Core::J_FSMCONDITION_VALUE_TYPE nowType = (Core::J_FSMCONDITION_VALUE_TYPE)((int)Core::J_FSMCONDITION_VALUE_TYPE::BOOL + j);
							std::string valueStr = Core::JFsmConditionValueType::ConditionEnumToString(nowType) + "##FSMconditionValue";
							if (ImGui::Selectable(valueStr.c_str()))
								selectedController->SetConditionValueType(nowCondition->GetName(), nowType);
						}
						ImGui::EndCombo();
					}

					ImGui::TableSetColumnIndex(3);
					ImGui::PushItemWidth(-FLT_MIN);
					if (valueType == Core::J_FSMCONDITION_VALUE_TYPE::BOOL)
					{
						bool nowValue = (bool)nowCondition->GetValue();
						if (ImGui::Checkbox(("##ConditionBoolean" + name).c_str(), &nowValue))
							nowCondition->SetValue((float)nowValue);
					}
					else if (valueType == Core::J_FSMCONDITION_VALUE_TYPE::INT)
					{
						int nowValue = (int)nowCondition->GetValue();
						if (ImGui::InputInt(("##ConditionInt" + name).c_str(), &nowValue))
						{
							if (nowValue != (int)nowCondition->GetValue())
								nowCondition->SetValue((float)nowValue);
						}
					}
					else if (valueType == Core::J_FSMCONDITION_VALUE_TYPE::FLOAT)
					{
						float nowValue = nowCondition->GetValue();
						if (ImGui::InputFloat(("##ConditionFloat" + name).c_str(), &nowValue))
						{
							if (nowValue != nowCondition->GetValue())
								nowCondition->SetValue(nowValue);
						}
					}
				}
				ImGui::EndTable();
			}

			if (parameterListPopup->IsOpen())
			{
				EDITOR_POPUP_NODE_RES res;
				size_t menuGuid;
				parameterListPopup->ExecutePopup(editorString.get(), res, menuGuid);
				if (res == EDITOR_POPUP_NODE_RES::CLICK_SLELECT_NODE)
				{
					auto func = parameterListPopupFunc.find(menuGuid);
					if (func != parameterListPopupFunc.end())
						func->second.Invoke(editorUtility);

					CloseAllPopup();
				}
			}
			if (ImGui::IsMouseInWindow(ImGui::GetWindowPos(), ImGui::GetWindowSize()))
			{
				if (editorUtility->rightMouseClick)
				{
					diagramListPopup->SetOpen(false);
					parameterListPopup->SetOpen(!parameterListPopup->IsOpen());
					diagramViewPopup->SetOpen(false);
				}
				else if (editorUtility->leftMouseClick && parameterListPopup->IsOpen() && !parameterListPopup->IsMouseInPopup())
					CloseAllPopup();
			}
		}
		ImGui::EndChild();
	}
	void AnimationControllerEditor::BuildAnimationDiagramView(EditorUtility* editorUtility, float cursorPosY)
	{
		ImVec2 windowSize = ImGui::GetWindowSize();
		ImGui::SetCursorPos(ImVec2(windowSize.x * 0.2f + (ImGui::GetStyle().WindowPadding.x * 2) + (ImGui::GetStyle().WindowBorderSize * 2), cursorPosY));
		ImGui::BeginChild("Diagram##AnimationControllerEditor", ImVec2(windowSize.x * 0.8f, windowSize.y), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);
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
				EditorDiagramNode* nowStateCoord = nowState->GetEditorDiagramNode();

				ImVec2 cursorPos = ImVec2(nowStateCoord->GetPosX(), nowStateCoord->GetPosY()) + shapeOffset;
				bool isSelect = true;
				ImGui::SetCursorPos(cursorPos);
				if (ImGui::Selectable(nowState->GetName().c_str(), &diagramViewSelectable[i], ImGuiSelectableFlags_SelectOnClick, ImVec2(stateShapeWidth, stateShapeHeight)))
				{
					if (selectedState == nullptr || selectedState->GetId() != nowState->GetId())
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
				EDITOR_POPUP_NODE_RES res;
				size_t menuGuid;
				diagramViewPopup->ExecutePopup(editorString.get(), res, menuGuid);
				if (res == EDITOR_POPUP_NODE_RES::CLICK_SLELECT_NODE)
				{
					auto func = diagramViewPopupFunc.find(menuGuid);
					if (func != diagramViewPopupFunc.end())
						func->second.Invoke(editorUtility, funcData);

					CloseAllPopup();
				}
			}
			if (ImGui::IsMouseInWindow(ImGui::GetWindowPos(), ImGui::GetWindowSize()))
			{
				if (editorUtility->rightMouseClick)
				{
					diagramListPopup->SetOpen(false);
					parameterListPopup->SetOpen(false);
					diagramViewPopup->SetOpen(!diagramViewPopup->IsOpen());
				}
				else if (editorUtility->leftMouseClick && diagramViewPopup->IsOpen() && !diagramViewPopup->IsMouseInPopup())
					CloseAllPopup();
			}
		}
		ImGui::EndChild();
	}
	void AnimationControllerEditor::CloseAllPopup()noexcept
	{
		diagramListPopup->SetOpen(false);
		parameterListPopup->SetOpen(false);
		diagramViewPopup->SetOpen(false);
	}
	void AnimationControllerEditor::ClearSelectableBuff()noexcept
	{
		for (uint i = 0; i < selectableBufLength; ++i)
		{
			diagramListSelectable[i] = false;
			parameterListSelectable[i] = false;
			diagramViewSelectable[i] = false;
		}
	}
	void AnimationControllerEditor::CreateNewDiagram(EditorUtility* editorUtility)noexcept
	{
		if (selectedController != nullptr)
			selectedController->CreateAnimationDiagram();
	}
	void AnimationControllerEditor::CreateNewParameter(EditorUtility* editorUtility)noexcept
	{
		if (selectedController != nullptr)
			selectedController->CreateParameter();
	}
	void AnimationControllerEditor::CreateNewState(EditorUtility* editorUtility, DiagramViewFuncData& funcData)noexcept
	{
		if (selectedController != nullptr)
		{
			ImVec2 windowPos = ImVec2(funcData.windowPosX, funcData.windowPosY);
			ImVec2 mousePos = ImGui::GetMousePos();
			ImVec2 shapeSize = ImVec2(stateShapeWidth, stateShapeHeight) * 0.5f;
			ImVec2 shapeOffset = ImVec2(funcData.windowWidth * 0.5f, funcData.windowHeight * 0.5f);
			ImVec2 newStateCoord = mousePos - windowPos - shapeOffset - shapeSize;

			selectedController->CreateAnimationClip(diagramIndex, newStateCoord.x, newStateCoord.y);
		}
	}
	void AnimationControllerEditor::EraseDiagram(EditorUtility* editorUtility)noexcept
	{
		if (selectedController != nullptr)
		{
			selectedController->EraseAnimationDiagram(diagramIndex);
			for (uint i = 0; i < selectableBufLength; ++i)
				diagramListSelectable[i] = false;
		}
	}
	void AnimationControllerEditor::EraseParameter(EditorUtility* editorUtility)noexcept
	{
		if (selectedController != nullptr && selectedCondition != nullptr)
		{
			selectedController->EraseParameter(selectedCondition->GetName());
			for (uint i = 0; i < selectableBufLength; ++i)
				parameterListSelectable[i] = false;
		}
	}
	void AnimationControllerEditor::EraseState(EditorUtility* editorUtility, DiagramViewFuncData& funcData)noexcept
	{
		if (selectedController != nullptr && selectedState != nullptr)
		{
			selectedController->EraseAnimationState(diagramIndex, selectedState->GetName());
			for (uint i = 0; i < selectableBufLength; ++i)
				diagramViewSelectable[i] = false;
		}
	}
	void AnimationControllerEditor::OnEvent(const size_t& senderGuid, const EDITOR_EVENT& eventType, EditorEventStruct* eventStruct)
	{
		if (senderGuid == GetGuid())
			return;

		if (eventType == EDITOR_EVENT::MOUSE_CLICK)
			CloseAllPopup();
		else if (eventType == EDITOR_EVENT::SELECT_GAMEOBJECT)
		{
			EditorSelectGameObjectEvStruct* selectGameObjEvStruct = static_cast<EditorSelectGameObjectEvStruct*>(eventStruct);
			std::vector<JAnimator*> animators = selectGameObjEvStruct->gameObject->GetComponentsWithChildren<JAnimator>();
			if (animators.size() > 0)
			{
				if (selectedController == nullptr || selectedController->GetGuid() !=
					animators[0]->GetAnimatorController()->GetGuid())
				{
					ClearSelectableBuff();
					selectedController = animators[0]->GetAnimatorController();
					diagramIndex = 0;
					if (selectedController->GetAnimationDiagramCount() > 0)
						diagramListSelectable[0] = true;

					selectedState = nullptr;
					selectedCondition = nullptr;
				}
			}
		}
		else if (eventType == EDITOR_EVENT::SELECT_RESOURCE)
		{
			EditorSelectResourceEvStruct* selectResourceEvStruct = static_cast<EditorSelectResourceEvStruct*>(eventStruct);
			if (selectResourceEvStruct->resourceObject->GetResourceType() == J_RESOURCE_TYPE::ANIMATION_CONTROLLER)
			{
				selectedController = static_cast<JAnimationController*>(selectResourceEvStruct->resourceObject);
				diagramIndex = 0;
				if (selectedController->GetAnimationDiagramCount() > 0)
					diagramListSelectable[0] = true;

				selectedState = nullptr;
				selectedCondition = nullptr;
			}
		}
		else if (eventType == EDITOR_EVENT::DESELECT_RESOURCE)
		{
			EditorSelectResourceEvStruct* selectResourceEvStruct = static_cast<EditorSelectResourceEvStruct*>(eventStruct);
			if (selectResourceEvStruct->resourceObject->GetResourceType() == J_RESOURCE_TYPE::ANIMATION_CONTROLLER)
			{
				if (selectedController != nullptr && selectedController->GetGuid() == selectResourceEvStruct->resourceObject->GetGuid())
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