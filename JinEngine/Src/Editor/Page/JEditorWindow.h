#pragma once
#include"JEditor.h"  
#include"JEditorPageEnum.h"
#include"JEditorWindowEnum.h" 
#include"JEditorWindowFontType.h" 
#include"../../Core/Func/Functor/JFunctor.h"
 
namespace JinEngine
{
	namespace Editor
	{
		class JEditorAttribute;
		class JEditorWindowDockUpdateHelper;
		class JEditorWindow : public JEditor
		{ 
		protected:
			using EventF= Core::JSFunctorType<void, JEditorWindow&, J_EDITOR_EVENT, JEditorEvStruct&>; 
		private:
			static EventF::Functor* evFunctor;
		private: 
			const J_EDITOR_PAGE_TYPE ownerPageType; 
			bool isWindowOpen = false;
			J_EDITOR_WINDOW_FLAG windowFlag;
		private:
			std::unique_ptr<JEditorWindowDockUpdateHelper> dockUpdateHelper = nullptr;
		public:
			JEditorWindow(const std::string name,
				std::unique_ptr<JEditorAttribute> attribute, 
				const J_EDITOR_PAGE_TYPE ownerPageType, 
				const J_EDITOR_WINDOW_FLAG windowFlag);
			~JEditorWindow();
			JEditorWindow(const JEditorWindow& rhs) = delete;
			JEditorWindow& operator=(const JEditorWindow& rhs) = delete;
		public:
			J_EDITOR_PAGE_TYPE GetOwnerPageType()const noexcept;
			virtual J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept = 0;
		public:
			virtual void UpdateWindow() = 0;
		protected:
			void EnterWindow(int guiWindowFlag);
			void CloseWindow();
		protected:
			void UpdateMouseClick();
			void UpdateDocking(); 
		protected:
			void SetSelectableColor(const JVector4<float>& factor)noexcept;
			void SetButtonColor(const JVector4<float>& factor)noexcept;
			void SetTreeNodeColor(const JVector4<float>& factor)noexcept; 
		protected:
			bool RegisterEventListener(const J_EDITOR_EVENT evType);
			bool RegisterEventListener(std::vector<J_EDITOR_EVENT>& evType);
			void DeRegisterEventListener(const J_EDITOR_EVENT evType);
			void DeRegisterListener();
		protected:
			void RequestOpenPage(const JEditorOpenPageEvStruct& evStruct, const bool doAct);
			void RequestClosePage(const JEditorClosePageEvStruct& evStruct); 
			void RequestSelectObject(const JEditorSelectObjectEvStruct& evStruct);
			void RequestDeSelectObject(const JEditorSelectObjectEvStruct& evStruct); 
			void RequesBind(const std::string& label, 
				std::unique_ptr<Core::JBindHandleBase>&& doHandle, 
				std::unique_ptr<Core::JBindHandleBase>&& undoHandle); 
		protected:
			void DoSetOpen()noexcept override;
			void DoSetClose()noexcept override;
		public:
			virtual void StoreEditorWindow(std::wofstream& stream);
			virtual void LoadEditorWindow(std::wifstream& stream);
		};
	}
}