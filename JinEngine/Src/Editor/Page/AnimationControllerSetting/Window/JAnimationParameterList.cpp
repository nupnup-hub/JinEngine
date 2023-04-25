#include"JAnimationParameterList.h"
#include"../../JEditorAttribute.h"
#include"../../JEditorPageShareData.h" 
#include"../../../Event/JEditorEvent.h"
#include"../../../Interface/JEditorObjectCreationInterface.h"
#include"../../../Popup/JEditorPopupMenu.h"
#include"../../../Popup/JEditorPopupNode.h"
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h" 
#include"../../../String/JEditorStringMap.h"
#include"../../../Helpers/JEditorInputBuffHelper.h"
#include"../../../EditTool/JEditorViewStructure.h" 
#include"../../../../Core/Reflection/JTypeTemplate.h" 
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

		/*
	class JAnimationParameterListCreationImpl
{
private:
	using ParameterCreationInterface = JEditorCreationRequestor<JEditorObjectCreateInterface<>>;
	using DestructionInterface = JEditorDestructionRequestor;
public:
	using ParameterCanCreateF = ParameterCreationInterface::CreateInteface::CanCreateF;
	using ParameterCreateF = ParameterCreationInterface::CreateInteface::ObjectCreateF; ;
public:
	using DataHandleStructure = ParameterCreationInterface::DataHandleStructure;
	using NotifyPtr = ParameterCreationInterface::NotifyPtr;
public:
	DataHandleStructure dS;
public:
	ParameterCreationInterface creation;
	DestructionInterface destructuion;
public:
	using RegisterEvF = Core::JSFunctorType<void, JAnimationParameterList*>;
public:
	std::unique_ptr<RegisterEvF::Functor> regCreateStateEvF;
	std::unique_ptr<RegisterEvF::Functor> regDestroyEvF;
public:
	JAnimationParameterListCreationImpl(RegisterEvF::Ptr regCreateStateEvPtr, RegisterEvF::Ptr regDestroyEvPtr)
	{
		regCreateStateEvF = std::make_unique<RegisterEvF::Functor>(regCreateStateEvPtr);
		regDestroyEvF = std::make_unique<RegisterEvF::Functor>(regDestroyEvPtr);
	}
	~JAnimationParameterListCreationImpl()
	{
		dS.Clear();
	}
};
*/
		DEFAULT_CD_REQUESTOR(JAnimationParameterListCreationImpl, JAnimationParameterList)
		using AniContUserPtr = Core::JUserPtr<JAnimationController>;	 
		class JAnimationParameterListSettingImpl
		{
		public:
			using SetParameterTypeFunctor = Core::JFunctor<void, const Core::J_FSM_PARAMETER_VALUE_TYPE, AniContUserPtr, size_t>;
			using SetParameterNameFunctor = Core::JFunctor<void, const std::string, AniContUserPtr, size_t>;
			using SetParameterBooleanValueFunctor = Core::JFunctor<void, const bool, AniContUserPtr, size_t>;
			using SetParameterIntValueFunctor = Core::JFunctor<void, const int, AniContUserPtr, size_t>;
			using SetParameterFloatValueFunctor = Core::JFunctor<void, const float, AniContUserPtr, size_t>;
		public:
			std::unique_ptr<SetParameterTypeFunctor> setParameterTypeF;
			std::unique_ptr<SetParameterNameFunctor> setParameterNameF;
			std::unique_ptr<SetParameterBooleanValueFunctor> setParameterBoolF;
			std::unique_ptr<SetParameterIntValueFunctor> setParameterIntF;
			std::unique_ptr<SetParameterFloatValueFunctor> setParameterFloatF;
		};

		JAnimationParameterList::JAnimationParameterList(const std::string& name,
			std::unique_ptr<JEditorAttribute> attribute,
			const J_EDITOR_PAGE_TYPE ownerPageType,
			const J_EDITOR_WINDOW_FLAG windowFlag)
			:JEditorWindow(name, std::move(attribute), ownerPageType, windowFlag)
		{
			editorString = std::make_unique<JEditorStringMap>(); 
			InitializeCreationImpl();
			InitializeSettingImpl();

			std::unique_ptr<JEditorPopupNode> parameterListRootNode =
				std::make_unique<JEditorPopupNode>("Animation Controller Editor Parameter List Popup Root", J_EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

			std::unique_ptr<JEditorPopupNode> createNewParameterNode =
				std::make_unique<JEditorPopupNode>("Create New Parameter", J_EDITOR_POPUP_NODE_TYPE::LEAF, parameterListRootNode.get());
			editorString->AddString(createNewParameterNode->GetNodeId(), { "Create New Parameter" , u8"애니메이션 패러미터 생성" });

			std::unique_ptr<JEditorPopupNode> destroyParameterNode =
				std::make_unique<JEditorPopupNode>("Destroy Parameter", J_EDITOR_POPUP_NODE_TYPE::LEAF, parameterListRootNode.get());
			editorString->AddString(destroyParameterNode->GetNodeId(), { "Destroy Parameter" , u8"애니메이션 패러미터 삭제" });

			using RequestEvF = JAnimationParameterListCreationImpl::RequestEvF;

			createNewParameterNode->RegisterSelectBind(std::make_unique<RequestEvF::CompletelyBind>(*creationImpl->reqCreateStateEvF, this));
			destroyParameterNode->RegisterSelectBind(std::make_unique<RequestEvF::CompletelyBind>(*creationImpl->reqDestroyEvF, this));
			destroyParameterNode->RegisterEnableBind(std::make_unique<JEditorPopupNode::EnableF::CompletelyBind>(*GetPassSelectedAboveOneFunctor(), this));

			parameterListPopup = std::make_unique<JEditorPopupMenu>(Constants::ParameterListName(GetName()), std::move(parameterListRootNode));
			parameterListPopup->AddPopupNode(std::move(createNewParameterNode));
			parameterListPopup->AddPopupNode(std::move(destroyParameterNode));
		}
		JAnimationParameterList::~JAnimationParameterList()
		{
			creationImpl.reset();
			settingImpl.reset();
		}
		void JAnimationParameterList::InitializeCreationImpl()
		{
			if (creationImpl != nullptr)
				return;

			auto requestCreateLam = [](JAnimationParameterList* paramList)
			{
				if (!paramList->aniCont.IsValid())
					return;

				JEditorCreationHint creationHint = JEditorCreationHint(paramList,
					true, false, false, true,
					Core::JTypeInstanceSearchHint(paramList->aniCont),
					Core::JTypeInstanceSearchHint(),
					&JEditorWindow::NotifyEvent);
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification, paramList->GetClearTaskFunctor());

				JAnimationParameterListCreationImpl* impl = paramList->creationImpl.get();
				impl->creation.RequestCreateObject(impl->dS, true, creationHint, Core::MakeGuid(), requestHint);
			};
			auto requestDestroyLam = [](JAnimationParameterList* paramList)
			{
				if (!paramList->aniCont.IsValid())
					return;

				std::vector<Core::JUserPtr<Core::JIdentifier>> objVec = paramList->GetSelectedObjectVec();
				if (objVec.size() == 0)
					return;

				JEditorCreationHint creationHint = JEditorCreationHint(paramList,
					true, false, false, true,
					Core::JTypeInstanceSearchHint(paramList->aniCont),
					Core::JTypeInstanceSearchHint(),
					&JEditorWindow::NotifyEvent);
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification, paramList->GetClearTaskFunctor());

				JAnimationParameterListCreationImpl* impl = paramList->creationImpl.get();
				impl->destructuion.RequestDestroyObject(impl->dS, true, creationHint, objVec, requestHint);
			};
			creationImpl = std::make_unique<JAnimationParameterListCreationImpl>(requestCreateLam, requestDestroyLam);

			auto canCreateParamLam = [](const size_t guid, const JEditorCreationHint& creationHint)
			{
				auto openSelectedPtr = Core::GetRawPtr(creationHint.openDataHint);
				if (openSelectedPtr == nullptr)
					return false;

				if (openSelectedPtr->GetTypeInfo().IsChildOf<JAnimationController>())
					return static_cast<JAnimationController*>(Core::GetRawPtr(creationHint.openDataHint))->CanCreateParameter();
				else
					return false;
			};
			auto createParamLam = [](const size_t guid, const JEditorCreationHint& creationHint)
			{
				static_cast<JAnimationController*>(Core::GetRawPtr(creationHint.openDataHint))->CreateFSMparameter(guid);
			};

			creationImpl->creation.GetCreationInterface()->RegisterCanCreationF(canCreateParamLam);
			creationImpl->creation.GetCreationInterface()->RegisterObjectCreationF(createParamLam);
		}
		void JAnimationParameterList::InitializeSettingImpl()
		{
			if (settingImpl != nullptr)
				return;

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

			settingImpl = std::make_unique<JAnimationParameterListSettingImpl>();
			using SetParameterTypeFunctor = JAnimationParameterListSettingImpl::SetParameterTypeFunctor;
			using SetParameterNameFunctor = JAnimationParameterListSettingImpl::SetParameterNameFunctor;
			using SetParameterBooleanValueFunctor = JAnimationParameterListSettingImpl::SetParameterBooleanValueFunctor;
			using SetParameterIntValueFunctor = JAnimationParameterListSettingImpl::SetParameterIntValueFunctor;
			using SetParameterFloatValueFunctor = JAnimationParameterListSettingImpl::SetParameterFloatValueFunctor;

			settingImpl->setParameterTypeF = std::make_unique<SetParameterTypeFunctor>(setParameterTypeLam);
			settingImpl->setParameterNameF = std::make_unique<SetParameterNameFunctor>(setParameterNameLam);
			settingImpl->setParameterBoolF = std::make_unique< SetParameterBooleanValueFunctor>(setParameterBoolLam);
			settingImpl->setParameterIntF = std::make_unique< SetParameterIntValueFunctor>(setParameterIntLam);
			settingImpl->setParameterFloatF = std::make_unique< SetParameterFloatValueFunctor>(setParameterFloatLam);
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

					const bool isSelect = IsSelectedObject( nowParameter->GetGuid()); 
					const JVector2<float> preCursorPos = ImGui::GetCursorScreenPos();			 
					
					if (isSelect)
						SetTreeNodeColor(GetSelectedColorFactor());
					if (JImGuiImpl::Selectable(name + "##Parameter_Selectable" + uniqueLabel, &isSelect, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap))
					{ 
						RequestPushSelectObject(Core::GetUserPtr(nowParameter));
						SetContentsClick(true);
					} 
					if (isSelect)
						SetTreeNodeColor(GetSelectedColorFactor() * -1);

					if (JImGuiImpl::IsMouseInRect(preCursorPos, ImGui::GetItemRectSize()))
					{
						SetHoveredObject(Core::GetUserPtr(nowParameter));
						if (ImGui::IsMouseClicked(1))
							SetContentsClick(true);
					} 
					ImGui::PushItemWidth(-FLT_MIN);
					if (isSelect)
					{
						//rename 추가 필요
					} 

					Core::J_FSM_PARAMETER_VALUE_TYPE valueType = nowParameter->GetParamType();
					JImGuiImpl::TableSetColumnIndex(1);
					ImGui::PushItemWidth(-FLT_MIN);

					JImGuiImpl::ComoboEnumSetT(name, uniqueLabel, valueType, *settingImpl->setParameterTypeF, aniCont, nowParameter->GetGuid());

					JImGuiImpl::TableSetColumnIndex(2);
					ImGui::PushItemWidth(-FLT_MIN);
					if (valueType == Core::J_FSM_PARAMETER_VALUE_TYPE::BOOL)
						JImGuiImpl::CheckBoxSetT(name, uniqueLabel, (bool)nowParameter->GetValue(), *settingImpl->setParameterBoolF, aniCont, nowParameter->GetGuid());
					else if (valueType == Core::J_FSM_PARAMETER_VALUE_TYPE::INT)
						JImGuiImpl::InputIntSetT(name, uniqueLabel, (int)nowParameter->GetValue(), *settingImpl->setParameterIntF, aniCont, nowParameter->GetGuid());
					else if (valueType == Core::J_FSM_PARAMETER_VALUE_TYPE::FLOAT)
						JImGuiImpl::InputFloatSetT(name, uniqueLabel, (float)nowParameter->GetValue(), *settingImpl->setParameterFloatF, aniCont, nowParameter->GetGuid());
				}
				JImGuiImpl::EndTable();
			}
			if (aniCont.IsValid())
				UpdatePopup(PopupSetting(parameterListPopup.get(), editorString.get()));
		}
		void JAnimationParameterList::DoSetClose()noexcept
		{
			aniCont.Clear();
			selectedDiagram.Clear(); 
		}
		void JAnimationParameterList::DoActivate()noexcept
		{
			JEditorWindow::DoActivate();
			std::vector<J_EDITOR_EVENT> enumVec
			{
				J_EDITOR_EVENT::MOUSE_CLICK, J_EDITOR_EVENT::PUSH_SELECT_OBJECT
			};
			RegisterEventListener(enumVec);
		}
		void JAnimationParameterList::DoDeActivate()noexcept
		{
			JEditorWindow::DoDeActivate();
			DeRegisterListener();
		}
		void JAnimationParameterList::OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* ev)
		{ 
			JEditorWindow::OnEvent(senderGuid, eventType, ev);
			if (senderGuid == GetGuid())
				return;

			if (eventType == J_EDITOR_EVENT::MOUSE_CLICK )
				parameterListPopup->SetOpen(false);
			else if (eventType == J_EDITOR_EVENT::PUSH_SELECT_OBJECT && ev->pageType == GetOwnerPageType())
			{
				JEditorPushSelectObjectEvStruct* evstruct = static_cast<JEditorPushSelectObjectEvStruct*>(ev);
				Core::JUserPtr< Core::JIdentifier> diagram = evstruct->GetFirstMatchedTypeObject(Core::JAnimationFSMdiagram::StaticTypeInfo());
				if (diagram.IsValid())
				{
					if (!selectedDiagram.IsValid() || selectedDiagram->GetGuid() != diagram->GetGuid())
						selectedDiagram.ConnnectChildUser(diagram);
				}
			}
		}
	}
}