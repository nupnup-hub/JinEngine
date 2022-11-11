#include"JEditorPageShareData.h" 
#include"../../Object/JObject.h"

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
			pageData.find(pageType)->second.selectObj = selectObj;
		}
		void JEditorPageShareData::Clear()noexcept
		{
			pageData.clear();
		}
	}
}