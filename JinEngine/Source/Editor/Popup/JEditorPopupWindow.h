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


#pragma once
#include"JEditorPopupType.h"
#include"../../Core/Func/Functor/JFunctor.h"
#include"../../Core/Math/JVector.h"
 
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
			virtual void Update(const std::string& uniqueLabel, const JVector2<float>& pagePos, const JVector2<float>& pageSize) = 0;
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
		public:
			bool IsOpen()const noexcept;
			bool IsIgnoreConfirm()const noexcept;
			bool IsPressConfirm()const noexcept;
			bool IsDefaultFuncType(const J_EDITOR_POPUP_WINDOW_FUNC_TYPE type)const noexcept;
			virtual bool IsSupportedFuncType(const J_EDITOR_POPUP_WINDOW_FUNC_TYPE type)const noexcept = 0;
		};

		class JEditorCautionPopup : public JEditorPopupWindow
		{
		public:
			void Update(const std::string& uniqueLabel, const JVector2<float>& pagePos, const JVector2<float>& pageSize) final;
		public:
			J_EDITOR_POPUP_WINDOW_TYPE GetPopupType()const noexcept final;
		public:
			bool IsSupportedFuncType(const J_EDITOR_POPUP_WINDOW_FUNC_TYPE type)const noexcept final;
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
			void Update(const std::string& uniqueLabel, const JVector2<float>& pagePos, const JVector2<float>& pageSize) final;
		public:
			J_EDITOR_POPUP_WINDOW_TYPE GetPopupType()const noexcept final;
		public:
			void SetOpen() noexcept final;
		protected:
			void SetCancel(const bool value)noexcept;	
		public:
			bool IsSupportedFuncType(const J_EDITOR_POPUP_WINDOW_FUNC_TYPE type)const noexcept final;
		};
	}
}