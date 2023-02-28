#include"JEditorPageShareData.h" 
#include"../Page/JEditorPage.h"
#include"../../Core/Identity/JIdentifier.h"
#include"../../Object/GameObject/JGameObject.h" 
#include<vector>

namespace JinEngine
{
	namespace Editor
	{
		using GetPageFlagF = typename JEditorPageShareData::GetPageFlagF;
		class ShareData
		{
		public:
			const uint guiWindowID;
			const uint dockSpaceID;
		public:
			Core::JUserPtr<Core::JIdentifier> openObject;
			Core::JUserPtr<Core::JIdentifier> selectObj;
			std::unique_ptr<GetPageFlagF::Functor> getFageFlagF;
			size_t dragGuid = 0; 
		public: 
			ShareData(const uint guiWindowID, const uint dockSpaceID, std::unique_ptr<GetPageFlagF::Functor> getFageFlagF)
				:guiWindowID(guiWindowID), dockSpaceID(dockSpaceID), getFageFlagF(std::move(getFageFlagF))
			{}
		};

		namespace
		{
			static std::unique_ptr<ShareData> pageData[(int)J_EDITOR_PAGE_TYPE::COUNT]; 
		}

		void JEditorPageShareData::RegisterPage(PageInitData& initData)noexcept
		{	 
			pageData[(int)initData.pageType] = std::make_unique<ShareData>(initData.guiWindowID, initData.dockSpaceID, std::move(initData.getPtr));
		}
		void JEditorPageShareData::UnRegisterPage(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			pageData[(int)pageType].reset();
		}
		void JEditorPageShareData::ClearPageData(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			auto pagedata = pageData[(int)pageType].get();
			if (pagedata->selectObj.IsValid() && pagedata->selectObj->GetTypeInfo().IsA<JGameObject>())
				SetSelectedGameObjectTrigger(static_cast<JGameObject*>(pagedata->selectObj.Get()), false);
			pagedata->openObject.Clear();
			pagedata->selectObj.Clear();
		}
		bool JEditorPageShareData::IsRegisteredPage(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			return pageData[(int)pageType] != nullptr ? true : false;
		}
		bool JEditorPageShareData::IsEditableSelectedObject(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{ 
			return pageData[(int)pageType]->selectObj.IsValid() &&
				pageData[(int)pageType]->selectObj->GetTypeInfo().IsChildOf<JObject>() &&
				!static_cast<JObject*>(pageData[(int)pageType]->selectObj.Get())->HasFlag(OBJECT_FLAG_UNEDITABLE);
		}
		bool JEditorPageShareData::HasValidOpenPageData(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			if (pageData[(int)pageType] == nullptr)
				return false;

			if (Core::HasSQValueEnum(GetPageFlag(pageType), J_EDITOR_PAGE_REQUIRE_INIT_OBJECT))
				return pageData[(int)pageType]->openObject.IsValid();
			else
				return true;
		}
		uint JEditorPageShareData::GetPageGuiWindowID(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			return pageData[(int)pageType]->guiWindowID;
		}
		uint JEditorPageShareData::GetPageDockSpaceID(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			return pageData[(int)pageType]->dockSpaceID;
		}
		JEditorOpenPageEvStruct JEditorPageShareData::GetOpendPageData(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{ 
			return JEditorOpenPageEvStruct{ pageType, pageData[(int)pageType]->openObject };
		}
		Core::JUserPtr<Core::JIdentifier> JEditorPageShareData::GetSelectedObj(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			return pageData[(int)pageType]->selectObj;
		}
		J_EDITOR_PAGE_FLAG JEditorPageShareData::GetPageFlag(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			return (*pageData[(int)pageType]->getFageFlagF)();
		}
		size_t* JEditorPageShareData::GetDragGuidPtr(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			return &(pageData[(int)pageType]->dragGuid);
		}
		void JEditorPageShareData::SetPageOpenData(JEditorOpenPageEvStruct* evStruct)noexcept
		{ 
			pageData[(int)evStruct->pageType]->openObject = evStruct->GetOpenSeleted();
		}
		void JEditorPageShareData::SetSelectObj(const J_EDITOR_PAGE_TYPE pageType, const Core::JUserPtr<Core::JIdentifier>& selectObj)noexcept
		{ 
			auto page = pageData[(int)pageType].get();
			if (page->selectObj.IsValid() && page->selectObj->GetTypeInfo().IsA<JGameObject>())
				SetSelectedGameObjectTrigger(static_cast<JGameObject*>(page->selectObj.Get()), false);
			page->selectObj = selectObj;
			if (page->selectObj.IsValid() && page->selectObj->GetTypeInfo().IsA<JGameObject>())
				SetSelectedGameObjectTrigger(static_cast<JGameObject*>(page->selectObj.Get()), true);
		}
		void JEditorPageShareData::Clear()noexcept
		{
			for (uint i = 0; i < (int)J_EDITOR_PAGE_TYPE::COUNT; ++i)
				pageData[i].reset(); 
		}
		void JEditorPageShareData::SetSelectedGameObjectTrigger(JGameObject* gObj, const bool triggerValue)
		{
			const uint childrenCount = gObj->GetChildrenCount();
			for (uint i = 0; i < childrenCount; ++i)
				SetSelectedGameObjectTrigger(gObj->GetChild(i), triggerValue);

			gObj->EditorInterface()->SetSelectedByEditorTrigger(triggerValue);
		}
	}
}