#pragma once
#include"../JEditorPage.h" 

namespace JinEngine
{ 
	class JScene;
	namespace Editor
	{
		class JObjectExplorer;
		class JAvatarEditor;  
		class JSceneViewer;
		class JObjectDetail;

		class JEditorSkeletonPage final: public JEditorPage
		{  
		private:
			std::unique_ptr< JObjectExplorer> explorer;
			std::unique_ptr< JAvatarEditor> avatarEdit;
			std::unique_ptr< JSceneViewer> avatarViewer;
			std::unique_ptr<JObjectDetail>avatarDetail;
		private:
			Core::JUserPtr<JScene> avatarScene;
		private:
			bool reqInitDockNode;
		public:
			JEditorSkeletonPage(bool hasMetadata);
			~JEditorSkeletonPage();
		public:
			J_EDITOR_PAGE_TYPE GetPageType()const noexcept final;
			void SetInitWindow() final;
		public:
			void Initialize()final;
			void UpdatePage()final;
		public:
			bool IsValidOpenRequest(const Core::JUserPtr<JObject>& selectedObj)noexcept final;
		private:
			bool StuffSkeletonAssetData(const Core::JUserPtr<JObject>& selectedObj);
		protected:
			void DoSetClose()noexcept final;
		public:
			void StorePage(std::wofstream& stream)final;
			void LoadPage(std::wifstream& stream)final;
		private:
			void BuildDockNode();
		};
	}
}
