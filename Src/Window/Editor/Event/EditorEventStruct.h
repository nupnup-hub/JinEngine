#pragma once
#include<vector>
#include<string>
#include"EditorEventType.h"
#include"../../../Core/JDataType.h"

namespace JinEngine
{
	class EditorUtility;
	class JGameObject;
	class JResourceObject;
	class EditorWindow;

	class EditorEventStruct
	{  
	public:
		EditorEventStruct() = default;
		virtual ~EditorEventStruct() = default;
		EditorEventStruct(const EditorEventStruct& rhs) = default;
		EditorEventStruct& operator=(const EditorEventStruct& rhs) = default;

		virtual bool PassDefectInspection()const noexcept = 0;
		virtual EDITOR_EVENT GetEventType()const noexcept = 0;
	};

	class EditorMouseClickEvStruct : public EditorEventStruct
	{
	public:
		std::string windowName;
		//0 Left 1 middle 2right
		const uint clickBtn;
	public:
		EditorMouseClickEvStruct(const std::string& windowName, const uint clickBtn);
		~EditorMouseClickEvStruct();
		EditorMouseClickEvStruct(const EditorMouseClickEvStruct& rhs) = default;
		EditorMouseClickEvStruct& operator=(const EditorMouseClickEvStruct& rhs) = default;

		bool PassDefectInspection()const noexcept final;
		EDITOR_EVENT GetEventType()const noexcept final;
	};
	class EditorSelectGameObjectEvStruct : public EditorEventStruct
	{
	public:
		JGameObject* gameObject;
	public:
		EditorSelectGameObjectEvStruct(JGameObject* gameObject);
		~EditorSelectGameObjectEvStruct();
		EditorSelectGameObjectEvStruct(const EditorSelectGameObjectEvStruct& rhs) = default;
		EditorSelectGameObjectEvStruct& operator=(const EditorSelectGameObjectEvStruct& rhs) = default;

		bool PassDefectInspection()const noexcept final;
		EDITOR_EVENT GetEventType()const noexcept final;
	};
	class EditorDeSelectGameObjectEvStruct : public EditorEventStruct
	{
	public:
		JGameObject* gameObject;
	public:
		EditorDeSelectGameObjectEvStruct(JGameObject* gameObject);
		~EditorDeSelectGameObjectEvStruct();
		EditorDeSelectGameObjectEvStruct(const EditorDeSelectGameObjectEvStruct& rhs) = default;
		EditorDeSelectGameObjectEvStruct& operator=(const EditorDeSelectGameObjectEvStruct& rhs) = default;

		bool PassDefectInspection()const noexcept final;
		EDITOR_EVENT GetEventType()const noexcept final;
	};
	class EditorSelectResourceEvStruct : public EditorEventStruct
	{
	public:
		JResourceObject* resourceObject;
	public:
		EditorSelectResourceEvStruct(JResourceObject* resourceObject);
		~EditorSelectResourceEvStruct();
		EditorSelectResourceEvStruct(const EditorSelectResourceEvStruct& rhs) = default;
		EditorSelectResourceEvStruct& operator=(const EditorSelectResourceEvStruct& rhs) = default;

		bool PassDefectInspection()const noexcept final;
		EDITOR_EVENT GetEventType()const noexcept final;
	};
	class EditorDeSelectResourceEvStruct : public EditorEventStruct
	{
	public:
		JResourceObject* resourceObject;
	public:
		EditorDeSelectResourceEvStruct(JResourceObject* resourceObject);
		~EditorDeSelectResourceEvStruct();
		EditorDeSelectResourceEvStruct(const EditorDeSelectResourceEvStruct& rhs) = default;
		EditorDeSelectResourceEvStruct& operator=(const EditorDeSelectResourceEvStruct& rhs) = default;

		bool PassDefectInspection()const noexcept final;
		EDITOR_EVENT GetEventType()const noexcept final;
	};
	class EditorOpenPageEvStruct : public EditorEventStruct
	{
	public:
		std::string openPageName;
	public:
		EditorOpenPageEvStruct(const std::string& openPageName);
		~EditorOpenPageEvStruct();
		EditorOpenPageEvStruct(const EditorOpenPageEvStruct& rhs) = default;
		EditorOpenPageEvStruct& operator=(const EditorOpenPageEvStruct& rhs) = default;

		bool PassDefectInspection()const noexcept final;
		EDITOR_EVENT GetEventType()const noexcept final;
	};
	class EditorClosePageEvStruct : public EditorEventStruct
	{
	public:
		std::string closePageName;
	public:
		EditorClosePageEvStruct(const std::string& closePageName);
		~EditorClosePageEvStruct();
		EditorClosePageEvStruct(const EditorClosePageEvStruct& rhs) = default;
		EditorClosePageEvStruct& operator=(const EditorClosePageEvStruct& rhs) = default;

		bool PassDefectInspection()const noexcept final;
		EDITOR_EVENT GetEventType()const noexcept final;
	};

	class EditorOpenWindowEvStruct : public EditorEventStruct
	{
	public:
		std::string openWindowName;
		bool actFocus;
	public:
		EditorOpenWindowEvStruct(const std::string& openWindowName, const bool actFocus);
		~EditorOpenWindowEvStruct();
		EditorOpenWindowEvStruct(const EditorOpenWindowEvStruct& rhs) = default;
		EditorOpenWindowEvStruct& operator=(const EditorOpenWindowEvStruct& rhs) = default;

		bool PassDefectInspection()const noexcept final;
		EDITOR_EVENT GetEventType()const noexcept final;
	};
	class EditorCloseWindowEvStruct : public EditorEventStruct
	{
	public:
		std::string closeWindowName; 
	public:
		EditorCloseWindowEvStruct(const std::string& closeWindowName);
		~EditorCloseWindowEvStruct();
		EditorCloseWindowEvStruct(const EditorCloseWindowEvStruct& rhs) = default;
		EditorCloseWindowEvStruct& operator=(const EditorCloseWindowEvStruct& rhs) = default;

		bool PassDefectInspection()const noexcept final;
		EDITOR_EVENT GetEventType()const noexcept final;
	};
	class EditorFocusWindowEvStruct : public EditorEventStruct
	{
	public:
		EditorWindow* focusWindow;
		EditorUtility* editorUtility;
	public:
		EditorFocusWindowEvStruct(EditorWindow* focusWindow, EditorUtility* editorUtility);
		~EditorFocusWindowEvStruct();
		EditorFocusWindowEvStruct(const EditorFocusWindowEvStruct& rhs) = default;
		EditorFocusWindowEvStruct& operator=(const EditorFocusWindowEvStruct& rhs) = default;

		bool PassDefectInspection()const noexcept final;
		EDITOR_EVENT GetEventType()const noexcept final;
	};

	class EditorActWindowEvStruct : public EditorEventStruct
	{
	public:
		std::string actWindowName;
		bool actFocus;
	public:
		EditorActWindowEvStruct(const std::string& actWindowName, const bool actFocus);
		~EditorActWindowEvStruct();
		EditorActWindowEvStruct(const EditorActWindowEvStruct& rhs) = default;
		EditorActWindowEvStruct& operator=(const EditorActWindowEvStruct& rhs) = default;

		bool PassDefectInspection()const noexcept final;
		EDITOR_EVENT GetEventType()const noexcept final;
	};
	class EditorDeActWindowEvStruct : public EditorEventStruct
	{
	public:
		std::string deActWindowName;
	public:
		EditorDeActWindowEvStruct(const std::string& deActWindowName);
		~EditorDeActWindowEvStruct();
		EditorDeActWindowEvStruct(const EditorDeActWindowEvStruct& rhs) = default;
		EditorDeActWindowEvStruct& operator=(const EditorDeActWindowEvStruct& rhs) = default;

		bool PassDefectInspection()const noexcept final;
		EDITOR_EVENT GetEventType()const noexcept final;
	};
}