#pragma once
#include"JEditorPopupType.h"
#include"../../Core/Func/Functor/JFunctor.h"
#include"../../Utility/JVector.h"

#include<string>
#include<memory>

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
		private:
			std::string desc;
		public:
			JEditorPopupWindow() = default;
			~JEditorPopupWindow() = default;
		public:
			virtual void RegisterBind(const J_EDITOR_POPUP_WINDOW_FUNC_TYPE type, std::unique_ptr<Core::JBindHandleBase>&& bind)noexcept;
			void RegisterPostCloseProccess(std::unique_ptr<Core::JBindHandleBase>&& bind);
		public:
			virtual void Update(const std::string& uniqueLabel, const JVector2<float> pagePos, const JVector2<float> pageSize) = 0;
		public:
			bool IsOpen()const noexcept;
			bool IsIgnoreConfirm()const noexcept;
			bool IsPressConfirm()const noexcept;
			bool IsDefaultFuncType(const J_EDITOR_POPUP_WINDOW_FUNC_TYPE type)const noexcept;
		public:
			virtual bool IsSupportedFuncType(const J_EDITOR_POPUP_WINDOW_FUNC_TYPE type)const noexcept = 0;
		public:
			virtual J_EDITOR_POPUP_WINDOW_TYPE GetPopupType()const noexcept = 0;	
			std::string GetDesc()const noexcept;
		public:
			virtual void SetOpen() noexcept;
			virtual void SetClose() noexcept;
			void SetIgnore(const bool value)noexcept;
			void SetDesc(const std::string& newDesc)noexcept;
		protected:
			void SetConfirm(const bool value)noexcept; 
		};

		class JEditorCautionPopup : public JEditorPopupWindow
		{
		public:
			void Update(const std::string& uniqueLabel, const JVector2<float> pagePos, const JVector2<float> pageSize) final;
		public:
			bool IsSupportedFuncType(const J_EDITOR_POPUP_WINDOW_FUNC_TYPE type)const noexcept final;
		public:
			J_EDITOR_POPUP_WINDOW_TYPE GetPopupType()const noexcept final;
		};

		class JEditorCloseConfirmPopup : public JEditorPopupWindow
		{ 
		private:
			std::unique_ptr<Core::JBindHandleBase> cancelBind;
			std::unique_ptr<Core::JBindHandleBase> contentsBind;
		private:
			bool isPressCancel = false;
		public:
			void RegisterBind(const J_EDITOR_POPUP_WINDOW_FUNC_TYPE type, std::unique_ptr<Core::JBindHandleBase>&& bind)noexcept final;
		public:
			void Update(const std::string& uniqueLabel, const JVector2<float> pagePos, const JVector2<float> pageSize) final;
		public:
			bool IsSupportedFuncType(const J_EDITOR_POPUP_WINDOW_FUNC_TYPE type)const noexcept final;
		public:
			J_EDITOR_POPUP_WINDOW_TYPE GetPopupType()const noexcept final;
		public:
			void SetOpen() noexcept final;
		protected:
			void SetCancel(const bool value)noexcept;	
		};
	}
}