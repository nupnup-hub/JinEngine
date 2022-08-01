#pragma once  
#include"../EditorWindow.h" 

namespace JinEngine
{
	struct SkeletonAssetSettingPageShareData;
	class JGameObject; 

	class SpecificObjectExplorer : public EditorWindow
	{
	private:
		SkeletonAssetSettingPageShareData* pageShareData;
	public:
		SpecificObjectExplorer(std::unique_ptr<EditorAttribute> attribute, 
			const size_t ownerPageGuid,
			SkeletonAssetSettingPageShareData* pageShareData);
		~SpecificObjectExplorer() = default;
		SpecificObjectExplorer(const SpecificObjectExplorer& rhs) = delete;
		SpecificObjectExplorer& operator=(const SpecificObjectExplorer& rhs) = delete;

		void Initialize(EditorUtility* editorUtility)noexcept;
		void UpdateWindow(EditorUtility* editorUtility)override;
	private:
		void BuildObjectExplorer(EditorUtility* editorUtility);
		void ObjectExplorerOnScreen(JGameObject* obj, EditorUtility* editorUtility);
	};
}