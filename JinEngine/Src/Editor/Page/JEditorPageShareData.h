#pragma once
#include"JEditorPageEnum.h"
#include"../Event/JEditorEventType.h"   
#include"../Event/JEditorEventStruct.h"  
#include<memory>

namespace JinEngine
{
	class JGameObject;
	namespace Editor
	{
		class JEditorPage;
		class JEditorSelectedObjInfo
		{
		public:
			J_EDITOR_WINDOW_TYPE selectedWnd;
			JUserPtr<Core::JIdentifier> selectedObj;
		public:
			JEditorSelectedObjInfo(J_EDITOR_WINDOW_TYPE selectedWnd, JUserPtr<Core::JIdentifier> selectedObj);
		};

		class JEditorPageShareData
		{
		public:
			using GetPageNameF = Core::JMFunctorType<JEditorPage, std::string>;
			using GetPageFlagF = Core::JMFunctorType<JEditorPage, J_EDITOR_PAGE_FLAG>; 
		public:
			struct PageInitData
			{
			public:
				size_t guiWindowID;
				size_t dockSpaceID;
				J_EDITOR_PAGE_TYPE pageType;
				std::unique_ptr<GetPageNameF::Functor> getNamePtr;
				std::unique_ptr<GetPageFlagF::Functor> getFlagPtr;
			};
	 		public:
			static void RegisterPage(PageInitData& initData)noexcept;
			static void UnRegisterPage(const J_EDITOR_PAGE_TYPE pageType)noexcept;
			static void ClearPageData(const J_EDITOR_PAGE_TYPE pageType)noexcept;
		public:
			static bool IsRegisteredPage(const J_EDITOR_PAGE_TYPE pageType)noexcept;
			static bool HasValidOpenPageData(const J_EDITOR_PAGE_TYPE pageType)noexcept;
			//static bool IsEditableSelectedObject(JUserPtr<Core::JIdentifier> obj)noexcept;
		public:
			static uint GetPageGuiWindowID(const J_EDITOR_PAGE_TYPE pageType)noexcept;
			static uint GetPageDockSpaceID(const J_EDITOR_PAGE_TYPE pageType)noexcept;
			static JEditorOpenPageEvStruct GetOpendPageData(const J_EDITOR_PAGE_TYPE pageType)noexcept;	 
			static std::string GetPageName(const J_EDITOR_PAGE_TYPE pageType)noexcept;
			static J_EDITOR_PAGE_FLAG GetPageFlag(const J_EDITOR_PAGE_TYPE pageType)noexcept; 
			static void SetPageOpenData(JEditorOpenPageEvStruct* evStruct)noexcept;  
		public:
			//Dragging 
			static Core::JTypeInstanceSearchHint* RegisterDraggingHint(const J_EDITOR_PAGE_TYPE pageType, Core::JIdentifier* iden)noexcept;
			static void DeRegisterDraggingHint(const J_EDITOR_PAGE_TYPE pageType)noexcept;
		public:
			static void Clear()noexcept; 
		};
	}
}