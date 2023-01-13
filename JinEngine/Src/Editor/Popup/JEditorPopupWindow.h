#pragma once
#include"JEditorPopupType.h"
#include"../../Core/Func/Functor/JFunctor.h"
#include"../../Utility/JVector.h"

#include<string>
namespace JinEngine
{
	namespace Editor
	{
		class JEditorPopupWindow
		{
		public:
			using PressFunc = Core::JSFunctorType<void>;
		private:
			bool isIgnoreConfirm = false;
			bool isPressConfirm = false;
		private:
			bool isOpen = false;
		private:
			//Completely Bind
			std::unique_ptr<Core::JBindHandleBase> confirmBind;
			std::unique_ptr<Core::JBindHandleBase> openBind;
			std::unique_ptr<Core::JBindHandleBase> closeBind;
		public:
			JEditorPopupWindow() = default;
			~JEditorPopupWindow() = default;
		protected:
			void RegisterBind(std::unique_ptr<Core::JBindHandleBase> newConfirmBind,
				std::unique_ptr<Core::JBindHandleBase> newOpenBind,
				std::unique_ptr<Core::JBindHandleBase> newCloseBind)noexcept;
		public:
			virtual void Update(const std::string& uniqueLabel,
				const std::string& desc,
				const JVector2<float> pagePos,
				const JVector2<float> pageSize) = 0;
		public:
			bool IsOpen()const noexcept;
			bool IsIgnoreConfirm()const noexcept;
			bool IsPressConfirm()const noexcept;
		public:
			virtual J_EDITOR_POPUP_WINDOW_TYPE GetPopupType()const noexcept = 0;
		public:
			virtual void SetOpen() noexcept;
			virtual void SetClose() noexcept;
			void SetIgnore(const bool value)noexcept;
		protected:
			void SetConfirm(const bool value)noexcept;
		};

		class JEditorCautionPopup : public JEditorPopupWindow
		{
		public:
			void Update(const std::string& uniqueLabel,
				const std::string& desc,
				const JVector2<float> pagePos,
				const JVector2<float> pageSize) final;
		public:
			J_EDITOR_POPUP_WINDOW_TYPE GetPopupType()const noexcept final;
		};

		class JEditorCloseConfirmPopup : public JEditorPopupWindow
		{ 
		private:
			std::unique_ptr<Core::JBindHandleBase> cancelBind;
		private:
			bool isPressCancel = false;
		public:
			void RegisterBind(std::unique_ptr<Core::JBindHandleBase> newConfirmBind,
				std::unique_ptr<Core::JBindHandleBase> newOpenBind,
				std::unique_ptr<Core::JBindHandleBase> newCloseBind,
				std::unique_ptr<Core::JBindHandleBase> newCancelBind);
		public:
			void Update(const std::string& uniqueLabel,
				const std::string& desc,
				const JVector2<float> pagePos,
				const JVector2<float> pageSize) final;
		public:
			J_EDITOR_POPUP_WINDOW_TYPE GetPopupType()const noexcept final;
		public:
			void SetOpen() noexcept final;
		protected:
			void SetCancel(const bool value)noexcept;
		};
	}
}