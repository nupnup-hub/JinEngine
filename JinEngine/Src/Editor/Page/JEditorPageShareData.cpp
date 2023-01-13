#include"JEditorPageShareData.h" 
#include"../Page/JEditorPage.h"
#include"../../Object/JObject.h"
#include"../../Object/GameObject/JGameObject.h"

namespace JinEngine
{
	namespace Editor
	{
		using GetPageFlagF = typename JEditorPageShareData::GetPageFlagF;
		class ShareData
		{
		public:
			Core::JUserPtr<JObject> openObject;
			Core::JUserPtr<JObject> selectObj;
			std::unique_ptr<GetPageFlagF::Functor> getFageFlagF;
			size_t dragGuid = 0;
		public: 
			ShareData(std::unique_ptr<GetPageFlagF::Functor> getFageFlagF)
				:getFageFlagF(std::move(getFageFlagF))
			{}
		};

		namespace
		{
			static std::unique_ptr<ShareData> pageData[(int)J_EDITOR_PAGE_TYPE::COUNT]; 
		}

		void JEditorPageShareData::RegisterPage(const J_EDITOR_PAGE_TYPE pageType, GetPageFlagF::CPtr getFlagPtr, JEditorPage* page)noexcept
		{
			pageData[(int)pageType] = std::make_unique<ShareData>(std::make_unique<GetPageFlagF::Functor>(getFlagPtr, page));
		}
		void JEditorPageShareData::UnRegisterPage(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			pageData[(int)pageType].reset();
		}
		void JEditorPageShareData::ClearPageData(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			auto pagedata = pageData[(int)pageType].get();
			if (pagedata->selectObj.IsValid() && pagedata->selectObj->GetObjectType() == J_OBJECT_TYPE::GAME_OBJECT)
				SetSelectedGameObjectTrigger(static_cast<JGameObject*>(pagedata->selectObj.Get()), false);
			pagedata->openObject.Clear();
			pagedata->selectObj.Clear();
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
		JEditorOpenPageEvStruct JEditorPageShareData::GetOpendPageData(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{ 
			return JEditorOpenPageEvStruct{ pageType, pageData[(int)pageType]->openObject };
		}
		Core::JUserPtr<JObject> JEditorPageShareData::GetSelectedObj(const J_EDITOR_PAGE_TYPE pageType)noexcept
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
			pageData[(int)evStruct->pageType]->openObject = evStruct->openSelected;
		}
		void JEditorPageShareData::SetSelectObj(const J_EDITOR_PAGE_TYPE pageType, const Core::JUserPtr<JObject>& selectObj)noexcept
		{ 
			auto page = pageData[(int)pageType].get();
			if (page->selectObj.IsValid() && page->selectObj->GetObjectType() == J_OBJECT_TYPE::GAME_OBJECT)
				SetSelectedGameObjectTrigger(static_cast<JGameObject*>(page->selectObj.Get()), false);
			page->selectObj = selectObj;
			if (page->selectObj.IsValid() && page->selectObj->GetObjectType() == J_OBJECT_TYPE::GAME_OBJECT)
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