#include"JAnimationParameterList.h"
#include"../../JEditorAttribute.h"
#include"../../JEditorPageShareData.h" 
#include"../../../Event/JEditorEvent.h"
#include"../../../Interface/JEditorObjectCreationInterface.h"
#include"../../../Popup/JEditorPopupMenu.h"
#include"../../../Popup/JEditorPopupNode.h"
#include"../../../Gui/JGui.h" 
#include"../../../String/JEditorStringMap.h"
#include"../../../EditTool/JEditorInputBuffHelper.h"
#include"../../../EditTool/JEditorViewStructure.h" 
#include"../../../../Core/Reflection/JTypeTemplate.h" 
#include"../../../../Core/FSM/JFSMparameter.h" 
#include"../../../../Object/Resource/AnimationController/JAnimationController.h"   
#include"../../../../Object/Resource/AnimationController/FSM/JAnimationStateType.h" 
#include"../../../../Object/Resource/AnimationController/FSM/JAnimationFSMdiagram.h" 

namespace JinEngine
{
	namespace Editor
	{
		namespace Private
		{
			static std::string ParameterListName(const std::string& uniqueLabel)noexcept
			{
				return "ParameterList##" + uniqueLabel;
			}
		}

		/*
	class JAnimationParameterListCreationFunctor
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
	JAnimationParameterListCreationFunctor(RegisterEvF::Ptr regCreateStateEvPtr, RegisterEvF::Ptr regDestroyEvPtr)
	{
		regCreateStateEvF = std::make_unique<RegisterEvF::Functor>(regCreateStateEvPtr);
		regDestroyEvF = std::make_unique<RegisterEvF::Functor>(regDestroyEvPtr);
	}
	~JAnimationParameterListCreationFunctor()
	{
		dS.Clear();
	}
};
*/
		DEFAULT_CD_REQUESTOR(JAnimationParameterListCreationFunctor, JAnimationParameterList)
		using AniContUserPtr = JUserPtr<JAnimationController>;	 
		class JAnimationParameterListSettingFunctor
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

			using RequestEvF = JAnimationParameterListCreationFunctor::RequestEvF;

			createNewParameterNode->RegisterSelectBind(std::make_unique<RequestEvF::CompletelyBind>(*creation->reqCreateStateEvF, this));
			destroyParameterNode->RegisterSelectBind(std::make_unique<RequestEvF::CompletelyBind>(*creation->reqDestroyEvF, this));
			destroyParameterNode->RegisterEnableBind(std::make_unique<PassPopupConditionF::CompletelyBind>(*GetPassSelectedAboveOneFunctor(), this));

			parameterListPopup = std::make_unique<JEditorPopupMenu>(Private::ParameterListName(GetName()), std::move(parameterListRootNode));
			parameterListPopup->AddPopupNode(std::move(createNewParameterNode));
			parameterListPopup->AddPopupNode(std::move(destroyParameterNode));
		}
		JAnimationParameterList::~JAnimationParameterList()
		{
			creation.reset();
			setting.reset();
		}
		void JAnimationParameterList::InitializeCreationImpl()
		{
			if (creation != nullptr)
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
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification);

				JAnimationParameterListCreationFunctor* impl = paramList->creation.get();
				impl->creation.RequestCreateObject(impl->dS, true, creationHint, Core::MakeGuid(), requestHint);
			};
			auto requestDestroyLam = [](JAnimationParameterList* paramList)
			{
				if (!paramList->aniCont.IsValid())
					return;

				std::vector<JUserPtr<Core::JIdentifier>> objVec = paramList->GetSelectedObjectVec();
				if (objVec.size() == 0)
					return;

				JEditorCreationHint creationHint = JEditorCreationHint(paramList,
					true, false, false, true,
					Core::JTypeInstanceSearchHint(paramList->aniCont),
					Core::JTypeInstanceSearchHint(),
					&JEditorWindow::NotifyEvent);
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification);

				JAnimationParameterListCreationFunctor* impl = paramList->creation.get();
				impl->destructuion.RequestDestroyObject(impl->dS, true, creationHint, objVec, requestHint);
			};
			creation = std::make_unique<JAnimationParameterListCreationFunctor>(requestCreateLam, requestDestroyLam);

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

			creation->creation.GetCreationInterface()->RegisterCanCreationF(canCreateParamLam);
			creation->creation.GetCreationInterface()->RegisterObjectCreationF(createParamLam);
		}
		void JAnimationParameterList::InitializeSettingImpl()
		{
			if (setting != nullptr)
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

			setting = std::make_unique<JAnimationParameterListSettingFunctor>();
			using SetParameterTypeFunctor = JAnimationParameterListSettingFunctor::SetParameterTypeFunctor;
			using SetParameterNameFunctor = JAnimationParameterListSettingFunctor::SetParameterNameFunctor;
			using SetParameterBooleanValueFunctor = JAnimationParameterListSettingFunctor::SetParameterBooleanValueFunctor;
			using SetParameterIntValueFunctor = JAnimationParameterListSettingFunctor::SetParameterIntValueFunctor;
			using SetParameterFloatValueFunctor = JAnimationParameterListSettingFunctor::SetParameterFloatValueFunctor;

			setting->setParameterTypeF = std::make_unique<SetParameterTypeFunctor>(setParameterTypeLam);
			setting->setParameterNameF = std::make_unique<SetParameterNameFunctor>(setParameterNameLam);
			setting->setParameterBoolF = std::make_unique< SetParameterBooleanValueFunctor>(setParameterBoolLam);
			setting->setParameterIntF = std::make_unique< SetParameterIntValueFunctor>(setParameterIntLam);
			setting->setParameterFloatF = std::make_unique< SetParameterFloatValueFunctor>(setParameterFloatLam);
		}
		J_EDITOR_WINDOW_TYPE JAnimationParameterList::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::ANIMATION_CONDITION_LIST;
		}
		void JAnimationParameterList::SetAnimationController(const JUserPtr<JAnimationController>& newAniCont)
		{
			aniCont = newAniCont;
		} 
		void JAnimationParameterList::UpdateWindow()
		{
			EnterWindow(J_GUI_WINDOW_FLAG_NO_SCROLL_BAR | J_GUI_WINDOW_FLAG_NO_COLLAPSE);
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
			J_GUI_TABLE_FLAG_ flag = J_GUI_TABLE_FLAG_BORDER_V |
				J_GUI_TABLE_FLAG_BORDER_OUTHER_H |
				J_GUI_TABLE_FLAG_ROW_BG |  
				J_GUI_TABLE_FLAG_CONTEXT_MENU_IN_BODY |
				J_GUI_TABLE_FLAG_RESIZABLE;

			if (aniCont.IsValid() && JGui::BeginTable("##ParameterList_Table" + GetName(), 3, flag))
			{
				//JGui::TableSetupColumn("", ImGuiTableColumnFlags_DefaultHide, 0.005f);
				JGui::TableSetupColumn("Name", J_GUI_TABLE_COLUMN_FLAG_WIDTH_STRETCH, 0.445f);
				JGui::TableSetupColumn("Type", J_GUI_TABLE_COLUMN_FLAG_WIDTH_STRETCH, 0.275f);
				JGui::TableSetupColumn("Value", J_GUI_TABLE_COLUMN_FLAG_WIDTH_STRETCH, 0.275f);
				JGui::TableHeadersRow();
				const uint parameterCount = aniCont->GetParameterCount();
				for (uint i = 0; i < parameterCount; ++i)
				{
					JUserPtr<Core::JFSMparameter> nowParameter = aniCont->GetParameterByIndex(i);
					std::string name = JCUtil::WstrToU8Str(nowParameter->GetName());
					std::string uniqueLabel = std::to_string(nowParameter->GetGuid()) + GetName();

					JGui::TableNextRow();
					JGui::TableSetColumnIndex(0);

					const bool isSelect = IsSelectedObject( nowParameter->GetGuid()); 
					const JVector2<float> preCursorPos = JGui::GetCursorScreenPos();
					
					JGui::PushTreeNodeColorSet(IsFocus(), true, isSelect);
					if (JGui::Selectable(name + "##Parameter_Selectable" + uniqueLabel, &isSelect, J_GUI_SELECTABLE_FLAG_SPAN_ALL_COLUMNS | J_GUI_SELECTABLE_FLAG_ALLOW_ITEM_OVERLAP))
					{ 
						RequestPushSelectObject(nowParameter);
						SetContentsClick(true);
					} 
					JGui::PopTreeNodeColorSet(true, isSelect);

					if (JGui::IsMouseInRect(preCursorPos, JGui::GetLastItemRectSize()))
					{
						SetHoveredObject(nowParameter);
						if (JGui::IsMouseClicked(Core::J_MOUSE_BUTTON::RIGHT))
							SetContentsClick(true);
					} 
					JGui::PushItemWidth(-FLT_MIN);
					if (isSelect)
					{
						//rename 추가 필요
					} 

					Core::J_FSM_PARAMETER_VALUE_TYPE valueType = nowParameter->GetParamType();
					JGui::TableSetColumnIndex(1);
					JGui::PushItemWidth(-FLT_MIN);

					JGui::ComoboEnumSetT(name, uniqueLabel, valueType, *setting->setParameterTypeF, aniCont, nowParameter->GetGuid());

					JGui::TableSetColumnIndex(2);
					JGui::PushItemWidth(-FLT_MIN);
					if (valueType == Core::J_FSM_PARAMETER_VALUE_TYPE::BOOL)
						JGui::CheckBoxSetT(name, uniqueLabel, (bool)nowParameter->GetValue(), *setting->setParameterBoolF, aniCont, nowParameter->GetGuid());
					else if (valueType == Core::J_FSM_PARAMETER_VALUE_TYPE::INT)
						JGui::InputIntSetT(name, uniqueLabel, (int)nowParameter->GetValue(), *setting->setParameterIntF, aniCont, nowParameter->GetGuid());
					else if (valueType == Core::J_FSM_PARAMETER_VALUE_TYPE::FLOAT)
						JGui::InputFloatSetT(name, uniqueLabel, (float)nowParameter->GetValue(), *setting->setParameterFloatF, aniCont, nowParameter->GetGuid());
				}
				JGui::EndTable();
			}
			if (aniCont.IsValid())
				UpdatePopup(PopupSetting(parameterListPopup.get(), editorString.get())); 
		}
		void JAnimationParameterList::DoSetClose()noexcept
		{
			aniCont.Clear();
			selectedDiagram.Clear(); 
			JEditorWindow::DoSetClose();
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
			DeRegisterListener();
			JEditorWindow::DoDeActivate();
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