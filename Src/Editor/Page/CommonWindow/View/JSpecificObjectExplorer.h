#pragma once  
#include"../../JEditorWindow.h" 

namespace JinEngine
{
	class JGameObject; 

	namespace Editor
	{
		struct JSkeletonSettingPageShareData;
		class JSpecificObjectExplorer : public JEditorWindow
		{
		private:
			JSkeletonSettingPageShareData* pageShareData;
		public:
			JSpecificObjectExplorer(std::unique_ptr<JEditorAttribute> attribute,
				const size_t ownerPageGuid,
				JSkeletonSettingPageShareData* pageShareData);
			~JSpecificObjectExplorer() = default;
			JSpecificObjectExplorer(const JSpecificObjectExplorer& rhs) = delete;
			JSpecificObjectExplorer& operator=(const JSpecificObjectExplorer& rhs) = delete;

			void Initialize(JEditorUtility* editorUtility)noexcept;
			void UpdateWindow(JEditorUtility* editorUtility)override;
		private:
			void BuildObjectExplorer(JEditorUtility* editorUtility);
			void ObjectExplorerOnScreen(JGameObject* obj, JEditorUtility* editorUtility);
		};
	}
}