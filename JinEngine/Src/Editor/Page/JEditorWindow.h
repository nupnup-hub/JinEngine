#pragma once
#include"JEditor.h"  
#include"JEditorPageEnum.h"
#include"JEditorWindowType.h" 
#include"JEditorWindowFontType.h" 
#include"../../Core/Func/Functor/JFunctor.h"
 
namespace JinEngine
{
	namespace Editor
	{
		class JEditorAttribute;
		class JEditorWindow : public JEditor
		{ 
		protected:
			using EventFunctor = Core::JFunctor<void, JEditorWindow&, J_EDITOR_EVENT, JEditorEvStruct&>;
			using EventFunctorBinder = Core::JBindHandle <EventFunctor, JEditorWindow&, J_EDITOR_EVENT, JEditorEvStruct&>;
		private:
			static EventFunctor* evFunctor;
		private: 
			const J_EDITOR_PAGE_TYPE ownerPageType; 
		public:
			JEditorWindow(const std::string name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE ownerPageType);
			~JEditorWindow();
			JEditorWindow(const JEditorWindow& rhs) = delete;
			JEditorWindow& operator=(const JEditorWindow& rhs) = delete;
		public:
			J_EDITOR_PAGE_TYPE GetOwnerPageType()const noexcept;
			virtual J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept = 0;
		public:
			virtual void UpdateWindow() = 0;
		protected:
			void EnterWindow(int windowFlag);
			void CloseWindow();
		protected:
			void UpdateMouseClick();
			void UpdateDocking();
		protected:
			void SetButtonColor(const float factor)noexcept; 
			void SetTreeNodeColor(const float factor)noexcept; 
		protected:
			bool RegisterEventListener(const J_EDITOR_EVENT evType);
			bool RegisterEventListener(std::vector<J_EDITOR_EVENT>& evType);
			void DeRegisterEventListener(const J_EDITOR_EVENT evType);
			void DeRegisterListener();
		protected:
			void RequestOpenPage(const JEditorOpenPageEvStruct& evStruct);
			void RequestClosePage(const JEditorClosePageEvStruct& evStruct); 
			void RequestSelectObject(const JEditorSelectObjectEvStruct& evStruct);
			void RequestDeSelectObject(const JEditorSelectObjectEvStruct& evStruct); 
		protected:
			void DoSetOpen()noexcept override;
			void DoSetClose()noexcept override;
		public:
			virtual void StoreEditorWindow(std::wofstream& stream);
			virtual void LoadEditorWindow(std::wifstream& stream);
		};
	}
}