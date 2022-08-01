#include"EditorEventStruct.h"

namespace JinEngine
{
	EditorMouseClickEvStruct::EditorMouseClickEvStruct(const std::string& windowName, const uint clickBtn)
		:windowName(windowName), clickBtn(clickBtn)
	{}
	EditorMouseClickEvStruct::~EditorMouseClickEvStruct(){}
	bool EditorMouseClickEvStruct::PassDefectInspection()const noexcept
	{
		return !windowName.empty();
	}
	EDITOR_EVENT EditorMouseClickEvStruct::GetEventType()const noexcept
	{
		return EDITOR_EVENT::MOUSE_CLICK;
	}

	EditorSelectGameObjectEvStruct::EditorSelectGameObjectEvStruct(JGameObject* gameObject)
		:gameObject(gameObject)
	{}
	EditorSelectGameObjectEvStruct::~EditorSelectGameObjectEvStruct() {}
	bool EditorSelectGameObjectEvStruct::PassDefectInspection()const noexcept
	{
		return gameObject != nullptr;
	}
	EDITOR_EVENT EditorSelectGameObjectEvStruct::GetEventType()const noexcept
	{
		return EDITOR_EVENT::SELECT_GAMEOBJECT;
	}

	EditorDeSelectGameObjectEvStruct::EditorDeSelectGameObjectEvStruct(JGameObject* gameObject)
		:gameObject(gameObject)
	{}
	EditorDeSelectGameObjectEvStruct::~EditorDeSelectGameObjectEvStruct() {}
	bool EditorDeSelectGameObjectEvStruct::PassDefectInspection()const noexcept
	{
		return gameObject != nullptr;
	}
	EDITOR_EVENT EditorDeSelectGameObjectEvStruct::GetEventType()const noexcept
	{
		return EDITOR_EVENT::SELECT_GAMEOBJECT;
	}

	EditorSelectResourceEvStruct::EditorSelectResourceEvStruct(JResourceObject* resourceObject)
		:resourceObject(resourceObject)
	{}
	EditorSelectResourceEvStruct::~EditorSelectResourceEvStruct() {}
	bool EditorSelectResourceEvStruct::PassDefectInspection()const noexcept
	{
		return resourceObject != nullptr;
	}
	EDITOR_EVENT EditorSelectResourceEvStruct::GetEventType()const noexcept
	{
		return EDITOR_EVENT::SELECT_RESOURCE;
	}

	EditorDeSelectResourceEvStruct::EditorDeSelectResourceEvStruct(JResourceObject* resourceObject)
		:resourceObject(resourceObject)
	{}
	EditorDeSelectResourceEvStruct::~EditorDeSelectResourceEvStruct() {}
	bool EditorDeSelectResourceEvStruct::PassDefectInspection()const noexcept
	{
		return resourceObject != nullptr;
	}
	EDITOR_EVENT EditorDeSelectResourceEvStruct::GetEventType()const noexcept
	{
		return EDITOR_EVENT::SELECT_RESOURCE;
	}

	EditorOpenPageEvStruct::EditorOpenPageEvStruct(const std::string& openPageName)
		:openPageName(openPageName)
	{}
	EditorOpenPageEvStruct::~EditorOpenPageEvStruct() {}
	bool EditorOpenPageEvStruct::PassDefectInspection()const noexcept
	{
		return !openPageName.empty();
	}
	EDITOR_EVENT EditorOpenPageEvStruct::GetEventType()const noexcept
	{
		return EDITOR_EVENT::OPEN_PAGE;
	}

	EditorClosePageEvStruct::EditorClosePageEvStruct(const std::string& closePageName)
		:closePageName(closePageName)
	{}
	EditorClosePageEvStruct::~EditorClosePageEvStruct() {}
	bool EditorClosePageEvStruct::PassDefectInspection()const noexcept
	{
		return !closePageName.empty();
	}
	EDITOR_EVENT EditorClosePageEvStruct::GetEventType()const noexcept
	{
		return EDITOR_EVENT::CLOSE_PAGE;
	}

	EditorOpenWindowEvStruct::EditorOpenWindowEvStruct(const std::string& openWindowName, const bool actFocus)
		:openWindowName(openWindowName), actFocus(actFocus)
	{}
	EditorOpenWindowEvStruct::~EditorOpenWindowEvStruct() {}
	bool EditorOpenWindowEvStruct::PassDefectInspection()const noexcept
	{
		return !openWindowName.empty();
	}
	EDITOR_EVENT EditorOpenWindowEvStruct::GetEventType()const noexcept
	{
		return EDITOR_EVENT::OPEN_WINDOW;
	}

	EditorCloseWindowEvStruct::EditorCloseWindowEvStruct(const std::string& closeWindowName)
		:closeWindowName(closeWindowName)
	{}
	EditorCloseWindowEvStruct::~EditorCloseWindowEvStruct() {}
	bool EditorCloseWindowEvStruct::PassDefectInspection()const noexcept
	{
		return !closeWindowName.empty();
	}
	EDITOR_EVENT EditorCloseWindowEvStruct::GetEventType()const noexcept
	{
		return EDITOR_EVENT::CLOSE_WINDOW;
	}

	EditorFocusWindowEvStruct::EditorFocusWindowEvStruct(EditorWindow* focusWindow, EditorUtility* editorUtility)
		:focusWindow(focusWindow), editorUtility(editorUtility)
	{}
	EditorFocusWindowEvStruct::~EditorFocusWindowEvStruct() {}
	bool EditorFocusWindowEvStruct::PassDefectInspection()const noexcept
	{
		return focusWindow != nullptr && editorUtility != nullptr;
	}
	EDITOR_EVENT EditorFocusWindowEvStruct::GetEventType()const noexcept
	{
		return EDITOR_EVENT::FOCUS_WINDOW;
	}

	EditorActWindowEvStruct::EditorActWindowEvStruct(const std::string& actWindowName, const bool actFocus)
		:actWindowName(actWindowName), actFocus(actFocus)
	{}
	EditorActWindowEvStruct::~EditorActWindowEvStruct() {}
	bool EditorActWindowEvStruct::PassDefectInspection()const noexcept
	{
		return !actWindowName.empty();
	}
	EDITOR_EVENT EditorActWindowEvStruct::GetEventType()const noexcept
	{
		return EDITOR_EVENT::ACTIVATE_WINDOW;
	}

	EditorDeActWindowEvStruct::EditorDeActWindowEvStruct(const std::string& deActWindowName)
		:deActWindowName(deActWindowName)
	{}
	EditorDeActWindowEvStruct::~EditorDeActWindowEvStruct() {}
	bool EditorDeActWindowEvStruct::PassDefectInspection()const noexcept
	{
		return !deActWindowName.empty();
	}
	EDITOR_EVENT EditorDeActWindowEvStruct::GetEventType()const noexcept
	{
		return EDITOR_EVENT::DEACTIVATE_WINDOW;
	}
}
