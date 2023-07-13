#include"JAnimationDiagramList.h"
#include"../../JEditorAttribute.h"
#include"../../JEditorPageShareData.h" 
#include"../../../Interface/JEditorObjectCreationInterface.h"
#include"../../../Event/JEditorEvent.h"
#include"../../../Popup/JEditorPopupMenu.h"
#include"../../../Popup/JEditorPopupNode.h"
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h" 
#include"../../../String/JEditorStringMap.h"
#include"../../../Helpers/JEditorInputBuffHelper.h"
#include"../../../EditTool/JEditorViewStructure.h" 
#include"../../../../Core/Reflection/JTypeTemplate.h" 
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
		 
		DEFAULT_CD_REQUESTOR(JAnimationDiagramListCreationImpl, JAnimationDiagramList)
 
		JAnimationDiagramList::JAnimationDiagramList(const std::string& name,
			std::unique_ptr<JEditorAttribute> attribute,
			const J_EDITOR_PAGE_TYPE ownerPageType,
			const J_EDITOR_WINDOW_FLAG windowFlag)
			:JEditorWindow(name, std::move(attribute), ownerPageType, windowFlag)
		{
			editorString = std::make_unique<JEditorStringMap>();
			InitializeCreationImpl();
			//Diagram List Popup
			std::unique_ptr<JEditorPopupNode> diagramListRootNode =
				std::make_unique<JEditorPopupNode>("Animation Controller Editor Diagram List Popup Root", J_EDITOR_POPUP_NODE_TYPE::ROOT, nullptr);

			std::unique_ptr<JEditorPopupNode> createNewDiagramNode =
				std::make_unique<JEditorPopupNode>("Create New Diagram", J_EDITOR_POPUP_NODE_TYPE::LEAF, diagramListRootNode.get());
			editorString->AddString(createNewDiagramNode->GetNodeId(), { "Create New Diagram" , u8"애니메이션 다이어그램 생성" });

			std::unique_ptr<JEditorPopupNode> destroyDigamraNode =
				std::make_unique<JEditorPopupNode>("Destroy Diagram", J_EDITOR_POPUP_NODE_TYPE::LEAF, diagramListRootNode.get());
			editorString->AddString(destroyDigamraNode->GetNodeId(), { "Destroy Diagram" , u8"애니메이션 다이어그램 삭제" });

			using RequestEvF = JAnimationDiagramListCreationImpl::RequestEvF;
			createNewDiagramNode->RegisterSelectBind(std::make_unique< RequestEvF::CompletelyBind>(*creationImpl->reqCreateStateEvF, this));
			destroyDigamraNode->RegisterSelectBind(std::make_unique< RequestEvF::CompletelyBind>(*creationImpl->reqDestroyEvF, this));
			destroyDigamraNode->RegisterEnableBind(std::make_unique<JEditorPopupNode::EnableF::CompletelyBind>(*GetPassSelectedAboveOneFunctor(), this));
			 
			diagramListPopup = std::make_unique<JEditorPopupMenu>(Constants::DiagramListName(GetName()), std::move(diagramListRootNode));
			diagramListPopup->AddPopupNode(std::move(createNewDiagramNode));
			diagramListPopup->AddPopupNode(std::move(destroyDigamraNode));
		}
		JAnimationDiagramList::~JAnimationDiagramList()
		{
			creationImpl.reset();
		}
		void JAnimationDiagramList::InitializeCreationImpl()
		{
			if (creationImpl != nullptr)
				return;

			auto requestCreateLam = [](JAnimationDiagramList* diagramList)
			{
				if (!diagramList->aniCont.IsValid())
					return;

				JEditorCreationHint creationHint = JEditorCreationHint(diagramList,
					true, false, false, true,
					Core::JTypeInstanceSearchHint(diagramList->aniCont),
					Core::JTypeInstanceSearchHint(),
					&JEditorWindow::NotifyEvent);
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification, diagramList->GetClearTaskFunctor());

				JAnimationDiagramListCreationImpl* impl = diagramList->creationImpl.get();
				impl->creation.RequestCreateObject(impl->dS, true, creationHint, Core::MakeGuid(), requestHint);
			};
			auto requestDestroyLam = [](JAnimationDiagramList* diagramList)
			{
				if (!diagramList->aniCont.IsValid())
					return;

				std::vector<JUserPtr<Core::JIdentifier>> objVec = diagramList->GetSelectedObjectVec();
				if (objVec.size() == 0)
					return;

				JEditorCreationHint creationHint = JEditorCreationHint(diagramList,
					true, false, false, true,
					Core::JTypeInstanceSearchHint(diagramList->aniCont),
					Core::JTypeInstanceSearchHint(),
					&JEditorWindow::NotifyEvent);
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification, diagramList->GetClearTaskFunctor());

				JAnimationDiagramListCreationImpl* impl = diagramList->creationImpl.get();
				impl->destructuion.RequestDestroyObject(impl->dS, true, creationHint, objVec, requestHint);
			};
			creationImpl = std::make_unique<JAnimationDiagramListCreationImpl>(requestCreateLam, requestDestroyLam);

			auto canCreateDiagramLam = [](const size_t guid, const JEditorCreationHint& creationHint)
			{
				auto openSelectedPtr = Core::GetRawPtr(creationHint.openDataHint);
				if (openSelectedPtr == nullptr)
					return false;

				if (openSelectedPtr->GetTypeInfo().IsChildOf<JAnimationController>())
					return static_cast<JAnimationController*>(Core::GetRawPtr(creationHint.openDataHint))->CanCreateDiagram();
				else
					return false;
			};
			auto createDiagramLam = [](const size_t guid, const JEditorCreationHint& creationHint)
			{
				static_cast<JAnimationController*>(Core::GetRawPtr(creationHint.openDataHint))->CreateFSMdiagram(guid);
			};
			 
			creationImpl->creation.GetCreationInterface()->RegisterCanCreationF(canCreateDiagramLam);
			creationImpl->creation.GetCreationInterface()->RegisterObjectCreationF(createDiagramLam);
		}
		J_EDITOR_WINDOW_TYPE JAnimationDiagramList::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::ANIMATION_DIAGRAM_LIST;
		}
		void JAnimationDiagramList::Initialize(JUserPtr<JAnimationController> newAniCont)noexcept
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

				const std::vector<JUserPtr<Core::JAnimationFSMdiagram>>& diagramVec = aniCont->GetDiagramVec();
				const uint diagramCount = (uint)diagramVec.size();
				for (uint i = 0; i < diagramCount; ++i)
				{
					JImGuiImpl::TableNextRow();
					JImGuiImpl::TableSetColumnIndex(0);

					const bool isSelect = IsSelectedObject(diagramVec[i]->GetGuid()); 
					const JVector2<float> preCursorPos = ImGui::GetCursorScreenPos();
					if (isSelect)
						SetTreeNodeColor(GetSelectedColorFactor());
					 
					if (JImGuiImpl::Selectable(JCUtil::WstrToU8Str(diagramVec[i]->GetName()), &isSelect))
					{ 
						RequestPushSelectObject(diagramVec[i]);
						SetContentsClick(true);
					}
					if (isSelect)
						SetTreeNodeColor(GetSelectedColorFactor() * -1);

					if (JImGuiImpl::IsMouseInRect(preCursorPos, ImGui::GetItemRectSize()))
					{
						SetHoveredObject(diagramVec[i]);
						if (ImGui::IsMouseClicked(1))
							SetContentsClick(true);
					}
			 
				}
				JImGuiImpl::EndTable();
			}
			if (aniCont.IsValid())
				UpdatePopup(PopupSetting(diagramListPopup.get(), editorString.get())); 
		}
		void JAnimationDiagramList::DoActivate()noexcept
		{
			JEditorWindow::DoActivate();
			std::vector<J_EDITOR_EVENT> enumVec
			{
				J_EDITOR_EVENT::MOUSE_CLICK
			};
			RegisterEventListener(enumVec);
		}
		void JAnimationDiagramList::DoDeActivate()noexcept
		{
			JEditorWindow::DoDeActivate();
			DeRegisterListener();
		}
		void JAnimationDiagramList::DoSetClose()noexcept
		{
			aniCont.Clear(); 
		}
		void JAnimationDiagramList::OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* ev)
		{
			JEditorWindow::OnEvent(senderGuid, eventType, ev);

			if (senderGuid == GetGuid())
				return; 

			if (eventType == J_EDITOR_EVENT::MOUSE_CLICK)
				diagramListPopup->SetOpen(false);
		}
	}
}