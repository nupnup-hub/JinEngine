#include"JAnimationParameterList.h"
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
			static std::string ParameterListName(const std::string& uniqueLabel)noexcept
			{
				return "ParameterList##" + uniqueLabel;
			}
		}

		JAnimationParameterList::JAnimationParameterList(const std::string& name,
			std::unique_ptr<JEditorAttribute> attribute,
			const J_EDITOR_PAGE_TYPE ownerPageType,
			const J_EDITOR_WINDOW_FLAG windowFlag)
			:JEditorWindow(name, std::move(attribute), ownerPageType, windowFlag)
		{
			editorString = std::make_unique<JEditorStringMap>(); 

			std::unique_ptr<JEditorPopupNode> parameterListRootNode =
				std::make_unique<JEditorPopupNode>("Animation Controller Editor Parameter List Popup Root", J_EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

			std::unique_ptr<JEditorPopupNode> createNewParameterNode =
				std::make_unique<JEditorPopupNode>("Create New Parameter", J_EDITOR_POPUP_NODE_TYPE::LEAF, parameterListRootNode.get());
			editorString->AddString(createNewParameterNode->GetNodeId(), { "Create New Parameter" , u8"애니메이션 패러미터 생성" });

			std::unique_ptr<JEditorPopupNode> destroyParameterNode =
				std::make_unique<JEditorPopupNode>("Destroy Parameter", J_EDITOR_POPUP_NODE_TYPE::LEAF, parameterListRootNode.get());
			editorString->AddString(destroyParameterNode->GetNodeId(), { "Destroy Parameter" , u8"애니메이션 패러미터 삭제" });

			regCreateParameterEvF = std::make_unique<RegisterEvF::Functor>(&JAnimationParameterList::RegisterCreateParameterEv, this);
			regDestroyParameterEvF = std::make_unique<RegisterEvF::Functor>(&JAnimationParameterList::RegisterDestroyParameterEv, this);
			createParameterF = std::make_unique<ParameterCreationFunctor>(&JAnimationParameterList::CreateParameter, this);
			destroyParameterF = std::make_unique<ParameterCreationFunctor>(&JAnimationParameterList::DestroyParameter, this);

			createNewParameterNode->RegisterSelectBind(std::make_unique<RegisterEvF::CompletelyBind>(*regCreateParameterEvF));
			destroyParameterNode->RegisterSelectBind(std::make_unique<RegisterEvF::CompletelyBind>(*regDestroyParameterEvF));

			parameterListPopup = std::make_unique<JEditorPopupMenu>(Constants::ParameterListName(GetName()), std::move(parameterListRootNode));
			parameterListPopup->AddPopupNode(std::move(createNewParameterNode));
			parameterListPopup->AddPopupNode(std::move(destroyParameterNode));

			auto setParameterTypeLam = [](const Core::J_FSM_PARAMETER_VALUE_TYPE vType, AniContUserPtr aniCont, size_t guid)
			{
				aniCont->GetParameter(guid)->SetParamType(vType);
			};
			auto setParameterNameLam = [](const std::string value, AniContUserPtr aniCont, size_t guid)
			{
				aniCont->GetParameter(guid)->SetName(JCUtil::U8StrToWstr(value));
			};
			auto setParameterBoolLam = [](const bool value, AniContUserPtr aniCont, size_t guid)
			{
				aniCont->GetParameter(guid)->SetValue(value);
			};
			auto setParameterIntLam = [](const int value, AniContUserPtr aniCont, size_t guid)
			{
				aniCont->GetParameter(guid)->SetValue(value);
			};
			auto setParameterFloatLam = [](const float value, AniContUserPtr aniCont, size_t guid)
			{
				aniCont->GetParameter(guid)->SetValue(value);
			};

			setParameterTypeF = std::make_unique<SetParameterTypeFunctor>(setParameterTypeLam);
			setParameterNameF = std::make_unique<SetParameterNameFunctor>(setParameterNameLam);
			setParameterBoolF = std::make_unique< SetParameterBooleanValueFunctor>(setParameterBoolLam);
			setParameterIntF = std::make_unique< SetParameterIntValueFunctor>(setParameterIntLam);
			setParameterFloatF = std::make_unique< SetParameterFloatValueFunctor>(setParameterFloatLam);
		}
		J_EDITOR_WINDOW_TYPE JAnimationParameterList::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::ANIMATION_CONDITION_LIST;
		}
		void JAnimationParameterList::Initialize(Core::JUserPtr<JAnimationController> newAniCont)noexcept
		{
			aniCont = newAniCont;
		}
		void JAnimationParameterList::UpdateWindow()
		{
			EnterWindow(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
			UpdateDocking();
			if (IsActivated())
			{
				UpdateMouseClick();
				BuildParameterList();
			}
			CloseWindow();
		}
		void JAnimationParameterList::BuildParameterList()
		{  
			ImGuiTableFlags flag = ImGuiTableFlags_BordersV |
				ImGuiTableFlags_BordersOuterH |
				ImGuiTableFlags_RowBg |
				ImGuiTableFlags_ContextMenuInBody |
				ImGuiTableFlags_Resizable;

			if (aniCont.IsValid() && JImGuiImpl::BeginTable("##ParameterList_Table" + GetName(), 3, flag))
			{
				//JImGuiImpl::TableSetupColumn("", ImGuiTableColumnFlags_DefaultHide, 0.005f);
				JImGuiImpl::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.445f);
				JImGuiImpl::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 0.275f);
				JImGuiImpl::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.275f);
				JImGuiImpl::TableHeadersRow();
				const uint parameterCount = aniCont->GetParameterCount();
				for (uint i = 0; i < parameterCount; ++i)
				{
					Core::JFSMparameter* nowParameter = aniCont->GetParameterByIndex(i);
					std::string name = JCUtil::WstrToU8Str(nowParameter->GetName());
					std::string uniqueLabel = std::to_string(nowParameter->GetGuid());

					JImGuiImpl::TableNextRow();
					JImGuiImpl::TableSetColumnIndex(0);

					const bool isSelect = selectedParameter.IsValid() ? nowParameter->GetGuid() == selectedParameter->GetGuid() : false;
					if (JImGuiImpl::Selectable(name + "##Parameter_Selectable" + uniqueLabel, &isSelect, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap))
					{
						if (!isSelect)
							RequestSelectObject(JEditorSelectObjectEvStruct(GetOwnerPageType(), Core::GetUserPtr(nowParameter)));
					}

					//JImGuiImpl::TableSetColumnIndex(1);
					ImGui::PushItemWidth(-FLT_MIN);
					if (isSelect)
					{
						//rename 추가 필요
					} 

					Core::J_FSM_PARAMETER_VALUE_TYPE valueType = nowParameter->GetParamType();
					JImGuiImpl::TableSetColumnIndex(1);
					ImGui::PushItemWidth(-FLT_MIN);

					JImGuiImpl::ComoboEnumSetT(uniqueLabel, valueType, *setParameterTypeF, aniCont, nowParameter->GetGuid());

					JImGuiImpl::TableSetColumnIndex(2);
					ImGui::PushItemWidth(-FLT_MIN);
					if (valueType == Core::J_FSM_PARAMETER_VALUE_TYPE::BOOL)
						JImGuiImpl::CheckBoxSetT(uniqueLabel, (bool)nowParameter->GetValue(), *setParameterBoolF, aniCont, nowParameter->GetGuid());
					else if (valueType == Core::J_FSM_PARAMETER_VALUE_TYPE::INT)
						JImGuiImpl::InputIntSetT(uniqueLabel, (int)nowParameter->GetValue(), *setParameterIntF, aniCont, nowParameter->GetGuid());
					else if (valueType == Core::J_FSM_PARAMETER_VALUE_TYPE::FLOAT)
						JImGuiImpl::InputFloatSetT(uniqueLabel, (float)nowParameter->GetValue(), *setParameterFloatF, aniCont, nowParameter->GetGuid());
				}
				JImGuiImpl::EndTable();
			}
			if (aniCont.IsValid())
			{
				if (parameterListPopup->IsOpen())
					parameterListPopup->ExecutePopup(editorString.get());
				parameterListPopup->Update();
			} 
		}
		void JAnimationParameterList::RegisterCreateParameterEv()
		{
			if (!aniCont.IsValid())
				return;

			using BindT = JAnimationParameterList::ParameterCreationBind;
			size_t guid = Core::MakeGuid();
			auto cUptr = std::make_unique<BindT>(*createParameterF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, std::move(guid));
			auto dUptr = std::make_unique<BindT>(*destroyParameterF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, std::move(guid));

			using CreationTask = Core::JTransitionCreationTask<DataHandleStructure, BindT, BindT>;
			Core::JTransition::Execute(std::make_unique<CreationTask>("Create Parameter", std::move(cUptr), std::move(dUptr), fsmdata));
		}
		void JAnimationParameterList::RegisterDestroyParameterEv()
		{
			if (!aniCont.IsValid() || !selectedParameter.IsValid())
				return;

			using BindT = JAnimationParameterList::ParameterCreationBind;
			size_t guid = Core::MakeGuid();
			auto cUptr = std::make_unique<BindT>(*createParameterF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, selectedParameter->GetGuid());
			auto dUptr = std::make_unique<BindT>(*destroyParameterF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, selectedParameter->GetGuid());

			using DestroyTask = Core::JTransitionCreationTask<DataHandleStructure, BindT, BindT>;
			Core::JTransition::Execute(std::make_unique<DestroyTask>("Destroy Parameter", std::move(dUptr), std::move(cUptr), fsmdata));
		}
		void JAnimationParameterList::CreateParameter(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t guid)
		{
			if (aniCont.IsValid())
			{
				Core::JOwnerPtr<Core::JIdentifier> owner = dS.Release(dH);
				if (owner.IsValid() && Core::Cast<Core::JFSMparameter>(owner.Get()))
				{
					auto parameter = Core::JOwnerPtr<Core::JFSMparameter>::ConvertChildUser(std::move(owner));
					auto ptr = parameter.Get();
					Core::JIdentifier::AddInstance(std::move(parameter));
					JEditorPageShareData::SetSelectObj(GetOwnerPageType(), Core::GetUserPtr(ptr));
				}
				else
					aniCont->CreateFSMparameter(guid);
				SetModifiedBit(aniCont, true);
			}
		}
		void JAnimationParameterList::DestroyParameter(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t guid)
		{
			if (aniCont.IsValid())
			{
				auto parameterPtr = aniCont->GetParameter(guid);
				if (parameterPtr != nullptr)
				{
					Core::JDataHandle newHandle = dS.Add(Core::JIdentifier::ReleaseInstance<Core::JFSMparameter>(parameterPtr->GetGuid()));
					dS.TransitionHandle(newHandle, dH);
					SetModifiedBit(aniCont, true);
					JEditorPageShareData::SetSelectObj(GetOwnerPageType(), Core::JUserPtr<Core::JIdentifier>{});
				}
			}
		}
		void JAnimationParameterList::DoSetClose()noexcept
		{
			aniCont.Clear();
			selectedDiagram.Clear();
			selectedParameter.Clear();
		}
	}
}