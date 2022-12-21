#include"JEditorPageShareData.h" 
#include"../../Object/JObject.h"
#include"../../Object/GameObject/JGameObject.h"

namespace JinEngine
{
	namespace Editor
	{
		class ShareData
		{
		public:
			Core::JUserPtr<JObject> openObject;
			Core::JUserPtr<JObject> selectObj;
			const J_EDITOR_PAGE_FLAG pageFlag;
			size_t dragGuid;
		public:
			ShareData(const J_EDITOR_PAGE_FLAG pageFlag)
				:pageFlag(pageFlag)
			{}
		};

		namespace
		{
			static std::unordered_map<J_EDITOR_PAGE_TYPE, ShareData> pageData;
		}

		void JEditorPageShareData::RegisterPage(const J_EDITOR_PAGE_TYPE pageType, const J_EDITOR_PAGE_FLAG pageFlag)noexcept
		{
			pageData.emplace(pageType, ShareData(pageFlag));
		}
		void JEditorPageShareData::UnRegisterPage(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			pageData.erase(pageType);
		}
		void JEditorPageShareData::ClearPageData(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			auto pagedata = pageData.find(pageType);
			if (pagedata->second.selectObj.IsValid() && pagedata->second.selectObj->GetObjectType() == J_OBJECT_TYPE::GAME_OBJECT)
				SetSelectedGameObjectTrigger(static_cast<JGameObject*>(pagedata->second.selectObj.Get()), false);
			pagedata->second.openObject.Clear();
			pagedata->second.selectObj.Clear();
		}
		bool JEditorPageShareData::HasValidOpenPageData(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			auto data = pageData.find(pageType);
			if (Core::HasSQValueEnum(data->second.pageFlag, J_EDITOR_PAGE_REQUIRE_INIT_OBJECT))
				return data->second.openObject.IsValid();
			else
				return true;
		}
		JEditorOpenPageEvStruct JEditorPageShareData::GetOpendPageData(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			auto data = pageData.find(pageType);
			return JEditorOpenPageEvStruct{ pageType, data->second.openObject };
		}
		Core::JUserPtr<JObject> JEditorPageShareData::GetSelectedObj(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			return pageData.find(pageType)->second.selectObj;
		}
		size_t* JEditorPageShareData::GetDragGuidPtr(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			return &(pageData.find(pageType)->second.dragGuid);
		}
		void JEditorPageShareData::SetPageOpenData(JEditorOpenPageEvStruct* evStruct)noexcept
		{
			auto data = pageData.find(evStruct->pageType);
			data->second.openObject = evStruct->openSelected;
		}
		void JEditorPageShareData::SetSelectObj(const J_EDITOR_PAGE_TYPE pageType, const Core::JUserPtr<JObject>& selectObj)noexcept
		{ 
			auto page = pageData.find(pageType);
			if (page->second.selectObj.IsValid() && page->second.selectObj->GetObjectType() == J_OBJECT_TYPE::GAME_OBJECT)
				SetSelectedGameObjectTrigger(static_cast<JGameObject*>(page->second.selectObj.Get()), false);
			page->second.selectObj = selectObj;
			if (page->second.selectObj.IsValid() && page->second.selectObj->GetObjectType() == J_OBJECT_TYPE::GAME_OBJECT)
				SetSelectedGameObjectTrigger(static_cast<JGameObject*>(page->second.selectObj.Get()), true);
		}
		void JEditorPageShareData::Clear()noexcept
		{
			pageData.clear();
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