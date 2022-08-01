#pragma once
#include"../EditorWindowPage.h" 

namespace JinEngine
{
	struct SkeletonAssetSettingPageShareData;
	class JScene;
	class AvatarEditor;
	class AvatarDetail;
	class PreviewSceneEditor;
	class SpecificObjectExplorer;
	class PreviewSceneGroup;
	class PreviewScene;

	class EditorSkeletonAssetPage : public EditorWindowPage
	{
	private:
		std::unique_ptr<SpecificObjectExplorer>skeletonExplorer;
		std::unique_ptr<AvatarEditor>avatarEdit;
		std::unique_ptr<PreviewSceneEditor> avatarSceneEditor;
		std::unique_ptr<AvatarDetail> avatarDetail;		 
		std::unique_ptr<SkeletonAssetSettingPageShareData> pageShareData;
		PreviewSceneGroup* previewGroup;
		size_t previewGroupGuid;
		std::string skeletonPath; 
	public:
		EditorSkeletonAssetPage();
		~EditorSkeletonAssetPage();
		void Initialize(EditorUtility* editorUtility, std::vector<EditorWindow*>& allEditorWindows, bool hasImguiTxt)override;
		bool Activate(EditorUtility* editorUtility)final;
		bool DeActivate(EditorUtility* editorUtility)final;
		void StorePage(std::wofstream& stream)override;
		void LoadPage(std::wifstream& stream, std::vector<EditorWindow*>& allEditorWindows, std::vector<EditorWindowPage*>& opendEditorPage, EditorUtility* editorUtility)override;
		void UpdatePage(EditorUtility* editorUtility)override;
	private:
		bool StuffSkeletonAssetData();
	};
}
