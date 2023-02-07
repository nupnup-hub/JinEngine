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
		class JEditorPageShareData
		{
		public:
			using GetPageFlagF = Core::JMFunctorType<JEditorPage, J_EDITOR_PAGE_FLAG>;
		public:
			struct PageInitData
			{
			public:
				size_t guiWindowID;
				size_t dockSpaceID;
				J_EDITOR_PAGE_TYPE pageType;
				std::unique_ptr<GetPageFlagF::Functor> getPtr;
			};
	 		public:
			static void RegisterPage(PageInitData& initData)noexcept;
			static void UnRegisterPage(const J_EDITOR_PAGE_TYPE pageType)noexcept;
			static void ClearPageData(const J_EDITOR_PAGE_TYPE pageType)noexcept;
		public:
			static bool IsRegisteredPage(const J_EDITOR_PAGE_TYPE pageType)noexcept;
			static bool HasValidOpenPageData(const J_EDITOR_PAGE_TYPE pageType)noexcept;
		public:
			static uint GetPageGuiWindowID(const J_EDITOR_PAGE_TYPE pageType)noexcept;
			static uint GetPageDockSpaceID(const J_EDITOR_PAGE_TYPE pageType)noexcept;
			static JEditorOpenPageEvStruct GetOpendPageData(const J_EDITOR_PAGE_TYPE pageType)noexcept;
			static Core::JUserPtr<JObject> GetSelectedObj(const J_EDITOR_PAGE_TYPE pageType)noexcept;
			static J_EDITOR_PAGE_FLAG GetPageFlag(const J_EDITOR_PAGE_TYPE pageType)noexcept;
			static size_t* GetDragGuidPtr(const J_EDITOR_PAGE_TYPE pageType)noexcept; 
			static void SetPageOpenData(JEditorOpenPageEvStruct* evStruct)noexcept;
			static void SetSelectObj(const J_EDITOR_PAGE_TYPE pageType, const Core::JUserPtr<JObject>& selectObj)noexcept;
		public:
			static void Clear()noexcept;
		private:
			static void SetSelectedGameObjectTrigger(JGameObject* gObj, const bool triggerValue);
		};
	}
}