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

		JEditorSelectObjectEvStruct::JEditorSelectObjectEvStruct(const J_EDITOR_PAGE_TYPE pageType, Core::JUserPtr<JObject> selectObj)
			:JEditorEvStruct(pageType), selectObj(selectObj)
		{}
		bool JEditorSelectObjectEvStruct::PassDefectInspection()const noexcept
		{
			return selectObj.IsValid();
		}
		J_EDITOR_EVENT JEditorSelectObjectEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::SELECT_OBJECT;
		}

		JEditorDeSelectObjectEvStruct::JEditorDeSelectObjectEvStruct(const J_EDITOR_PAGE_TYPE pageType)
			:JEditorEvStruct(pageType)
		{}
		bool JEditorDeSelectObjectEvStruct::PassDefectInspection()const noexcept
		{
			return JEditorPageShareData::GetSelectedObj(pageType).IsValid();
		}
		J_EDITOR_EVENT JEditorDeSelectObjectEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::DESELECT_OBJECT;
		}

		JEditorOpenPageEvStruct::JEditorOpenPageEvStruct(const J_EDITOR_PAGE_TYPE pageType, Core::JUserPtr<JObject> openSelected)
			:JEditorEvStruct(pageType), openSelected(openSelected)
		{}
		bool JEditorOpenPageEvStruct::PassDefectInspection()const noexcept
		{
			return true;
		}
		J_EDITOR_EVENT JEditorOpenPageEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::OPEN_PAGE;
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

		JEditorActPageEvStruct::JEditorActPageEvStruct(JEditorPage* actPage)
			:JEditorEvStruct(actPage->GetPageType()), actPage(actPage)
		{}
		bool JEditorActPageEvStruct::PassDefectInspection()const noexcept
		{
			return actPage != nullptr;
		}
		J_EDITOR_EVENT JEditorActPageEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::ACTIVATE_PAGE;
		}
		JEditorDeActPageEvStruct::JEditorDeActPageEvStruct(JEditorPage* deActPage)
			:JEditorEvStruct(deActPage->GetPageType()), deActPage(deActPage)
		{}
		bool JEditorDeActPageEvStruct::PassDefectInspection()const noexcept
		{
			return deActPage != nullptr;
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
		void JEditorBindFuncEvStruct::Execute()
		{
			bindHandle->InvokeCompletelyBind();
		}

		JEditorTBindFuncEvStruct::JEditorTBindFuncEvStruct(const std::string& taskName, const J_EDITOR_PAGE_TYPE pageType)
			:JEditorEvStruct(pageType)
		{}
		J_EDITOR_EVENT JEditorTBindFuncEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::T_BIND_FUNC;
		}

		JEditorTSetBindFuncEvStruct::JEditorTSetBindFuncEvStruct(const std::string& taskName,
			const J_EDITOR_PAGE_TYPE pageType,
			std::unique_ptr<Core::JBindHandleBase> doBindHandle,
			std::unique_ptr<Core::JBindHandleBase> undoBindHandle)
			:JEditorTBindFuncEvStruct(taskName, pageType),
			doBindHandle(std::move(doBindHandle)),
			undoBindHandle(std::move(undoBindHandle))
		{}
		bool JEditorTSetBindFuncEvStruct::PassDefectInspection()const noexcept
		{
			return doBindHandle != nullptr && undoBindHandle != nullptr;
		}
		void JEditorTSetBindFuncEvStruct::Execute()
		{
			Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>(taskName,
				std::move(doBindHandle), std::move(undoBindHandle)));
		}
	}
}