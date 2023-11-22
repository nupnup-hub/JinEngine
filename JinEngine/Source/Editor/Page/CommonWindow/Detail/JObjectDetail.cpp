#include"JObjectDetail.h"   
#include"../../JEditorPageShareData.h"
#include"../../JEditorAttribute.h"
#include"../../../EditTool/JReflectionGuiWidgetHelper.h"  
#include"../../../EditTool/JEditorSearchBarHelper.h"  
#include"../../../Gui/JGui.h"
#include"../../../../Core/FSM/JFSMinterface.h" 
#include"../../../../Core/Utility/JCommonUtility.h"    
#include"../../../../Object/GameObject/JGameObject.h"
#include"../../../../Object/Component/JComponent.h"
#include"../../../../Object/Component/JComponentCreator.h"
#include"../../../../Object/Resource/JResourceObject.h"
#include"../../../../Object/Directory/JDirectory.h"  

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
			EnterWindow(J_GUI_WINDOW_FLAG_NO_SCROLL_BAR | J_GUI_WINDOW_FLAG_NO_COLLAPSE);
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
					GameObjectDetailOnScreen(Core::ConnectChildUserPtr<JGameObject>(selected));
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
			/*
			J_GUI_TREE_NODE_FLAG_DEFAULT_OPEN = 1 << 0,
			J_GUI_TREE_NODE_FLAG_EXTEND_HIT_BOX_WIDTH = 1 << 1,
			J_GUI_TREE_NODE_FLAG_FRAMED = 1 << 2,
			J_GUI_TREE_NODE_FLAG_OPEN_ON_ARROW = 1 << 3,
			*/
			J_GUI_TREE_NODE_FLAG_ baseFlags = J_GUI_TREE_NODE_FLAG_OPEN_ON_ARROW |
				J_GUI_TREE_NODE_FLAG_EXTEND_HIT_BOX_WIDTH |
				J_GUI_TREE_NODE_FLAG_FRAMED |
				J_GUI_TREE_NODE_FLAG_DEFAULT_OPEN;

			auto compVec = gObj->GetAllComponent();
			for (const auto& comp : compVec)
			{ 
				//JGui::Separator(); 
				if (JGui::TreeNodeEx(JGui::CreateGuiLabel(Core::ErasePrefixJ(comp->GetTypeInfo().Name()), comp->GetGuid(), GetName() + "TreeNode"), baseFlags))
				{ 
					JGui::TreePop(); 
					guiHelper->BeginGuiWidget(comp);
					guiHelper->UpdateGuiWidget(comp, &comp->GetTypeInfo());
					guiHelper->EndGuiWidget();
				} 
			}

			if (JGui::Button("AddComponent"))
				JGui::OpenPopup("##AddComponentPopup");

			if (JGui::BeginPopup("##AddComponentPopup"))
			{
				//JGui::BeginGroup();
				JGui::Text("Search");
				JGui::SameLine();
				searchBarHelper->UpdateSearchBar();

				std::vector<Core::JTypeInfo*> derivedTypeInfo = _JReflectionInfo::Instance().GetDerivedTypeInfo(JComponent::StaticTypeInfo());
				for (const auto& compType : derivedTypeInfo)
				{
					if (compType->IsAbstractType())
						continue;

					if (!searchBarHelper->CanSrcNameOnScreen(compType->NameWithOutModifier()))
						continue;

					if (JGui::Selectable(compType->NameWithOutModifier()))
					{
						JCCI::CreateComponent(*compType, gObj);
						SetModifiedBit(gObj, true); 
					}
				}

				//bool clickAnyMouse = JGui::IsMouseClicked(0) || JGui::IsMouseClicked(1) || JGui::IsMouseClicked(2);
				//if (clickAnyMouse && !JGui::IsMouseInRect(JGui::GetWindowPos(), JGui::GetWindowSize()))
				//	isPressAddGameObject = false;
				//JGui::EndGroup();
				JGui::EndPopup();
			}
		}
		void JObjectDetail::ObjectOnScreen(JUserPtr<Core::JIdentifier> fObj)
		{ 
			J_GUI_TREE_NODE_FLAG_ baseFlags = J_GUI_TREE_NODE_FLAG_OPEN_ON_ARROW |
				J_GUI_TREE_NODE_FLAG_EXTEND_HIT_BOX_WIDTH |
				J_GUI_TREE_NODE_FLAG_FRAMED |
				J_GUI_TREE_NODE_FLAG_DEFAULT_OPEN;
			JGui::Separator(); 
			if (JGui::TreeNodeEx(JGui::CreateGuiLabel(Core::ErasePrefixJ(fObj->GetTypeInfo().Name()), fObj->GetGuid(), GetName() + "TreeNode"), baseFlags))
			{
				JGui::TreePop(); 
				guiHelper->BeginGuiWidget(fObj);
				guiHelper->UpdateGuiWidget(fObj, &fObj->GetTypeInfo());
				guiHelper->EndGuiWidget();
			} 
		}
		void JObjectDetail::DoActivate()noexcept
		{
			JEditorWindow::DoActivate();
			std::vector<J_EDITOR_EVENT> listenEvTypeVec{ J_EDITOR_EVENT::PUSH_SELECT_OBJECT, J_EDITOR_EVENT::POP_SELECT_OBJECT };
			RegisterEventListener(listenEvTypeVec);
		}
		void JObjectDetail::DoDeActivate()noexcept
		{
			guiHelper->Clear();
			searchBarHelper->ClearInputBuffer(); 
			DeRegisterListener();
			JEditorWindow::DoDeActivate();
		}
		void JObjectDetail::OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStructure)
		{ 
			JEditorWindow::OnEvent(senderGuid, eventType, eventStructure);
			if (!eventStructure->CanExecuteOtherEv(senderGuid, GetGuid()))
				return;

			if (eventType == J_EDITOR_EVENT::PUSH_SELECT_OBJECT && eventStructure->pageType == GetOwnerPageType())
			{
				JEditorPushSelectObjectEvStruct* evstruct = static_cast<JEditorPushSelectObjectEvStruct*>(eventStructure);
				auto newSelected = evstruct->GetFirstMatchedTypeObject(Core::JIdentifier::StaticTypeInfo());				
				const bool isSame = newSelected.IsValid() && selected.IsValid() && newSelected->GetGuid() == selected->GetGuid();
				if (!isSame)
				{
					selected.ConnnectChild(std::move(newSelected));
					guiHelper->Clear();
					searchBarHelper->ClearInputBuffer(); 
				}
			}
			else if (eventType == J_EDITOR_EVENT::POP_SELECT_OBJECT && eventStructure->pageType == GetOwnerPageType())
			{
				if (!selected.IsValid())
					return;

				JEditorPopSelectObjectEvStruct* evstruct = static_cast<JEditorPopSelectObjectEvStruct*>(eventStructure);
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