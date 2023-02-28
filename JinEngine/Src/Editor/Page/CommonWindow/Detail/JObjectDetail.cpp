#include"JObjectDetail.h"   
#include"../../JEditorPageShareData.h"
#include"../../JEditorAttribute.h"
#include"../../../Helpers/JReflectionGuiWidgetHelper.h"  
#include"../../../Helpers/JEditorSearchBarHelper.h"  
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../../Core/FSM/JFSMInterface.h"
#include"../../../../Object/Component/JComponentFactory.h"
#include"../../../../Object/GameObject/JGameObject.h"
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
			guiHelper = std::make_unique<JReflectionGuiWidgetHelper>(pageType);
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
			auto nowSelected = JEditorPageShareData::GetSelectedObj(GetOwnerPageType());
			if (!nowSelected.IsValid())
			{
				guiHelper->Clear();
				searchBarHelper->ClearInputBuffer();
				isPressAddGameObject = false;
				return;
			}
			 
			Core::JTypeInfo& typeInfo = nowSelected->GetTypeInfo();
			if (typeInfo.IsChildOf<JObject>())
			{ 
				switch (static_cast<JObject*>(nowSelected.Get())->GetObjectType())
				{
				case J_OBJECT_TYPE::GAME_OBJECT:
					GameObjectDetailOnScreen(Core::JUserPtr<JGameObject>::ConvertChildUser(std::move(nowSelected)));
					break;
				case J_OBJECT_TYPE::RESOURCE_OBJECT:
					ObjectOnScreen(nowSelected);
					break;
				case J_OBJECT_TYPE::DIRECTORY_OBJECT:
					ObjectOnScreen(nowSelected);
					break;
				default:
					break;
				}
			}
			else if (typeInfo.IsChildOf<Core::JFSMInterface>())
				ObjectOnScreen(nowSelected);
		}
		void JObjectDetail::GameObjectDetailOnScreen(Core::JUserPtr<JGameObject> gObj)
		{
			auto compVec = gObj->GetComponentVec();
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

				std::vector<Core::JTypeInfo*> derivedTypeInfo = Core::JReflectionInfo::Instance().GetDerivedTypeInfo(JComponent::StaticTypeInfo());
				for (const auto& compType : derivedTypeInfo)
				{
					if (!searchBarHelper->CanSrcNameOnScreen(compType->NameWithOutPrefix()))
						continue;

					if (JImGuiImpl::Selectable(compType->NameWithOutPrefix()))
					{
						JCFIB::CreateByName(compType->Name(), *gObj.Get());
						SetModifiedBit(gObj, true);
						isPressAddGameObject = false;
					}
				}

				bool clickAnyMouse = ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1) || ImGui::IsMouseClicked(2);
				if (clickAnyMouse && !JImGuiImpl::IsMouseInRect(ImGui::GetWindowPos(), ImGui::GetWindowSize()))
					isPressAddGameObject = false;
				//ImGui::EndGroup();
				ImGui::EndPopup();
			}
		}
		void JObjectDetail::ObjectOnScreen(Core::JUserPtr<Core::JIdentifier> fObj)
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
				guiHelper->UpdateGuiWidget(fObj.Get(), &fObj->GetTypeInfo());
			}
			ImGui::EndGroup();
		}
		void JObjectDetail::DoActivate()noexcept
		{
			JEditorWindow::DoActivate(); 
			std::vector<J_EDITOR_EVENT> listenEvTypeVec{ J_EDITOR_EVENT::SELECT_OBJECT, J_EDITOR_EVENT::DESELECT_OBJECT };
			RegisterEventListener(listenEvTypeVec); 
		}
		void JObjectDetail::DoDeActivate()noexcept
		{
			JEditorWindow::DoDeActivate();
			guiHelper->Clear();
			searchBarHelper->ClearInputBuffer();
			isPressAddGameObject = false;
			DeRegisterListener();
		}
		void JObjectDetail::OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)
		{ 
			if (senderGuid == GetGuid())
				return;
		}
	}
}