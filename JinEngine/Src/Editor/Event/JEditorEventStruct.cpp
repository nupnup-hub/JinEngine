#include"JEditorEventStruct.h"
#include"../../Core/Reflection/JReflectionInfo.h"
#include"../../Object/GameObject/JGameObject.h" 
#include"../../Object/Resource/JResourceManager.h"
#include"../../Object/Resource/JResourceObject.h"
#include"../../Object/Directory/JDirectory.h" 
#include"../Page/JEditorPageShareData.h"
#include"../Page/JEditorPage.h"

namespace JinEngine
{
	namespace Editor
	{
		JEditorEvStruct::JEditorEvStruct(const J_EDITOR_PAGE_TYPE pageType)
			:pageType(pageType)
		{}
		JEditorMouseClickEvStruct::JEditorMouseClickEvStruct(const std::string& windowName, const uint clickBtn, const J_EDITOR_PAGE_TYPE pageType)
			: JEditorEvStruct(pageType), windowName(windowName), clickBtn(clickBtn)
		{}
		bool JEditorMouseClickEvStruct::PassDefectInspection()const noexcept
		{
			return !windowName.empty();
		}
		J_EDITOR_EVENT JEditorMouseClickEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::MOUSE_CLICK;
		}

		JEditorPushSelectObjectEvStruct::JEditorPushSelectObjectEvStruct(const J_EDITOR_PAGE_TYPE pageType,
			const J_EDITOR_WINDOW_TYPE wndType,
			JUserPtr<Core::JIdentifier> selectObj)
			:JEditorEvStruct(pageType), wndType(wndType)
		{
			selectObjVec.push_back(selectObj);
		}
		JEditorPushSelectObjectEvStruct::JEditorPushSelectObjectEvStruct(const J_EDITOR_PAGE_TYPE pageType,
			const J_EDITOR_WINDOW_TYPE wndType,
			const std::vector<JUserPtr<Core::JIdentifier>> selectObj)
			: JEditorEvStruct(pageType), wndType(wndType), selectObjVec(selectObj)
		{}
		bool JEditorPushSelectObjectEvStruct::PassDefectInspection()const noexcept
		{
			return selectObjVec.size() > 0;
		}
		J_EDITOR_EVENT JEditorPushSelectObjectEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::PUSH_SELECT_OBJECT;
		}
		JUserPtr<Core::JIdentifier> JEditorPushSelectObjectEvStruct::GetFirstMatchedTypeObject(const Core::JTypeInfo& typeInfo)const noexcept
		{
			const uint count = (uint)selectObjVec.size();
			for (uint i = 0; i < count; ++i)
			{
				if (selectObjVec[i]->GetTypeInfo().IsChildOf(typeInfo))
					return selectObjVec[i];
			}
			return JUserPtr<Core::JIdentifier>{};
		}
		JUserPtr<Core::JIdentifier> JEditorPushSelectObjectEvStruct::GetLastMatchedTypeObject(const Core::JTypeInfo& typeInfo)const noexcept
		{
			const int count = (int)selectObjVec.size();
			for (int i = count - 1; i >= 0; --i)
			{
				if (selectObjVec[i]->GetTypeInfo().IsChildOf(typeInfo))
					return selectObjVec[i];
			}
			return JUserPtr<Core::JIdentifier>{};
		}

		JEditorPopSelectObjectEvStruct::JEditorPopSelectObjectEvStruct(const J_EDITOR_PAGE_TYPE pageType, JUserPtr<Core::JIdentifier> selectObj)
			:JEditorEvStruct(pageType)
		{
			selectObjVec.push_back(selectObj);
		}
		JEditorPopSelectObjectEvStruct::JEditorPopSelectObjectEvStruct(const J_EDITOR_PAGE_TYPE pageType, const std::vector<JUserPtr<Core::JIdentifier>> selectObj)
			: JEditorEvStruct(pageType), selectObjVec(selectObj)
		{}
		bool JEditorPopSelectObjectEvStruct::PassDefectInspection()const noexcept
		{
			return selectObjVec.size() > 0;
		}
		J_EDITOR_EVENT JEditorPopSelectObjectEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::POP_SELECT_OBJECT;
		}
		bool JEditorPopSelectObjectEvStruct::IsPopTarget(const size_t guid)const noexcept
		{
			for (const auto& data : selectObjVec)
			{
				if (data->GetGuid() == guid)
					return true;
			}
			return false;
		}

		JEditorClearSelectObjectEvStruct::JEditorClearSelectObjectEvStruct(const J_EDITOR_PAGE_TYPE pageType)
			:JEditorEvStruct(pageType)
		{}
		bool JEditorClearSelectObjectEvStruct::PassDefectInspection()const noexcept
		{
			return true;
		}
		J_EDITOR_EVENT JEditorClearSelectObjectEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::CLEAR_SELECT_OBJECT;
		}

		JEditorOpenPageEvStruct::JEditorOpenPageEvStruct(const J_EDITOR_PAGE_TYPE pageType, JUserPtr<Core::JIdentifier> openSelected)
			:JEditorEvStruct(pageType),
			typeGuid(openSelected.IsValid() ? openSelected->GetTypeInfo().TypeGuid() : 0),
			openSeletedGuid(openSelected.IsValid() ? openSelected->GetGuid() : 0),
			hasOpenSeleted(openSelected.IsValid())
		{}
		bool JEditorOpenPageEvStruct::PassDefectInspection()const noexcept
		{
			return true;
		}
		J_EDITOR_EVENT JEditorOpenPageEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::OPEN_PAGE;
		}
		JUserPtr<Core::JIdentifier> JEditorOpenPageEvStruct::GetOpenSeleted()const noexcept
		{
			return hasOpenSeleted ? Core::GetUserPtr<Core::JIdentifier>(typeGuid, openSeletedGuid) : JUserPtr<Core::JIdentifier>();
		}

		JEditorClosePageEvStruct::JEditorClosePageEvStruct(const J_EDITOR_PAGE_TYPE pageType)
			:JEditorEvStruct(pageType)
		{}
		bool JEditorClosePageEvStruct::PassDefectInspection()const noexcept
		{
			return true;
		}
		J_EDITOR_EVENT JEditorClosePageEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::CLOSE_PAGE;
		}

		JEditorActPageEvStruct::JEditorActPageEvStruct(const J_EDITOR_PAGE_TYPE pageType)
			:JEditorEvStruct(pageType)
		{}
		bool JEditorActPageEvStruct::PassDefectInspection()const noexcept
		{
			return true;
		}
		J_EDITOR_EVENT JEditorActPageEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::ACTIVATE_PAGE;
		}
		JEditorDeActPageEvStruct::JEditorDeActPageEvStruct(const J_EDITOR_PAGE_TYPE pageType)
			:JEditorEvStruct(pageType)
		{}
		bool JEditorDeActPageEvStruct::PassDefectInspection()const noexcept
		{
			return true;
		}
		J_EDITOR_EVENT JEditorDeActPageEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::DEACTIVATE_PAGE;
		}

		JEditorFocusPageEvStruct::JEditorFocusPageEvStruct(JEditorPage* focusPage)
			:JEditorEvStruct(focusPage->GetPageType()), focusPage(focusPage)
		{}
		bool JEditorFocusPageEvStruct::PassDefectInspection()const noexcept
		{
			return focusPage != nullptr;
		}
		J_EDITOR_EVENT JEditorFocusPageEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::FOCUS_PAGE;
		}

		JEditorUnFocusPageEvStruct::JEditorUnFocusPageEvStruct(JEditorPage* unFocusPage)
			:JEditorEvStruct(unFocusPage->GetPageType()), unFocusPage(unFocusPage)
		{}
		bool JEditorUnFocusPageEvStruct::PassDefectInspection()const noexcept
		{
			return unFocusPage != nullptr;
		}
		J_EDITOR_EVENT JEditorUnFocusPageEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::UNFOCUS_PAGE;
		}

		JEditorOpenWindowEvStruct::JEditorOpenWindowEvStruct(const std::string& openWindowName, const J_EDITOR_PAGE_TYPE pageType)
			:JEditorEvStruct(pageType), openWindowName(openWindowName)
		{}
		bool JEditorOpenWindowEvStruct::PassDefectInspection()const noexcept
		{
			return !openWindowName.empty();
		}
		J_EDITOR_EVENT JEditorOpenWindowEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::OPEN_WINDOW;
		}
		JEditorCloseWindowEvStruct::JEditorCloseWindowEvStruct(const std::string& closeWindowName, const J_EDITOR_PAGE_TYPE pageType)
			:JEditorEvStruct(pageType), closeWindowName(closeWindowName)
		{}
		bool JEditorCloseWindowEvStruct::PassDefectInspection()const noexcept
		{
			return !closeWindowName.empty();
		}
		J_EDITOR_EVENT JEditorCloseWindowEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::CLOSE_WINDOW;
		}

		JEditorActWindowEvStruct::JEditorActWindowEvStruct(JEditorWindow* actWindow, const J_EDITOR_PAGE_TYPE pageType)
			:JEditorEvStruct(pageType), actWindow(actWindow)
		{}
		bool JEditorActWindowEvStruct::PassDefectInspection()const noexcept
		{
			return actWindow != nullptr;
		}
		J_EDITOR_EVENT JEditorActWindowEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::ACTIVATE_WINDOW;
		}

		JEditorDeActWindowEvStruct::JEditorDeActWindowEvStruct(JEditorWindow* deActWindow, const J_EDITOR_PAGE_TYPE pageType)
			:JEditorEvStruct(pageType), deActWindow(deActWindow)
		{}
		bool JEditorDeActWindowEvStruct::PassDefectInspection()const noexcept
		{
			return deActWindow != nullptr;
		}
		J_EDITOR_EVENT JEditorDeActWindowEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::DEACTIVATE_WINDOW;
		}

		JEditorFocusWindowEvStruct::JEditorFocusWindowEvStruct(JEditorWindow* focusWindow, const J_EDITOR_PAGE_TYPE pageType)
			:JEditorEvStruct(pageType), focusWindow(focusWindow)
		{}
		bool JEditorFocusWindowEvStruct::PassDefectInspection()const noexcept
		{
			return focusWindow != nullptr;
		}
		J_EDITOR_EVENT JEditorFocusWindowEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::FOCUS_WINDOW;
		}

		JEditorUnFocusWindowEvStruct::JEditorUnFocusWindowEvStruct(JEditorWindow* unFocusWindow, const J_EDITOR_PAGE_TYPE pageType)
			:JEditorEvStruct(pageType), unFocusWindow(unFocusWindow)
		{}
		bool JEditorUnFocusWindowEvStruct::PassDefectInspection()const noexcept
		{
			return unFocusWindow != nullptr;
		}
		J_EDITOR_EVENT JEditorUnFocusWindowEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::UNFOCUS_WINDOW;
		}

		JEditorOpenPopupWindowEvStruct::JEditorOpenPopupWindowEvStruct(JEditorPopupWindow* popupWindow, const J_EDITOR_PAGE_TYPE pageType)
			:JEditorEvStruct(pageType), popupWindow(popupWindow)
		{}
		JEditorOpenPopupWindowEvStruct::JEditorOpenPopupWindowEvStruct(const J_EDITOR_POPUP_WINDOW_TYPE popupType, const J_EDITOR_PAGE_TYPE pageType)
			: JEditorEvStruct(pageType), popupType(popupType)
		{}

		bool JEditorOpenPopupWindowEvStruct::PassDefectInspection()const noexcept
		{
			return true;
		}
		J_EDITOR_EVENT JEditorOpenPopupWindowEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::OPEN_POPUP_WINDOW;
		}
		void JEditorOpenPopupWindowEvStruct::AddBind(const J_EDITOR_POPUP_WINDOW_FUNC_TYPE type, std::unique_ptr< Core::JBindHandleBase>&& bind)
		{
			tupleVec.push_back(std::make_tuple(type, std::move(bind)));
		}
		void JEditorOpenPopupWindowEvStruct::AddDesc(const std::string& newDesc)
		{
			desc = newDesc;
		}

		JEditorClosePopupWindowEvStruct::JEditorClosePopupWindowEvStruct(JEditorPopupWindow* popupWindow, const J_EDITOR_PAGE_TYPE pageType)
			:JEditorEvStruct(pageType), popupWindow(popupWindow)
		{}
		JEditorClosePopupWindowEvStruct::JEditorClosePopupWindowEvStruct(const J_EDITOR_POPUP_WINDOW_TYPE popupType, const J_EDITOR_PAGE_TYPE pageType)
			: JEditorEvStruct(pageType), popupType(popupType)
		{}
		bool JEditorClosePopupWindowEvStruct::PassDefectInspection()const noexcept
		{
			return true;
		}
		J_EDITOR_EVENT JEditorClosePopupWindowEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::CLOSE_POPUP_WINDOW;
		}

		JEditorBindFuncEvStruct::JEditorBindFuncEvStruct(std::unique_ptr<Core::JBindHandleBase> bindHandle, const J_EDITOR_PAGE_TYPE pageType)
			:JEditorEvStruct(pageType), bindHandle(std::move(bindHandle))
		{}
		bool JEditorBindFuncEvStruct::PassDefectInspection()const noexcept
		{
			return bindHandle != nullptr;
		}
		J_EDITOR_EVENT JEditorBindFuncEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::BIND_FUNC;
		}
		void JEditorBindFuncEvStruct::SetLog(std::unique_ptr<Core::JLogBase>newLog)
		{
			log = std::move(newLog);
		}
		void JEditorBindFuncEvStruct::Execute()
		{  
			if (log != nullptr)
				JEditorTransition::Instance().Log(*log);
			bindHandle->InvokeCompletelyBind();
		}

		JEditorTBindFuncEvStruct::JEditorTBindFuncEvStruct(const std::string& taskName, const std::string& taskDesc, const J_EDITOR_PAGE_TYPE pageType)
			:JEditorEvStruct(pageType), taskName(taskName), taskDesc(taskDesc)
		{}
		J_EDITOR_EVENT JEditorTBindFuncEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::T_BIND_FUNC;
		}

		JEditorTSetBindFuncEvStruct::JEditorTSetBindFuncEvStruct(const std::string& taskName,
			const std::string& taskDesc,
			const J_EDITOR_PAGE_TYPE pageType,
			std::unique_ptr<Core::JBindHandleBase> doBindHandle,
			std::unique_ptr<Core::JBindHandleBase> undoBindHandle)
			:JEditorTBindFuncEvStruct(taskName, taskDesc, pageType),
			doBindHandle(std::move(doBindHandle)),
			undoBindHandle(std::move(undoBindHandle))
		{}
		bool JEditorTSetBindFuncEvStruct::PassDefectInspection()const noexcept
		{
			return doBindHandle != nullptr && undoBindHandle != nullptr;
		}
		void JEditorTSetBindFuncEvStruct::Execute()
		{
			JEditorTransition::Instance().Execute(std::make_unique<Core::JTransitionSetValueTask>(taskName, taskDesc,
				std::move(doBindHandle), std::move(undoBindHandle)));
		}
	}
}