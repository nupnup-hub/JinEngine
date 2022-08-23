#pragma once
#include"../../JEditorWindow.h" 

namespace JinEngine
{
	namespace Editor
	{
		class JGraphicResourceWatcher : public JEditorWindow
		{
		public:
			JGraphicResourceWatcher(std::unique_ptr<JEditorAttribute> attribute, const size_t ownerPageGuid);
			~JGraphicResourceWatcher();
			JGraphicResourceWatcher(const JGraphicResourceWatcher& rhs) = delete;
			JGraphicResourceWatcher& operator=(const JGraphicResourceWatcher& rhs) = delete;

			void UpdateWindow(JEditorUtility* editorUtility)override;
		};
	}
}
