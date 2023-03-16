#include"JEditorPageShareData.h" 
#include"../Page/JEditorPage.h" 
#include"../../Core/Identity/JIdentifier.h"
#include"../../Utility/JCommonUtility.h"
#include<vector>
#include<unordered_map>

namespace JinEngine
{
	namespace Editor
	{
		using GetPageNameF = typename JEditorPageShareData::GetPageNameF;
		using GetPageFlagF = typename JEditorPageShareData::GetPageFlagF;
		class ShareData
		{
		public:
			const uint guiWindowID;
			const uint dockSpaceID;
		public:
			Core::JUserPtr<Core::JIdentifier> openObject;
			std::unique_ptr<GetPageNameF::Functor> getPageNameF;
			std::unique_ptr<GetPageFlagF::Functor> getPageFlagF; 
		public:
			std::unique_ptr<Core::JTypeInstanceSearchHint> draggingHint;
		public: 
			ShareData(const uint guiWindowID,
				const uint dockSpaceID, 
				std::unique_ptr<GetPageNameF::Functor> getPageNameF,
				std::unique_ptr<GetPageFlagF::Functor> getPageFlagF)
				:guiWindowID(guiWindowID), 
				dockSpaceID(dockSpaceID),
				getPageNameF(std::move(getPageNameF)),
				getPageFlagF(std::move(getPageFlagF))
			{}
		};

		namespace
		{
			static std::unique_ptr<ShareData> pageData[(int)J_EDITOR_PAGE_TYPE::COUNT]; 
		}

		JEditorSelectedObjInfo::JEditorSelectedObjInfo(J_EDITOR_WINDOW_TYPE selectedWnd, Core::JUserPtr<Core::JIdentifier> selectedObj)
			:selectedWnd(selectedWnd), selectedObj(selectedObj)
		{}

		void JEditorPageShareData::RegisterPage(PageInitData& initData)noexcept
		{	 
			pageData[(int)initData.pageType] = std::make_unique<ShareData>(initData.guiWindowID,
				initData.dockSpaceID, 
				std::move(initData.getNamePtr),
				std::move(initData.getFlagPtr));
		}
		void JEditorPageShareData::UnRegisterPage(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			pageData[(int)pageType].reset();
		}
		void JEditorPageShareData::ClearPageData(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			auto pagedata = pageData[(int)pageType].get(); 
			pagedata->openObject.Clear(); 
		}
		bool JEditorPageShareData::IsRegisteredPage(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			return pageData[(int)pageType] != nullptr ? true : false;
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
		std::string JEditorPageShareData::GetPageName(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			return (*pageData[(int)pageType]->getPageNameF)();
		}
		J_EDITOR_PAGE_FLAG JEditorPageShareData::GetPageFlag(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			return (*pageData[(int)pageType]->getPageFlagF)();
		}
		void JEditorPageShareData::SetPageOpenData(JEditorOpenPageEvStruct* evStruct)noexcept
		{ 
			pageData[(int)evStruct->pageType]->openObject = evStruct->GetOpenSeleted();
		} 
		Core::JTypeInstanceSearchHint* JEditorPageShareData::RegisterDraggingHint(const J_EDITOR_PAGE_TYPE pageType, Core::JIdentifier* iden)noexcept
		{ 
			pageData[(int)pageType]->draggingHint = std::make_unique<Core::JTypeInstanceSearchHint>(Core::GetUserPtr(iden));
			return pageData[(int)pageType]->draggingHint.get();
		}
		void JEditorPageShareData::DeRegisterDraggingHint(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			pageData[(int)pageType]->draggingHint.reset();
		}
		void JEditorPageShareData::Clear()noexcept
		{
			for (uint i = 0; i < (int)J_EDITOR_PAGE_TYPE::COUNT; ++i)
				pageData[i].reset();
		}
	}
}