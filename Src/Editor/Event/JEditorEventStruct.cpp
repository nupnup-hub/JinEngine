#include"JEditorEventStruct.h"
#include"../../Core/Reflection/JReflectionInfo.h"
#include"../../Object/GameObject/JGameObject.h"
#include"../../Object/Resource/JResourceManager.h"
#include"../../Object/Resource/JResourceObject.h"
#include"../../Object/Directory/JDirectory.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"

namespace JinEngine
{
	namespace Editor
	{
		JEditorMouseClickEvStruct::JEditorMouseClickEvStruct(const std::string& windowName, const uint clickBtn, const J_EDITOR_PAGE_TYPE pageType)
			:windowName(windowName), clickBtn(clickBtn), pageType(pageType)
		{}
		bool JEditorMouseClickEvStruct::PassDefectInspection()const noexcept
		{
			return !windowName.empty();
		}
		J_EDITOR_EVENT JEditorMouseClickEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::MOUSE_CLICK;
		}

		JEditorSelectObjectEvStruct::JEditorSelectObjectEvStruct(JGameObject& gObj, const J_EDITOR_PAGE_TYPE pageType)
			:objName(gObj.GetName()), objGuid(gObj.GetGuid()), mainType((int)gObj.GetObjectType()), subType(-1), pageType(pageType)
		{}
		JEditorSelectObjectEvStruct::JEditorSelectObjectEvStruct(JResourceObject& rObj, const J_EDITOR_PAGE_TYPE pageType)
			: objName(rObj.GetName()), objGuid(rObj.GetGuid()), mainType((int)rObj.GetObjectType()), subType((int)rObj.GetResourceType()), pageType(pageType)
		{}
		JEditorSelectObjectEvStruct::JEditorSelectObjectEvStruct(JDirectory& dObj, const J_EDITOR_PAGE_TYPE pageType)
			: objName(dObj.GetName()), objGuid(dObj.GetGuid()), mainType((int)dObj.GetObjectType()), subType(-1), pageType(pageType)
		{}
		bool JEditorSelectObjectEvStruct::PassDefectInspection()const noexcept
		{
			if (mainType == (int)J_OBJECT_TYPE::GAME_OBJECT)
				return Core::JReflectionInfo::Instance().GetTypeInfo(JGameObject::TypeName())->GetInstance(objGuid) != nullptr;
			else if (mainType == (int)J_OBJECT_TYPE::RESOURCE_OBJECT)
				return JResourceManager::Instance().GetResource((J_RESOURCE_TYPE)mainType, objGuid) != nullptr;
			else if (mainType == (int)J_OBJECT_TYPE::DIRECTORY_OBJECT)
				return JResourceManager::Instance().GetDirectory(objGuid) != nullptr;
			else
				return false;
		}
		J_EDITOR_EVENT JEditorSelectObjectEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::SELECT_OBJECT;
		}
		JObject* JEditorSelectObjectEvStruct::GetJObject()noexcept
		{
			if (mainType == (int)J_OBJECT_TYPE::GAME_OBJECT)
				return Core::JReflectionInfo::Instance().GetTypeInfo(JGameObject::TypeName())->GetInstance(objGuid);
			else if (mainType == (int)J_OBJECT_TYPE::RESOURCE_OBJECT)
				return JResourceManager::Instance().GetResource((J_RESOURCE_TYPE)mainType, objGuid);
			else if (mainType == (int)J_OBJECT_TYPE::DIRECTORY_OBJECT)
				return JResourceManager::Instance().GetDirectory(objGuid);
			else
				return nullptr;
		}

		JEditorDeSelectObjectEvStruct::JEditorDeSelectObjectEvStruct(const J_EDITOR_PAGE_TYPE pageType)
			:pageType(pageType)
		{}
		bool JEditorDeSelectObjectEvStruct::PassDefectInspection()const noexcept
		{
			return JImGuiImpl::GetSelectedObj(pageType) != nullptr;
		}
		J_EDITOR_EVENT JEditorDeSelectObjectEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::DESELECT_OBJECT;
		}

		JEditorOpenPageEvStruct::JEditorOpenPageEvStruct(const J_EDITOR_PAGE_TYPE pageType)
			:objTypeName(" "), objGuid(0), pageType(pageType), hasOpenInitObjType(false)
		{}
		JEditorOpenPageEvStruct::JEditorOpenPageEvStruct(const std::string& objTypeName, const size_t objGuid, const J_EDITOR_PAGE_TYPE pageType)
			: objTypeName(objTypeName), objGuid(objGuid), pageType(pageType), hasOpenInitObjType(true)
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
			:pageType(pageType)
		{}
		bool JEditorClosePageEvStruct::PassDefectInspection()const noexcept
		{
			return true;
		}
		J_EDITOR_EVENT JEditorClosePageEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::CLOSE_PAGE;
		}

		JEditorFrontPageEvStruct::JEditorFrontPageEvStruct(JEditorPage* page)
			:page(page)
		{}
		bool JEditorFrontPageEvStruct::PassDefectInspection()const noexcept
		{
			return page != nullptr;
		}
		J_EDITOR_EVENT JEditorFrontPageEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::FRONT_PAGE;
		}

		JEditorBackPageEvStruct::JEditorBackPageEvStruct(JEditorPage* page)
			:page(page)
		{}
		bool JEditorBackPageEvStruct::PassDefectInspection()const noexcept
		{
			return page != nullptr;
		}
		J_EDITOR_EVENT JEditorBackPageEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::BACK_PAGE;
		}

		JEditorActPageEvStruct::JEditorActPageEvStruct(JEditorPage* actPage)
			:actPage(actPage)
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
			:deActPage(deActPage)
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
			:focusPage(focusPage)
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
			:unFocusPage(unFocusPage)
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
			:openWindowName(openWindowName), pageType(pageType)
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
			:closeWindowName(closeWindowName), pageType(pageType)
		{}
		bool JEditorCloseWindowEvStruct::PassDefectInspection()const noexcept
		{
			return !closeWindowName.empty();
		}
		J_EDITOR_EVENT JEditorCloseWindowEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::CLOSE_WINDOW;
		}

		JEditorFrontWindowEvStruct::JEditorFrontWindowEvStruct(JEditorWindow* frontWindow, const J_EDITOR_PAGE_TYPE pageType)
			:frontWindow(frontWindow), pageType(pageType)
		{}
		bool JEditorFrontWindowEvStruct::PassDefectInspection()const noexcept
		{
			return frontWindow != nullptr;
		}
		J_EDITOR_EVENT JEditorFrontWindowEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::FRONT_WINDOW;
		}

		JEditorBackWindowEvStruct::JEditorBackWindowEvStruct(JEditorWindow* backWindow, const J_EDITOR_PAGE_TYPE pageType)
			:backWindow(backWindow), pageType(pageType)
		{}
		bool JEditorBackWindowEvStruct::PassDefectInspection()const noexcept
		{
			return backWindow != nullptr;
		}
		J_EDITOR_EVENT JEditorBackWindowEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::BACK_WINDOW;
		}

		JEditorActWindowEvStruct::JEditorActWindowEvStruct(JEditorWindow* actWindow, const J_EDITOR_PAGE_TYPE pageType)
			:actWindow(actWindow), pageType(pageType)
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
			:deActWindow(deActWindow), pageType(pageType)
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
			:focusWindow(focusWindow), pageType(pageType)
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
			:unFocusWindow(unFocusWindow), pageType(pageType)
		{}
		bool JEditorUnFocusWindowEvStruct::PassDefectInspection()const noexcept
		{
			return unFocusWindow != nullptr;
		}
		J_EDITOR_EVENT JEditorUnFocusWindowEvStruct::GetEventType()const noexcept
		{
			return J_EDITOR_EVENT::UNFOCUS_WINDOW;
		}
	}
}