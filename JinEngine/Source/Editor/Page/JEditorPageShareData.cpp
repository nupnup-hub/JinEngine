/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#include"JEditorPageShareData.h" 
#include"../Page/JEditorPage.h" 
#include"../../Core/Identity/JIdentifier.h"
#include"../../Core/Utility/JCommonUtility.h" 

namespace JinEngine
{
	namespace Editor
	{ 
		using GetGuiIDF = typename JEditorPageShareData::GetGuiIDF;
		using GetDockSpaceIDF = typename JEditorPageShareData::GetDockSpaceIDF;
		using GetPageNameF = typename JEditorPageShareData::GetPageNameF;
		using GetPageFlagF = typename JEditorPageShareData::GetPageFlagF;
		using GetPublicStateF = typename  JEditorPageShareData::GetPublicBooleanStateF;
		class ShareData
		{ 
		public:
			JUserPtr<Core::JIdentifier> openObject;
			std::unique_ptr<GetGuiIDF::Functor> getGuiIDF;
			std::unique_ptr<GetDockSpaceIDF::Functor>getDockSpaceIDF;
			std::unique_ptr<GetPageNameF::Functor> getNameF;
			std::unique_ptr<GetPageNameF::Functor> getDockSpaceNameF;
			std::unique_ptr<GetPageFlagF::Functor> getFlagF;
			std::unique_ptr<GetPublicStateF::Functor> getPublicStateF;
		public:
			std::unique_ptr<JDragDropData> draggingHint;
		public:
			ShareData(std::unique_ptr<GetGuiIDF::Functor>&& getPageGuiIDF,
				std::unique_ptr<GetDockSpaceIDF::Functor>&&  getPageDockSpaceIDF,
				std::unique_ptr<GetPageNameF::Functor>&& getPageNameF,
				std::unique_ptr<GetPageNameF::Functor>&& getPageDockSpaceNameF,
				std::unique_ptr<GetPageFlagF::Functor>&& getPageFlagF,
				std::unique_ptr<GetPublicStateF::Functor>&& getPublicStateF)
				:getGuiIDF(std::move(getPageGuiIDF)),
				getDockSpaceIDF(std::move(getPageDockSpaceIDF)),
				getNameF(std::move(getPageNameF)),
				getDockSpaceNameF(std::move(getPageDockSpaceNameF)),
				getFlagF(std::move(getPageFlagF)),
				getPublicStateF(std::move(getPublicStateF))
			{}
		};

		namespace
		{
			static std::unique_ptr<ShareData> pageData[(int)J_EDITOR_PAGE_TYPE::COUNT];
		}

		JEditorSelectedObjInfo::JEditorSelectedObjInfo(J_EDITOR_WINDOW_TYPE selectedWnd, JUserPtr<Core::JIdentifier> selectedObj)
			:selectedWnd(selectedWnd), selectedObj(selectedObj)
		{}

		void JEditorPageShareData::RegisterPage(PageInitData& initData)noexcept
		{
			pageData[(int)initData.pageType] = std::make_unique<ShareData>(std::move(initData.getGuiIDF),
				std::move(initData.getDockSpaceIDF),
				std::move(initData.getNameF),
				std::move(initData.getDockSpaceNameF),
				std::move(initData.getFlagF),
				std::move(initData.getPublicStateF));
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
		GuiID JEditorPageShareData::GetPageGuiWindowID(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			return (*pageData[(int)pageType]->getGuiIDF)();
		}
		GuiID JEditorPageShareData::GetPageDockSpaceID(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			if (pageData[(int)pageType]->getDockSpaceIDF == nullptr)
				return 0;

			return (*pageData[(int)pageType]->getDockSpaceIDF)();
		}
		JEditorOpenPageEvStruct JEditorPageShareData::GetOpendPageData(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			return JEditorOpenPageEvStruct{ pageType, pageData[(int)pageType]->openObject };
		}
		std::string JEditorPageShareData::GetPageName(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			return (*pageData[(int)pageType]->getNameF)();
		}
		std::string JEditorPageShareData::GetPageDockSpaceName(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			return (*pageData[(int)pageType]->getDockSpaceNameF)();
		}
		J_EDITOR_PAGE_FLAG JEditorPageShareData::GetPageFlag(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			return (*pageData[(int)pageType]->getFlagF)();
		}
		bool JEditorPageShareData::GetPublicState(const J_EDITOR_PAGE_TYPE pageType, const J_EDITOR_PAGE_PUBLIC_STATE state)noexcept
		{
			return (*pageData[(int)pageType]->getPublicStateF)(state);
		}
		void JEditorPageShareData::SetPageOpenData(JEditorOpenPageEvStruct* evStruct)noexcept
		{
			pageData[(int)evStruct->pageType]->openObject = evStruct->GetOpenSeleted();
		}
		JDragDropData* JEditorPageShareData::RegisterDragDropHint(const J_EDITOR_PAGE_TYPE pageType, std::unique_ptr<JDragDropData>&& data)noexcept
		{
			pageData[(int)pageType]->draggingHint = std::move(data);
			return 	pageData[(int)pageType]->draggingHint.get();
		}
		void JEditorPageShareData::DeRegisterDragDropHint(const J_EDITOR_PAGE_TYPE pageType)noexcept
		{
			pageData[(int)pageType]->draggingHint = nullptr;
		} 
		void JEditorPageShareData::Clear()noexcept
		{
			for (uint i = 0; i < (int)J_EDITOR_PAGE_TYPE::COUNT; ++i)
				pageData[i].reset();
		}
	}
}