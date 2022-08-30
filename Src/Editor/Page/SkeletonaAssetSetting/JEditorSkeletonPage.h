#pragma once
#include"../JEditorPage.h" 

namespace JinEngine
{ 
	namespace Editor
	{
		class JObjectExplorer;
		class JAvatarEditor; 
		class JEditorViewer;
		class JObjectDetail;

		class JEditorSkeletonPage : public JEditorPage
		{ 
		private:
			std::unique_ptr<JObjectExplorer> skeletonExplorer;
			std::unique_ptr<JAvatarEditor>avatarEdit;
			std::unique_ptr<JEditorViewer> modelViewer;
			std::unique_ptr<JObjectDetail>avatarDetail;		 
		public:
			JEditorSkeletonPage(bool hasMetadata);
			~JEditorSkeletonPage();
		public:
			void Initialize(bool hasImguiTxt)override;
			void UpdatePage()final;
		public:
			J_EDITOR_PAGE_TYPE GetPageType()const noexcept final;
		public:
			bool IsValidOpenRequest(const Core::JUserPtr<JObject>& selectedObj)noexcept final;
		private:
			bool StuffSkeletonAssetData(const Core::JUserPtr<JObject>& selectedObj);
		public:
			void StorePage(std::wofstream& stream)override;
			void LoadPage(std::wifstream& stream)final;
		};
	}
}
