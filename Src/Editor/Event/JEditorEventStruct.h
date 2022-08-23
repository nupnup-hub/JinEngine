#pragma once
#include<vector>
#include<string>
#include"JEditorEventType.h"
#include"../Page/JEditorPageType.h"
#include"../../Core/JDataType.h" 
#include"../../Object/Resource/JResourceType.h"

namespace JinEngine
{ 
	class JGameObject;
	class JResourceObject;
	class JDirectory;

	namespace Editor
	{  
		class JEditorPage;
		class JEditorWindow;

		struct JEditorEventStruct
		{
		public:
			virtual ~JEditorEventStruct() = default;
		public:
			virtual bool PassDefectInspection()const noexcept = 0;
			virtual J_EDITOR_EVENT GetEventType()const noexcept = 0;
		};
		struct JEditorMouseClickEvStruct : public JEditorEventStruct
		{
		public:
			const std::string windowName;
			//0 Left 1 middle 2right
			const uint clickBtn;
			const J_EDITOR_PAGE_TYPE pageType;
		public:
			JEditorMouseClickEvStruct(const std::string& windowName, const uint clickBtn, const J_EDITOR_PAGE_TYPE pageType);
			~JEditorMouseClickEvStruct() = default;
			JEditorMouseClickEvStruct(const JEditorMouseClickEvStruct& rhs) = default;
			JEditorMouseClickEvStruct& operator=(const JEditorMouseClickEvStruct& rhs) = default;
			JEditorMouseClickEvStruct(JEditorMouseClickEvStruct&& rhs) = default;
			JEditorMouseClickEvStruct& operator=(JEditorMouseClickEvStruct&& rhs) = default;
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorSelectObjectEvStruct : public JEditorEventStruct
		{
		public:
			const std::string objName;
			const size_t objGuid;
			const int mainType;
			const int subType;
			const J_EDITOR_PAGE_TYPE pageType;
		public:
			JEditorSelectObjectEvStruct(JGameObject& gObj, const J_EDITOR_PAGE_TYPE pageType);
			JEditorSelectObjectEvStruct(JResourceObject& rObj, const J_EDITOR_PAGE_TYPE pageType);
			JEditorSelectObjectEvStruct(JDirectory& dObj, const J_EDITOR_PAGE_TYPE pageType);
			~JEditorSelectObjectEvStruct() = default;
			JEditorSelectObjectEvStruct(const JEditorSelectObjectEvStruct& rhs) = default;
			JEditorSelectObjectEvStruct& operator=(const JEditorSelectObjectEvStruct& rhs) = default;
			JEditorSelectObjectEvStruct(JEditorSelectObjectEvStruct&& rhs) = default;
			JEditorSelectObjectEvStruct& operator=(JEditorSelectObjectEvStruct&& rhs) = default;
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
			JObject* GetJObject()noexcept;
		};
		struct JEditorDeSelectObjectEvStruct : public JEditorEventStruct
		{
		public: 
			const J_EDITOR_PAGE_TYPE pageType;
		public:
			JEditorDeSelectObjectEvStruct(const J_EDITOR_PAGE_TYPE pageType); 
			~JEditorDeSelectObjectEvStruct() = default;
			JEditorDeSelectObjectEvStruct(const JEditorDeSelectObjectEvStruct& rhs) = default;
			JEditorDeSelectObjectEvStruct& operator=(const JEditorDeSelectObjectEvStruct& rhs) = default;
			JEditorDeSelectObjectEvStruct(JEditorDeSelectObjectEvStruct&& rhs) = default;
			JEditorDeSelectObjectEvStruct& operator=(JEditorDeSelectObjectEvStruct&& rhs) = default;
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final; 
		};

		struct JEditorOpenPageEvStruct : public JEditorEventStruct
		{
		public:  
			const std::string objTypeName;
			const size_t objGuid; 
			const J_EDITOR_PAGE_TYPE pageType;
			const bool hasOpenInitObjType;
		public:
			JEditorOpenPageEvStruct(const J_EDITOR_PAGE_TYPE pageType);
			JEditorOpenPageEvStruct(const std::string& objTypeName, const size_t objGuid, const J_EDITOR_PAGE_TYPE pageType);
			~JEditorOpenPageEvStruct() = default;
			JEditorOpenPageEvStruct(const JEditorOpenPageEvStruct& rhs) = default;
			JEditorOpenPageEvStruct& operator=(const JEditorOpenPageEvStruct& rhs) = default;
			JEditorOpenPageEvStruct(JEditorOpenPageEvStruct&& rhs) = default;
			JEditorOpenPageEvStruct& operator=(JEditorOpenPageEvStruct&& rhs) = default;
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorClosePageEvStruct : public JEditorEventStruct
		{
		public:
			const J_EDITOR_PAGE_TYPE pageType;
		public:
			JEditorClosePageEvStruct(const J_EDITOR_PAGE_TYPE pageType);
			~JEditorClosePageEvStruct() = default;
			JEditorClosePageEvStruct(const JEditorClosePageEvStruct& rhs) = default;
			JEditorClosePageEvStruct& operator=(const JEditorClosePageEvStruct& rhs) = default;
			JEditorClosePageEvStruct(JEditorClosePageEvStruct&& rhs) = default;
			JEditorClosePageEvStruct& operator=(JEditorClosePageEvStruct&& rhs) = default;
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorFrontPageEvStruct : public JEditorEventStruct
		{
		public:
			JEditorPage* page;
		public:
			JEditorFrontPageEvStruct(JEditorPage* page); 
			~JEditorFrontPageEvStruct() = default;
			JEditorFrontPageEvStruct(const JEditorFrontPageEvStruct& rhs) = default;
			JEditorFrontPageEvStruct& operator=(const JEditorFrontPageEvStruct& rhs) = default;
			JEditorFrontPageEvStruct(JEditorFrontPageEvStruct&& rhs) = default;
			JEditorFrontPageEvStruct& operator=(JEditorFrontPageEvStruct&& rhs) = default;
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorBackPageEvStruct : public JEditorEventStruct
		{
		public:
			JEditorPage* page;
		public:
			JEditorBackPageEvStruct(JEditorPage* page);
			~JEditorBackPageEvStruct() = default;
			JEditorBackPageEvStruct(const JEditorBackPageEvStruct& rhs) = default;
			JEditorBackPageEvStruct& operator=(const JEditorBackPageEvStruct& rhs) = default;
			JEditorBackPageEvStruct(JEditorBackPageEvStruct&& rhs) = default;
			JEditorBackPageEvStruct& operator=(JEditorBackPageEvStruct&& rhs) = default;
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorActPageEvStruct : public JEditorEventStruct
		{
		public:
			JEditorPage* actPage;
		public:
			JEditorActPageEvStruct(JEditorPage* actPage);
			~JEditorActPageEvStruct() = default;
			JEditorActPageEvStruct(const JEditorActPageEvStruct& rhs) = default;
			JEditorActPageEvStruct& operator=(const JEditorActPageEvStruct& rhs) = default;
			JEditorActPageEvStruct(JEditorActPageEvStruct&& rhs) = default;
			JEditorActPageEvStruct& operator=(JEditorActPageEvStruct&& rhs) = default;
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorDeActPageEvStruct : public JEditorEventStruct
		{
		public:
			JEditorPage* deActPage;
		public:
			JEditorDeActPageEvStruct(JEditorPage* deActPage);
			~JEditorDeActPageEvStruct() = default;
			JEditorDeActPageEvStruct(const JEditorDeActPageEvStruct & rhs) = default;
			JEditorDeActPageEvStruct& operator=(const JEditorDeActPageEvStruct & rhs) = default;
			JEditorDeActPageEvStruct(JEditorDeActPageEvStruct&& rhs) = default;
			JEditorDeActPageEvStruct& operator=(JEditorDeActPageEvStruct&& rhs) = default;
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorFocusPageEvStruct : public JEditorEventStruct
		{
		public:
			JEditorPage* focusPage;
		public:
			JEditorFocusPageEvStruct(JEditorPage* focusPage);
			~JEditorFocusPageEvStruct() = default;
			JEditorFocusPageEvStruct(const JEditorFocusPageEvStruct& rhs) = default;
			JEditorFocusPageEvStruct& operator=(const JEditorFocusPageEvStruct& rhs) = default;
			JEditorFocusPageEvStruct(JEditorFocusPageEvStruct&& rhs) = default;
			JEditorFocusPageEvStruct& operator=(JEditorFocusPageEvStruct&& rhs) = default;
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorUnFocusPageEvStruct : public JEditorEventStruct
		{
		public:
			JEditorPage* unFocusPage;
		public:
			JEditorUnFocusPageEvStruct(JEditorPage* unFocusPage);
			~JEditorUnFocusPageEvStruct() = default;
			JEditorUnFocusPageEvStruct(const JEditorUnFocusPageEvStruct& rhs) = default;
			JEditorUnFocusPageEvStruct& operator=(const JEditorUnFocusPageEvStruct& rhs) = default;
			JEditorUnFocusPageEvStruct(JEditorUnFocusPageEvStruct&& rhs) = default;
			JEditorUnFocusPageEvStruct& operator=(JEditorUnFocusPageEvStruct&& rhs) = default;
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};

		struct JEditorOpenWindowEvStruct : public JEditorEventStruct
		{
		public:
			const std::string openWindowName;
			const J_EDITOR_PAGE_TYPE pageType;
		public:
			JEditorOpenWindowEvStruct(const std::string& openWindowName, const J_EDITOR_PAGE_TYPE pageType);
			~JEditorOpenWindowEvStruct() = default;
			JEditorOpenWindowEvStruct(const JEditorOpenWindowEvStruct& rhs) = default;
			JEditorOpenWindowEvStruct& operator=(const JEditorOpenWindowEvStruct& rhs) = default;
			JEditorOpenWindowEvStruct(JEditorOpenWindowEvStruct&& rhs) = default;
			JEditorOpenWindowEvStruct& operator=(JEditorOpenWindowEvStruct&& rhs) = default;
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorCloseWindowEvStruct : public JEditorEventStruct
		{
		public:
			const std::string closeWindowName;
			const J_EDITOR_PAGE_TYPE pageType;
		public:
			JEditorCloseWindowEvStruct(const std::string& closeWindowName, const J_EDITOR_PAGE_TYPE pageType);
			~JEditorCloseWindowEvStruct() = default;
			JEditorCloseWindowEvStruct(const JEditorCloseWindowEvStruct& rhs) = default;
			JEditorCloseWindowEvStruct& operator=(const JEditorCloseWindowEvStruct& rhs) = default;
			JEditorCloseWindowEvStruct(JEditorCloseWindowEvStruct&& rhs) = default;
			JEditorCloseWindowEvStruct& operator=(JEditorCloseWindowEvStruct&& rhs) = default;
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorFrontWindowEvStruct : public JEditorEventStruct
		{
		public:
			JEditorWindow* frontWindow;
			const J_EDITOR_PAGE_TYPE pageType;
		public:
			JEditorFrontWindowEvStruct(JEditorWindow* frontWindow, const J_EDITOR_PAGE_TYPE pageType);
			~JEditorFrontWindowEvStruct() = default;
			JEditorFrontWindowEvStruct(const JEditorFrontWindowEvStruct& rhs) = default;
			JEditorFrontWindowEvStruct& operator=(const JEditorFrontWindowEvStruct& rhs) = default;
			JEditorFrontWindowEvStruct(JEditorFrontWindowEvStruct&& rhs) = default;
			JEditorFrontWindowEvStruct& operator=(JEditorFrontWindowEvStruct&& rhs) = default;
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorBackWindowEvStruct : public JEditorEventStruct
		{
		public:
			JEditorWindow* backWindow;
			const J_EDITOR_PAGE_TYPE pageType;
		public:
			JEditorBackWindowEvStruct(JEditorWindow* backWindow, const J_EDITOR_PAGE_TYPE pageType);
			~JEditorBackWindowEvStruct() = default;
			JEditorBackWindowEvStruct(const JEditorBackWindowEvStruct& rhs) = default;
			JEditorBackWindowEvStruct& operator=(const JEditorBackWindowEvStruct& rhs) = default;
			JEditorBackWindowEvStruct(JEditorBackWindowEvStruct&& rhs) = default;
			JEditorBackWindowEvStruct& operator=(JEditorBackWindowEvStruct&& rhs) = default;
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};	 
		struct JEditorActWindowEvStruct : public JEditorEventStruct
		{
		public:
			JEditorWindow* actWindow; 
			const J_EDITOR_PAGE_TYPE pageType;
		public:
			JEditorActWindowEvStruct(JEditorWindow* actWindow, const J_EDITOR_PAGE_TYPE pageType);
			~JEditorActWindowEvStruct() = default;
			JEditorActWindowEvStruct(const JEditorActWindowEvStruct& rhs) = default;
			JEditorActWindowEvStruct& operator=(const JEditorActWindowEvStruct& rhs) = default;
			JEditorActWindowEvStruct(JEditorActWindowEvStruct&& rhs) = default;
			JEditorActWindowEvStruct& operator=(JEditorActWindowEvStruct&& rhs) = default;
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorDeActWindowEvStruct : public JEditorEventStruct
		{
		public:
			JEditorWindow* deActWindow;
			const J_EDITOR_PAGE_TYPE pageType;
		public:
			JEditorDeActWindowEvStruct(JEditorWindow* deActWindow, const J_EDITOR_PAGE_TYPE pageType);
			~JEditorDeActWindowEvStruct() = default;
			JEditorDeActWindowEvStruct(const JEditorDeActWindowEvStruct& rhs) = default;
			JEditorDeActWindowEvStruct& operator=(const JEditorDeActWindowEvStruct& rhs) = default;
			JEditorDeActWindowEvStruct(JEditorDeActWindowEvStruct&& rhs) = default;
			JEditorDeActWindowEvStruct& operator=(JEditorDeActWindowEvStruct&& rhs) = default;
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};

		struct JEditorFocusWindowEvStruct : public JEditorEventStruct
		{
		public:
			JEditorWindow* focusWindow;
			const J_EDITOR_PAGE_TYPE pageType;
		public:
			JEditorFocusWindowEvStruct(JEditorWindow* focusWindow, const J_EDITOR_PAGE_TYPE pageType);
			~JEditorFocusWindowEvStruct() = default;
			JEditorFocusWindowEvStruct(const JEditorFocusWindowEvStruct& rhs) = default;
			JEditorFocusWindowEvStruct& operator=(const JEditorFocusWindowEvStruct& rhs) = default;
			JEditorFocusWindowEvStruct(JEditorFocusWindowEvStruct&& rhs) = default;
			JEditorFocusWindowEvStruct& operator=(JEditorFocusWindowEvStruct&& rhs) = default;
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorUnFocusWindowEvStruct : public JEditorEventStruct
		{
		public:
			JEditorWindow* unFocusWindow;
			const J_EDITOR_PAGE_TYPE pageType;
		public:
			JEditorUnFocusWindowEvStruct(JEditorWindow* unFocusWindow, const J_EDITOR_PAGE_TYPE pageType);
			~JEditorUnFocusWindowEvStruct() = default;
			JEditorUnFocusWindowEvStruct(const JEditorUnFocusWindowEvStruct& rhs) = default;
			JEditorUnFocusWindowEvStruct& operator=(const JEditorUnFocusWindowEvStruct& rhs) = default;
			JEditorUnFocusWindowEvStruct(JEditorUnFocusWindowEvStruct&& rhs) = default;
			JEditorUnFocusWindowEvStruct& operator=(JEditorUnFocusWindowEvStruct&& rhs) = default;
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
	}
}