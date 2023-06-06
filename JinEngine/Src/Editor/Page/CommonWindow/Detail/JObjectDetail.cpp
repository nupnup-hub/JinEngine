#include"JObjectDetail.h"   
#include"../../JEditorPageShareData.h"
#include"../../JEditorAttribute.h"
#include"../../../Helpers/JReflectionGuiWidgetHelper.h"  
#include"../../../Helpers/JEditorSearchBarHelper.h"  
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../../Core/FSM/JFSMinterface.h" 
#include"../../../../Object/GameObject/JGameObject.h"
#include"../../../../Object/Component/JComponent.h"
#include"../../../../Object/Component/JComponentCreator.h"
#include"../../../../Object/Resource/JResourceObject.h"
#include"../../../../Object/Directory/JDirectory.h" 
#include"../../../../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Editor
	{
		JObjectDetail::JObjectDetail(const std::string& name,
			std::unique_ptr<JEditorAttribute> attribute,
			const J_EDITOR_PAGE_TYPE pageType,
			const J_EDITOR_WINDOW_FLAG windowFlag)
			:JEditorWindow(name, std::move(attribute), pageType, windowFlag)
		{
			guiHelper = std::make_unique<JReflectionGuiWidgetHelper>(this);
			searchBarHelper = std::make_unique<JEditorSearchBarHelper>(false);
		}
		JObjectDetail::~JObjectDetail() {}
		J_EDITOR_WINDOW_TYPE JObjectDetail::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::OBJECT_DETAIL;
		}
		void JObjectDetail::UpdateWindow()
		{
			EnterWindow(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
			UpdateDocking();
			if (IsActivated())
			{
				UpdateMouseClick();
				BuildObjectDetail();
			}
			CloseWindow();
		}
		void JObjectDetail::BuildObjectDetail()
		{ 
			if (!selected.IsValid())
				return;

			Core::JTypeInfo& typeInfo = selected->GetTypeInfo();
			if (typeInfo.IsChildOf<JObject>())
			{
				switch (static_cast<JObject*>(selected.Get())->GetObjectType())
				{
				case J_OBJECT_TYPE::GAME_OBJECT:
					GameObjectDetailOnScreen(Core::GetUserPtr<JGameObject>(selected.Get()));
					break;
				case J_OBJECT_TYPE::RESOURCE_OBJECT:
					ObjectOnScreen(selected);
					break;
				case J_OBJECT_TYPE::DIRECTORY_OBJECT:
					ObjectOnScreen(selected);
					break;
				default:
					break;
				}
			}
			else if (typeInfo.IsChildOf<Core::JFSMinterface>())
				ObjectOnScreen(selected);
		}
		void JObjectDetail::GameObjectDetailOnScreen(JUserPtr<JGameObject> gObj)
		{
			auto compVec = gObj->GetAllComponent();
			ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnArrow |
				ImGuiTreeNodeFlags_SpanAvailWidth |
				ImGuiTreeNodeFlags_Framed |
				ImGuiTreeNodeFlags_DefaultOpen;

			for (const auto& comp : compVec)
			{
				ImGui::BeginGroup();
				ImGui::Separator();
				if (JImGuiImpl::TreeNodeEx(JCUtil::WstrToU8Str(Core::ErasePrefixJW(comp->GetName()) + L"##TreeNode" + gObj->GetName()), baseFlags))
				{
					JImGuiImpl::TreePop();
					guiHelper->UpdateGuiWidget(comp, &comp->GetTypeInfo());
				}
				ImGui::EndGroup();
			}

			if (JImGuiImpl::Button("AddComponent"))
				ImGui::OpenPopup("##AddComponentPopup");

			if (ImGui::BeginPopup("##AddComponentPopup"))
			{
				//ImGui::BeginGroup();
				JImGuiImpl::Text("Search");
				ImGui::SameLine();
				searchBarHelper->UpdateSearchBar();

				std::vector<Core::JTypeInfo*> derivedTypeInfo = _JReflectionInfo::Instance().GetDerivedTypeInfo(JComponent::StaticTypeInfo());
				for (const auto& compType : derivedTypeInfo)
				{
					if (!searchBarHelper->CanSrcNameOnScreen(compType->NameWithOutModifier()))
						continue;

					if (JImGuiImpl::Selectable(compType->NameWithOutModifier()))
					{
						JCCI::CreateComponent(*compType, gObj);
						SetModifiedBit(gObj, true); 
					}
				}

				//bool clickAnyMouse = ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1) || ImGui::IsMouseClicked(2);
				//if (clickAnyMouse && !JImGuiImpl::IsMouseInRect(ImGui::GetWindowPos(), ImGui::GetWindowSize()))
				//	isPressAddGameObject = false;
				//ImGui::EndGroup();
				ImGui::EndPopup();
			}
		}
		void JObjectDetail::ObjectOnScreen(JUserPtr<Core::JIdentifier> fObj)
		{
			ImGui::BeginGroup();
			ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnArrow |
				ImGuiTreeNodeFlags_SpanAvailWidth |
				ImGuiTreeNodeFlags_Framed |
				ImGuiTreeNodeFlags_DefaultOpen;

			ImGui::Separator();
			if (JImGuiImpl::TreeNodeEx(Core::ErasePrefixJ(fObj->GetTypeInfo().Name()) + JCUtil::WstrToU8Str(L"##TreeNode" + fObj->GetName()), baseFlags))
			{
				JImGuiImpl::TreePop();
				guiHelper->UpdateGuiWidget(fObj, &fObj->GetTypeInfo());
			}
			ImGui::EndGroup();
		}
		void JObjectDetail::DoActivate()noexcept
		{
			JEditorWindow::DoActivate();
			std::vector<J_EDITOR_EVENT> listenEvTypeVec{ J_EDITOR_EVENT::PUSH_SELECT_OBJECT, J_EDITOR_EVENT::POP_SELECT_OBJECT };
			RegisterEventListener(listenEvTypeVec);
		}
		void JObjectDetail::DoDeActivate()noexcept
		{
			JEditorWindow::DoDeActivate();
			guiHelper->Clear();
			searchBarHelper->ClearInputBuffer(); 
			DeRegisterListener();
		}
		void JObjectDetail::OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)
		{ 
			JEditorWindow::OnEvent(senderGuid, eventType, eventStruct);
			if (senderGuid == GetGuid())
				return;
		
			if (eventType == J_EDITOR_EVENT::PUSH_SELECT_OBJECT && eventStruct->pageType == GetOwnerPageType())
			{
				JEditorPushSelectObjectEvStruct* evstruct = static_cast<JEditorPushSelectObjectEvStruct*>(eventStruct);
				auto newSelected = evstruct->GetFirstMatchedTypeObject(Core::JIdentifier::StaticTypeInfo());				
				const bool isSame = newSelected.IsValid() && selected.IsValid() && newSelected->GetGuid() == selected->GetGuid();
				if (!isSame)
				{
					selected.ConnnectChild(std::move(newSelected));
					guiHelper->Clear();
					searchBarHelper->ClearInputBuffer(); 
				}
			}
			else if (eventType == J_EDITOR_EVENT::POP_SELECT_OBJECT && eventStruct->pageType == GetOwnerPageType())
			{
				if (!selected.IsValid())
					return;

				JEditorPopSelectObjectEvStruct* evstruct = static_cast<JEditorPopSelectObjectEvStruct*>(eventStruct);
				if (evstruct->IsPopTarget(selected->GetGuid()))
				{
					selected.Clear();
					guiHelper->Clear();
					searchBarHelper->ClearInputBuffer(); 
				}
			}
		}
	}
}