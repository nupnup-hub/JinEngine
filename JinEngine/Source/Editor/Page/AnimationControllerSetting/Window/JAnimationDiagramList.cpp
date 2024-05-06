/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#include"JAnimationDiagramList.h"
#include"../../JEditorAttribute.h"
#include"../../JEditorPageShareData.h" 
#include"../../../Interface/JEditorObjectCreationInterface.h"
#include"../../../Event/JEditorEvent.h"
#include"../../../Popup/JEditorPopupMenu.h"
#include"../../../Popup/JEditorPopupNode.h"
#include"../../../Gui/JGui.h" 
#include"../../../String/JEditorStringMap.h"
#include"../../../EditTool/JEditorInputBuffHelper.h"
#include"../../../EditTool/JEditorViewStructure.h" 
#include"../../../../Core/Reflection/JTypeTemplate.h"  
#include"../../../../Object/Resource/AnimationController/FSM/JAnimationStateType.h"   
#include"../../../../Object/Resource/AnimationController/FSM/JAnimationFSMdiagram.h"   
#include"../../../../Object/Resource/AnimationController/JAnimationController.h"   

namespace JinEngine
{
	namespace Editor
	{
		namespace Private
		{
			static std::string DiagramListName(const std::string& uniqueLabel)noexcept
			{
				return "DiagramList##" + uniqueLabel;
			}
		}

		DEFAULT_CD_REQUESTOR(JAnimationDiagramListCreationFunctor, JAnimationDiagramList)

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

			using RequestEvF = JAnimationDiagramListCreationFunctor::RequestEvF;
			createNewDiagramNode->RegisterSelectBind(std::make_unique< RequestEvF::CompletelyBind>(*creation->reqCreateStateEvF, this));
			destroyDigamraNode->RegisterSelectBind(std::make_unique< RequestEvF::CompletelyBind>(*creation->reqDestroyEvF, this));
			destroyDigamraNode->RegisterEnableBind(std::make_unique<PassPopupConditionF::CompletelyBind>(*GetPassSelectedAboveOneFunctor(), this));

			diagramListPopup = std::make_unique<JEditorPopupMenu>(Private::DiagramListName(GetName()), std::move(diagramListRootNode));
			diagramListPopup->AddPopupNode(std::move(createNewDiagramNode));
			diagramListPopup->AddPopupNode(std::move(destroyDigamraNode));
		}
		JAnimationDiagramList::~JAnimationDiagramList()
		{
			creation.reset();
		}
		void JAnimationDiagramList::InitializeCreationImpl()
		{
			if (creation != nullptr)
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
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification);

				JAnimationDiagramListCreationFunctor* impl = diagramList->creation.get();
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
				JEditorRequestHint requestHint = JEditorRequestHint(&JEditorWindow::AddEventNotification);

				JAnimationDiagramListCreationFunctor* impl = diagramList->creation.get();
				impl->destructuion.RequestDestroyObject(impl->dS, true, creationHint, objVec, requestHint);
			};
			creation = std::make_unique<JAnimationDiagramListCreationFunctor>(requestCreateLam, requestDestroyLam);

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

			creation->creation.GetCreationInterface()->RegisterCanCreationF(canCreateDiagramLam);
			creation->creation.GetCreationInterface()->RegisterObjectCreationF(createDiagramLam);
		}
		J_EDITOR_WINDOW_TYPE JAnimationDiagramList::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::ANIMATION_DIAGRAM_LIST;
		}
		void JAnimationDiagramList::SetAnimationController(const JUserPtr<JAnimationController>& newAniCont)
		{
			aniCont = newAniCont;
		}	 
		void JAnimationDiagramList::UpdateWindow()
		{
			EnterWindow(J_GUI_WINDOW_FLAG_NO_SCROLL_BAR | J_GUI_WINDOW_FLAG_NO_COLLAPSE);
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
			J_GUI_TABLE_FLAG_ flag = J_GUI_TABLE_FLAG_BORDERS | J_GUI_TABLE_FLAG_REORDERABLE | J_GUI_TABLE_FLAG_HIDEABLE;
			if (aniCont.IsValid() && JGui::BeginTable("##DiagramListTable" + GetName(), 1, flag))
			{
				JGui::TableSetupColumn("Name", J_GUI_TABLE_COLUMN_FLAG_WIDTH_STRETCH);
				JGui::TableHeadersRow();

				const std::vector<JUserPtr<JAnimationFSMdiagram>>& diagramVec = aniCont->GetDiagramVec();
				const uint diagramCount = (uint)diagramVec.size();
				for (uint i = 0; i < diagramCount; ++i)
				{
					JGui::TableNextRow();
					JGui::TableSetColumnIndex(0);

					const bool isSelect = IsSelectedObject(diagramVec[i]->GetGuid());
					const JVector2<float> preCursorPos = JGui::GetCursorScreenPos();
					JGui::PushTreeNodeColorSet(IsFocus(), true, isSelect);
					if (JGui::Selectable(JGui::CreateGuiLabel(diagramVec[i], GetName() + "Selectable"), &isSelect))
					{
						RequestPushSelectObject(diagramVec[i]);
						SetContentsClick(true);
					}
					JGui::PopTreeNodeColorSet(true, isSelect);

					if (JGui::IsMouseInRect(preCursorPos, JGui::GetLastItemRectSize()))
					{
						SetHoveredObject(diagramVec[i]);
						if (JGui::IsMouseClicked(Core::J_MOUSE_BUTTON::RIGHT))
							SetContentsClick(true);
					}

				}
				JGui::EndTable();
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
			DeRegisterListener();
			JEditorWindow::DoDeActivate();
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