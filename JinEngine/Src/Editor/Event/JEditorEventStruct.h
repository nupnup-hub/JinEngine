#pragma once
#include<vector>
#include<string>
#include<memory>
#include"JEditorEventType.h"
#include"../Page/JEditorPageEnum.h"
#include"../Popup/JEditorPopupType.h"
#include"../../Core/Undo/JTransition.h"
#include"../../Core/Func/Functor/JFunctor.h"
#include"../../Core/JDataType.h" 
#include"../../Core/Pointer/JOwnerPtr.h"
#include"../../Object/Resource/JResourceType.h"

namespace JinEngine
{ 
	class JObject;
	namespace Editor
	{  
		class JEditorPage;
		class JEditorWindow;
		class JEditorPopupWindow;

		struct JEditorEvStruct
		{
		public:
			const J_EDITOR_PAGE_TYPE pageType;
		public:
			JEditorEvStruct(const J_EDITOR_PAGE_TYPE pageType);
			virtual ~JEditorEvStruct() = default;
		public:
			virtual bool PassDefectInspection()const noexcept = 0;
			virtual J_EDITOR_EVENT GetEventType()const noexcept = 0;
		};
		struct JEditorMouseClickEvStruct : public JEditorEvStruct
		{
		public:
			const std::string windowName;
			//0 Left 1 middle 2right
			const uint clickBtn; 
		public:
			JEditorMouseClickEvStruct(const std::string& windowName, const uint clickBtn, const J_EDITOR_PAGE_TYPE pageType);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorSelectObjectEvStruct : public JEditorEvStruct
		{
		public: 
			const Core::JUserPtr<JObject> selectObj;
		public:
			JEditorSelectObjectEvStruct(const J_EDITOR_PAGE_TYPE pageType, Core::JUserPtr<JObject> selectObj);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final; 
		};
		struct JEditorDeSelectObjectEvStruct : public JEditorEvStruct
		{ 
		public:
			JEditorDeSelectObjectEvStruct(const J_EDITOR_PAGE_TYPE pageType);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final; 
		};

		struct JEditorOpenPageEvStruct : public JEditorEvStruct
		{
		public:   
			const Core::JUserPtr<JObject> openSelected;
		public: 
			JEditorOpenPageEvStruct(const J_EDITOR_PAGE_TYPE pageType, Core::JUserPtr<JObject> openSelected = {});
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorClosePageEvStruct : public JEditorEvStruct
		{ 
		public:
			JEditorClosePageEvStruct(const J_EDITOR_PAGE_TYPE pageType);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorActPageEvStruct : public JEditorEvStruct
		{ 
		public:
			JEditorActPageEvStruct(const J_EDITOR_PAGE_TYPE pageType);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorDeActPageEvStruct : public JEditorEvStruct
		{ 
		public:
			JEditorDeActPageEvStruct(const J_EDITOR_PAGE_TYPE pageType);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorFocusPageEvStruct : public JEditorEvStruct
		{
		public:
			JEditorPage* focusPage;
		public:
			JEditorFocusPageEvStruct(JEditorPage* focusPage);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorUnFocusPageEvStruct : public JEditorEvStruct
		{
		public:
			JEditorPage* unFocusPage;
		public:
			JEditorUnFocusPageEvStruct(JEditorPage* unFocusPage);	
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};

		struct JEditorOpenWindowEvStruct : public JEditorEvStruct
		{
		public:
			const std::string openWindowName; 
		public:
			JEditorOpenWindowEvStruct(const std::string& openWindowName, const J_EDITOR_PAGE_TYPE pageType);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorCloseWindowEvStruct : public JEditorEvStruct
		{
		public:
			const std::string closeWindowName;
		public:
			JEditorCloseWindowEvStruct(const std::string& closeWindowName, const J_EDITOR_PAGE_TYPE pageType);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorActWindowEvStruct : public JEditorEvStruct
		{
		public:
			JEditorWindow* actWindow;  
		public:
			JEditorActWindowEvStruct(JEditorWindow* actWindow, const J_EDITOR_PAGE_TYPE pageType);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorDeActWindowEvStruct : public JEditorEvStruct
		{
		public:
			JEditorWindow* deActWindow; 
		public:
			JEditorDeActWindowEvStruct(JEditorWindow* deActWindow, const J_EDITOR_PAGE_TYPE pageType);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};

		struct JEditorFocusWindowEvStruct : public JEditorEvStruct
		{
		public:
			JEditorWindow* focusWindow; 
		public:
			JEditorFocusWindowEvStruct(JEditorWindow* focusWindow, const J_EDITOR_PAGE_TYPE pageType);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};
		struct JEditorUnFocusWindowEvStruct : public JEditorEvStruct
		{
		public:
			JEditorWindow* unFocusWindow; 
		public:
			JEditorUnFocusWindowEvStruct(JEditorWindow* unFocusWindow, const J_EDITOR_PAGE_TYPE pageType);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};

		struct JEditorOpenPopupWindowEvStruct : public JEditorEvStruct
		{
		public:
			using PopupWndFuncTuple = std::tuple <J_EDITOR_POPUP_WINDOW_FUNC_TYPE, std::unique_ptr< Core::JBindHandleBase>>;
		public:
			JEditorPopupWindow* popupWindow;
			std::vector<PopupWndFuncTuple> tupleVec;
			std::string desc;
		public:
			JEditorOpenPopupWindowEvStruct(JEditorPopupWindow* popupWindow, const J_EDITOR_PAGE_TYPE pageType);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		public:
			void AddBind(const J_EDITOR_POPUP_WINDOW_FUNC_TYPE type, std::unique_ptr< Core::JBindHandleBase>&& bind);
			void AddDesc(const std::string& newDesc);
		};
		struct JEditorClosePopupWindowEvStruct : public JEditorEvStruct
		{
		public:
			JEditorPopupWindow* popupWindow;
		public:
			JEditorClosePopupWindowEvStruct(JEditorPopupWindow* popupWindow, const J_EDITOR_PAGE_TYPE pageType);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		};

		struct JEditorBindFuncEvStruct : public JEditorEvStruct
		{
		private:
			std::unique_ptr<Core::JBindHandleBase> bindHandle;
		public:
			JEditorBindFuncEvStruct(std::unique_ptr<Core::JBindHandleBase> bindHandle, const J_EDITOR_PAGE_TYPE pageType);
		public:
			bool PassDefectInspection()const noexcept final;
			J_EDITOR_EVENT GetEventType()const noexcept final;
		public:
			void Execute();
		};

		//support redo undo
		struct JEditorTBindFuncEvStruct : public JEditorEvStruct
		{
		public:
			const std::string taskName; 
		public:
			JEditorTBindFuncEvStruct(const std::string& taskName, const J_EDITOR_PAGE_TYPE pageType);
		public: 
			J_EDITOR_EVENT GetEventType()const noexcept final;
		public:
			virtual void Execute() = 0;
		};

		struct JEditorTSetBindFuncEvStruct : public JEditorTBindFuncEvStruct
		{
		public: 
			std::unique_ptr<Core::JBindHandleBase> doBindHandle;
			std::unique_ptr<Core::JBindHandleBase> undoBindHandle;
		public:
			JEditorTSetBindFuncEvStruct(const std::string& taskName,
				const J_EDITOR_PAGE_TYPE pageType,
				std::unique_ptr<Core::JBindHandleBase> doBindHandle,
				std::unique_ptr<Core::JBindHandleBase> undoBindHandle);
		public:
			bool PassDefectInspection()const noexcept final;
			void Execute() final;
		};

		template<typename DataStructure, typename doHandle, typename undoHandle>
		struct JEditorTCreateBindFuncEvStruct : public JEditorTBindFuncEvStruct
		{
		public:
			std::unique_ptr<doHandle> doBindHandle;
			std::unique_ptr<undoHandle> undoBindHandle;
			DataStructure& structure;
		public:
			JEditorTCreateBindFuncEvStruct(const std::string& taskName,
				const J_EDITOR_PAGE_TYPE pageType,
				std::unique_ptr<doHandle> doBindHandle,
				std::unique_ptr<undoHandle> undoBindHandle,
				DataStructure& structure)
				:JEditorTBindFuncEvStruct(taskName, pageType),
				doBindHandle(std::move(doBindHandle)),
				undoBindHandle(std::move(undoBindHandle)),
				structure(structure)
			{}
		public:
			bool PassDefectInspection()const noexcept final
			{
				return doBindHandle != nullptr && undoBindHandle != nullptr;
			}
			void Execute() final
			{
				using JCreationTask = Core::JTransitionCreationTask< DataStructure, doHandle, undoHandle>;
				Core::JTransition::Execute(std::make_unique<JCreationTask>(taskName,
					std::move(doBindHandle),
					std::move(undoBindHandle),
					structure));
			}
		};
	}
}