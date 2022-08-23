#pragma once
#include"JEditor.h"  
#include"JEditorPageType.h"
#include"JEditorWindowType.h"
#include"../Event/JEditorEventType.h"
#include"../Event/JEditorEventStruct.h"  
#include"../../Core/Event/JEventListener.h"
#include"../../Core/Func/Functor/JFunctor.h"

namespace JinEngine
{
	namespace Editor
	{
		class JEditorAttribute;
		class JEditorWindow : public JEditor, private Core::JEventListener<size_t, J_EDITOR_EVENT, JEditorEventStruct*>
		{
			REGISTER_CLASS(JEditorWindow)
		protected:
			using EventFunctor = Core::JFunctor<void, JEditorWindow&, J_EDITOR_EVENT, JEditorEventStruct&>;
			using EventFunctorBinder = Core::JBindHandle <EventFunctor, JEditorWindow&, J_EDITOR_EVENT, JEditorEventStruct&>;
		private:
			static EventFunctor* evFunctor;
		private:
			const J_EDITOR_PAGE_TYPE ownerPageType;
		public:
			virtual void EnterWindow();
			virtual void UpdateWindow();
			void UpdateDocking();
			virtual void CloseWindow();
		public:
			J_EDITOR_PAGE_TYPE GetOwnerPageType()const noexcept;
			virtual J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept = 0;
		protected:
			void SetButtonSelectColor()noexcept;
			void SetButtonDefaultColor()noexcept;
			void SetTreeNodeSelectColor()noexcept;
			void SetTreeNodeDefaultColor()noexcept;
		protected:
			void RegisterEventListener(const J_EDITOR_EVENT evType);
			void RegisterEventListener(std::vector<J_EDITOR_EVENT>& evType);
			void DeRegisterEventListener(const J_EDITOR_EVENT evType);
			void DeRegisterListener();

			void RequestOpenPage(const JEditorOpenPageEvStruct& evStruct);
			void RequestClosePage(const JEditorClosePageEvStruct& evStruct);
			void RequestSelectObject(const JEditorSelectObjectEvStruct& evStruct);
			void RequestDeSelectObject(const JEditorSelectObjectEvStruct& evStruct);
		public:
			virtual void StoreEditorWindow(std::wofstream& stream);
			virtual void LoadEditorWindow(std::wifstream& stream);
		private:
			static void RegisterJFunc();
		protected:
			JEditorWindow(std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE ownerPageType);
			~JEditorWindow();
			JEditorWindow(const JEditorWindow& rhs) = delete;
			JEditorWindow& operator=(const JEditorWindow& rhs) = delete;
		};
	}
}