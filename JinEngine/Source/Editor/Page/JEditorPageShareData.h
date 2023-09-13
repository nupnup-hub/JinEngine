#pragma once
#include"JEditorPageEnum.h"
#include"../Event/JEditorEventType.h"   
#include"../Event/JEditorEventStruct.h"  
#include"../Gui/JGuiType.h"
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
			using GetGuiIDF = Core::JMFunctorType<JEditorPage, GuiID>;
			using GetDockSpaceIDF = Core::JMFunctorType<JEditorPage, GuiID>;
			using GetPageNameF = Core::JMFunctorType<JEditorPage, std::string>;
			using GetPageFlagF = Core::JMFunctorType<JEditorPage, J_EDITOR_PAGE_FLAG>;
			using GetPublicBooleanStateF = Core::JMFunctorType<JEditorPage, bool, J_EDITOR_PAGE_PUBLIC_STATE>;
		public:
			struct PageInitData
			{
			public:
				J_EDITOR_PAGE_TYPE pageType;
				std::unique_ptr<GetGuiIDF::Functor> getGuiIDF;
				std::unique_ptr<GetDockSpaceIDF::Functor>getDockSpaceIDF;
				std::unique_ptr<GetPageNameF::Functor> getNameF;
				std::unique_ptr<GetPageNameF::Functor> getDockSpaceNameF;
				std::unique_ptr<GetPageFlagF::Functor> getFlagF;
				std::unique_ptr<GetPublicBooleanStateF::Functor> getPublicStateF;
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
			static GuiID GetPageGuiWindowID(const J_EDITOR_PAGE_TYPE pageType)noexcept;
			static GuiID GetPageDockSpaceID(const J_EDITOR_PAGE_TYPE pageType)noexcept;
			static JEditorOpenPageEvStruct GetOpendPageData(const J_EDITOR_PAGE_TYPE pageType)noexcept;	 
			static std::string GetPageName(const J_EDITOR_PAGE_TYPE pageType)noexcept;
			static std::string GetPageDockSpaceName(const J_EDITOR_PAGE_TYPE pageType)noexcept;
			static J_EDITOR_PAGE_FLAG GetPageFlag(const J_EDITOR_PAGE_TYPE pageType)noexcept; 
			static bool GetPublicState(const J_EDITOR_PAGE_TYPE pageType, const J_EDITOR_PAGE_PUBLIC_STATE state)noexcept;
		public:
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