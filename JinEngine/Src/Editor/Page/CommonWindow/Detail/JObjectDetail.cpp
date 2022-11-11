#include"JObjectDetail.h"   
#include"../../JEditorPageShareData.h"
#include"../../JEditorAttribute.h"
#include"../../../Utility/JReflectionGuiWidgetHelper.h"  
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../../Object/GameObject/JGameObject.h"
#include"../../../../Object/Resource/JResourceObject.h"
#include"../../../../Object/Directory/JDirectory.h" 
#include"../../../../Utility/JCommonUtility.h"
 
namespace JinEngine
{
	namespace Editor
	{
		JObjectDetail::JObjectDetail(const std::string& name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE pageType)
			:JEditorWindow(name, std::move(attribute), pageType)
		{
			guiHelper = std::make_unique< JReflectionGuiWidgetHelper>();
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
			if (!preSelected.IsValid())
				return;

			auto nowSelected = preSelected;
			switch (preSelected->GetObjectType())
			{
			case J_OBJECT_TYPE::GAME_OBJECT:
				GameObjectDetailOnScreen(Core::JUserPtr<JGameObject>::ConvertChildType(std::move(nowSelected)));
				break;
			case J_OBJECT_TYPE::RESOURCE_OBJECT:
				ResourceObjectDetailOnScreen(Core::JUserPtr<JResourceObject>::ConvertChildType(std::move(nowSelected)));
				break;
			case J_OBJECT_TYPE::DIRECTORY_OBJECT:
				DirectoryObjectDetailOnScreen(Core::JUserPtr<JDirectory>::ConvertChildType(std::move(nowSelected)));
				break;
			default:
				break;
			}
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
				if (JImGuiImpl::TreeNodeEx(JCUtil::WstrToU8Str(Core::ErasePrefixJW(comp->GetName()) + L"##TreeNode" + gObj->GetName()), baseFlags))
				{
					JImGuiImpl::TreePop();
					ImGui::Separator();
					const auto paramVec = comp->GetTypeInfo().GetPropertyVec();
					for (const auto& param : paramVec)
						PropertyOnScreen(comp, param);

					const auto methodVec = comp->GetTypeInfo().GetMethodVec();
					for (const auto& method : methodVec)
						MethodOnScreen(comp, method);
				}
				ImGui::EndGroup();
			}
		}
		void JObjectDetail::ResourceObjectDetailOnScreen(Core::JUserPtr<JResourceObject> rObj)
		{
			ImGui::BeginGroup();
			ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnArrow |
				ImGuiTreeNodeFlags_SpanAvailWidth |
				ImGuiTreeNodeFlags_Framed |
				ImGuiTreeNodeFlags_DefaultOpen;
			if (JImGuiImpl::TreeNodeEx(Core::ErasePrefixJ(rObj->GetTypeInfo().Name()) + JCUtil::WstrToU8Str(L"##TreeNode" + rObj->GetName()), baseFlags))
			{
				JImGuiImpl::TreePop();
				ImGui::Separator();
				const auto paramVec = rObj->GetTypeInfo().GetPropertyVec();
				for (const auto& param : paramVec) 
					PropertyOnScreen(rObj.Get(), param);

				const auto methodVec = rObj->GetTypeInfo().GetMethodVec();
				for (const auto& method : methodVec)
					MethodOnScreen(rObj.Get(), method);
			}
			ImGui::EndGroup();
		}
		void JObjectDetail::DirectoryObjectDetailOnScreen(Core::JUserPtr<JDirectory> dObj)
		{
			ImGui::BeginGroup();
			ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnArrow |
				ImGuiTreeNodeFlags_SpanAvailWidth |
				ImGuiTreeNodeFlags_Framed |
				ImGuiTreeNodeFlags_DefaultOpen;

			if (JImGuiImpl::TreeNodeEx(Core::ErasePrefixJ(dObj->GetTypeInfo().Name()) + JCUtil::WstrToU8Str(L"##TreeNode" + dObj->GetName()), baseFlags))
			{
				JImGuiImpl::TreePop();
				ImGui::Separator();
				const auto paramVec = dObj->GetTypeInfo().GetPropertyVec();
				for (const auto& param : paramVec)
					PropertyOnScreen(dObj.Get(), param);

				const auto methodVec = dObj->GetTypeInfo().GetMethodVec();
				for (const auto& method : methodVec)
					MethodOnScreen(dObj.Get(), method);
			}
			ImGui::EndGroup();
		}
		void JObjectDetail::PropertyOnScreen(JObject* obj, Core::JPropertyInfo* pInfo)
		{
			auto optInfo = pInfo->GetOptionInfo();
			if (optInfo->HasWidgetInfo() && optInfo->GetWidgetInfo()->GetSupportWidgetType() != Core::Constant::NotSupportGuiWidget)
				guiHelper->UpdatePropertyGuiWidget(obj, pInfo);
		}
		void JObjectDetail::MethodOnScreen(JObject* obj, Core::JMethodInfo* mInfo)
		{
			auto optInfo = mInfo->GetOptionInfo();
			if (optInfo->HasWidgetInfo() && optInfo->GetWidgetInfo()->GetSupportWidgetType() != Core::Constant::NotSupportGuiWidget)
				guiHelper->UpdateMethodGuiWidget(obj, mInfo);
		}
		void JObjectDetail::DoActivate()noexcept
		{
			JEditorWindow::DoActivate(); 
			RegisterEventListener(J_EDITOR_EVENT::SELECT_OBJECT);
		}
		void JObjectDetail::DoDeActivate()noexcept
		{
			JEditorWindow::DoDeActivate();
			guiHelper->Clear();
			DeRegisterListener();
		}
		void JObjectDetail::OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)
		{ 
			if (senderGuid == GetGuid())
				return;
			if (eventType == J_EDITOR_EVENT::SELECT_OBJECT)
			{
				auto nowSelected = static_cast<JEditorSelectObjectEvStruct*>(eventStruct)->selectObj;
				if (!preSelected.IsValid() || nowSelected->GetGuid() != preSelected->GetGuid())
				{
					guiHelper->Clear();
					preSelected = nowSelected;
				}

			}
		}
	}
}