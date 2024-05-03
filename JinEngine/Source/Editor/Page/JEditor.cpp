#include"JEditor.h"
#include"JEditorAttribute.h"
#include"JEditorPageShareData.h"
#include"../Event/JEditorEvent.h"
#include"../Gui/JGui.h"
#include"../../Core/Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Editor
	{
		JEditor::JEditor(const std::string& name, std::unique_ptr<JEditorAttribute> attribute)
			: name(name),
			guid(CalculateGuid(name)),
			attribute(std::move(attribute))
		{}
		JEditor::~JEditor()
		{}
		bool JEditor::IsOpen()const noexcept
		{
			return attribute->isOpen;
		}
		bool JEditor::IsFocus()const noexcept
		{
			return attribute->isFocus;
		}
		bool JEditor::IsActivated()const noexcept
		{
			return attribute->isActivated;
		}
		bool JEditor::IsLastActivated()const noexcept
		{
			return attribute->isLastAct;
		}
		std::string JEditor::GetName()const noexcept
		{
			return name;
		}
		std::string JEditor::GetDockNodeName()const noexcept
		{
			return GetName() + "DockNode";
		}
		size_t JEditor::GetGuid()const noexcept
		{
			return guid;
		}
		bool* JEditor::GetOpenPtr()const noexcept
		{
			return &attribute->isOpen;
		}
		JEditor::EventF::Functor* JEditor::GetEvFunctor()noexcept
		{
			static JEditor::EventF::Functor* evFunctor = nullptr;
			if (evFunctor == nullptr)
			{
				auto evFuncLam = [](JEditor& editor, J_EDITOR_EVENT evType, JEditorEvStruct& evStruct)
				{
					editor.AddEventNotification(*JEditorEvent::EvInterface(),
						editor.GetGuid(),
						evType,
						&evStruct);
				};
				static EventF::Functor eventFunctor{ evFuncLam };
				evFunctor = &eventFunctor;
			}
			return evFunctor;
		}
		JEditor::ClearTaskF::Functor* JEditor::GetClearTaskFunctor()noexcept
		{
			static JEditor::ClearTaskF::Functor* clearTaskFunctor = nullptr;
			if (clearTaskFunctor == nullptr)
			{
				auto clearTaskLam = [](std::vector<size_t> guidVec)
				{
					for (const auto& data : guidVec)
						JEditorEvent::SetCanDestroyBit(data, true);
				};
				static ClearTaskF::Functor _clearTaskFunctor{ clearTaskLam };
				clearTaskFunctor = &_clearTaskFunctor;
			}
			return clearTaskFunctor;
		}
		void JEditor::SetName(const std::string& newName)noexcept
		{
			if (!newName.empty())
				name = newName;
		}

		void JEditor::RequestOpenPage(const JEditorOpenPageEvStruct& evStruct, const bool doAct)
		{
			std::string taskName = "Open page";
			std::string taskDesc = "page name: " + JEditorPageShareData::GetPageName(evStruct.pageType);

			std::vector<size_t> evGuidVec;
			if (doAct)
				evGuidVec.resize(3);
			else
				evGuidVec.resize(2);

			JEditorEvStruct* openEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorOpenPageEvStruct>(evStruct), evGuidVec[0], true);
			JEditorEvStruct* closeEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorClosePageEvStruct>(evStruct.pageType), evGuidVec[1], true);

			auto doBinder = std::make_unique<EventF::CompletelyBind>(*GetEvFunctor(), *this, J_EDITOR_EVENT::OPEN_PAGE, *openEvStruct);
			auto undoBinder = std::make_unique<EventF::CompletelyBind>(*GetEvFunctor(), *this, J_EDITOR_EVENT::CLOSE_PAGE, *closeEvStruct);
			auto task = std::make_unique<Core::JTransitionSetValueTask>(taskName, taskDesc, std::move(doBinder), std::move(undoBinder));
			if (doAct)
			{
				JEditorEvStruct* actEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorActPageEvStruct>(evStruct.pageType), evGuidVec[2], true);
				std::vector<std::unique_ptr<Core::JBindHandleBase>> postDoVec;
				postDoVec.push_back(std::make_unique<EventF::CompletelyBind>(*GetEvFunctor(), *this, J_EDITOR_EVENT::ACTIVATE_PAGE, *actEvStruct));

				task->RegisterAddtionalProcess(Core::JTransitionTask::ADDITONAL_PROCESS_TYPE::DO_POST, std::move(postDoVec));
			}
			task->RegisterClearTask(std::make_unique< ClearTaskF::CompletelyBind>(*GetClearTaskFunctor(), std::move(evGuidVec)));
			JEditorTransition::Instance().Execute(std::move(task));
		}
		void JEditor::RequestClosePage(const JEditorClosePageEvStruct& evStruct, const bool isAct)
		{
			const J_EDITOR_PAGE_TYPE pageType = evStruct.pageType;
			std::string taskName = "Close page";
			std::string taskDesc = "page name: " + JEditorPageShareData::GetPageName(pageType);
			if (JEditorPageShareData::HasValidOpenPageData(pageType))
			{
				std::vector<size_t> evGuidVec;
				if (isAct)
					evGuidVec.resize(3);
				else
					evGuidVec.resize(2);

				JEditorEvStruct* closeEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorClosePageEvStruct>(evStruct), evGuidVec[0], true);
				JEditorEvStruct* openEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorOpenPageEvStruct>(JEditorPageShareData::GetOpendPageData(pageType)), evGuidVec[1], true);
				auto doBinder = std::make_unique<EventF::CompletelyBind>(*GetEvFunctor(), *this, J_EDITOR_EVENT::CLOSE_PAGE, *closeEvStruct);
				auto undoBinder = std::make_unique<EventF::CompletelyBind>(*GetEvFunctor(), *this, J_EDITOR_EVENT::OPEN_PAGE, *openEvStruct);
				auto task = std::make_unique<Core::JTransitionSetValueTask>(taskName, taskDesc, std::move(doBinder), std::move(undoBinder));
				if (isAct)
				{
					JEditorEvStruct* actEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorActPageEvStruct>(evStruct.pageType), evGuidVec[2], true);
					std::vector<std::unique_ptr<Core::JBindHandleBase>> postUndoVec;
					postUndoVec.push_back(std::make_unique<EventF::CompletelyBind>(*GetEvFunctor(), *this, J_EDITOR_EVENT::ACTIVATE_PAGE, *actEvStruct));
					task->RegisterAddtionalProcess(Core::JTransitionTask::ADDITONAL_PROCESS_TYPE::UNDO_POST, std::move(postUndoVec));
				}

				task->RegisterClearTask(std::make_unique<ClearTaskF::CompletelyBind>(*GetClearTaskFunctor(), std::move(evGuidVec)));
				JEditorTransition::Instance().Execute(std::move(task));
			}
			else
			{
				JEditorEvStruct* closeEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorClosePageEvStruct>(evStruct));
				auto doBinder = EventF::CompletelyBind{ *GetEvFunctor(), *this,  J_EDITOR_EVENT::CLOSE_PAGE, *closeEvStruct };
				JEditorTransition::Instance().Log(taskName, taskDesc);
				doBinder.InvokeCompletelyBind();
			}
		}
		size_t JEditor::CalculateGuid(const std::string& str)
		{
			return JCUtil::CalculateGuid(str);
		}
		void JEditor::SetOpen()noexcept
		{
			if (!attribute->isOpen)
				DoSetOpen();
		}
		void JEditor::SetClose()noexcept
		{
			if (attribute->isOpen)
				DoSetClose();
		}
		void JEditor::SetFocus()noexcept
		{
			if (!attribute->isFocus)
				DoSetFocus();
		}
		void JEditor::SetUnFocus()noexcept
		{
			if (attribute->isFocus)
				DoSetUnFocus();
		}
		void JEditor::Activate()noexcept
		{
			if (!attribute->isActivated)
				DoActivate();
		}
		void JEditor::DeActivate()noexcept
		{
			if (attribute->isActivated)
				DoDeActivate();
		}
		void JEditor::SetLastActivated(bool value)noexcept
		{
			attribute->isLastAct = value;
		}
		void JEditor::DoSetOpen()noexcept
		{
			attribute->isOpen = true;
		}
		void JEditor::DoSetClose()noexcept
		{
			attribute->isOpen = false;
		}
		void JEditor::DoSetFocus()noexcept
		{
			attribute->isFocus = true;
		}
		void JEditor::DoSetUnFocus()noexcept
		{
			attribute->isFocus = false;
		}
		void JEditor::DoActivate()noexcept
		{
			attribute->isActivated = true;
		}
		void JEditor::DoDeActivate()noexcept
		{
			attribute->isActivated = false;
		}
		void JEditor::OnEvent(const size_t& iden, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)
		{
		}
	}
}

/*
		bool JEditor::Activate(JEditorUtility* editorUtility)
		{
			if (!attribute->isActivated && attribute->isOpen && attribute->isFront)
			{
				attribute->isActivated = true;
				return true;
			}
			else
				return false;
		}
		bool JEditor::DeActivate(JEditorUtility* editorUtility)
		{
			if (attribute->isActivated && attribute->isOpen && attribute->isFront)
			{
				if (attribute->isFocus)
					SetUnFocus(editorUtility);
				attribute->isActivated = false;
				return true;
			}
			else
				return false;
		}
*/