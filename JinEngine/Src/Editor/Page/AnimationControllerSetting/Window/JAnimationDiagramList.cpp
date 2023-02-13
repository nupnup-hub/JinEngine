#include"JAnimationDiagramList.h"
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
#include"../../../../Core/FSM/AnimationFSM/JAnimationStateType.h"
#include"../../../../Core/FSM/AnimationFSM/JAnimationFSMdiagram.h" 
#include"../../../../Object/Resource/AnimationController/JAnimationController.h"   

namespace JinEngine
{
	namespace Editor
	{
		namespace Constants
		{
			static std::string DiagramListName(const std::string& uniqueLabel)noexcept
			{
				return "DiagramList##" + uniqueLabel;
			}
		}

		JAnimationDiagramList::JAnimationDiagramList(const std::string& name,
			std::unique_ptr<JEditorAttribute> attribute,
			const J_EDITOR_PAGE_TYPE ownerPageType,
			const J_EDITOR_WINDOW_FLAG windowFlag)
			:JEditorWindow(name, std::move(attribute), ownerPageType, windowFlag)
		{
			editorString = std::make_unique<JEditorStringMap>();

			//Diagram List Popup
			std::unique_ptr<JEditorPopupNode> diagramListRootNode =
				std::make_unique<JEditorPopupNode>("Animation Controller Editor Diagram List Popup Root", J_EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

			std::unique_ptr<JEditorPopupNode> createNewDiagramNode =
				std::make_unique<JEditorPopupNode>("Create New Diagram", J_EDITOR_POPUP_NODE_TYPE::LEAF, diagramListRootNode.get());
			editorString->AddString(createNewDiagramNode->GetNodeId(), { "Create New Diagram" , u8"애니메이션 다이어그램 생성" });

			std::unique_ptr<JEditorPopupNode> destroyDigamraNode =
				std::make_unique<JEditorPopupNode>("Destroy Diagram", J_EDITOR_POPUP_NODE_TYPE::LEAF, diagramListRootNode.get());
			editorString->AddString(destroyDigamraNode->GetNodeId(), { "Destroy Diagram" , u8"애니메이션 다이어그램 삭제" });

			regCreateDiagramEvF = std::make_unique<RegisterEvF::Functor>(&JAnimationDiagramList::RegisterCreateDiagramEv, this);
			regDestroyDiagramEvF = std::make_unique<RegisterEvF::Functor>(&JAnimationDiagramList::RegisterDestroyDiagramEv, this);
			createDiagramF = std::make_unique<DiagramCreationFunctor>(&JAnimationDiagramList::CreateDiagram, this);
			destroyDiagramF = std::make_unique<DiagramCreationFunctor>(&JAnimationDiagramList::DestroyDiagram, this);

			createNewDiagramNode->RegisterSelectBind(std::make_unique< RegisterEvF::CompletelyBind>(*regCreateDiagramEvF));
			destroyDigamraNode->RegisterSelectBind(std::make_unique< RegisterEvF::CompletelyBind>(*regDestroyDiagramEvF));

			diagramListPopup = std::make_unique<JEditorPopupMenu>(Constants::DiagramListName(GetName()), std::move(diagramListRootNode));
			diagramListPopup->AddPopupNode(std::move(createNewDiagramNode));
			diagramListPopup->AddPopupNode(std::move(destroyDigamraNode));
		}
		J_EDITOR_WINDOW_TYPE JAnimationDiagramList::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::ANIMATION_DIAGRAM_LIST;
		}
		void JAnimationDiagramList::Initialize(Core::JUserPtr<JAnimationController> newAniCont)noexcept
		{
			aniCont = newAniCont; 
		}
		void JAnimationDiagramList::UpdateWindow()
		{
			EnterWindow(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
			UpdateDocking();
			if (IsActivated())
			{
				UpdateMouseClick();
				BuildDiagramList();
			}
			CloseWindow();
		}
		void JAnimationDiagramList::BuildDiagramList()
		{ 
			ImGuiTableFlags flag = ImGuiTableFlags_Borders | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
			if (aniCont.IsValid() && JImGuiImpl::BeginTable("##DiagramListTable" + GetName(), 1, flag))
			{
				JImGuiImpl::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
				JImGuiImpl::TableHeadersRow();

				const std::vector<Core::JAnimationFSMdiagram*>& diagramVec = aniCont->GetDiagramVec();
				const uint diagramCount = (uint)diagramVec.size();
				for (uint i = 0; i < diagramCount; ++i)
				{
					JImGuiImpl::TableNextRow();
					JImGuiImpl::TableSetColumnIndex(0);

					const bool isSelect = selectedDiagram.IsValid() ? diagramVec[i]->GetGuid() == selectedDiagram->GetGuid() : false;
					if (JImGuiImpl::Selectable(JCUtil::WstrToU8Str(diagramVec[i]->GetName()), &isSelect))
					{
						if (!isSelect)
							RequestSelectObject(JEditorSelectObjectEvStruct(GetOwnerPageType(), Core::GetUserPtr(diagramVec[i])));
					}
				}
				JImGuiImpl::EndTable();
			}
			if (aniCont.IsValid())
			{
				if (diagramListPopup->IsOpen())
					diagramListPopup->ExecutePopup(editorString.get());
				diagramListPopup->Update();
			} 
		}
		void JAnimationDiagramList::RegisterCreateDiagramEv()
		{
			if (!aniCont.IsValid())
				return;

			using BindT = JAnimationDiagramList::DiagramCreationBind;
			size_t guid = Core::MakeGuid();
			auto cUptr = std::make_unique<BindT>(*createDiagramF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, std::move(guid));
			auto dUptr = std::make_unique<BindT>(*destroyDiagramF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, std::move(guid));

			using CreationTask = Core::JTransitionCreationTask<DataHandleStructure, BindT, BindT>;
			Core::JTransition::Execute(std::make_unique<CreationTask>("Create Diagram", std::move(cUptr), std::move(dUptr), fsmdata));
		}
		void JAnimationDiagramList::RegisterDestroyDiagramEv()
		{
			if (!aniCont.IsValid() || !selectedDiagram.IsValid() || aniCont->GetDiagramCount() < 2)
				return;

			using BindT = JAnimationDiagramList::DiagramCreationBind;
			auto cUptr = std::make_unique<BindT>(*createDiagramF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, selectedDiagram->GetGuid());
			auto dUptr = std::make_unique<BindT>(*destroyDiagramF, Core::empty, Core::empty, Core::JUserPtr{ aniCont }, selectedDiagram->GetGuid());

			using DestroyTask = Core::JTransitionCreationTask<DataHandleStructure, BindT, BindT>;
			Core::JTransition::Execute(std::make_unique<DestroyTask>("Destroy Diagram", std::move(dUptr), std::move(cUptr), fsmdata));
		}
		void JAnimationDiagramList::CreateDiagram(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t guid)
		{
			if (aniCont.IsValid())
			{
				Core::JOwnerPtr<Core::JIdentifier> owner = dS.Release(dH);
				if (owner.IsValid() && Core::Cast<Core::JAnimationFSMdiagram>(owner.Get()))
				{
					auto diagram = Core::JOwnerPtr<Core::JAnimationFSMdiagram>::ConvertChildUser(std::move(owner));
					auto ptr = diagram.Get();
					Core::JIdentifier::AddInstance(std::move(diagram));
					JEditorPageShareData::SetSelectObj(GetOwnerPageType(), Core::GetUserPtr(ptr));
				}
				else
					aniCont->CreateFSMDiagram(guid);
				SetModifiedBit(aniCont, true);
			}
		}
		void JAnimationDiagramList::DestroyDiagram(DataHandleStructure& dS, Core::JDataHandle& dH, AniContUserPtr aniCont, const size_t guid)
		{
			if (aniCont.IsValid())
			{
				Core::JOwnerPtr<Core::JIdentifier> owner = dS.Release(dH);
				if (owner.IsValid() && Core::Cast<Core::JAnimationFSMdiagram>(owner.Get()))
				{
					auto diagram = Core::JOwnerPtr<Core::JAnimationFSMdiagram>::ConvertChildUser(std::move(owner));
					auto ptr = diagram.Get();
					Core::JIdentifier::AddInstance(std::move(diagram));
					JEditorPageShareData::SetSelectObj(GetOwnerPageType(), Core::GetUserPtr(ptr));
				}
				else
					aniCont->CreateFSMDiagram(guid);
				SetModifiedBit(aniCont, true);
			}
		}
		void JAnimationDiagramList::DoSetClose()noexcept
		{
			aniCont.Clear();
			selectedDiagram.Clear(); 
		}
	}
}