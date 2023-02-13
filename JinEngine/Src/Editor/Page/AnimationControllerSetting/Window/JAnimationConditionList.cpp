#include"JAnimationConditionList.h"
#include"../../JEditorAttribute.h"
#include"../../JEditorPageShareData.h" 
#include"../../../Event/JEditorEvent.h"
#include"../../../Popup/JEditorPopupMenu.h"
#include"../../../Popup/JEditorPopupNode.h"
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h" 
#include"../../../String/JEditorStringMap.h"
#include"../../../Helpers/JEditorInputBuffHelper.h"
#include"../../../EditTool/JEditorViewStructure.h" 
#include"../../../../Core/Reflection/JTypeTemplate.h"
#include"../../../../Core/FSM/JFSMfactory.h"
#include"../../../../Core/FSM/JFSMparameter.h"
#include"../../../../Core/FSM/AnimationFSM/JAnimationStateType.h"
#include"../../../../Core/FSM/AnimationFSM/JAnimationFSMdiagram.h" 
#include"../../../../Object/Resource/AnimationController/JAnimationController.h"   

namespace JinEngine
{
	namespace Editor
	{
		namespace Constants
		{
			static std::string ConditionListName(const std::string& uniqueLabel)noexcept
			{
				return "ConditionList##" + uniqueLabel;
			}
		}

		JAnimationConditionList::JAnimationConditionList(const std::string& name,
			std::unique_ptr<JEditorAttribute> attribute,
			const J_EDITOR_PAGE_TYPE ownerPageType,
			const J_EDITOR_WINDOW_FLAG windowFlag)
			:JEditorWindow(name, std::move(attribute), ownerPageType, windowFlag)
		{
			editorString = std::make_unique<JEditorStringMap>(); 

			std::unique_ptr<JEditorPopupNode> conditionListRootNode =
				std::make_unique<JEditorPopupNode>("Animation Controller Editor Condition List Popup Root", J_EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

			std::unique_ptr<JEditorPopupNode> createNewConditionNode =
				std::make_unique<JEditorPopupNode>("Create New Condition", J_EDITOR_POPUP_NODE_TYPE::LEAF, conditionListRootNode.get());
			editorString->AddString(createNewConditionNode->GetNodeId(), { "Create New Condition" , u8"애니메이션 패러미터 생성" });

			std::unique_ptr<JEditorPopupNode> destroyConditionNode =
				std::make_unique<JEditorPopupNode>("Destroy Condition", J_EDITOR_POPUP_NODE_TYPE::LEAF, conditionListRootNode.get());
			editorString->AddString(destroyConditionNode->GetNodeId(), { "Destroy Condition" , u8"애니메이션 패러미터 삭제" });

			regCreateConditionEvF = std::make_unique<RegisterEvF::Functor>(&JAnimationConditionList::RegisterCreateConditionEv, this);
			regDestroyConditionEvF = std::make_unique<RegisterEvF::Functor>(&JAnimationConditionList::RegisterDestroyConditionEv, this);
			createConditionF = std::make_unique<ConditionCreationFunctor>(&JAnimationConditionList::CreateCondition, this);
			destroyConditionF = std::make_unique<ConditionCreationFunctor>(&JAnimationConditionList::DestroyCondition, this);

			createNewConditionNode->RegisterSelectBind(std::make_unique<RegisterEvF::CompletelyBind>(*regCreateConditionEvF));
			destroyConditionNode->RegisterSelectBind(std::make_unique<RegisterEvF::CompletelyBind>(*regDestroyConditionEvF));

			conditionListPopup = std::make_unique<JEditorPopupMenu>(Constants::ConditionListName(GetName()), std::move(conditionListRootNode));
			conditionListPopup->AddPopupNode(std::move(createNewConditionNode));
			conditionListPopup->AddPopupNode(std::move(destroyConditionNode));

			auto setConditionTypeLam = [](const Core::J_FSM_PARAMETER_VALUE_TYPE vType, AniContUserPtr aniCont, size_t guid)
			{
				aniCont->GetParameter(guid)->SetValueType(vType);
			};
			auto setConditionNameLam = [](const std::string value, AniContUserPtr aniCont, size_t guid)
			{
				aniCont->GetParameter(guid)->SetName(JCUtil::U8StrToWstr(value));
			};
			auto setConditionBoolLam = [](const bool value, AniContUserPtr aniCont, size_t guid)
			{
				aniCont->GetParameter(guid)->SetValue(value);
			};
			auto setConditionIntLam = [](const int value, AniContUserPtr aniCont, size_t guid)
			{
				aniCont->GetParameter(guid)->SetValue(value);
			};
			auto setConditionFloatLam = [](const float value, AniContUserPtr aniCont, size_t guid)
			{
				aniCont->GetParameter(guid)->SetValue(value);
			};

			setConditionTypeF = std::make_unique<SetConditionTypeFunctor>(setConditionTypeLam);
			setConditionNameF = std::make_unique<SetConditionNameFunctor>(setConditionNameLam);
			setConditionBoolF = std::make_unique< SetConditionBooleanValueFunctor>(setConditionBoolLam);
			setConditionIntF = std::make_unique< SetConditionIntValueFunctor>(setConditionIntLam);
			setConditionFloatF = std::make_unique< SetConditionFloatValueFunctor>(setConditionFloatLam);
		}
		J_EDITOR_WINDOW_TYPE JAnimationConditionList::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::ANIMATION_CONDITION_LIST;
		}
		void JAnimationConditionList::Initialize(Core::JUserPtr<JAnimationController> newAniCont)noexcept
		{
			aniCont = newAniCont;
		}
		void JAnimationConditionList::UpdateWindow()
		{
			EnterWindow(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
			UpdateDocking();
			if (IsActivated())
			{
				UpdateMouseClick();
				BuildConditionList();
			}
			CloseWindow();
		}
		void JAnimationConditionList::BuildConditionList()
		{  
			ImGuiTableFlags flag = ImGuiTableFlags_BordersV |
				ImGuiTableFlags_BordersOuterH |
				ImGuiTableFlags_RowBg |
				ImGuiTableFlags_ContextMenuInBody |
				ImGuiTableFlags_Resizable;

			if (aniCont.IsValid() && JImGuiImpl::BeginTable("##ConditionList_Table" + GetName(), 4, flag))
			{
				JImGuiImpl::TableSetupColumn("", ImGuiTableColumnFlags_DefaultHide, 0.005f);
				JImGuiImpl::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.445f);
				JImGuiImpl::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 0.275f);
				JImGuiImpl::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.275f);
				JImGuiImpl::TableHeadersRow();
				const uint conditionCount = aniCont->GetParameterCount();
				for (uint i = 0; i < conditionCount; ++i)
				{
					Core::JFSMparameter* nowCondition = aniCont->GetParameterByIndex(i);
					std::string name = JCUtil::WstrToU8Str(nowCondition->GetName());

					JImGuiImpl::TableNextRow();
					JImGuiImpl::TableSetColumnIndex(0);

					const bool isSelect = selectedCondition.IsValid() ? nowCondition->GetGuid() == selectedCondition->GetGuid() : false;
					if (JImGuiImpl::Selectable(("##ConditionName_Selectable" + name).c_str(), &isSelect, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap))
					{
						if (!isSelect)
							RequestSelectObject(JEditorSelectObjectEvStruct(GetOwnerPageType(), Core::GetUserPtr(nowCondition)));
					}

					JImGuiImpl::TableSetColumnIndex(1);
					ImGui::PushItemWidth(-FLT_MIN);
					if (isSelect)
					{
						//rename 추가 필요
					}
					else
						JImGuiImpl::Text(name.c_str());

					Core::J_FSM_PARAMETER_VALUE_TYPE valueType = nowCondition->GetValueType();
					JImGuiImpl::TableSetColumnIndex(2);
					ImGui::PushItemWidth(-FLT_MIN);

					JImGuiImpl::ComoboEnumSetT(GetName(), valueType, *setConditionTypeF, aniCont, nowCondition->GetGuid());

					JImGuiImpl::TableSetColumnIndex(3);
					ImGui::PushItemWidth(-FLT_MIN);
					if (valueType == Core::J_FSM_PARAMETER_VALUE_TYPE::BOOL)
						JImGuiImpl::CheckBoxSetT(GetName(), (bool)nowCondition->GetValue(), *setConditionBoolF, aniCont, nowCondition->GetGuid());
					else if (valueType == Core::J_FSM_PARAMETER_VALUE_TYPE::INT)
						JImGuiImpl::InputIntSetT(GetName(), (int)nowCondition->GetValue(), *setConditionIntF, aniCont, nowCondition->GetGuid());
					else if (valueType == Core::J_FSM_PARAMETER_VALUE_TYPE::FLOAT)
						JImGuiImpl::InputFloatSetT(GetName(), (float)nowCondition->GetValue(), *setConditionFloatF, aniCont, nowCondition->GetGuid());
				}
				JImGuiImpl::EndTable();
			}
			if (aniCont.IsValid())
			{
				if (conditionListPopup->IsOpen())
					conditionListPopup->ExecutePopup(editorString.get());
				conditionListPopup->Update();
			} 
		}
		void JAnimationConditionList::RegisterCreateConditionEv()
		{
			if (!aniCont.IsValid())
				return;

			using BindT = JAnimationConditionList::ConditionCreationBind;
			size_t guid = Core::MakeGuid();
			auto cUptr = std::make_unique<BindT>(*createConditionF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, std::move(guid));
			auto dUptr = std::make_unique<BindT>(*destroyConditionF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, std::move(guid));

			using CreationTask = Core::JTransitionCreationTask<DataHandleStructure, BindT, BindT>;
			Core::JTransition::Execute(std::make_unique<CreationTask>("Create Condition", std::move(cUptr), std::move(dUptr), fsmdata));
		}
		void JAnimationConditionList::RegisterDestroyConditionEv()
		{
			if (!aniCont.IsValid() || !selectedCondition.IsValid())
				return;

			using BindT = JAnimationConditionList::ConditionCreationBind;
			size_t guid = Core::MakeGuid();
			auto cUptr = std::make_unique<BindT>(*createConditionF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, selectedCondition->GetGuid());
			auto dUptr = std::make_unique<BindT>(*destroyConditionF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, selectedCondition->GetGuid());

			using DestroyTask = Core::JTransitionCreationTask<DataHandleStructure, BindT, BindT>;
			Core::JTransition::Execute(std::make_unique<DestroyTask>("Destroy Condition", std::move(dUptr), std::move(cUptr), fsmdata));
		}
		void JAnimationConditionList::CreateCondition(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t guid)
		{
			if (aniCont.IsValid())
			{
				Core::JOwnerPtr<Core::JIdentifier> owner = dS.Release(dH);
				if (owner.IsValid() && Core::Cast<Core::JFSMparameter>(owner.Get()))
				{
					auto condition = Core::JOwnerPtr<Core::JFSMparameter>::ConvertChildUser(std::move(owner));
					auto ptr = condition.Get();
					Core::JIdentifier::AddInstance(std::move(condition));
					JEditorPageShareData::SetSelectObj(GetOwnerPageType(), Core::GetUserPtr(ptr));
				}
				else
					aniCont->CreateFSMCondition(guid);
				SetModifiedBit(aniCont, true);
			}
		}
		void JAnimationConditionList::DestroyCondition(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t guid)
		{
			if (aniCont.IsValid())
			{
				auto conditionPtr = aniCont->GetParameter(guid);
				if (conditionPtr != nullptr)
				{
					Core::JDataHandle newHandle = dS.Add(Core::JIdentifier::ReleaseInstance<Core::JFSMparameter>(conditionPtr->GetGuid()));
					dS.TransitionHandle(newHandle, dH);
					SetModifiedBit(aniCont, true);
					JEditorPageShareData::SetSelectObj(GetOwnerPageType(), Core::JUserPtr<Core::JIdentifier>{});
				}
			}
		}
		void JAnimationConditionList::DoSetClose()noexcept
		{
			aniCont.Clear();
			selectedDiagram.Clear();
			selectedCondition.Clear();
		}
	}
}